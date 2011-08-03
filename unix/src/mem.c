/*
 *DESC: Memory management routines.
 */

#include "alice.h"
#include <curses.h>			/* strange place to define TRUE! */
#include "flags.h"
#include "typecodes.h"
#include "mem.h"

#ifdef DB_MEM
# define PRINTT
#endif
#include "printt.h"

long MemUsed = 0;

char	ZeroBlock;			/* malloc(0) points to this */

static _treefree( nodep	_np );

/*
 * mmalloc() keeps track of the memory used.
 */
char *
mmalloc(size)
unsigned	size;
{
	char	*p;
	char	*calloc();

	if (size == 0)
		return &ZeroBlock;

	p = calloc(1, size);		/* calloc() zeroes the memory */

	/* ((size + 1) & ~1) is probably wrong on the IBM */
	if (p && MemUsed >= 0)
		MemUsed += ((size + 1) & ~1) + sizeof(unsigned *);

	printt2("mmalloc(%d) returns %lx\n", size, (long)p);

#ifdef Notdef
	{
	/* normalize pointer */
	unsigned seg, ofs;

	seg = ((long)p) >> 16;
	ofs = (int) (((long)p) & 0xffff);

	if( ofs > 16 ) {
		
		p = (char *)((((long)(seg + (ofs >> 4) - 1)) << 16) +
			(long)( 16 + (ofs & 0xf) ));
		}
	}
#endif notdef large

	return p;
}

#ifdef DMFREE
mfree(p, file, line)
char	*p;
char	*file;
int	line;
#else DMFREE
mfree(p)
char	*p;
#endif
{
	register unsigned	size;

	if (p == &ZeroBlock)			/* not allocated */
		return;

#if defined(ONYX) || defined(QNX)
	if (!(*((unsigned *)p - 1) & 1))
#ifdef DMFREE
		warning(ER(221,"Internal error: mfree(%X), file %s line %d"),
			(long)p, file, line);
#else
		warning(ER(222,"Internal error: mfree(%x)"), p);
#endif
	else {
#endif ONYX
		/*
		 * This works for both the UNIX and QNX allocators, but
		 * seems to be broken on msdos.
		 */
#ifdef unix
		MemUsed = -1;
#else
		size = (unsigned)(*((unsigned *)p - 1) & ~1)
			- (unsigned)((unsigned *)p - 1);
		MemUsed -= size;
#endif

		printt2("mfree(%lx) frees %d\n", (long)p, size);
		free(p);
#if defined(ONYX) || defined(QNX)
	}
#endif
}
char	*ExtraMem = 0;			/* extra user mem if he runs out */
char	*ExtraSysMem = 0;		/* extra sys mem if above runs out */

extern int redraw_status;

#ifndef OLMSG
static char LowMemMsg[]	= "lowmem`You are running out of memory.  Save your work and restart immediately!";
#endif
#ifdef DEMO
static char NoMemMsg[]	= "memory`Out of Memory! (Limited memory in DEMO version)";
#else
#ifndef OLMSG
static char NoMemMsg[]	= "memory`Out of Memory!";
#endif
#endif

funcptr mem_cleanup = 0;	/* routine to to cleanup if out of mem */

#ifdef SAI
char *
checkalloc( size )
int size;
{
	register char *p;

	p = (char *) mmalloc( size );
	if( p ) return p;

	sai_print( "Out of Memory !\n" );
	sai_print( "See the description of the 'f=xxx', 's=xxx' and 'l=xxx'\n");
	sai_print( "command line options\n" );
	done();
}
#else

char *
checkalloc(size )
int	size;
{
	register char	*p;

	/* If we have given away our reserves, */
	/* try and grab them back */
	if (!ExtraSysMem) {
		if( ExtraSysMem = mmalloc(EXTRASYSMEM) )
			redraw_status = TRUE;
		}

	if (!ExtraSysMem || (p = mmalloc(size)) == (char *)NULL) {
		/*
		 * Running low on memory.  Free ExtraMem, which
		 * we allocated at the start for just such an emergency.
		 * Hopefully the user will have enough sense to save at
		 * this point.
		 */

		if (ExtraMem) {
			dmfree(ExtraMem);
			ExtraMem = NULL;
			warning(ER(223,LowMemMsg));
			p = mmalloc(size);
			if( p )
				return p;
			}
		/*
		 * No, the foolish user didn't have enough sense to save
		 * when he was warned earlier.  Free up a last reserve
		 * of storage (which will keep such things as fopen and
		 * save going).  
		 */
		 /* try to cleanup what he was doing. If he was doing some
		  * sort of special thing, clean that up.  Special routines
		  * are expected to do their own abort and issue their own
		  * error message.  Anyway, first, we do a special cleanup undo
		  * which undoes what has been done this command prior to
		  * running out of storage.  This should restore the world
		  * to a reasonable state.  This might happen a lot once
		  * out of memory.  In order for deletes to work in freeing
		  * up memory, they will need to be able to call malloc/talloc
		  * directly and check themselves
		  */
		if (ExtraSysMem) {
			/* extern char *M_outmem[]; */
			dmfree(ExtraSysMem);
			ExtraSysMem = NULL;
			redraw_status = TRUE;
			/* if( com_menu( M_outmem, TRUE ) ) */
			}
		memerr();	/* gives error message */
	}

	return p;
}
#endif SAI
#ifdef ES 
nodep
talloc( size )
int size;
{
	char *ret;
	if( ret = esalloc( size ) ) {
		return  ret;
		}
	 else {	
		memerr();
		}
}
#endif

