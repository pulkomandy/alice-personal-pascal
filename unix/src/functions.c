/*
 *  Built in routines of Pascal
 */
#define INTERPRETER	1
#define FUNCTIONS	1

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "alctype.h"
#include "extramem.h"
#include <stdlib.h>

/*
 *DESC: The main functions to handle built-in routines.
 */


#include "typecodes.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "bfuncs.h"
#include "interp.h"
#include "dbflags.h"

#ifdef FLOATING
#ifndef OLMSG
static char intout[] = "cantconv`Floating value %g can't be converted to integer";
#endif
#else
static char intout[] = "cantconv`Sorry, no real values in this version";
#endif


do_trunc( argc, argv )
rint *argv;
{
	rfloat arg = ((rfloat *)argv)[-1];
#ifdef UDEBUG
	if( arg >= FLOATNUM(MAXINT+1.0) || arg <= FLOATNUM(-MAXINT-1.0) )
		run_error( ER(31,intout), arg );
#endif UDEBUG
	*argv = (rint) arg;
}

do_round( argc, argv )
rint *argv;
{
	rfloat arg = ((rfloat *)argv)[-1];

#ifdef UDEBUG
	if( arg >= FLOATNUM(MAXINT+0.5) || arg <= FLOATNUM(-MAXINT-0.5))
		run_error( ER(31,intout), arg );
#endif UDEBUG
	*argv = (rint) (arg >= 0 ? arg + FLOATNUM(0.5) : arg - FLOATNUM(0.5));
}
do_sin( argc, x )
rfloat *x;
{
	extern rfloat sin();

	/* bounds on angle ? */

	*x = sin( x[-1] );
}
do_cos( argc, x )
rfloat *x;
{
	extern rfloat cos();

	/* bounds on angle ? */

	*x = cos( x[-1] );
}

do_arctan( argc, x )
rfloat *x;
{
	extern rfloat atan();


	*x = atan( x[-1] );
}
do_ln( argc, x )
rfloat *x;
{

	extern rfloat log();

	if( x[-1] <= FLOATNUM(0.0))
		run_error(ER(32,"ln`Logarithm of number <= 0.0") );

	*x = log( x[-1] );

}
do_exp( argc, x )
rfloat *x;
{

	extern rfloat exp();

	*x = exp( x[-1] );

}
do_sqrt( argc, x )
rfloat *x;
{
	extern rfloat sqrt();
	if( x[-1] < FLOATNUM(0.0))
		run_error( ER(33,"sqrt`Square root of negative number") );
	*x = sqrt( x[-1] );
}

#ifdef QNX
int	callNumber;
#endif

/*
 * Do a system call
 */
do_sysproc(argc, argv)
int	argc;
rint	*argv;		/* argv points to the top of the stack */
{
#ifndef ICRIPPLE
#ifdef QNX
	callNumber = argv[-1*P_S];	/* get system call number */
	if (callNumber >= 128)
		sys62(argv[-2*P_S], argv[-3*P_S], argv[-4*P_S], argv[-5*P_S], argv[-6*P_S], argv[-7*P_S], argv[-8*P_S], argv[-9*P_S], argv[-10*P_S]);
	else
		sys72(argv[-2*P_S], argv[-3*P_S], argv[-4*P_S], argv[-5*P_S], argv[-6*P_S], argv[-7*P_S], argv[-8*P_S], argv[-9*P_S], argv[-10*P_S]);
	fflush(stdout);
#else
	funcptr ptr;

#  ifdef HYBRID
	FP_OFF(ptr) = argv[-1];
	FP_SEG(ptr) = get_DS();
#  else
	ptr = ((funcptr *)argv)[-1];
#  endif LARGE
	(*ptr)(argc, argv, sp.st_generic);
#endif qnx
#else
#endif ICRIPPLE
}

