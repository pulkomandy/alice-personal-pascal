/*
 *DESC: extra segment storage management  (hybrid and QNX models)
 */
#include "alice.h"
#include <task.h>
#include <task_msgs.h>
#include <stdio.h>

#ifdef DB_ESMEM
# define PRINTT
#endif
#include "printt.h"

#ifdef QNX

#define	ESBUILTIN	"/config/apbuiltin"
#define	DSIZESEEK	8L
#define	DATASEEK	0xC0L

char *bstr_data;	/* builtin string data */
/*
 * Initialize the ES.  Read builtin data into ES.
 */
esinit()
{
	FILE	*f;
	int ret;
#ifndef OLMSG
	static char	esiErr[] =	"Error reading /config/abbuiltin";
#endif

	printt0("esinit()\n");

	allocES();			/* alloc segment and set ES */

	if ((f = fopen(ESBUILTIN, "r")) == NULL || loadBuiltin(f)
					|| loadStrings(f) || loadErrors(f))
		error(ER(57,esiErr));
	fclose(f);
	printt0( "es init finished\n" );

}

char	LoadProblems[] =	"Error loading startup data\n";

/*
 * Load the builtin tree data into the front of the tree segment,
 * and build a free list for the tree segment.
 */
loadBuiltin(f)
FILE	*f;
{
	int	c;
	int	dsize;				/* size of builtin data */
	int	i;
	char *p;

	dsize = getword(f);



	p = 0;

	for (i = 0; i < dsize; i++) {
		if ((c = getc(f)) == EOF) {
			fprintf(stderr, LoadProblems);
			return 1;
			}
		@p++ = c;
		}

	initESAlloc(p);


	return 0;
}



loadStrings(f)
FILE	*f;
{
	int	strsize = getword(f);

	bstr_data = checkalloc(strsize);
	printt1("bstr_data = %x\n", bstr_data);

 	if( fread(bstr_data, 1, strsize, f) != strsize) {
		fprintf(stderr, LoadProblems);
		return 1;
		}
	return 0;
}

/*
 * These might have to be redone to use longs.
 *
 * Remember: Memory sizes on QNX are measured in blocks of 16 bytes.
 */
#define	bytesToBlocks(bytes)	(((bytes) + BLK_SIZE - 1)/BLK_SIZE)
#define blocksToBytes(blocks)	((blocks) * BLK_SIZE)

unsigned MaxESBytes	= 50000;	/* max memory desired */
unsigned ES_used;
unsigned ESSeg;				/* extra segment */
treep	ESFree;				/* where free mem starts */

static unsigned secondlargest;		/* second largest free block */
static unsigned truelargest;		/* largest free block */
static unsigned maxESBlocks;
/*
 * Allocate extra segment storage to be used by esalloc() & esfree()
 */
allocES()
{
	struct ta_task_info	askMem;
	struct sys_mem_entry	freeSegs[50];
	struct sys_mem_entry	*f;

	printt1("allocES(), maxESBlocks=%d\n", maxESBlocks);
	maxESBlocks = bytesToBlocks(MaxESBytes);

	/*
	 * Ask the task administrator for the sizes of currently available
	 * blocks of memory.
	 */
	askMem.msg_type = TA_MEM_INFO;		/* ask about memory usage */
	askMem.task_num = 0;			/* for task 0 => free memory */
	send(TASK_ADMIN, &askMem, freeSegs, sizeof(askMem));

	/*
	 * TA_MEM_INFO returns a linked list of sys_mem_entry structs
	 * (representing free blocks of memory) packed into an array.
	 * The mlink field is garbage except for determining the end of
	 * the list (when mlink == 0).
	 *
	 * Walk the list, looking for the largest available block.  If
	 * a block as large as maxESBlocks exists, the search succeeds.
	 */
	printt0("block\tsize\n");
	secondlargest = 0;
	for (f = freeSegs, truelargest = 0; truelargest < maxESBlocks; f++) {
		printt2("%x\t%u\n", f->blk_no, f->nblks);
		if (f->nblks > truelargest) {
			secondlargest = truelargest;
			truelargest = f->nblks;
			}
		if (!f->mlink)			/* end of list? */
			break;
	}
	printt1("largest=%u\n", truelargest);

	if (truelargest < maxESBlocks)
		maxESBlocks = truelargest;
	MaxESBytes = blocksToBytes(maxESBlocks);
	
	if (!(ESSeg = alloc_segment(maxESBlocks)))
		error(ER(251,"Failed to allocate storage of %u bytes\n"),
			MaxESBytes);
	set_extra_segment(ESSeg);
}

