
#include "alice.h"
#include "interp.h"
#include "dbflags.h"
#include "extramem.h"

#ifdef Trace

int trace_queue_size = 0;		/* number of trace exceptions pending*/

int trace_excepts[MAX_TRACE];		/* index numbers for trace exceptions */


struct tr_range tr_locs[MAX_TRACE];	/* trace slot table */

int most_trlocs = 0;		/* current max uses in trace slot table */
int within_trace = FALSE;

addslot( tr_position )
struct tr_range *tr_position;	/* pointer to a slot */
{
	if( trace_queue_size >= MAX_TRACE )
		run_error( ER(307,"trace`Too many trace exceptions at once!") );

	/* this pointer subtract finds the queue location */
	if( tr_position->tr_code_list )
		trace_excepts[trace_queue_size++] = tr_position - tr_locs;
}

#ifndef msdos
scan_trace( lower, size )
pointer lower;	/* lower bound */
asize size;	/* number of bytes */
{
	register int i;
	register struct tr_range *p;
	register truepointer trueupper;
	truepointer truelower;

	ptrmap( truelower, lower );
	trueupper = truelower + size;
	for( p = &tr_locs[most_trlocs-1]; p >= tr_locs; p-- ) 
		if( trueupper > p->tr_lower && truelower < p->tr_upper )
			addslot( p );
}
#endif

int break_disable = FALSE;
/* turn on trace with a TRACE statement */

traceon( loc, size )
pointer loc;		/* thing to be traced */
unsigned int size;	/* size of object */
{
	int i;

	register struct tr_range *p;
	truepointer nloc;
	int gotone = FALSE;
	listp lkid = kid2(ex_loc);
	truepointer upb;


	if( loc == BRK_DISABLE )
		break_disable = TRUE;

	ptrmap(nloc, loc);

	upb = nloc + size;	/* one past the last byte */

	EDEBUG( 3, "TUrn on trace loc %lx, size %d\n", (long)loc, size );

	for( i = 0; i <= MAX_TRACE; i++ ) {
		p = &tr_locs[i];
		if( p->tr_lower == nloc && p->tr_upper == upb ) {
			EDEBUG( 4, "Replacing old trace\n", 0, 0 );
			p->tr_code_list = NIL;
			if( listcount(lkid) == 1 && is_a_stub(kid1(lkid)) )
				return;
			}
		if( i >= most_trlocs || !p->tr_code_list ) {
			p->tr_code_list = lkid;
			p->tr_lower = nloc;
			p->tr_upper = upb;	/* one past last byte */
			p->tr_display = st_display;
			if( i >= most_trlocs )
				most_trlocs = i+1;
			gotone = TRUE;
			return;
			}
		}
	if( !gotone )
		run_error( ER(306,"trace`Too many locations under trace") );

}

/* turn off trace because memory (stack frame, dispose) was freed */

traceoff( allofthem, xlower, xupper )
int allofthem;
pointer xlower, xupper;
{
	register int i;
	int max_trl;
	register truepointer low;
	truepointer lower, upper;

	ptrmap( lower, xlower );
	ptrmap( upper, xupper );

	EDEBUG(5, "trace off, all=%d, upper=%lx\n", allofthem, (long)upper );

	if( allofthem ) {
		break_disable = within_trace = trace_queue_size = most_trlocs = 0;
		return;
		}

	max_trl = 0;
	for( i = 0; i < most_trlocs; i++ )
		if( tr_locs[i].tr_code_list )
			if( (low=tr_locs[i].tr_lower) >= lower && low<upper ) {
				tr_locs[i].tr_code_list = NIL;
				}
			else 
				max_trl = i+1;
	most_trlocs = max_trl;
}

#endif not Trace

long tc_result;
#ifdef msdos


#define MAX_TLIB 10