do_sysfunc(argc, argv)
int	argc;
rint	*argv;		/* argv points to the top of the stack */
{
#ifndef ICRIPPLE
#ifdef QNX
	int	ret;

	callNumber = argv[-1];	/* get system call number */
	if (callNumber >= 128)
		sys62(argv[-2*P_S], argv[-3*P_S], argv[-4*P_S], argv[-5*P_S], argv[-6*P_S], argv[-7*P_S], argv[-8*P_S], argv[-9*P_S], argv[-10*P_S]);
	else
		sys72(argv[-2*P_S], argv[-3*P_S], argv[-4*P_S], argv[-5*P_S], argv[-6*P_S], argv[-7*P_S], argv[-8*P_S], argv[-9*P_S], argv[-10*P_S]);

	asm("mov -2[bp],ax");	/* ret <- result of sys. call */

	*argv = ret;

	fflush(stdout);
#else
	funcptr ptr;

#ifdef HYBRID
	FP_OFF(ptr) = argv[-1];
	FP_SEG(ptr) = get_DS();
#else
	ptr = ((funcptr)((pointer *)argv)[-1]);
#endif
	*argv = (*ptr)(argc, argv, sp.st_generic);
#endif
#else
#endif ICRIPPLE
}

#ifdef QNX
#include "systemCall.h"
#endif

do_peek( argc, argv )
int argc;		/* how any args, already checked */
reg rint *argv;		/* arguments and return */
{
	*argv = (unsigned)*(bits8 far *)(long)(int_stop);
}

/* the chr builtin - a nop except for range checking */
do_chr( argc, argv )
int argc;
reg rint *argv;
{
#ifdef UDEBUG
	if( int_stop >= 256 )
		run_error( ER(34,"badchr`chr: can't convert %d to character"), int_stop );
#endif UDEBUG
	*argv = int_stop & 255;
}

do_odd( argc, argv )
int argc;
reg rint *argv;
{
	*argv = int_stop & 1;
}

do_abs(argc, xargv )
int argc;
nodep * xargv;
{
	register nodep *argv;
	argv = xargv;

	if( is_integer(comp_type(argv[-1])) )
		*(rint *)argv = abs(int_stop);
	 else {
		rfloat thearg;
		thearg = f_stop;

		*(rfloat *)argv = thearg >= FLOATNUM(0.0) ? thearg : - thearg;
		}
}

do_sqr( argc, xargv )
int argc;
rint *xargv;
{
	register rint *argv;
	rint num;
	argv = xargv;

	if( is_integer(comp_type(((nodep *)argv)[-1]) ) ) { 
		num = int_stop;
		/* NOTE THIS CODE ASSUMES 16 BIT INTEGERS */
		if( num > 181 || num < -181 ) {
#ifdef SAI
			sai_print( "integer overflow\n" );
			done();
#else
# ifdef msdos
			overror(2);	/* integer multiplication overflow*/
# endif
# ifdef QNX
			extern char *OvflowCause;
			OvflowCause = "square";
			ovflow();
# endif QNX
#endif
			}
		*argv = num * num;
		}
	 else {
		rfloat thearg;
		thearg = f_stop;
		*(rfloat*)argv = thearg * thearg;
		}
}

do_succ( argc, xargv )
int argc;
rint *xargv;
{
	register rint *argv = xargv;
	register nodep passtype;
	rint ordv;
	
	ordv = int_stop;

	passtype = comp_type(((nodep *)argv)[-1]);
#ifdef UDEBUG
	if( ordv >= typ_bound( passtype, 1 )  )
		run_error( ER(35,"succ`the value %s doesn't succ"),
			ordname( passtype, ordv) );
#endif UDEBUG
	*argv = ordv + 1;
}

/* assumes pointer and integer the same */
do_pred( argc, xargv )
int argc;
rint *xargv;
{
#ifndef SMALLIN
	register rint *argv = xargv;
	register nodep passtype;
	rint ordv;
	
	ordv = int_stop;

	passtype = comp_type(((nodep *)argv)[-1]);
#ifdef UDEBUG
	if( typ_bound( passtype, 0 ) >= ordv )
		run_error( ER(36,"pred`pred of %s is beyond bounds"),
				ordname( passtype,ordv ) );
#endif UDEBUG
	*argv = ordv - 1;
#endif SMALLIN
}

