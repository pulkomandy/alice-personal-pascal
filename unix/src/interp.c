#define INTERPRETER 1
/*
 *DESC: The C version of the Alice interpreter
 * My guess is that this is not very efficient and the whole thing
 * has to be coded in assembler.  At least the inner core
 *  Mostly a big switch statement, soon to become a jump table
 *  if necessary at a theatre near you.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"


#include "typecodes.h"
#include "interp.h"

/* Set the flag NOBFUNC so that the symbol table does not overflow */
/*#define NOBFUNC */

#include "bfuncs.h"
#include "flags.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "dbflags.h"
#include "break.h"

#ifdef QNX
char	*OvflowCause;		/* overflow cause */
#endif

#ifdef STUPID_COMPILER
#define ISTAT 
#else
#define ISTAT static
#endif

#ifndef OLMSG
char rttype_err[] = "stop`Stop! There is an error here";
#endif

extern struct pas_file fil_input, fil_output;
extern int call_depth;

/*register*/ stack_pointer sp;
stack_pointer *st_display;	/* all the fps and aps we need */
estackloc *loc_stack = 0;
bits8 *undef_bitmap = 0;
estackloc *ex_stack;
/* here are the externals we will be using */

long step_delay = 0;		/* delay in cursor follow */
int must_stop = 0;		/* stop at once, I say */

ISTAT listp arg_list;		/* general aguments */
ISTAT int built_index;			/* index into arguments */
StateNum cur_state;			/* state we are in now */
nodep new_loc;				/* new location */
int within_hide;			/* are we within hidden code */
ISTAT nf_type el_flags;			/* flags of location type */
ISTAT rint *intstar;			/* quick integer pointer */
ISTAT rint temp2;			/* two quick temporaries */
					/* temp1 a register */
ISTAT unsigned untemp;			/* unsigned temporary */
ISTAT rfloat fltemp;			/* floating temp */
ISTAT rfloat *flstar;			/* floating temp ptr */
ISTAT bits8 btemp1;			/* a quick byte temporary */
ISTAT int thescope;			/* scope of new symbol */
ISTAT int nkid;				/* kid in a list */
ISTAT bits16 nod_flags;			/* flags of our node friend */
ISTAT pointer tempptr;			/* temporary pointer */
ISTAT pointer resptr;			/* pointer to a result on stack */
ISTAT nodep type_ptr;			/* for type checking */
ISTAT nodep tempnode;			/* quick node to have */
ISTAT stack_pointer *dummy;		/* for ge_routine */
ISTAT Boolean notfirst;			/* not first time in for loop */
extern symptr get_routine();
int step_flag;			/* are we stepping */
nodep ex_loc;				/* storage for program counter */

static Boolean	WasBreak	= FALSE;