int num_libs = 0;
/* the turbo pascal call routine */
#define MOV 0x8B
#define CALL 0xFF
#define CS 0x2E
static unsigned char tcall[] = {
/* In large model, this is CS:0 */
0x90, 0x90, 0x90, 0x90,		/* nop, nop, nop, nop */
#define ST_STORAGE 2		/* number of words for stack storage */
0x55,				/* push bp */
MOV, 0xEC,			/* mov bp, sp */
#ifdef LARGE
0xfa,				/* cli */
MOV, 0x66, 8,			/* mov sp, 8[bp] */
MOV, 0x46, 12,			/* mov ax, 12[bp] */
MOV, 0x56,14,                 	/* mov	dx,14[bp] */
MOV,0x4E,10,                 	/* mov	cx,10[bp] */
0xBB, ALICE_VERSION, 0,        	/* mov	bx,131	*/
MOV,0x76,6,                 	/* mov	si,6[bp]*/
CS,0x8C,0x16,0, 0,         	/* mov	cs:ssloc,ss */
CS,0x89,0x2E,2, 0,         	/* mov	cs:sploc,bp */
0x8E,0xD1,                    	/* mov	ss,cx  */
0xfb,				/* sti */
0x81,0xc6,0x39,0,		/* add	si,sizeof(tcall) */
CALL,0xd6,	             	/* call	si */
CS,0x8E,0x16,0,0,         	/* mov	ss,cs:ssloc  */
CS,MOV,0x26,2,0,         	/* mov	sp,cs:sploc ;restore old stack  */

#else

MOV, 0x66, 8,			/* mov sp, 8[bp] */
MOV, 0x46, 10,			/* mov ax,10[bp] */
0x8C, 0xDA,			/* mov dx,ds */
0xBB, ALICE_VERSION, 0,		/* mov bx, alice_version  ; change if >256*/
CALL, 0x56, 6,			/* call 6[bp] */
MOV, 0xE5,			/* mov sp, bp */
#endif
0x5D,				/* pop bp */
0xCB				/* ret far */
};

#ifdef LARGE
# define ex_checkalloc checkalloc
#endif

char far *ex_checkalloc();
typedef long (*lpfunc)();
lpfunc doturcall;

struct tlib {
	char *libname;
	unsigned libadr;		/* address of library */
	} active_libs[MAX_TLIB];

#define SAV_RSIZE 100

do_tplib( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
	char *nam;
	register int libnum;
	pointer argbase;
	int tomove;	/* number of bytes to move into save region */
	pointer saveregion[SAV_RSIZE];
	extern pointer p_array[];

#ifdef LARGE
	nam = argv[-2] + STR_START;
#else
	nam = argv[-1] + STR_START;
#endif
	for( libnum = 0; libnum < num_libs; libnum++ )
		if( !strcmp( nam, active_libs[libnum].libname ) )  {
			goto gotlib;
			}

	if( num_libs < MAX_TLIB-1 ) {
		unsigned libload, tlload();
#ifndef LARGE
		if( !num_libs ) {
			doturcall = (lpfunc)ex_checkalloc( sizeof(tcall) );
				/* check for error */
			dos_blk( doturcall, tcall, get_DS(), sizeof(tcall) );
			}
#endif
		
		if( libload = tlload( nam ) ) {
			active_libs[num_libs].libadr = libload;
			active_libs[num_libs++].libname = allocstring(nam);
			}
		 else 
			run_error(ER(308,"llib`Could not load library %s"),nam);
		}
	 else
		run_error( ER(309,"llib`Too many loaded libraries : %s"), nam );

gotlib:
	if( argc == 2 ) {
		argbase = (pointer)&(get_fpointers(st_display)[EXTR_DISP_WORDS-2]);
		tomove = min( argbase - sp.st_generic, sizeof(saveregion) );
		blk_move( saveregion, argbase - tomove, tomove );
		}
 	 else
		argbase = sp.st_generic;
#ifdef LARGE
	{
	lpfunc doturcall;
	unsigned int funcadr;
	unsigned int *newstack, *argcopy();

	funcadr = (unsigned)argv[-4];
	if( argc == 2 ) {
		extern framesize argmap();
		pointer frameptr;
		int scope;
		scope = (bits8)st_display[0].st_stackloc;
		frameptr = st_display[-scope].st_stackloc;
		newstack = (unsigned int *)(frameptr) - scan_decl(
			kid2(my_block(ex_loc)),argmap,(framesize)0,
				TRUE,FALSE,frameptr);
		}
	 else 
		newstack = argcopy( argc, argv-4, (unsigned int *)argv );
	FP_OFF(doturcall) = ST_STORAGE*sizeof(int);	/* two storage words */
	FP_SEG(doturcall) = active_libs[libnum].libadr;
	tc_result = (*doturcall)( funcadr, newstack, p_array );
	}
#else
	/* if no args, let him use the ones of this routine, including even
           the return value.  We must save away the display that he is going
	   to step all over, and as much of the stack, too */
	tc_result = (*doturcall)(active_libs[libnum].libadr +(unsigned)argv[-2],
		argbase, p_array );
#endif
	/* restore copied section of the display */
	if( argc == 2 ) 
		blk_move( argbase - tomove, saveregion, tomove );
	if( sym_saveid(sym) == -130 )
		*(rint *)argv = (rint)tc_result;
}
#else
#ifndef GEM
do_tplib(){}
int active_libs;
#endif
#endif