do_ord( argc, argv )
int argc;
rint *argv;
{
	*argv = int_stop;
}

do_poke( argc, argv )
int argc;		/* how any args, already checked */
reg rint *argv;		/* arguments and return */
{
	/* There should be checking on the range of this poke */
	*(bits8 far *)(long)(argv[-1*P_S]) = (bits8) int_stop;
}

/* strconcat, two strings, with types */
do_strcat( argc, argv )
int argc;
pointer *argv;
{
#ifndef SMALLIN
	register char *st1, *st2;
	int stlen1, stlen2;
	int totlen;


	st1 = argv[-2] + STR_START;
	st2 = str_spop() + STR_START;
	/* fix this */
	stlen1 = strlen( st1 );
	stlen2 = strlen( st2 );

	do_undef( st1, U_CHECK, min(stlen1 + 1,get_stsize(argv[-1]) ));
	do_undef( st2, U_CHECK, min(stlen2 + 1,get_stsize(argv[-3],st2) ));
	checksvar( NCAST argv[-1], "StrConcat first argument" );
	if( stlen1 + stlen2 > get_stsize( NCAST argv[-1] ) )
		run_error( ER(37,"catlong`StrConcat makes string too long to fit") );
	strcat( st1, st2 );
	do_undef( st1, U_SET, (totlen = stlen1 + stlen2) + 1 );
#ifdef TURBO
	st1[-1] = totlen;
#endif
#endif SMALLIN

}

checksvar( sttype, badarg )
nodep sttype;
char *badarg;
{
	if( sttype == Basetype( BT_char) || ntype( sttype ) == N_CON_STRING )
		run_error( ER(38,"strvar`%s must be a string variable"), badarg );
}


/* strdelete, string, length, off_set */
do_strdelete( argc, xargv, sym )
int argc;
pointer *xargv;
symptr sym;
{
#ifndef SMALLIN
	register pointer *argv = xargv;
	register char *st1;		/* string and pointer for moving */
	rint length, off_set;
	rint stlen;

	st1 = argv[-2] + STR_START;
	checksvar( NCAST argv[-1], "String Delete string" );
	length = ((rint *)argv)[-4*P_S];
	off_set = ((rint *)argv)[-6*P_S];
#ifdef TURBO
	if( sym_saveid( sym ) < -90 ) {
		int temp;
		temp = length;
		length = off_set;
		off_set = temp;
		}
#endif
	off_set--;	/* decrement for 0 origin strings */
	if( off_set + length > (stlen = strlen( st1 ))|| off_set < 0 || length<0)
		run_error( ER(39,"strdelete`String Delete: Attempt to delete beyond bounds string: Offset %d"), off_set + 1 );
	do_undef( st1, U_CHECK, off_set + length );
	/* 1 + for zero byte */
	blk_move( st1 + off_set, st1 + off_set + length, 1+stlen-off_set-length);
	/* mark other locations as undefined */
	do_undef( st1 + stlen + 1 - length, U_CLEAR, length );
#ifdef TURBO
	st1[-1] = stlen - length;
#endif

	
#endif SMALLIN
}

#ifdef TURBO
do_copy(argc, argv )
int argc;
pointer argv;
{
	int pos, len;
	unsigned char *src;
	pointer newstack;

	len = int_spop();
	pos = int_spop() - 1;	/* to adjust for zero origin */
	if( pos < 0 || pos > 254 )
		run_error( ER(315,"strplus`Copy string position %d out of bounds"), pos+1);
	src = str_spop() + STR_START;
	len = min( len, src[-1] - pos );
	if( len < 0 )
		len = 0;
	 else
		do_undef( src + pos, U_CHECK, len );

	/* now push the substring */
	str_ret_push( argv, src+pos, len );

}
/* push a string on the stack for return */