interp(loc,restart, setstep)
nodep loc;	/* starting position to execute from, setup done */
nodep restart;	/* where we came from if a restart */
int setstep;	/* single stepping flag */
{
	register nodep eloc;		/* the "program counter" */
	register symptr thesym;			/* symbol table entry */
	register rint temp1;

	/* most variables will be externals for speed */

	step_flag = setstep;
	EDEBUG(4, "Step flag set to %x\n", step_flag,0);
	new_loc = loc;
	/* otherwise the old eloc had better be saved */
	eloc = restart;
#ifdef UDEBUG
#ifndef SAI
	clear_break();
#endif
#endif
	
/* Grab an extra level of indent */

for(;;) { /* THE FOREVER LOOP */

	/* if there is a new location, go to the first state for it,
	 * otherwise pop the stack and go for the state you got there */


#ifndef ASM_ICORE
	if( new_loc ) {
		espush( eloc, e_loc );	/* save current, we are going down */
					/* state should already be pushed */
		/* loop until there is not a new loc to do */
		while( new_loc ){
			/* assume old lock and return state have been pushed */
			eloc = new_loc;
			if( not_a_list( eloc ) && node_flag(eloc) & NF_ERROR ) {
				ex_loc = eloc;
				run_error( ER(59,rttype_err));
			}
			cur_state = ntype(eloc);
			if( (el_flags = ntype_info(cur_state)) &
						(F_E1KID|F_E2KIDS) ){
				/* push primary state of new location on stack*/
				state_push( cur_state );
				espush( eloc, e_loc );
				if( el_flags & F_E2KIDS ) {
					/* push special grab kid2 state */
					state_push( S_GRABK2 );
					espush( eloc, e_loc );
					}
				/* now get set to execute kid 1 */
				new_loc = kid1( eloc );
				}
		 	else
				new_loc = NIL;
			}
		}
	 else {
		/* in this case we are asked to go up the stack */
		if( eloc = FNCAST espop(e_loc) )
			cur_state = (StateNum) espop(e_state);
		 else {
#ifndef SAI
			EDEBUG(1, "Execution terminated\n", 0, 0 );
			terminated();
#endif SAI
			return;
			}
		}
	/* save the program counter in external */

	ex_loc = eloc;

#else ASM_ICORE
	/* risky near function definition */

	extern int near istate();

	if( istate() ) {
#ifndef SAI
		EDEBUG(1, "Execution terminated\n", 0, 0 );
		terminated();
#endif
		return;
		}
#endif

	/* and now the massive automata switch statement */

	/* new_loc is set to NIL in advance, so states that wish to pop
	 * upwards need do nothing.  States that want to move, have to set
	 * new_loc of course, and push their node and next state on the
	 * execution stack.  They can push S_NOP for a state that just pops
	 * up at the end of their reign
	 */


	 /* We use a special macro in the case labels to generate constants.
	  * during cc, a cpp macro will produce the appropriate constants.
	  * the Main macro gives the node number.  Other constants are produced
	  * by means of an m4 pass on the source that builds the .h file with
	  * constant definitions
	  */

#ifdef XDEBUG
	dbout();		/* put out trace */
#endif

	switch( cur_state ) {

#ifndef STUPID_COMPILER

#include "ilowstates.c"
#include "mlowstat.c"

#else
	 default:
		/* execute low frequency state */
		if( cur_state < FIRST_EXPRESSION )
			statstate();
		 else
			expstate( );
		break;
#endif

	 case Main(N_CON_INT):
		int_spush( int_kid(1,eloc) );
		break;
#ifndef ICRIPPLE
	
	 case Main(N_CON_REAL):
		f_spush( fl_kid(eloc) );
		break;
#endif ICRIPPLE
	
	 case Main(N_CON_CHAR):
		/* char is contained in the 0 kid */
		int_spush( (unsigned)int_kid(0,eloc) );
		break;

	 case Main(N_CON_STRING):
		/* Just push address.  Type? */
		/* add one to avoid pusing starting quote */
		p_spush( str_kid(0,eloc)+1 );
		break;

	 case S_NOP:
	 case Main(N_ST_COMMENT):
		/* boring */
		break;

	 case Main(N_ST_WHILE):
		new_loc = kid1(eloc);
		state_push( S_WHILE1 );
		break;
	 case S_WHILE1:
		if( int_spop() ) {
			state_push( Main(N_ST_WHILE) ); /* go right back here */
			new_loc = kid2(eloc);
			}
		break;

#ifndef ICRIPPLE
	 case S_RPT2:
		if( int_spop() )
			break;		/* if condition true, quit */
	 case Main(N_ST_REPEAT):
		new_loc = kid1(eloc);
		state_push( S_RPT1 );
		break;
	 case S_RPT1:
		new_loc = kid2(eloc); /* the condition */
		state_push( S_RPT2 );
		break;


	 case Main(N_VAR_FIELD):
		thesym = kid_id(kid2(eloc));
		tempptr = p_spop() + sym_offset(thesym);
		do_pushing( tempptr, sym_size(thesym), sym_type(thesym) );
		break;

#endif ICRIPPLE
	 case Main(N_VAR_ARRAY):
		{
		static char runerrstr[] = "bounds`Array index %s out of bounds";

		/* both the array address and index are ready */
		/* simple setup for single indexing */
		type_ptr = gexp_type( kid1( eloc ) );
#ifdef TURBO
		if( ntype(type_ptr) == N_TYP_STRING ) {
			temp1 = int_spop();
# ifdef UDEBUG
			if( temp1 < 0 || temp1 > int_kid( 1, type_ptr ) )
				run_error( ER(60,runerrstr), ordname(Basetype(
					BT_integer), temp1 ) );
# endif
			do_pushing( str_spop() + temp1, 1, Basetype(BT_char) );
			break;
			}
#endif TURBO
		temp2 = int_kid( 2, type_ptr );
#ifdef FULL
		if( node_flag(eloc) & NF_FUNNYVAR ) {
			if( sym_dtype(kid_id(kid1(eloc))) == T_PORTVAR ) {
				if( node_flag(eloc) & NF_RVALUE ) {
					temp1 = int_spop();
					int_spush( inp(temp1) | (temp2 > 1 ?
						inp(temp1+1) : 0 ) );
					}
				/* else just leave integer address on stack */
				}
# ifdef LARGEPTR
			 else /* must be mem */{
				tempptr = (pointer)int_spop();
				if( ntype(kid2(eloc)) == N_OEXP_2 )
					FP_SEG(tempptr) = int_spop();
				do_pushing( tempptr,temp2, gexp_type(eloc) );
				}
# endif
			}
		 else
#endif FULL
			{
			temp1 = int_spop() - int_kid(3, type_ptr );
#ifdef UDEBUG
			if( temp1 < 0 || temp1 >= int_kid(4, type_ptr ) ) {
				run_error( ER(60,runerrstr),
					ordname(kid1(type_ptr), temp1 +
					int_kid(3, type_ptr )) );
				}
#endif
			/* calc the address */
			/* should we optimze, not multiply if 1 or 2 */
			do_pushing( p_spop()+(temp2 == 2 ? (temp1<<1) :
				(temp2 == 1 ? temp1 : temp1 * temp2 ) ),
				(int)temp2, gexp_type(eloc));
			}
		}
		break;


#ifndef ICRIPPLE
	 case Main(N_VAR_POINTER):
		tempptr = p_spop();

		if( ntype(gexp_type(kid1(eloc)) ) == N_TYP_FILE ) {
			fileptr tfp;
			tfp = (fileptr) tempptr;
			/* if lazy i/o file, cause the input if necessary */
			if( node_flag(eloc) & NF_RVALUE &&
						tfp->f_flags & FIL_NREAD )
				_a_get( tfp, TRUE );
			tempptr = &(tfp->f_buf);
			}
		if( tempptr == NULL )
			run_error( ER(61,"nil`Error - This pointer is NIL") );
		type_ptr = gexp_type(eloc);
		do_pushing( tempptr, calc_size(type_ptr, TRUE), type_ptr);
		break;

	
#endif ICRIPPLE
	/* all the following expression operators have had both their
	   children executed and pushed on the stack by now */

	 case Main(N_EXP_PLUS):
		type_ptr = kid3(eloc);
		if (type_ptr == Basetype(BT_integer)) {
#ifdef IOP_FUNCS
# ifdef LARGE
#  define NEAR 
# endif
			extern int near intadd();
			intadd();
#else
			temp1 = int_spop();
#ifdef QNX
			OvflowCause = "addition";
#endif
			int_stop += temp1;
#ifdef QNX
			asm("jo <overflow>");
#endif
#endif
			}
		else if (type_ptr == Basetype(BT_real)) {
			fltemp = f_spop();
			f_stop += fltemp;
			}
#ifdef TURBO
		else if( type_ptr == Basetype(SP_string)) {
			string_add();
			}
#endif
		else if (is_set_type(type_ptr)) {
			tempptr = (pointer)set_spop();
			set_union((pointer)set_stop, tempptr);
			}
		break;

	 case Main(N_EXP_MINUS):
		type_ptr = kid3(eloc);
		if (type_ptr == Basetype(BT_integer)) {
#ifdef IOP_FUNCS
			extern int near intsub();
			intsub();
#else
			temp1 = int_spop();
#ifdef QNX
			OvflowCause = "subtraction";
#endif
			int_stop -= temp1;
#ifdef QNX
			asm("jo <overflow>");
#endif
#endif
		}
		else if (type_ptr == Basetype(BT_real)) {
			fltemp = f_spop();
			f_stop -= fltemp;
		}
		else if (is_set_type(type_ptr)) {
			tempptr = (pointer)set_spop();
			set_subtract((pointer)set_stop, tempptr);
		}
		break;

	 case Main(N_EXP_OR):
		temp1 = int_spop();
		int_stop |= temp1;
		break;

#ifdef TURBO
	 case Main(N_EXP_XOR):
		temp1 = int_spop();
		int_stop ^= temp1;
		break;
#endif
	
	 case Main(N_EXP_TIMES):
		type_ptr = kid3(eloc);
		if (type_ptr == Basetype(BT_integer)) {
#ifdef IOP_FUNCS
			extern int near intmult();
			intmult();
#else
			temp1 = int_spop();
#ifdef QNX
			OvflowCause = "multiplication";
#endif
			int_stop *= temp1;
#ifdef QNX
			asm("jo <overflow>");
#endif
#endif
		}
		else if (type_ptr == Basetype(BT_real)) {
			fltemp = f_spop();
			f_stop *= fltemp;
		}
		else if (is_set_type(type_ptr)) {
			tempptr = (pointer)set_spop();
			set_intersect((pointer)set_stop, tempptr);
		}
		break;


	 case Main(N_VF_WITH):

	 case Main(N_ID):
		thesym = kid_id(eloc);
		thescope = sym_scope(thesym);
		switch( sym_dtype(thesym) ) {
			case T_VAR:
				tempptr = (pointer)
					(st_display[thescope].st_stackloc -
					sym_offset( thesym ));
				EDEBUG(8, "Variable from scope %d gives address %x\n", thescope, (int)tempptr );
				break;
			case T_ABSVAR:	/* absolute variable */
			case T_INIT:
				tempptr = (pointer)sym_value(thesym);
				break;
			
#ifndef ICRIPPLE
			case T_FIELD: /* must be within a VF_WITH */
				EDEBUG(8,"T_FIELD: thesym = %x, sym_offset = %x\n",
					(int)thesym, sym_offset(thesym));
				EDEBUG(8,"Kid2: %x  Kid3: %x\n",
				(int)kid2(eloc),(int)kid3(kid2(eloc)) );
				tempptr = sym_offset(thesym) +
						(pointer) kid3(kid2(eloc));
				EDEBUG(7,"Field in with %x gives address %x\n",
					(int)kid2(eloc), (int)tempptr );
				break;

			case T_FORM_PROCEDURE:

			case T_PROCEDURE:
				{
				symptr psym;
				pointer pdisplay;

				psym = get_routine( thesym, &pdisplay );
				EDEBUG(5,"Pushing procedure on stack %x, %x\n", 
						(int)st_display, (int)thesym );
				p_spush( pdisplay );
				p_spush( (pointer)psym );
				}
				goto already_pushed;

			case T_FORM_VALUE:
			/* special adjust necessary for stack chars that
			   were pushed, in case system is high-low order
			   inside words instead of sane low-high */
				tempptr = (pointer)
					(st_display[-thescope].st_stackloc -
					sym_offset( thesym ));
				EDEBUG(8,"Value parameter from scope %d gives address %x\n", thescope, (int)tempptr );
				break;
			case T_FORM_REF:
				tempptr =  *((pointer *)
					(st_display[-thescope].st_stackloc -
					sym_offset(thesym)));
				EDEBUG(8,"Ref parameter from scope %d gives address %x\n", thescope, (int)tempptr );
				break;
#endif ICRIPPLE
			case T_FUNCTION:
				/* return value is zero offset argument */
				tempptr = WB_ADJUST(sym_size(thesym)) +(pointer)
					(st_display[-thescope].st_stackloc);
				EDEBUG(8,"Return value scope %d gives address %x\n", thescope, (int)tempptr );

				break;
			case T_ENUM_ELEMENT:
			case T_BENUM:
				int_spush( sym_ivalue(thesym) );
			case T_MEMVAR:
			case T_PORTVAR:
				goto already_pushed;
			case T_CONST:
			case T_BCONST:
				type_ptr = sym_type(thesym);
				if( type_ptr == Basetype(BT_integer)||
						type_ptr == Basetype(BT_char)
						|| sym_size(thesym) <= 2 )
					int_spush( sym_ivalue(thesym) );
				else if( type_ptr == Basetype(BT_real) ) {
#ifdef QNX 
					/* a bug in ES float assignments causes
					   this kludge */
					f_spush( 0.0 );
					esdsblk( &(f_stop), sym_value(thesym),
						sizeof(rfloat), FALSE );
#else
# ifdef ES_TREE
					f_spush(STAR((rfloat far *)tfarptr(sym_value(thesym))) );
# else
					f_spush( *(rfloat *)sym_value(thesym));
# endif
#endif
					if( sym_mflags(thesym) & SF_NEGREAL )
						f_stop = - f_stop;
					}
				 else {
					p_spush( (pointer)sym_value(thesym) );
					}
				goto already_pushed;
			case T_FILENAME:
				{
				extern fileptr predef_files[];
				p_spush((pointer)predef_files[sym_value(thesym)] );
				goto already_pushed;
				}
			case T_UNDEF:
				run_error( ER(59,rttype_err));

			default:
				error( ER(62,"Funny symbol type %d\n"), sym_dtype(thesym) );
			 }
		do_pushing( tempptr, sym_size(thesym), sym_type(thesym) );
	already_pushed:
		 break;

	 case Main(N_LIST):
		/* loop with counter on stack */
		if( listcount( FLCAST eloc ) ) {
			/* push current stack pointer for use by goto */
			espush( sp.st_generic, e_pointer );
			espush(0, e_int);
			state_push(S_LIST1);

			new_loc = node_kid( eloc, 0 );
			}
		break;
	 case S_LIST1:
		/* just got back.  increment, check, go */
		nkid = ++estop(e_int);
		if( nkid < listcount(FLCAST eloc) ) {
			state_push(S_LIST1);
			new_loc = node_kid( eloc, nkid );
			}
		 else {	
			espop(e_int);	/* eliminate count */
			espop(e_pointer);	/* eliminate stack */
			}
		break;

	 case S_GRABK2:		/* grab the second child */
		state_push( S_NOP );
		new_loc = kid2( eloc );
		break;
	 }

#ifndef SAI

#ifdef UDEBUG
	/* VERY nonportable code */

#ifndef ASM_ICORE
	if( got_break() ) {
		breakkey();
		clear_break();
		}
#endif
	if( step_flag && new_loc && not_a_list(new_loc) && ((ntype_info(ntype(
			new_loc)) & F_LINE && !within_hide) || must_stop) ) {
		if( (temp1 = node_flag(new_loc) & NF_BREAKPOINT) ||
				step_flag & STEP_SINGLE ){
			if( temp1 )
				message( ER(213,"bp`Breakpoint!") );
			else if (WasBreak) {
				message( ER(212,"break`Break!") );
				WasBreak = FALSE;
				}
			do_suspend( new_loc, eloc, TRUE );
			return;
			}
		 else if( step_flag & DBF_CURSOR ){
			long delay_ctr;
#ifdef PROC_INTERP
			dis_cursor( new_loc );
#else
			/*
			 * Cursor following and graphics do not mix well.
			 * I am starting to feel the user needs the ability
			 * to have a little graphics debug window...
			 */
			suspendGraphics();

			/* the pause */
			for( delay_ctr = step_delay; delay_ctr; delay_ctr-- )
				;
			find_phys_cursor( curr_window, new_loc, FALSE );
			wmove( curr_window->w_desc, srch_row, srch_real );
			wrefresh(curr_window->w_desc);

			resumeGraphics(step_flag);
#endif
			}
		}
#else UDEBUG

#ifndef ASM_ICORE
	if( got_break() ) {
		breakkey();
		clear_break();
		return;
		}
#endif
#endif UDEBUG

#endif SAI

	if( !new_loc && not_a_list(eloc) && cur_state >= 0 ) {
#ifdef Trace
		extern int trace_queue_size, within_trace;
#endif
		if( (node_flag(eloc) & NF_VALMASK) == NF_RCONVERT ) {
			/* pop the integer that must be converted */
			temp1 = int_spop();
			EDEBUG(6, "Changing integer %d into floating point\n", temp1,0);
			f_spush( (rfloat) temp1 );
			}
		 else if( node_flag(eloc) & NF_CHRSTR ) {
			temp1 = int_spop();
			EDEBUG(6, "Changing char %d into str\n", temp1,0);
			bump_stack( word_allign(3) );
			sp.st_generic[0] = 1;
			sp.st_generic[1] = temp1;
			sp.st_generic[2] = 0;
			do_undef( sp.st_generic, U_SET, 3 );
			p_spush( (pointer) 0 );
			}
#ifdef Trace
		if( trace_queue_size && !within_trace ) {
			extern int trace_excepts[MAX_TRACE];
			extern struct tr_range tr_locs[MAX_TRACE];
			extern int within_trace;
			int i;

			i = trace_excepts[ --trace_queue_size ];
			p_spush( (pointer)st_display );
			int_spush( within_hide );
			new_loc = tr_locs[i].tr_code_list;
			st_display = tr_locs[i].tr_display;
			within_hide = tr_locs[i].tr_hide;
			within_trace = TRUE;
			state_push( S_UNTRACE );

			}
#endif
		}

	} /* end of the forever while loop */
}

