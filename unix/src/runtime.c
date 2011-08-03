

/*
 *DESC: Main interpreter support
 */
#define INTERPRETER 1

#include "alice.h"
#include <curses.h>

#ifdef GEM
#include <gemdefs.h>
#endif

#ifdef SAI
extern int sai_out;
#endif

#include "typecodes.h"
#include "flags.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "bfuncs.h"
#include "interp.h"
#include "dbflags.h"
#include "extramem.h"

nodep suspended;	/* THIS IS TO BE DELETED */
int in_execws = FALSE;
struct susp_block *curr_suspend;		/* what current suspension is */
int count_suspend = 0;				/* how many suspensions */
pointer free_top;				/* top of free list */
extern fileptr open_array[MAX_F_OPEN];
extern int ofile_count;
#ifdef ICRIPPLE
char Crip_Error[] = "Sorry, you can't do this with the Demo interpreter";
#endif

	/* run the program */
run_prog(loc,setstep)
nodep loc;	/* where we interpret from */
int setstep;	/* what the step flag will become */
{
	extern int win_default;
	/* first we must descend all declarations and set up offsets
	and that sort of thing */
	extern unsigned char inp_buf[];
	extern unsigned char *inp_bp;
	extern WINDOW *pg_out;

	inp_bp = inp_buf;
	inp_buf[0] = 0;

	prep_stacks(TRUE);

	resetGraphics();

#ifdef GEM
	PgTitle( "Running" );
	wind_update( BEG_MCTRL );
#endif

#ifdef SAI
	if( sai_out == SAI_SCREEN ) 
#endif SAI
		reswflags( pg_out, win_default );

	interp(loc,(nodep)NIL, setstep);

#ifdef GEM
	PgTitle( "Output" );
	wind_update( END_MCTRL );
#endif

}

/*
 * terminated() -- called from interp when program finishes, terminated()
 *	cleans up the mess.
 */
terminated()
{
	extern WINDOW	*pg_out;

	if (!count_suspend)
		finishGraphics();

	curOn(pg_out);
}

int fv_count = 0;
#ifndef OLMSG
static char stoverrs[]="stack`Runtime stack overflow! - out of memory";
#endif

prep_stacks(do_estack)
int do_estack;		/* flag indicating what is to be done with ex_stack */
{
	extern struct pas_file fil_input, fil_output, fil_kbd;
	extern fileptr predef_files[];
#ifdef QNX
	extern FILE *predf_streams[];		/* streams for files */
#else
	extern FILE *predf_streams[];		/* streams for files */
#endif
	extern unsigned int predf_flags[];	/* flags for files */
	extern int predef_number;	/* number of predefined files */
	extern int call_depth;
	extern int code_damaged;
	extern int rand_modulus;		/* for watcom randoms */
	extern unsigned char *inp_bp;		/* line at a time input */
	extern unsigned char inp_buf[];
	extern unsigned SSeg;			/* exec stack segment */
	int i;	/* file counter */
	fileptr ftinit;
#ifdef SAI
	extern int sai_out;
#endif
#if defined(LARGE) && defined(msdos)
	extern struct symbol_node ndCSeg, ndSSeg, ndDSeg;
	extern int _psp;


	s_sym_value( &ndCSeg, _psp );
	s_sym_value( &ndSSeg, SSeg );
	s_sym_value( &ndDSeg, SSeg );

#endif

	if( !(exec_stack && loc_stack) )
		run_error(ER(105,stoverrs));
	sp.st_stackloc = &(exec_stack[max_stack_size]);
	/* newClear now done in do_run_prog */
/* 		newClear();	set up new segment */
			/* downward growing stack */
	st_display = (stack_pointer *) 0;	/* no active display yet */
	if( do_estack )
		ex_stack = &(loc_stack[lc_stack_size-1]);

	EDEBUG( 3, "Set stack ptr to %x, ex_stack to %x\n", (int)sp.st_stackloc,
					(int)ex_stack );
/*	zero( undef_bitmap, 1+max_stack_size/BITS_IN_BYTE ); (not needed?)*/

	within_hide = FALSE;
	EDEBUG(4, "within hide set to %d\n", within_hide,0 );
	code_damaged = FALSE;
	curr_suspend = (struct susp_block *)0;
	inp_bp = inp_buf;
	*inp_bp = count_suspend = fv_count = 0;	/* counter to put in file name */
	io_res_on = last_ioerr = rand_modulus = 0;
	/* is this safe when files can be opened with the execute key? */
	/* args do not matter if TRUE is first arg */
	closeall(TRUE,(pointer)0, (pointer)0);/* close any old files that were open */

	for( i = 0; i < predef_number; i++ ) {
		ftinit = predef_files[i];
		ftinit->f_name = 0;
		ftinit->f_size = sizeof(char);
#ifdef QNX
		ftinit->desc.f_stream = *predf_streams[i];
#else
		ftinit->desc.f_stream = predf_streams[i];
#endif
		ftinit->f_flags = predf_flags[i];
		}
		
#ifdef SAI
	if( sai_out == SAI_NORMAL ) {
		fil_input.f_flags = FIL_NREAD | FIL_OPEN | FIL_READ | 
			FIL_LAZY | FIL_TEXT;
		}
	else {
		fil_input.f_flags = FIL_NREAD | FIL_OPEN | FIL_LAZY | FIL_TEXT
			| FIL_READ | FIL_KEYBOARD;
		}
#else
	fil_input.f_flags = FIL_NREAD | FIL_OPEN | FIL_LAZY | FIL_TEXT
			| FIL_READ | FIL_KEYBOARD;
#endif SAI
	/* undef doesn't work on builtin guys yet */
	/* do_undef( &(fil_output.f_buf), U_CLEAR, 1 ); */
#ifdef SAI
	if( sai_out == SAI_NORMAL ) {
		fil_output.f_flags = FIL_WRITE | FIL_OPEN | FIL_EOF | FIL_TEXT;
	} else {
		fil_output.f_flags = FIL_OPEN | FIL_EOF | FIL_TEXT | FIL_SCREEN
				    | FIL_WRITE;
	}
#else
	fil_output.f_flags = FIL_WRITE | FIL_OPEN | FIL_EOF | FIL_TEXT | FIL_SCREEN;
#endif
#if defined(FULL) && defined(TURBO)
	{
	extern struct pas_file fil_con, fil_trm;
	fil_con.f_flags = fil_trm.f_flags = (FIL_DEVICE | fil_output.f_flags
		| fil_input.f_flags) & ~(FIL_EOF|FIL_EOL);
	}
	fullinit();
#endif
	call_depth = ofile_count = 0;
#ifndef SAI
	redraw_status = TRUE;
#endif SAI
}