str_ret_push( argv, str, len )
pointer argv;
char *str;
int len;	/* length of string */
{
	/* the magical 8 here is the pseudo size of the copy return value */
	sp.st_generic = argv - (len + 2 - 8);
	if( len )
		blk_move( sp.st_generic+1, str, len );
	sp.st_generic[0] = len;
	sp.st_generic[len+1] = 0;
	do_undef( sp.st_generic, U_SET, len+2 );
	p_spush( (pointer)0 );
	estop( e_pointer ) = sp.st_generic; 
	
}
#endif TURBO

/* strinsert, two strings, off_set */
do_strinsert( argc, xargv, sym )
int argc;
pointer *xargv;
symptr sym;
{
#ifndef SMALLIN
	register pointer *argv = xargv;
	register char *st1;		/* string and pointer for moving */
	nodep dest_type;		/* destination type */
	char *st2;
	rint off_set;
	rint stlen;
	rint stlen2;

	off_set = int_spop() - 1;
	skip_type();
	if( sym_saveid( sym ) < -90 ) {
		/* turbo insert */
		st1 = str_spop() + STR_START;
		dest_type = NCAST p_spop();
		st2 = str_spop() + STR_START;
		}
	 else {
		st2 = str_spop() + STR_START;
		skip_type();
		st1 = str_spop() + STR_START;
		dest_type = NCAST p_spop();
		}
	checksvar( dest_type, "String Insert string" );
	/* remove the type */
	stlen = strlen( st1 );
	/* allow insertion before the StrEnd character */
	if( off_set < 0 || off_set > stlen )
		run_error( ER(40,"stringbound`String Insert: Offset %d beyond the bounds of string %s"),
				off_set+1, st1 );
	stlen2 = strlen( st2 );
	do_undef( st2, U_CHECK, stlen2 );
#ifdef UDEBUG
	if( stlen + stlen2 > get_stsize( dest_type ) )
		run_error( ER(41,"stringbig`String too long") );
#endif UDEBUG
	/* move the bytes out of the way */
	blk_move( st1 + off_set + stlen2, st1 + off_set, stlen - off_set + 1 );
	/* put in the new bytes */
	blk_move( st1 + off_set, st2, stlen2 );
	do_undef( st1, U_SET, stlen + stlen2 + 1 );
#ifdef TURBO
	st1[-1] = stlen + stlen2;
#endif

#endif SMALLIN
}


/* substr, string, length, off_set, dest string */
do_substr( argc, xargv )
int argc;
pointer *xargv;
{
#ifndef SMALLIN
	register pointer *argv = xargv;
	register char *st1;		/* string and pointer for moving */
	nodep dest_type;
	char *st2;
	rint length;
	rint off_set;
	rint stlen;


	st2 = str_spop() + STR_START;
	checksvar( dest_type = NCAST p_spop(), "SubStr destination string" );
	off_set = int_spop() - 1;
	skip_type();	/* type of offset */
	length = int_spop();
	skip_type();	/* type of length */
	st1 = str_spop() + STR_START;
#ifdef UDEBUG
	stlen = strlen( st1 );

	if( length + off_set > stlen )
		run_error( ER(42,"substrbound`SubStr: requested substring beyond bounds of string") );
	if( length > get_stsize( dest_type ) )
		run_error( ER(43,"substrfit`SubStr: Substring too long to fit in destination") );
	do_undef( st1 + off_set, U_CHECK, length );
#endif UDEBUG
	blk_move( st2, st1 + off_set, length );
	st2[length] = 0;
	do_undef( st2, U_SET, length + 1 );
#endif SMALLIN
}


/* strlen of string, function */
do_strlen( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
	register unsigned char *st;
	register int len;

	st = str_spop() + STR_START;
#ifdef TURBO
	if( sym_saveid( sym ) == -89 ) {
		*(rint *)argv = st[-1];
		check_undef( st-1, 1 );
		}
	 else {
#endif
		len = strlen( st );
		*(rint *)argv = len;
		do_undef( st, U_CHECK, min(len+1, get_stsize(argv[-1],st) ));
		}

}