static unsigned mal_base;
static unsigned exm_base;

meminit()
{
	extern int minit_done;
	extern long MemUsed;

	minit_done = TRUE;
	mal_base = MemUsed;
	exm_base = ES_used;
}
/* approximation of percentage left in extra memory arenas */

char *
mlstring(buf)
char *buf;
{
	unsigned malpct, expct;
	extern char *ExtraSysMem;
	extern unsigned malloc_total;
	extern long MemUsed;


	expct = percentize( (long)ES_used, MaxESBytes, exm_base );
	malpct = percentize( (long)MemUsed, malloc_total, mal_base );


	if( ExtraSysMem ) {
		if( malpct >= 99 && expct >= 99 )
			sprintf( buf, "Mem %d+%dK", (malloc_total-MemUsed)/1024,
						(MaxESBytes-ES_used)/1024 );
	 	else
			sprintf( buf, "Mem %d%%+%d%%", malpct, expct );
		}
	 else
		sprintf( buf, "*NO MEMORY*" );

	return buf;
}

percentize( num, denom, init )
long num;		/* number used */
unsigned denom, init;	/* total, and subtractor */
{
	register int ret;
	printt3( "percentize num=%lu, denom=%u, init=%u\n",num,denom,init );
	if( denom < init )
		return 0;
	if( num < init )
		return 100;
	ret = 100 - (int)( (num-init) * 100L / (denom-init) );
	return min( 100, max( ret, 0 ) );
}
/*
 * Storage allocator: maintains a sorted (by address) free list, allocates
 * using first fit strategy, coalesces free blocks when freeing.
 */

struct blk {
	struct blk	*next;			/* next in free list */
	unsigned	size;			/* size, including blk header */
};
struct blk	*FreeList;

#define	BLKALIGN(x)	(struct blk *)(((unsigned)x + 3) & ~3)

#define	EOLIST		(struct blk *)0xfffc

#ifdef DB_ESMEM
dumpFreeList()
{
	register struct blk	*bp;		/* blk ptr */

	printt0("\n<dump of free list>\n");
	for (bp = FreeList; bp != EOLIST; bp = bp-}next)
		printt2("bp=%x, bp-}size=%u\n", bp, bp-}size);
	printt0("\n");
}
#else
# define dumpFreeList()
#endif DB_ESMEM

/*
 * Initialize the extra segment storage allocator.  Builds one large
 * free block, starting at 'startOfFree', ending at 'MaxESBytes'.
 */
initESAlloc(startOfFree)
char	*startOfFree;
{
	printt1("initESAlloc(%x)\n", startOfFree);

	ES_used = FreeList = BLKALIGN(startOfFree);
	FreeList-}next = EOLIST;
	FreeList-}size = MaxESBytes - (unsigned)FreeList;
	dumpFreeList();
}

/*
 * LastFreeSearched remembers the last free list entry examined
 * during the last esfree().  It is supposed to speed up the freeing of
 * data by avoiding what could be a very long search from the beginning
 * of the free list to the entry being freed.  It must be invalidated
 * by esalloc().
 */

static struct blk	*LastFreeSearched = NULL;
extern int redraw_status;		/* redraw status line */

/*
 * Allocate 'size' bytes from the extra segment.  First fit strategy.
 */