#if defined(SAI) && !defined(HYBRID)
/* malloc/free New() allocator for stand alone interpreter */

newClear()
{
	;
}
pointer newAlloc(size)
unsigned int size;
{
	return checkalloc(size);
}
newFree( obj, size )
{
	free( obj );
}

#endif SAI

	/* get the block we are suspended in */
nodep 
get_sblock() {
	extern stack_pointer *tb_frame;
	if( curr_suspend && tb_frame )
		return( get_fpointers(tb_frame)[1] );
	return NIL;
}

/*
 * Set assignment.  Now does range checking by finding the first and
 * last bit in the source set and sr_check()'ing them; if they are in
 * range then the entire set in within the subrange.
 */
set_assign(dest, _src, eloc)
pointer	dest;
pointer	_src;
nodep eloc;
{
#ifndef ICRIPPLE
	register char	*src = _src;
	register int	i;
	nodep subrangep;		/* pointer to subrange tree */
	int	high;
	int	low;
	
	if (int_kid(2, eloc) & CC_SUBRANGE) {
		for (i = 0; i < SET_BYTES; i++)
			if (src[i])
				break;
		if (i < SET_BYTES) {
			for (low = 0; low <= 7; low++)
				if (src[i] & (1 << low))
					break;
			low |= i << 3;
			subrangep = find_realtype(kid1(gexp_type(kid1(eloc))));
			sr_check(subrangep, low);
			for (i = SET_BYTES - 1; i >= 0; i--)
				if (src[i])
					break;
			for (high = 7; high >= 0; high--)
				if (src[i] & (1 << high))
					break;
			high |= i << 3;
			sr_check(subrangep, high);
		}
	}
	blk_move(dest, src, SET_BYTES);
	do_undef(dest, U_SET, SET_BYTES);
#else
#endif ICRIPPLE
}

/*
 * Set operators.  The first four (=, <=, >=, in) are passed pointers
 * to sets (which have presumably been popped from the top of the stack),
 * and return a Boolean value.
 *
 * The other three (+, -, *) are passed lhs and rhs, but are expected to
 * leave the result in lhs (which is the top of the stack).
 */
int
set_eq(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++)
		if (*lhs++ != *rhs++)
			return FALSE;
	return TRUE;
}

int
set_le(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++, lhs++)
		if ((*lhs & *rhs++) != *lhs)
			return FALSE;
	return TRUE;
}

int
set_ge(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++, rhs++)
		if ((*lhs++ & *rhs) != *rhs)
			return FALSE;
	return TRUE;
}