memerr()
{
#ifndef SAI
	undo(TRUE);
#endif
	if( mem_cleanup )
		(*mem_cleanup)(ABORT);
	error( ER(79,NoMemMsg) );
}
/*
 * checkrealloc() no longer calls realloc() -- our scheme of freeing
 * some extra memory when it got low wouldn't work on the ICON and maybe
 * not even on the ONYX.  What we do instead is get a new area,
 * copy the min of the sizes of bytes to it, and free the old one.
 */
char *
trealloc(old, oldsize, newsize)
char	*old;
int	oldsize;
int	newsize;
{
	register char	*new;

	new = talloc(newsize);
	/* TREE based block move - assumes checkrealloc only in tree */
	t_blk_move(new, old, min(oldsize, newsize));
	tfree(old);

	return new;
}

char *
allocstring(str)
char *str;
{
	char	*new = checkalloc(strlen(str) + 1);

	strcpy(new, str);
	return new;
}

char *strc_alloc( str )
char *str;
{
#ifdef TURBO
	register int len;
	register char *new;

	len = strlen(str);
	new = checkalloc( len + 2 );
	strcpy( new+2, str+1 );
	new[0] = str[0];	/* the quote char */
	new[1] = len-1;		/* the length */
	return new;
#else
	return allocstring(str);
#endif
}


/*
 * allocate at most n bytes to store the first n-1 chars of str
 */
char *
allocnstring(str, n)
char	*str;
int	n;
{
	register char	*new;
	register int	size;

	size = min(n, strlen(str) + 1);
	new = checkalloc(size);
	strncpy(new, str, size);
	new[size-1] = '\0';
	return new;
}

#ifndef SAI

treefree(np)
nodep	np;
{
	if (!np || tparent(np) != NIL) {
		printt1("tparent(np)=%x != NIL!\n", tparent(np));
		return;
	}

	marksGoing(np);
	_treefree(np);
}

static
_treefree(_np)
nodep	_np;
{
	register nodep np = _np;
	int	i;
	NodeNum npt;
	int	n_kids;

	stackcheck();

	printt1("treefree(%x)\n", np);

	/* don't free the tree if it's null or is attached somewhere */
	if (np == NULL) {
		printt0("null, nothing to free!\n");
		return;
	}

	/*
	 * Free all of the kids, except the declaration kid of an id
	 */
	npt = ntype(np);
	if (npt != N_ID) {
		n_kids = reg_kids(np);
		for (i = 0; i < n_kids; i++)
			_treefree(node_kid(np, i));
		}

	/*
	 * Free hidden things
	 */
	switch (npt) {
	case N_EXP_ERROR:
	case N_T_COMMENT:
	case N_CON_INT:
	case N_CON_REAL:
#if defined(ES) || defined(HYBRID)
		printt1("freeing hidden es kid %x\n", kid1(np));
		freeESString(str_kid(0, np));
		break;
#endif
	case N_NAME:
	case N_CON_STRING:
		printt1("freeing hidden kid %x\n", kid1(np));
		dmfree(str_kid(0,np));
		break;
	case N_DECL_ID:
		if( sym_dtype( (symptr)np ) == T_UNDEF && sym_mflags((symptr)np)
					& SF_REFERENCED )
			return;	/* loaded undefs are not referenced */
				/* del_decl sets bit on created guys */
		dmfree( sym_name( (symptr)np ) );
		break;
	default:
		if (ntype_info(npt) & F_SYMBOL)
			tfree(sym_kid(np));
		break;
	}

	printt0("freeing self\n");
	tfree(np);
}
#endif