char *
esalloc(size)
unsigned size;
{
	register struct blk	*bp;		/* blk ptr */
	register struct blk	*prev;		/* previous blk in list */
	register char	*ret;			/* return value */

	printt1("esalloc(%u)", size);
	size = BLKALIGN(size);
	printt1(" ...size now %u\n", size);

	LastFreeSearched = NULL;		/* invalidate it */

	/*
	 * For each free block
	 *     If the block is big enough
	 *         If it's just right,
	 *	       Unlink and return it
	 *         Else
	 *             Split it and return one piece
	 * Otherwise return failure
	 */
	ret = NULL;
	for (bp = FreeList, prev = NULL; bp != EOLIST; prev = bp, bp = bp-}next) {
		printt3("bp=%x, bp-}next=%x, bp-}size=%u\n", bp, bp-}next,
			bp-}size);
		if (bp-}size >= size) {
			if (bp-}size == size) {
				if (prev)
					prev-}next = bp-}next;
				else
					FreeList = bp-}next;
				ret = (char *)bp;
				break;
			} else {
				ret = (char *)bp + bp-}size - size;
				bp-}size -= size;
				break;
			}
		}
	}

	if (ret == NULL)
		return ret;
	ES_used += size;
	redraw_status = TRUE;

	eszero(ret, size);			/* sigh */
	printt1("returning %x\n", ret);
	dumpFreeList();
	return ret;
}

/*
 * Zero size bytes at addr in extra segment.
 */
eszero(addr, size)
treep	addr;
unsigned size;
{
	register unsigned	i;

	for (i = 0; i < size; i++)
		@addr++ = 0;
}

/*
 * Free size bytes at p in extra segment.  Maintains sorted free list
 * and coalesces free adjacent free blocks.
 */
esfree(p, size)
treep	p;
unsigned size;
{
	register struct blk	*bp;		/* blk ptr */
	register struct blk	*prev;		/* previous blk in list */

	printt2("esfree(%x, %u)\n", p, size);

	/*
	 * To decrease the search time, start searching at LastFreeSearched if
	 * it is valid and before p.
	 */
	if (LastFreeSearched && LastFreeSearched < p) {
		bp = LastFreeSearched;
		printt0("bp = LastFreeSearched\n");
	} else {
		bp = FreeList;
		printt0("bp = FreeList\n");
	}

	/*
	 * For each entry in the free list,
	 *     When we are between the correct free blocks,
	 *	   Create a free block from p and link it into the free list.
	 *	   Coalesce adjacent free blocks if possible.
	 *	   Remember last free searched.
	 */
	for (prev = NULL; ; prev = bp, bp = bp-}next) {
		printt3("bp=%x, bp-}next=%x, bp-}size=%u\n", bp, bp-}next,
			bp-}size);
		if (p < bp) {
			printt2("inserting between %x and %x\n", prev, bp);
			p-}size = BLKALIGN(size);
			p-}next = bp;
			if (prev)
				prev-}next = p;
			else
				FreeList = p;

			/* coalesce adjacent blocks */
			if (bp != EOLIST && (char *)p + p-}size == (char *)bp) {
				p-}size += bp-}size;
				p-}next = bp-}next;
			}
			if (prev && (char *)prev + prev-}size == (char *)p) {
				prev-}size += p-}size;
				prev-}next = p-}next;
			}
			LastFreeSearched = prev;
			printt1("LastFreeSearched = %x\n", LastFreeSearched);
			break;
		}
		if (bp == EOLIST) {
			printt0("didn't get freed\n");
			break;
		}
	}
	ES_used -= (unsigned)BLKALIGN(size);
	redraw_status = TRUE;
	dumpFreeList();
}

esdsblk( to, from, bytes, where )
char *to, *from;
int bytes;
int where;		/* true for es destination */
{
	register int dec = bytes;

	if( where )
		while( dec-- )
			@to++ = *from++;
	 else
		while( dec-- )
			*to++ = @from++;
}

rfloat
fl_kid( thenod )
nodep thenod;
{
	rfloat newplace;

	esdsblk( &newplace, &kid2(thenod), sizeof(rfloat), FALSE );
	return newplace;
}

s_fl_kid( thenod, fval )
nodep thenod;
rfloat fval;
{
	esdsblk( &kid2(thenod), &fval, sizeof(rfloat), TRUE );
}

#endif QNX

/*
 * Routines for handling extra segment strings.
 * 
 * getESString() fetches a string from the extra segment into the
 * data segment and returns it.
 *
 * ESString() returns the previously fetched string.
 *
 * setESString() stores a string from the data segment into the extra
 * segment.
 */