int
set_in(_item, rhs)
rint _item;
pointer rhs;
{
	register rint item = _item;

	if (item < MIN_SET || item > MAX_SET )
		return FALSE;
	return (rhs[item >> 3] & (1 << (item & 7))) != 0;
}

set_union(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++)
		*lhs++ |= *rhs++;
}
		
set_subtract(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++)
		*lhs++ &= ~*rhs++;
}

set_intersect(_lhs, _rhs)
pointer	_lhs;
pointer _rhs;
{
	register int	i;
	register pointer lhs = _lhs;
	register pointer rhs = _rhs;

	for (i = 0; i < SET_BYTES; i++)
		*lhs++ &= *rhs++;
}

/*  Do all the procedure call stuff setting up a stack frame.
 *  For a picture of the stack frame, see the document in
 *  ../frame
 *  This assumes that the return value space has been pushed,
 *  and all the arguments as well.  Time to push the st_display and
 *  leave room for the variables as well 
 *  assumes the ap can be found on the estack
 */

make_frame( scope, varsize, caller, block, calldisp, hiddenrot )
int scope;		/* scope level of new creation */
unsigned varsize;	/* how many bytes for variables */
nodep caller;		/* where the call came from */
nodep block;		/* block associated with call */
stack_pointer *calldisp;	/* display for caller */
int hiddenrot;			/* is this routine hidden */
{
	register pointer ourap;	/* our argument pointer */
	register stack_pointer *newdisp;/* new st_display created */
	register int cpdisp;		/* pointer to copy st_display stuff */
	unsigned int indispsize;	/* size of inner display */
	pointer vptemp;			/* variable pointer temporary */
	extern int within_hide;

	EDEBUG( 3, "Creating new stack frame scope %d, varsize %d\n", scope,
		varsize );

	newdisp = (stack_pointer *)sp.st_pointer - (scope + EXTR_DISP_WORDS );
	
	/* leave room for variable pointers, arg pointers & extra disp words,
	   in calculating what the variable pointer will be */

	vptemp = (pointer)(sp.st_pointer - (2 * scope + EXTR_DISP_WORDS));
	/* now push a pointer to where the call came from and the old
	   display */

	indispsize = (2*(scope-1)+1)*sizeof(pointer);
	if(  ( (unsigned)(sp.st_generic - free_top) < STACK_BUFFSZ + varsize +
			indispsize + EXTR_DISP_WORDS*sizeof(pointer) )
			|| ex_stack < loc_stack + 40 ) 

		rstackov( (unsigned)9965000 );	/* guarantee failure */
	p_spush( (pointer)ex_stack );
#ifdef UDEBUG
	p_spush( (pointer)within_hide );
	/* just set within_hide to hiddenrot if you want calls out of hidden
	   routines to be traced through */
#ifdef OLD
	if( hiddenrot )
		within_hide = TRUE;
#else
		within_hide = hiddenrot;
#endif
	EDEBUG(4, "within hide set to %d\n", within_hide,0 );
	p_spush( (pointer)block );
	p_spush( (pointer)caller );
#else
	/* information not of value outside debug version */
	bump_stack( 3*sizeof(pointer) );
#endif
	/* should this be calldisp, and will there be debug pop problems? */
	p_spush( (pointer)st_display ); /* old display */
	/* push our variable pointer */
	p_spush( vptemp );
	/* use block move to copy over old display pointers */
	bump_stack( indispsize );
	if( scope > 1 && calldisp )
		blk_move( sp.st_generic, &calldisp[1-scope], indispsize );
	* (pointer )newdisp = scope;	/* save away scope */
	st_display = newdisp;
	ourap = espop(e_pointer);
	EDEBUG( 4, "Argument pointer this frame is %x\n", (int)ourap, 0 );
	/* push our argument pointer */
	p_spush(ourap);
	/* we used to mark the args as initialized, now it is done as pushed */
	/* do_undef( sp.st_generic, U_SET, ourap - sp.st_generic ); */
	/* ok, now leave room for the variables */
	bump_stack(varsize);
		/* abort on run errors? */
	/* make sure strings have zero bytes */
	zero( sp.st_generic, varsize );
	/* now mark the variables as uninitialized */
	do_undef( sp.st_generic, U_CLEAR, varsize );
}

/* check for runtime stack overflow */

rstackov( needed )
unsigned int needed;
{
	if( (unsigned)(sp.st_generic - free_top) < needed ) {
		run_error(  ER(105,stoverrs) );
		}
}