/* strscan of two strings, function */
do_strscan( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
#ifndef SMALLIN
	int s1len, s2len;
	register char *st1, *pos;
	char *st2;
	char firstc;

	*(rint *)argv = 0;
	st2 = str_spop() + STR_START;
	skip_type();
	st1 = str_spop() + STR_START;
	if( sym_saveid(sym) == -88 ) {
		/* oh boy turbo */
		char *temp;
		/* swap em */
		temp = st1;
		st1 = st2;
		st2 = temp;
		}
	if( !(s2len = strlen( st2 ) ) )
		return *(rint *)argv = 1;
	if(!(s1len = strlen( st1 )) )
		return;
	firstc = *st2;
	do_undef( st1, U_CHECK, s1len );
	do_undef( st2, U_CHECK, s2len );
	for( pos = st1; pos = strchr( pos, firstc ); ++pos)
		if( !strncmp( pos, st2, s2len ) )
			return *(rint *)argv = pos - st1 + 1;
#endif SMALLIN
}

/* strsize of string, function */
do_stsize( argc, argv )
int argc;
nodep  *argv;
{
	*(rint *)argv = get_stsize(argv[-1],(char *)0);
}

#ifndef FULL
do_pack(argc, argv)
int	argc;
nodep	*argv;
{
	run_error(ER(44,"unfinished`Pack and unpack not implemented"));
}
#endif

do_pause( argc, argv )
{
#ifdef UDEBUG
	/* turn on single step */
	extern int step_flag;
	extern int within_hide;
	EDEBUG( 3, "Got to Pause procedure\n", 0, 0 );

	if( step_flag & DBF_ON || argc > 0 )
		step_flag |= STEP_SINGLE;
	if( argc > 0 )
		within_hide = FALSE;
#endif UDEBUG
}

rint rand_modulus = 0;		/* random # modulus */

do_initrandom( argc, argv )
int argc;
rint *argv;
{
	rand_modulus = int_spop();
	if( rand_modulus <= 0 )
		run_error( ER(45,"initrand`Random modulus must be greater than zero"));
	srand( int_stop );
}

do_random( argc, argv )
int argc;
rint *argv;
{
#ifdef TURBO
	extern int	turbo_flag;

	if( argc )
		*argv = (rint)(((double)rand() * (long)int_stop)/ RAND_MAX);
	 else if( turbo_flag )
		*(rfloat *)argv = rand() / 32768.0;
	 else 
#endif
		{
		
		if( rand_modulus > 0 )
			*argv = (rint)(((double)rand() * rand_modulus)/RAND_MAX);
	 	else
			run_error( ER(46,"rand`You must call initrandom before using random") );
		}
}

do_memfunc( argc, argv, sym )
int argc;
rint  *argv;
symptr sym;
{
	unsigned total, biggest;

	total = newSize( &biggest );
	/* return in paragraphs */
	*argv = ( sym_saveid(sym) == -128 ? total : biggest ) / 16;
}


do_new( argc, argv, sym )
int argc;
nodep  *argv;
symptr sym;
{
	register int howbig;
	extern pointer free_top;
	extern pointer newAlloc();
	pointer segres;
	register pointer *wh_put_ptr;	/* where to put our new pointer */

	if( sym_saveid(sym) < -100 )
		howbig = int_spop();
	 else if( argv[-1] == Basetype(BT_pointer) )
		howbig = 1;
	 else
		howbig = calc_size( find_realtype( kid1( argv[-1]) ), TRUE );

	wh_put_ptr = (pointer *) argv[-2];
/*
 * Use checkalloc in the non HYBRID SAI 
 */
#if defined( SAI ) && !defined(HYBRID)
	segres = checkalloc( howbig );
#else
	segres = newAlloc( howbig );
#endif

	if( !segres )
		run_error( ER(47,"newout`New - out of memory") );

	/* mark the memory as un-assigned */
#ifndef SAI
	do_undef( segres, U_CLEAR, howbig );
	/* mark the pointer to it as assigned */
	do_undef( (pointer)wh_put_ptr, U_SET, sizeof( pointer ) );
#endif

	*wh_put_ptr = segres;
	/* bump the top of the free list */
}