#ifdef QNX
/*
 * The <overflow> label is jumped to directly from inside the interpreter.
 * This will probably break continue...so this should be changed somehow.
 */
ovflow()
{
	asm("<overflow>:");
	run_error(ER(63,"intovflow`integer %s overflow"), OvflowCause);
}
#endif

#ifdef IOP_FUNCS

static char *overrs[] = {
"addition",
"subtraction",
"multiplication"
};

overror( code )
int code;
{
	run_error( ER(63,"intovflow`integer %s overflow"), overrs[code] );
}

#endif


breakkey()
{
	/* now flush keyboard input pending */
	extern int must_stop;

#ifdef Trace
	extern int break_disable;
	if( break_disable ) {
		scan_trace( BRK_DISABLE, BRK_DISABLE+1 );
		return;
		}
#endif

	WasBreak = TRUE;
	step_flag |= STEP_SINGLE;
	must_stop = TRUE;
}

#ifdef XDEBUG
dbout()
{
	char *nname;
	/* this array must be allowed */
	if( ed_level >= 5 ) {
#ifdef SMALL_TPL
		nname = "No Names";
#else
		nname = NodeName(ntype(ex_loc));
#endif

#ifdef LARGE
		fprintf(etrace, "@ %lx, St:%d  sp=%lx:%lx,%lx,%lx,%lx,%lx %s\n", 
#else
		fprintf(etrace,"@ %x, St:%d	sp=%x:%4x,%4x,%4x,%4x,%4x %s\n",
#endif
			ex_loc, cur_state, sp.st_int,
			sp.st_pint[0],sp.st_pint[1], sp.st_pint[2], sp.st_pint[3],
			sp.st_pint[4],
			nname ? nname :"Blank" );
		}
}
#endif