start_call( where )
nodep where;		/* location in the tree of call */
{
	register symptr asym;
	register listp arg_list;
	stack_pointer *calldisp;
	extern int call_depth;
	extern symptr get_routine();


	asym = get_routine( where, &calldisp );

	arg_list = FLCAST kid2(where);
	switch( sym_dtype( asym ) ) {
#ifndef ICRIPPLE
		case T_FORM_PROCEDURE:
		case T_FORM_FUNCTION:
			/* address on stack? */
		case T_PROCEDURE:
		case T_FUNCTION:
			espush( (pointer)sp.st_stackloc, e_pointer );
			if( listcount( arg_list ) ) {
				espush( 0, e_int );
				state_push(S_CALL1);
				new_loc = kid1(arg_list);
				}
			 else {
				nodep rnode = tparent(asym);
				NodeNum part = ntype(rnode);
				make_frame( sym_scope(asym), act_size(rnode),
					where, rnode, calldisp,
					sym_mflags(asym) & SF_HIDDEN );
				++call_depth;
				state_push(S_RETURN);
				new_loc = node_kid(rnode, kid_count(part) - 1 );
				}
			break;
#endif ICRIPPLE
		case T_BTPROC:
		case T_BTFUNC:
		case T_BPROC:
		case T_BFUNC:
			/*
			 * a builtin routine.  Some want types on stack too
			 * must push old stack, and a count for the list
			 */
			if(listcount(arg_list)){
				/* save the old stack away */
				espush( (pointer)sp.st_stackloc, e_pointer );
				espush( 0, e_int );
				if( sym_dtype(asym) < T_BPROC )
					p_spush( (char *)gexp_type(
						kid1(arg_list)) );
				state_push(S_CALL2);
				new_loc = kid1(arg_list);
				}
			 else
				/* call the procedure or function */
				bfptr(asym)(0,sp.st_generic,asym);
			break;
		case T_WRITELN:
		case T_READLN:
			/* calls must be individual */

			/* check for file parameter */
			if( listcount(arg_list) ){
			/* Push onto the execution stack the file that is
			   having i/o done.  This starts out as a zero which
			   indicates the default (input/output) but later
			   may get changed if there is a file argument */

				espush( (pointer)0, e_pointer );
				espush( 0, e_int );
				state_push(S_CALL3);
				new_loc = kid1(arg_list);
				}
			 else {
				/* call it with no args */
				EDEBUG(4,"Call I/O with no args at %x\n",
					where, 0 );
				bfptr(asym)(0,NIL,NIL,NIL);
				}
		}
}

/*VARARGS1*/
run_error( str, a1, a2, a3, a4, a5 )
ErrorMsg str;
char *a1, *a2, *a3, *a4, *a5;
{
	extern int imm_mode;
	extern int call_depth;
#ifndef SAI
	extern FILE	*LogFile;

#ifdef TOS
	wind_update( END_MCTRL );
#endif

	if( !imm_mode || call_depth != 0  )
		do_suspend( ex_loc, NIL, FALSE );

	if (LogFile) {
		fprintf(LogFile, "Error: ");
		fprintf(LogFile, getERRstr(str), a1, a2, a3, a4, a5);
		fprintf(LogFile, "\n" );
	}
#endif
	last_ioerr = 0;		/* no last io error if we got a message */

#ifdef MarkWilliams
	warning( "%r", &str );
#else
	warning( str, a1, a2, a3, a4, a4, a5 );
#endif

#ifndef SAI
	re_return();
#else SAI
	done((char *)0);
#endif
}


str_assign( dest, src, dest_size )
register unsigned char *dest;		/* the strings to assign */
register unsigned char *src;
int dest_size;		/* how much memory at destination */

{
#ifndef ICRIPPLE
	register int minsiz;
	int srclen;

#ifdef TURBO
	extern int turbo_flag;
	src++;
	dest++;
#endif

	/* this is not 100% pure but it is the best we can do to give
	   turbo compatibility without wrecking regular strings > 255 */
	srclen = strlen( src );
	if( srclen < 256 )
		srclen = src[-1];
	EDEBUG( 8, "Source len %d, dest size %d\n", srclen, dest_size );
	minsiz = dest_size < srclen ? dest_size : srclen;
	do_undef( src, U_CHECK, minsiz + (turbo_flag ? 0 : 1) );
	strncpy( dest, src, minsiz );
	/* stuff a zero in case we didn't fill the destination, or
	   we didn't put in a zero because it was full.  String vars
	   have an extra byte on the end for this purpose */
	dest[minsiz] = 0;
	do_undef( dest-STR_START, U_SET, minsiz+1+STR_START );
	/* check this on the final zero? */
	if( minsiz < dest_size  && !turbo_flag )
		do_undef( dest + minsiz + 1, U_CLEAR,
				dest_size - minsiz );
#ifdef TURBO
	/* stuff the length for TURBO users */
	dest[-1] = minsiz;
#endif
	EDEBUG(3, "Assign %s onto dest with size %d\n", src, minsiz );
#else
#endif ICRIPPLE
}

	/* search for a case in a case statement */