/*
 * Currently, DISPOSE does not return memory to a free list.  This is
 * easy to do, and also provides full checking for use of dangling
 * pointers.   Some day we'll fix this
 * Modified to take a value argument since you can pass a function result
 * to this guy..
 */

do_dispose( argc, argv, sym )
int argc;
nodep  *argv;
symptr sym;
{
	register int howbig;
	extern pointer free_top;
	register pointer theptr;


	if( sym_saveid(sym) < -100 ) {
		/* variable parameter for freemem */
		theptr = *(pointer *)argv[-2];
		howbig = int_stop;
		}
	 else {
		theptr = (pointer) argv[-2];
		howbig = calc_size( find_realtype( kid1( argv[-1] ) ), TRUE );
		}
#if !defined(SAI) || defined(HYBRID)

	/* check_undef( wh_put_ptr, 1 ); - value pointer */
	if( theptr == NULL )
		run_error( ER(61,"nil`Error - This pointer is NIL") );
	/* mark the memory as un-assigned */
#ifndef SAI
	do_undef( theptr, U_CLEAR, howbig );
	/* mark this pointer as un-assigned */
	/* can't be done with value argument */
	/* do_undef( wh_put_ptr, U_CLEAR, sizeof( pointer ) ); */
#endif

	closeall( FALSE, theptr, theptr + howbig );

	if (newFree( theptr, howbig ) == -1)
		run_error(ER(205, "dispose`Attempt to dispose memory already disposed of"));
#else
	if( theptr == NULL )
		run_error( ER(61,"nil`Error - This pointer is NIL") );
	closeall( FALSE, theptr, theptr + howbig );
	mfree( theptr );
#endif SAI
}

do_address( argc, argv )
int argc;
rint *argv;
{
	if( argc == 1 || ! int_spop() )
		*argv = (rint)p_stop;
	 else
#if defined(LARGE) || defined(POINT32)
		*argv = ((long)p_stop) >> 16;
#else
		*argv = get_DS();
#endif
}

	/* turn an lvalue into a generic pointer */ 

do_makeptr( argc, argv )
int argc;
pointer *argv;
{
	*argv = p_stop;
}

do_setattr(argc, argv )
int argc;
rint *argv;
{
	extern WINDOW *pg_out;
#ifdef SAI
	extern int sai_out;
	if( sai_out == SAI_NORMAL ) return;
#endif
	reswflags( pg_out, int_stop << 8 );
}

do_sizeof( argc, argv )
int argc;
pointer *argv;
{
	*(rint *)argv = calc_size(argv[-1],TRUE);
}
	/* form pointer from two integers, off_set, segment */

do_blkmov( argc, argv, dir )
int argc;
pointer *argv;
int dir;	/* direction */
{
	int blksize;
	pointer varadr;
	int seg, off;
	blksize = calc_size( argv[-1], TRUE );
	varadr = argv[-2];
	off = ((rint *)argv)[-4*P_S];
#ifdef msdos
	if( argc >= 3 )
		seg = ((rint *)argv)[-6*P_S];
	 else
#ifndef LARGE
		seg = get_DS();
	/* in fact, this is only for hybrid model msoft compiler */
	if( dir )	/* true means var to mem */
		dos_blk( off, seg, varadr, get_DS(), blksize );
	 else {
		dos_blk( varadr, get_DS(), off, seg, blksize );
		do_undef( varadr, U_SET, blksize );
		}
#else
		seg = SSeg;
	/* large model ? */
	if( dir )
		dos_blk( off, seg, varadr, blksize );
	 else {
		dos_blk( varadr, off, seg, blksize );
		do_undef( varadr, U_SET, blksize );
		}
#endif
#else
	if( dir )
		blk_move( off, varadr, blksize );
	 else
		blk_move( varadr, off, blksize );
#endif
}