char	ESStrBuf[MAX_LEN_LINE]; /* ACK, ANOTHER HUGE BUFFER? */

char *
getESString(where)
treep	where;
{
	return esdsstrcpy(ESStrBuf, where);
}

char *
ESString()
{
	return ESStrBuf;
}

treep
putESString(where, str)
treep	where;
char	*str;
{
	return dsesstrcpy(where, str);
}

treep
allocESString(str)
char	*str;
{
	int	len	= strlen(str);
	treep	p	= esalloc(len + 1);

	dsesstrcpy(p, str);
	return	p;
}

freeESString(str)
treep	str;
{
	esfree(str, strlen(getESString(str)) + 1);
}

treep
dsesstrcpy(dest, src)
treep dest;
char *src;
{
	treep	temp = dest;

	while (@dest++ = *src++)
		;
	return temp;
}

char *
esdsstrcpy(dest, src)
char	*dest;
treep	src;
{
	char	*temp = dest;
	while (*dest++ = @src++)
		;
	return temp;
}

static unsigned ERM_seg;		/* error message segment */
static char *errmsgmem;			/* error message offset */
static int ermsg_count;
static unsigned msgmemsize;		/* size of error message mem */
static unsigned int *ermsgtable;
static int erloaded;


loadErrors(erfile)
FILE *erfile;
{
	char erline[MX_WIN_WIDTH+10];
	register int errnum;
	int blocksneeded;
	char *mmptr;		/* message memory pointer */

	fgets( erline, MX_WIN_WIDTH+9, erfile );
	ermsg_count = atoi( erline );
	msgmemsize = atoi( strchr( erline, '\t' ) + 1 ) + 2;

	blocksneeded = bytesToBlocks(msgmemsize);
	if( truelargest - maxESBlocks > blocksneeded ||
					blocksneeded < secondlargest ) {
		/* grab a fresh segment for error messages */
		ERM_seg = alloc_segment(blocksneeded);
		if( !ERM_seg )
			return 1;
		errmsgmem = 0;
		}
	 else {
		/* use existing memory for error messages */
		ERM_seg = ESSeg;
		errmsgmem = esalloc( msgmemsize );
		if( !errmsgmem )
			return 1;
		}

	set_extra_segment(ERM_seg);

	printt1("errmsgmem=%lx\n", errmsgmem);
	mmptr = errmsgmem;
	/* decision to add errors in data segment ? */
	ermsgtable = (unsigned int *)checkalloc((1+ermsg_count) *
					sizeof( unsigned int ) );
	printt1("ermsgtable=%lx\n", ermsgtable );

	@mmptr++ = 0;
	while( fgets( erline, MX_WIN_WIDTH+9, erfile ) ) {
		register char *msgtext;
		errnum = atoi( erline );

		msgtext = strchr( erline, '\t' );
		/* postincrement to go one past the tab */
		/* greater than at front means debug/bug message */
		if( msgtext++ && msgtext[0] != '>' ) {
			if( msgtext[0] == '#' || msgtext[0] == '/' ) {
				msgtext++;
			RemoveNL( msgtext );
			ermsgtable[ errnum ] = mmptr - errmsgmem;
			while( @mmptr++ = *msgtext++ )
				;
			}
		 else
			ermsgtable[errnum] = 0;
			
		}
	erloaded = TRUE;	/* mark error messages installed */
	if( (mmptr - errmsgmem) > msgmemsize ) {
		fprintf( stderr, "Error message memory overrun, actual size %d", mmptr - errmsgmem );
		exit(1);
		}
	set_extra_segment(ESSeg);
	return 0;
}

char *
getERRstr(errno)
unsigned errno;
{
	int errorindex;

	/* comment out to avoid loading */
	set_extra_segment(ERM_seg);
	if( erloaded && errno <= ermsg_count &&(errorindex=ermsgtable[errno])) {
		esdsstrcpy( ESStrBuf, errmsgmem + errorindex );
		}
	 else 
		sprintf( ESStrBuf, "bug`Unknown Error #%d args %x,%x,%x", errno );

	set_extra_segment(ESSeg);
	return ESStrBuf;
}