static nodep elselist;		/* list of else statements where found */

find_case( value, caselist, start )
rint value;		/* the ordinal value in the selector */
reg listp caselist;	/* list of cases */
int start;		/* is this the top level */
{
#ifndef ICRIPPLE
#ifndef SMALLIN
	register listp conlist;	/* list of constants in a case */
	register int index;		/* index in cases */
	register nodep cnode;		/* case node */
	NodeNum ctype;		/* case type */
	int index2;		/* go through constants */

	EDEBUG( 7, "Case loop, looking for %d in list %x\n", value, (int)caselist);
	if( start )
		elselist = NIL;

	for( index = 0; index < listcount(caselist); index++ ) {
		cnode = node_kid( caselist, index );
		switch( ctype = ntype(cnode) ) {
		 case N_CASE_ELSE :
			if( elselist )
				run_error( ER(106,"caseelse`Too many elses in case") );
			elselist = kid1(cnode);
			continue;
		 case N_CASE:
			conlist = FLCAST kid1(cnode);
			for( index2 = 0; index2 < listcount(conlist);index2++) {
				rint con1, con2;
				nodep conrange;

				conrange = node_kid(conlist,index2);
				if( ntype(conrange) == N_TYP_SUBRANGE ) {
					con1 = eval_const(kid1(conrange),FALSE);
					con2 = eval_const(kid2(conrange),FALSE);
					}
				 else
					con1=con2=eval_const(conrange,FALSE);
				EDEBUG(11,"Comparing with const %d %d\n", con1,con2);
				if( value >= con1 && value <= con2 ) {
					state_push( S_NOP );
					new_loc = kid2(cnode);
					return TRUE;
					}
				}
			continue;
		 case N_HIDE:
		 case N_REVEAL:
			if(find_case(value,FLCAST kid2(cnode),FALSE))
				return TRUE;
			continue;
		 case N_STUB:
		 case N_ST_COMMENT:
		 case N_NOTDONE:
			continue;
		 default:
			run_error( ER(107,"funnycase`Help!  Funny node in case list") );
		 }
		}

	/* oh well, we didn't find it */
	EDEBUG(7, "Didn't find it, looking for else\n", 0,0 );
	if( start ) {
		extern int turbo_io;
		if( elselist ) {
			state_push(S_NOP);
			new_loc = elselist;
			}
		 else
			/* if in turbo i/o mode, just fall through */
			if( !turbo_io )
				run_error(ER(108,"undefcase`Undefined case %s"),
				    ordname( gexp_type(kid1(ex_loc)), value ) );
		}
	return FALSE;
#endif SMALLIN
#else
#endif ICRIPPLE
}

/* find the type of the nth formal parameter */

nodep 
arg_indtype( routname, which )
symptr routname;			/* which routine */
int which;
{
	register listp formlist;
	register nodep paramguy;
	register int index;

	if( sym_declaration(routname) == NIL )
		return Basetype(BT_integer);
	formlist = FLCAST kid2( sym_declaration( routname ) );
	if( not_a_list(formlist) )
		return Basetype(BT_integer);
	for( index = 0; index < listcount(formlist); index++ ) {
		paramguy = node_kid( formlist, index );
		if( is_a_list( kid1(paramguy) ) )
			which -= listcount( FLCAST kid1(paramguy) );
		 else
			which--;
		if( which < 0 )
			return paramguy;
		}
	/* bug( "Ran off formal parameter list finding string type" ); */
	/* return something not a string or N_FORM_FUNCTION */
	return Basetype(BT_integer);
}

/* check for undefined memory */
#ifdef UDEBUG

copy_mask( to, from, bytes )
pointer to;
pointer from;
unsigned bytes;	/* how many bytes to move */
{
	register unsigned todiff, fromdiff;		/* byte offsets */
	pointer mapto, mapfrom;			/* location in map */
	int tooffset, fromoffset;	/* offsets in bytes */
	int i;

#ifdef Trace
	extern int most_trlocs, within_trace;
	if( most_trlocs && !within_trace )
		scan_trace( to, bytes );
#endif

	/* if dest is not on stack, do nothing */
	if( (todiff = to - (pointer)exec_stack) > max_stack_size )
		return;
	/* if source is not on stack, just set dest as defined */
	mapto = (pointer)(undef_bitmap + (todiff >> 3 ));
	if( (fromdiff = from - (pointer)exec_stack) > max_stack_size ) {
		do_undef( mapto, U_SET, bytes );
		return;
		}
	/* do it a real slow, stupid way for now */
	for( i = 0; i < bytes; i++ ) {
		register pointer dsbyte;
		register int cbit;
		int fbit;
		dsbyte = (pointer)&undef_bitmap[todiff >> 3 ];
		cbit = todiff++ & 7;
		fbit = fromdiff & 7;
		*dsbyte &= ~(1 << cbit);
		*dsbyte |= ((undef_bitmap[fromdiff++ >> 3] & (1 <<fbit)) > 0)
				<< cbit;

		}
#ifdef fasterway
	tooffset = todiff & 7;
	mapfrom = (pointer)(undef_bitmap + (fromdiff >> 3 ));
	fromoffset = fromdiff & 7;
#endif
}