do_vartomem( argc, argv )
int argc;
pointer *argv;
{
	do_blkmov( argc, argv, TRUE );
}

do_memtovar( argc, argv )
int argc;
pointer *argv;
{
	do_blkmov( argc, argv, FALSE );
}

do_point( argc, argv )
int argc;
rint *argv;
{
#ifndef POINT32
	/* segment can be missing */
	pointer ret;
	unsigned segarg;
	FP_OFF(ret) = (pointer)argv[-P_S];

	if( argc > 1 ) {
		segarg = int_spop();
#ifndef LARGE
		if( segarg != get_DS() )
			run_error( ER(274, "smallerr`Segment must be Data Segment in non-large ALICE" ) );
		}
#else
		FP_SEG(ret) = segarg;
		}
	 else	/* use stack/data segment */
		FP_SEG(ret) = ((long)&ret) >> 16;
#endif
	

	*(pointer *)argv = ret;
#endif
}

do_delay(argc,argv)
int argc;
rint *argv;
{
	long ctr;
	int del = int_stop;

#if defined(msdos) || defined( QNX )
	tdelay( del / 10 );
#else
	for( ctr = 0; ctr < del * 100l; ctr++ )
		;
#endif
}

do_sound(argc,argv)
int argc;
rint *argv;
{
#if defined(msdos) || defined(ICON)
	soundon( argc ? int_stop : 0 );
#else
	putchar( 7 );
#endif
}

do_halt(argc,argv)
int argc;
rint *argv;
{
	prep_stacks(TRUE);
	/* set to terminate */
	espush( 0, e_loc );
}

do_clreol()
{
	extern WINDOW *pg_out;

	checkScreen();

	wclrtoeol(pg_out);
	wrefresh(pg_out);
}

do_insdel(argc, argv, bsym)
int argc;
rint *argv;
symptr bsym;
{
	extern WINDOW *pg_out;
	/* -85 for InsLine, -86 for DelLine */

	checkScreen();

	if( sym_saveid(bsym) == -85 )
		winsertln(pg_out);
	 else
		wdeleteln(pg_out);
	wrefresh( pg_out );
}

do_flpart(argc, argv, sym)
int argc;
rfloat *argv;
symptr sym;
{
#ifdef FLOATING
	extern double modf();
	double fracpart, intpart;

	fracpart = modf( argv[-1], &intpart );
	/* 91 for int, 92 for frac */
	*argv = sym_saveid( sym ) == -91 ? intpart : fracpart;
#endif
}

do_val(argc, argv)
int argc;
rfloat *argv;
{
	extern rfloat atof();
	rint *rescode;			/* address of result code */
	nodep vartype;			/* type of numeric */
	char sbuf[255];
	int slen;			/* length of string */
	int i;
	int ret;
	pointer numvariable;		/* numeric variable */
	char *inpstr;			/* input string */
	int tokennum;			/* parser token */
	extern int turbo_io;

	rescode = (rint *)p_spop();
	skip_type();
	numvariable = p_spop();
	vartype = comp_type(NCAST p_spop());	/* type of numeric */
	inpstr = str_spop() + STR_START;

	slen = strlen(inpstr);
	do_undef( rescode, U_SET, sizeof(rint) );
	do_undef( numvariable, U_CLEAR, 1 );
	for( i = slen; i > 0; i-- ) {
		sprintf( sbuf, "%0.*s ", i, inpstr );
		if( is_integer( vartype ) )
			ret = validint( sbuf );
		 else
			ret = validreal( sbuf );
		if( ret == ' ' ) {
			if( i == slen ) { /* valid all the way */
				char *p;
				if( turbo_io && (p = strchr( inpstr, ' ' ))) {
					*rescode = 1 + (p - inpstr);
					return;
					}
				if( is_integer(vartype) ) {
					if( badint( inpstr, FALSE ) ) {
						*rescode = 1;
						return;
						}
					*(rint *)numvariable = atoi(inpstr);
					}
				 else
					*(rfloat *)numvariable = atof(inpstr);
				do_undef( numvariable, U_SET, 1 );
				*rescode = 0;
				return;
				}
			 else
				break;
			}
		}
	/* dropped out of loop */
	*rescode = i+1;

}