#if defined(LARGE) && defined(msdos)
unsigned
tlload( fname )
char *fname;
{
	FILE *cf;
	FILE *qfopen();
	pointer arena; 
	pointer readloc;
	long flen, filelength();

	if( cf = qfopen( fname, "rb" ) ) {
		flen = filelength( fileno( cf ) );
		if( flen > 0l && flen < (65520l - sizeof(tcall) ) ) {
			if( arena = ex_checkalloc((int)flen+15+sizeof(tcall))){
				/* generate normalized pointer for work */
				FP_SEG(readloc) = FP_SEG(arena) +
					(FP_OFF(arena) >> 4) + 1;
				FP_OFF(readloc) = 0;
				blk_move( readloc, tcall, sizeof(tcall) );
				fread( readloc+sizeof(tcall),(int)flen,1,cf );
				}
			}
		fclose( cf );
		return FP_SEG(readloc);
		}
	return 0;
}
#else
#ifndef GEM
unsigned
tlload( fname )
char *fname;
{
	long *clret, *comload();

	clret = comload( fname );
	return (unsigned int) *clret;
}
#endif GEM
#endif

#ifdef LARGE

#ifndef GEM

#define MAXIARGS 24

unsigned int *
argcopy(argc, getargs, argdest)
int argc;
pointer getargs;
unsigned int *argdest;		/* location where they are going */
				/* probably overlaps with getargs */
{
	int i;
	int typesize;
	int argarp = 0;

	for( i = 2; i < argc; i++ ) {
		int wordcount;
		int dex;
		nodep ourtype;

		getargs -= sizeof(pointer);
		ourtype = * ((nodep *) getargs);
		/* most things are just pushed as pointers */
		if( is_real(ourtype) )
#ifdef FLOATING
			typesize = sizeof(rfloat);
#else
			typesize = sizeof(double);
#endif
		 else if( ntype(ourtype) == N_TYP_SET )
			typesize = SET_BYTES;
		 else if( is_ordinal(ourtype) )
			typesize = sizeof(rint);
		  else
			typesize = sizeof(pointer);
		getargs -= max( typesize, sizeof(pointer) ); 
		wordcount = typesize / sizeof(unsigned int);
		/* loop through words of this argument, put in array */
		for( dex = 0; dex < wordcount; dex++ ) {
			*--argdest = ((rint *)getargs)[dex];
			if( argarp++ >= MAXIARGS )
				run_error( ER( 9, "cfunc`Too many arguments to C function") );
			}
		}
	return argdest;
}
framesize
argmap( thedecl, so_far, type_parent, argv )
symptr thedecl;		/* what symbol is declared */
reg framesize so_far;		/* how many bytes so far */
nodep type_parent;	/* the N_VAR_DECL node */
int * argv;		/* general argument, arg vector to sub from */
{
	int pushsize;
	int * base;
	int i;

	if( ntype(type_parent) == N_FORM_REF )
		pushsize = sizeof(pointer)/sizeof(int);
	 else
		pushsize = sym_size(thedecl)/sizeof(int);
	base = argv - (sym_offset(thedecl) / 2);
	so_far += pushsize;
	for( i = pushsize - 1; i >= 0; i-- )
		argv[i-so_far] = base[i];
	return so_far;
}
#endif GEM

#endif LARGE