CUerror()
{
	run_error(ER(109,"uninit`Attempt to use a variable that has not been assigned to!"));
}

#ifdef BUGGY
do_undef(loc, op, len)
pointer	loc;
int	op;
int	len;
{
	register pointer bitMap;
	register int	startBit;
	unsigned	delta;
	bits8	startMask;
	bits8	endMask;
	int	byteCount;
#define	ALLONES		0xff

	delta = loc - (pointer)exec_stack;
	if (delta > max_stack_size)
		return;
	bitMap = undef_bitmap + (delta >> 3);

	startBit = delta & 7;
	startMask = (ALLONES << startBit) & ~(ALLONES << (startBit + len));
	switch(op) {
	case U_CHECK:
		if ((*bitMap & startMask) != startMask)
			CUerror();
		break;
	case U_SET:
		*bitMap |= startMask;
		break;
	case U_CLEAR:
		*bitMap &= ~startMask;
		break;
	}
	if (startBit + len <= 8)
		return;

	byteCount = ((startBit + len) / 8) - 1;
	endMask = 0xff >> (8 - ((startBit + len) & 7));
	switch(op) {
	case U_CHECK:
		for ( ; byteCount > 0; byteCount--)
			if (*++bitMap != ALLONES)
				CUerror();
		if ((*++bitMap & endMask) != endMask)
			CUerror();
		break;
	case U_SET:
		for ( ; byteCount > 0; byteCount--)
			*++bitMap = ALLONES;
		*++bitMap |= endMask;
		break;
	case U_CLEAR:
		for ( ; byteCount > 0; byteCount--)
			*++bitMap = 0;
		*++bitMap &= ~endMask;
		break;
	}
}

#else BUGGY

do_undef( loc, setit, bytes )
pointer loc;		/* pointer to memory on stack */
int setit;		/* to check or set */
int bytes;		/* how many bytes to handle */
{
	register pointer maploc;
	int sbpb;
	int startbit;
	unsigned bytedif;
#ifdef XDEBUG
	if( ed_level > 11 )
		fprintf(etrace, "do_undef( loc=%lx, mode=%d, bytes=%d )\n",
				(long)loc, setit, bytes );
#endif
	/* Assumes 8 bits per byte */
	/* using unsigned we just compare for greater than.  Is this risky
	   on any machines? */
#ifdef Trace
	{
	extern int most_trlocs, within_trace;
	if( most_trlocs && setit == U_SET && !within_trace ) 
		scan_trace( loc, bytes );
	}
#endif

#ifdef LARGE
	if( FP_SEG(loc) != SSeg  ||  (bytedif = loc - (pointer)exec_stack) > max_stack_size )
#else
	if( (bytedif = loc - (pointer)exec_stack) > max_stack_size )
#endif
		return;		/* checking memory out of stack, string ? */
	maploc = (pointer)(undef_bitmap + (bytedif >> 3 ));
	startbit = bytedif & 7;

	if( bytes <= 2 && startbit < 7 ) {
		domask( maploc, (bytes | 1) << startbit, setit );
		}
	 else {
		register int togo;
		int amask;

		sbpb = startbit + bytes;

		/* figure out how many bits in first byte, how many
	   	full bytes, and how many in last byte */

		amask = sbpb > 7 ? 0xff : 0xff >> (8-sbpb);
	
		domask( maploc++, (-1 << startbit) & amask, setit );
		/* set togo in zero origin, see if more bytes needed */
		/* this becomes how many more bits to set in beyond bytes */
		if( (togo = startbit + bytes - 9) >= 0 ) {
			while( (togo -= 8) >= 0 )
				domask( maploc++, 0xff, setit );
			/* and now do last byte */
			domask( maploc, 0xff >> (-1 - togo), setit ); 
			}
			
		}
}