do_intr( argc, argv )
int argc;
pointer *argv;
{
#ifdef msdos
	pointer theregs;
	nodep sttype;
	theregs = p_spop();
	sttype = (nodep)p_spop();
	if( calc_size(sttype, TRUE) < 10 *sizeof(int) ) {
		run_error( ER(324,"toosmall`Structure too small to hold registers") );
		}
	turboint( argc > 1 ? int_stop : 0x21, theregs );
	do_undef( theregs, U_SET, 10*sizeof(int) );
#else
	run_error( ER(331,"wrongos`This routine is not implemented on this machine") );
#endif
}

do_upcase(argc, argv)
int argc;
rint *argv;
{
	register unsigned char c = int_stop;
	*argv = islower(c) ? c - 32 : c;
}


do_exit()
{
	/* subtract 1 for the RETURN and caller address */
	/* as the value stored is the address of the RETURN */
	ex_stack = (estackloc *)(get_fpointers(st_display)[3]) - 1;
}



#ifdef ES_TREE
funcptr builtins[] = {
	do_write,
	do_lnwrite,
	do_read,
	do_lnread,
	do_trunc,
	do_round,
	do_sin,
	do_cos,
	do_arctan,
	do_ln,
	do_exp,
	do_sqrt,
	do_peek,
	do_chr,
	do_odd,
	do_abs,
	do_sqr,
	do_succ,
	do_pred,
	do_ord,
	do_poke,
	do_strcat,
	do_strdelete,
	do_strinsert,
	do_substr,
	do_strlen,
	do_strscan,
	do_stsize,
	do_pause,
	do_sysproc,
	do_new,
	do_dispose,
	do_eof,
	do_eoln,
	do_get,
	do_put,
	do_reset,
	do_rewrite,
	do_sysfunc,
	do_initrandom,
	do_random,
	do_getch,
	do_charwait,
	do_page,
	do_address,
	do_append,
	do_update,
	do_setnext,
	do_cursorto,
	do_sizeof,
	do_scrfunc,
	do_setattr,
	do_point,
	do_makeptr,
	do_cintfunc,
	do_cptrfunc,
	do_cproc,
	do_getptr,
	do_pack,
	do_vartomem,
	do_memtovar,
	do_delay,
	do_sound,
	do_halt,
	0,
	do_insdel,
	do_clreol,
	do_assign,
	do_flpart,
	do_str,
	do_val,
	do_copy,
	do_intr,
	do_draw,
	do_plot,
	do_window,
	do_grmode,
	do_textmode,
	do_colour,
	do_ioresult,
	do_iocheck,
	do_clongfunc,
	do_close,
	do_memfunc,
	do_tplib,
	do_vid,
	do_upcase,
	do_exit,
	0,
#ifdef FULL
	do_nop,
	do_hilo,
	do_where,
	do_filefunc,
	do_rndomize,
	do_ptr,
	do_segoff,
	do_move,
	do_fillch,
	do_rename,
	do_dirfunc,
	do_getdir,
	do_markrel,
	do_blockio,
	do_longseek,
	do_parstr,
	do_parcnt,
	do_unpack,
	do_opendir,
	do_readdir,
	do_strtoar,
	do_artostr,
#endif
#ifdef TURTLE
	Fbackward,
	Fforward,
	do_setpal,
	Fgetheading,
	Fhome,
	Fpen,
	Fheading,
	Fsetpencolor,
	Fsetposition,
	Fleft,
	Fright,
#endif
	0,0,0,0,0,0
};
#endif ES_TREE