/* do masking check */
domask( byte, mask, setit )
unsigned char *byte;	/* pointer to bitmap */
bits8 mask;		/* mask to apply */
int setit;		/* flag for checking */
{
	switch( setit ) {
		case U_CHECK:
			if( (*byte & mask ) != mask )
				CUerror();
			break;
		case U_SET:
			*byte |= mask;
			break;
		case U_CLEAR:
			*byte &= ~mask;
			break;
		}
}
#endif UDEBUG

#endif BUGGY









/* string compare with undef check */

ustrcmp( str1, str2 )
char *str1, *str2;
{
#ifdef TURBO
	str1++;
	str2++;
#endif
	do_undef( str1, U_CHECK, strlen(str1) );
	do_undef( str2, U_CHECK, strlen(str2) );
	return strcmp( str1, str2 );
}

#ifndef QNX

zero( loc, bytes )
pointer loc;
int bytes;
{
	fillchar( loc, 0, bytes );
}

fillchar( loc, fillbyte, bytes )
pointer loc;
register char fillbyte;
unsigned int bytes;
{
#ifdef POINT32
	seg_fillchar( loc, fillbyte, bytes );
#else
	seg_fillchar( loc, get_DS(), fillbyte, bytes );
#endif
}
# if (defined(unix) || defined(ATARI520ST)) && defined(POINT32)
seg_fillchar( loc, fillbyte, bytes )
pointer loc;
register char fillbyte;
int bytes;
{
	while( bytes-- )
		*loc++ = fillbyte;
}
# endif

#else QNX
fillchar( loc, fillbyte, bytes )
pointer loc;
register char fillbyte;
unsigned int bytes;
{
	while( bytes-- )
		*loc++ = fillbyte;
}

#endif


#ifndef FixedTheAssemblerVersionYet
do_pushing( address, bytes, type )
pointer address;		/* location to possibly be dereferenced */
int bytes;			/* size for thing being pushed */
nodep type;			/* type of thing pushed */
{
	register pointer tempptr = address;
	register NodeNum ttype;

	if( node_flag(ex_loc) & NF_RVALUE ) {
		if (bytes == sizeof(int))
			int_spush( *(rint *)tempptr );
		 else if( bytes == 1 ) 
			ib_spush( *(bits8 *)tempptr );
		 else if( type == Basetype( BT_real ) )
			f_spush( *(rfloat *)tempptr );
		 else if ((ttype = ntype(type)) == N_TYP_FILE ||
						type == Basetype(SP_file) ) {
			p_spush(tempptr);
			return;
			}
		 else if(ttype  == N_TYP_SET) {
			set_spush( tempptr );
			}
		 else if( ttype == N_TYP_POINTER || type == Basetype(BT_pointer)) {
			p_spush( *(pointer *)tempptr );
			}
		else
			bug(ER(264,"funny type %d in do_pushing"), ntype(type));
		EDEBUG(8, "By dereference it is %d\n", int_stop, 0);
		check_undef( tempptr, bytes );
		}

	 else
		p_spush( tempptr );
}

#endif !FixedTheAssemblerVersion

#ifdef OLD

do_pushing( address, bytes, type )
pointer address;		/* location to possibly be dereferenced */
int bytes;			/* size for thing being pushed */
nodep type;			/* type of thing pushed */
{
	register pointer tempptr = address;

	/* if size unknown, calculate it here */
	if( bytes == 0 )
		bytes = calc_size( type, TRUE );

	if( node_flag(ex_loc) & NF_RVALUE ) {
		if( bytes == 1 ) 
			ib_spush( *(bits8 *)tempptr );
		 else if(ntype(type) == N_TYP_SET) {
			set_spush( tempptr );
			}
		 else if( ntype(type) == N_TYP_FILE  )
			p_spush( tempptr );
			return;
			}
		 else if(ntype(type) == N_TYP_POINTER ) {
			p_spush( *(pointer *)tempptr;
		 else if( type == Basetype( BT_real ) )
			f_spush( *(rfloat *)tempptr );
		 else
			int_spush( *(rint *)tempptr );
		EDEBUG(8, "By dereference it is %d\n", int_stop, 0);
		check_undef( tempptr, bytes );
		}
	 else
		p_spush( tempptr );
}

#endif OLD

/* routine to do runtime output from program */
prg_output( type, str )
int type;		/* ignored in non segmented code */
char *str;		/* string to print */
{
	extern FILE   *LogFile;
	extern WINDOW *pg_out;

#ifndef SAI
	if (LogFile)
		fputs(str, LogFile);

	checkScreen();

	waddstr( pg_out, str );
#else
	sai_print( str );
#endif
}

flush_scr(filedesc)
fileptr filedesc;
{
	extern WINDOW *pg_out;
	extern struct pas_file fil_output;

	if( filedesc == 0 || filedesc->f_flags & FIL_SCREEN )
#ifdef SAI
		if( sai_out == SAI_SCREEN ) wrefresh( pg_out );
#else
		wrefresh( pg_out );
#endif SAI
}

/* in a special executed workspace */

/* create a suspended process state */

/* Cannot suspend state in stand alone interp */
#ifndef SAI
do_suspend( new, caller, type )
nodep new;	/* code about to be executed */
nodep caller;	/* location that wanted to come here */
int type;	/* true if code is valid as well as data */
{

	extern int call_depth, call_current;
	extern int must_stop;
	extern stack_pointer *tb_frame;
	extern WINDOW *pg_out;

	suspendGraphics();

	curOn(pg_out);

	if( in_execws || !st_display ) {
		/* total reset */
		EDEBUG( 3, "Suspend while executing workspace, in_execws=%d\n", in_execws, 0 );
		prep_stacks(TRUE);
		in_execws = FALSE;
		return;
		}
	bump_stack( sizeof( struct susp_block ) );
	sp.st_susp->susp_type = type;
	sp.st_susp->susp_count = ++count_suspend;
	sp.st_susp->susp_prev = curr_suspend;
	sp.st_susp->susp_resume = caller;
	sp.st_susp->susp_todo = new;
	call_current = sp.st_susp->susp_cdepth = call_depth;
	if( new )
		cursor = hidebound(new);
	tb_frame = sp.st_susp->susp_display = st_display;
	sp.st_susp->susp_estack = ex_stack;
	curr_suspend = sp.st_susp;
	redraw_status = TRUE;
	must_stop = FALSE;	/* clear break must-stop status */
	EDEBUG( 2, "Suspending: Suspend block %x count now %d", (int)curr_suspend,
				count_suspend );
	EDEBUG( 2, " caller=%x, new=%x\n", (int)caller,(int)new );
}

/* check legality of resume, if it was not legal, restore to old state */

res_check(restore)
struct susp_block *restore;
{
	extern int code_damaged;

	if( !restore )
		restore = curr_suspend;
	if( !curr_suspend ) {
		curr_suspend = restore;
		error( ER(110,"notsusp`Your program is not currently in a suspended state") );
		}
	
	if( !code_damaged && curr_suspend->susp_type )
		sp.st_susp = curr_suspend;
	 else {
		curr_suspend = restore;
		error( ER(111,"cantresume`Sorry, unable to continue running the program") );
		}
}

resume( stepping )
int stepping;		/* what type of single step */
{
	nodep res;		/* restart old location */
	nodep tdo;		/* restart to do location */
	struct susp_block *old_csusp;	/* old curr suspend */
	extern int call_depth;


	res_check(old_csusp = curr_suspend);	/* check legality of resume */

	/* check for "immediate mode" suspension, and skip over them */
	while( curr_suspend->susp_type < 0 ) {
		curr_suspend = curr_suspend->susp_prev;
		--count_suspend;
		res_check(old_csusp);
		}
	/* restore stacks */
	set_stacks();
	/* get restart locations */
	res = curr_suspend->susp_resume;
	tdo = curr_suspend->susp_todo;
	call_depth = curr_suspend->susp_cdepth;
	--count_suspend;	/* one less suspension */
	/* pop suspension stack */
	curr_suspend = curr_suspend->susp_prev;

	/* restore stack pointer to where it belongs */
	unload_stack( sizeof( struct susp_block ) );

	if( tdo == NIL ) {
		espush( res, e_loc );
		}
	redraw_status = TRUE;

	resumeGraphics(stepping);

	interp( tdo, res, stepping );
}

#endif SAI

set_stacks()
{
	st_display = curr_suspend->susp_display;
	ex_stack = curr_suspend->susp_estack;
	sp.st_susp = curr_suspend;
}

#ifndef SAI
do_superstep( )
{
	nodep tdo;
	res_check(curr_suspend);
	/* right now there will be a problem if a super step is interrupted
	   and resumed, 
	   in that they will have to hit the step key again to see what
	   the new statement is.  This can (I think) be fixed by having
	   the interpreter detect the special super-step suspension state
	   when it terminates normally, and doing a resume on that state if
	   so */


	tdo = curr_suspend->susp_todo;
	curr_suspend->susp_todo = NIL;	/* signal special resume */
	/* if do is NIL, we're just clearing an interrupted super step */
	if( tdo )  {
		/* run the block itself */
		resumeGraphics(STEP_SUPER | DBF_ON);
		interp( tdo, NIL, STEP_SUPER | DBF_ON );
		}
	/* now do the resume that gets to the next statement */
	/* we actually do a single step to stop right away */
	resume( STEP_SINGLE | DBF_ON );

}
#endif SAI
