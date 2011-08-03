
/*
 * Runtime routines that are part of the interpreter
 *DESC: Interpreter support of a general nature
 */
#define INTERPRETER 1

#include <curses.h>
#include "alice.h"
#include "alctype.h"
#include "typecodes.h"
#include "keys.h"
#include "break.h"


#include "flags.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "interp.h"

extern WINDOW *pg_out;

#ifdef SAI
extern int sai_out;
#endif SAI

int call_depth;		/* how many calls deep */
int call_current;	/* location in traceback stack */

/*
 * Routine to execute a statement immediately
 *
 * The deal here is we must create a stack frame with an appropriate
 * display for the parent routines of the immediate block.  This could
 * be active routines if suspended, or fake routines created if not suspended
 * in the right place or at all.
 */
extern stack_pointer *find_active();
int imm_mode = FALSE;

#ifndef SAI
ex_immediate( loc, stepmode )
nodep loc;
int stepmode;
{
	register nodep ascend;		/* to find imm block */
	int sizes[MAX_SCOPE];		/* sizes of intermediate scopes */
	nodep frames[MAX_SCOPE];	/* array of frames to create */
	int inter_count = 0;		/* intermediaries */
	char afscope = 0;		/* scope of highest active frame */
	int mscope;			/* scope for made frame */
	int madeone = FALSE;		/* made a frame */
	nodep last_inact = NIL;		/* last inactive scope */
	stack_pointer *aframe;		/* frame for new routine */
	extern int had_type_error;

	if( ntype(loc) < FIRST_RUN || ntype(loc) > LAST_RUN )
		error( ER(88,"cantexec`Can't be executed in immediate mode") );

	c_typecheck( loc, 0, TC_DESCEND | TC_NOSTUBS|TC_MOVECURSOR|TC_ONERROR );
	/* give up we got an error */
	if( had_type_error )
		return;

	for( ascend = loc; ascend != NIL; ascend = tparent(ascend) )
		if( ntype_info(ntype(ascend)) & F_SYMBOL ) {
			if( aframe = find_active( ascend ) ) {
				/* got an active frame, prepare others */
				afscope = *(pointer)aframe;
				EDEBUG( 3, "Got active frame %x scope %d\n",
						(int)aframe, afscope );
				break;
				}
			 else {
				last_inact = ascend;
				frames[inter_count] = ascend;
				sizes[inter_count++] = act_size( ascend );
				}
			}

	mscope = afscope + 1;
	/* should this be c_comp_decls?  I doubt it */
	if( last_inact )
		comp_decls(mscope, last_inact,FALSE, TC_FULL|TC_DESCEND|TC_MOVECURSOR,FALSE);
	if( curr_suspend )
		set_stacks();
	 else
		prep_stacks(TRUE);	 /* newClear no longer done! */
	call_depth = 0;
	redraw_status = TRUE;
	while( inter_count-- ) {
		EDEBUG( 4, "Making fake frame scope %d size %d\n", mscope,
					sizes[inter_count] );
		espush( (pointer)sp.st_stackloc, e_pointer );
		make_frame(mscope++,sizes[inter_count],NIL,frames[inter_count],
				st_display, FALSE );
		madeone = TRUE;		/* we created a special frame */
		}
	/* if we made a special frame, create an immediate mode suspend state */
	if( madeone )
		do_suspend( NIL, NIL, -1 );
	/* so now it's all ready to execute */
	imm_mode = TRUE;
	resumeGraphics(stepmode);
	interp( loc, NIL, stepmode );
	suspendGraphics();
	imm_mode = FALSE;
}
#endif /* not SAI */

stack_pointer *
find_active( rout )
nodep  rout;		/* block of routine */
{
	register stack_pointer *up_display;

	EDEBUG( 7, "Look for stack frame for %x\n", (int)rout, 0 );

	/* if not suspended, then no activiation present */
	if( !curr_suspend )
		return (stack_pointer *)0;

	for( up_display = curr_suspend->susp_display; up_display;
					up_display = up_frame(up_display) ) {
		nodep block;

		block = get_fpointers( up_display )[1];
		EDEBUG( 8, "Checking activation at %x for block %x\n",
			(int)up_display, (int)block );
		if( block && block == rout ) {
				return up_display;	/* got activation */
			}
		}
	/* never found one */
	return (stack_pointer *)0;
}

	/* eliminate any chance of resuming the code */
int code_damaged = FALSE;
clear_resume()
{
	extern int count_suspend;
	code_damaged = TRUE;
#ifndef SAI
	if( count_suspend )
		redraw_status = TRUE;
#endif SAI
}

	/* go from one stack frame up to the parent one */
stack_pointer *
up_frame( oldframe )
stack_pointer *oldframe;		/* former frame */
{
	register char ascope;

	ascope = *(pointer)oldframe;

	return (stack_pointer *)oldframe[ascope+1].st_pointer;
}


nodep *
get_fpointers( aframe )
stack_pointer *aframe;
{
	register char ascope;
	ascope = *(pointer)aframe;
	return (nodep *) &( aframe[ascope+2].st_pointer );
}

act_size( xblock )
nodep xblock;
{
	register nodep block = xblock;
	register NodeNum btype = ntype(block);

	return (int)int_kid( kid_count(btype)+1, block );
}

/* get the declaration for the routine we are calling */

symptr
get_routine( cnode, cdisp )
nodep cnode;			/* call node */
stack_pointer **cdisp;
{
	register symptr thesym;
	register pointer tempptr;
	type_code sdt;
	int thescope;

	if( ntype(cnode) == N_DECL_ID )
		thesym = (symptr)cnode;
	 else
		thesym = kid_id(kid1(cnode));

	sdt = sym_dtype(thesym);
	if( sdt == T_FORM_FUNCTION ||
			sdt == T_FORM_PROCEDURE ) {
		thescope = sym_scope(thesym);
		tempptr = (pointer) (st_display[-thescope].st_stackloc
				- sym_offset( thesym ));
		/* we now have the address of the symbol & display */
		EDEBUG(10, "Address of pushed sym and disp is %x\n", (int)tempptr,0);
		*cdisp = ((stack_pointer **)tempptr) [1];
		thesym = ((symptr *)tempptr)[0];
		EDEBUG( 4, "Getting Passed procedure %x with display %x\n",
			(int)thesym, (int)*cdisp );
		EDEBUG( 4, "dtype of symbol %d, value %d\n", sym_dtype(thesym),
				(int)sym_value(thesym) );
		return thesym;
		}
	*cdisp = st_display;
	return thesym;
}
	/* start of regular procedure call */
do_1call( xeloc )
nodep xeloc;
{
#ifndef ICRIPPLE
	register nodep eloc = xeloc;
	register char *tempptr;
	int nkid;
	nodep gotkid;
	nodep gktype;		/* type for got kid */
	symptr thesym;
	listp arg_list;
	stack_pointer *calldisp;
	unsigned int formsize;
	unsigned int gkflags;	/* flags of pushed node */

	nkid = ++estop(e_int);
	thesym = get_routine(eloc, &calldisp );
		/* handle large things by value */
	arg_list = FLCAST kid2(eloc);
	gotkid = node_kid( arg_list, nkid-1 );
	ex_loc = gotkid;	/* for run_errors */
	gktype = gexp_type( gotkid );
	gkflags = node_flag(gotkid);
	if( (gkflags & NF_VALMASK) == NF_STPUSH || gkflags & NF_CHRSTR) {
		if( is_stringtype( gktype ) ) {
			int whatpush;
			tempptr = str_spop();	/* address of object */

			/* egads they will pay for this */
			formsize=get_stsize(find_realtype(kid2(
					arg_indtype(thesym,nkid-1))));
			whatpush = size_adjust(formsize + 1 + STR_START);
			rstackov( whatpush );
			/* one more byte for zero is allowed */
			bump_stack( whatpush );
			EDEBUG(4, "Pushing string %s length %d on stack",
				tempptr, formsize );
			str_assign( sp.st_generic, tempptr, formsize );
			}
		 else {
			/* must be array or struct */
			tempptr = p_spop();
			formsize = calc_size( gktype, TRUE );
			rstackov( formsize );	/* stack overflow check */
			bump_stack( size_adjust(formsize) );
			EDEBUG( 4, "Pushing object at %x size %d onto stack\n",
					(int)tempptr, formsize );
			/* check_undef( tempptr, formsize ); */
			/*do_undef( sp.st_generic, U_SET, formsize ); */
			copy_mask( sp.st_generic, tempptr, formsize );
			blk_move( sp.st_generic, tempptr, formsize );
			}

		}
	 else {
		/* mark pushed item as a defined element */
		/* this could be done faster by leaving the old stack on
		   the loc stack and getting the size from that */
#ifdef UDEBUG
		if( gkflags & NF_PASSFUNC )
			formsize = 2 * sizeof(pointer);
		 else
			formsize = 2; /* calc_size( gktype, TRUE ); */
		/* actually 2 because on 6800 chars are in high byte */
		/* I do know that calc_size is wrong for var params */
		/* the number we really want is how much was pushed */
		/* I have a strong suspicion formsize = 1 will suffice */
		do_undef( sp.st_generic, U_SET, formsize );
#endif
		if( gkflags & NF_ISTRING ) /* string to pointer */
			++(p_stop);
		if( node_2flag( gotkid ) & NF2_GENERIC ) {
			struct anyvar theadr;
			int tcode;
			theadr.av_tcode = -1;

			if( not_variable( gotkid ) ) {
				if( is_ordinal(gktype) ) {
					theadr.obj.integer = int_spop();
					theadr.av_tcode = CC_ORDVALUE;
					}
				 else if( is_real(gktype) ) {
					theadr.obj.real = f_spop();
					theadr.av_tcode = CC_REALVALUE;
					}
				}
			 else {
				theadr.obj.varp.point = p_spop();
				/* if pointer, give size of pointed to type */
				if( ntype( gktype ) == N_TYP_ARRAY )
					theadr.obj.varp.comp_len = int_kid(2,gktype);
				if( (theadr.av_tcode = asg_code( gktype ))==CC_POINTER)
					theadr.obj.varp.comp_len =
					  gktype == Basetype(BT_pointer) ? 1 :
						calc_size(kid1(gktype),TRUE);
				if( theadr.av_tcode < 0 ) {
					bits8 *p, *table_lookup();
					static bits8 tct[] = { N_TYP_RECORD,
								CC_RECORD,
							N_TYP_ARRAY, CC_ARRAY,
							N_TYP_FILE, CC_FILE,
							N_TYP_ENUM, CC_ENUM,
								0 };
					p = table_lookup( tct, ntype(gktype), 2 );
					if( p )
						theadr.av_tcode = *p;
					}
				}
			theadr.av_len = calc_size( gktype, TRUE );
			bump_stack( size_adjust(sizeof(struct anyvar)) );
			do_undef( sp.st_generic, U_SET, sizeof(struct anyvar)); 
			blk_move( sp.st_generic, &theadr, sizeof(struct anyvar) );
			}
		}
	if( nkid < listcount(arg_list) ) {
		state_push(S_CALL1);
		new_loc = node_kid( arg_list, nkid );
		}
	 else {
		nodep block;
		NodeNum btype;
		block = tparent(thesym);
		btype = ntype(block);
		espop(e_int);	/* eliminate count */
		make_frame(sym_scope(thesym), act_size(block),
				eloc, block, calldisp,
				sym_mflags(thesym) & SF_HIDDEN );
		state_push(S_RETURN);
		++call_depth;
		new_loc = node_kid(block, kid_count(btype) - 1 );
		}
#else
#endif ICRIPPLE
}


/* subrange check */

#ifdef UDEBUG
sr_ccheck( xtnode, value )	/* check if subrange first */
nodep xtnode;
rint value;
{
	register nodep notype;

	notype = gexp_type( xtnode );

	if( ntype(notype) == N_TYP_SUBRANGE )
		sr_check( notype, value );
}

sr_check( xtnode, value )
nodep xtnode;		/* type node of N_TYP_SUBRANGE */
rint value;		/* value checked */
{
	register nodep tnode = xtnode;

	if( ntype(tnode) != N_TYP_SUBRANGE )
		run_error( ER(89,"bug`BUG: subrange check on non subrange %x\n"),
					tnode );

	if( value < eval_const( kid1(tnode), FALSE ) ||
			value > eval_const( kid2(tnode), FALSE ) )
		
		run_error( ER(90,"subrange`Value %s is not in subrange bounds for this assignment"), ordname( tnode, value ) );
}
#endif UDEBUG
	
	/* routine to perform the got operation */
do_a_goto()
{
#ifndef ICRIPPLE
	nodep gowhere;
	register listp gparent;
	register estackloc *stscan;
	symptr thelab;

	thelab = kid_id( kid1( ex_loc ) );
	gowhere = NCAST sym_value( thelab );

	if( gowhere == NIL )
		run_error( ER(91,"badgo`Can't find the label %s"), sym_name(thelab) );
	
	gparent = FLCAST tparent( gowhere );	/* should be a list node*/
#ifdef DEBUG
	if( not_a_list( gparent ) )
		run_error( ER(92,"bug`BUG:goto label not found or not in list") );
#endif

	/* scan the execution stack for this parent */
	EDEBUG(7, "Goto: label is at %x, parent %x\n", (int)gowhere, (int)gparent );

#ifdef Trace
	{
	extern int within_trace;
	within_trace = FALSE;	/* take a risk, no goto in trace without */
				/* possibly invoking pending exceptions */
	}
#endif
	stscan = ex_stack; 
	while( stscan < loc_stack+lc_stack_size )
		if( (*stscan).e_loc == FNCAST gparent && stscan[1].e_state == S_LIST1 ) {
			/* got it. */
			/* Alter the list count for resumption after it */
			stscan[2].e_int = get_kidnum( gowhere );
			/* restore stack to former state */
			sp.st_generic = stscan[3].e_pointer;
			/* loop upward to find what display stack is in */
			/* we find the first display that is higher up than us*/
			while( st_display && sp.st_generic > (pointer) st_display )
				st_display = up_frame( st_display );
			/* set exec stack to be ready to pop */ 
			ex_stack = stscan;
			EDEBUG(7,"Goto: real stack to %x, ex_stack to %x\n",
						(int)sp.st_generic, (int)ex_stack );
			EDEBUG(7,"Display set to %x\n", (int)st_display, 0 );
			return;
			}
		 else
			stscan++;

	run_error( ER(93,"badgo`A jump to label %s is not valid from here"),
			sym_name(thelab) );


#else
#endif ICRIPPLE
}

unsigned char *
validdigs( xstr )
unsigned char *xstr;
{
	register unsigned char *str = xstr;
	/* should not give this error fatally from input */
	if( *str && !isdigit( *str ) )
		return (unsigned char *) 0;
		/*run_error( "bnum`Badly formed number %s on input", str );*/

	while( isdigit( *str ) )
		str++;
	return str;
}
	/* validity function.  Return -1 for bad number, zero for number in
	 * progress, and first char after number for completed number */

validint( xstr )
unsigned char *xstr;
{
	register unsigned char *str = xstr;
	unsigned char *vdret;
	if( *str == '-' || *str == '+' )
		str++;
	vdret = validdigs( str );
	return (vdret) ? *vdret : -1;
}

validreal( xstr )
unsigned char *xstr;
{
	register unsigned char *str = xstr;
	if( *str == '-' || *str == '+' )
		str++;
	/* deviation, allow real number starting with just a dot */
	if( *str != '.' )
		str = validdigs( str );
	if( !str )
		return -1;
	if( !*str )
		return FALSE;
	if( *str == '.' ) {
		/* other side of dot */
		if( !(str = validdigs( str+1 )) )
			return -1;
		}
	if( *str == 'e' || *str == 'E' ) {
		str++;
		if( *str == '-' || *str == '+' )
			str++;
		str = validdigs(str);
		if( !str )
			return -1;
		if( !*str )
			return FALSE;
		}
	return (unsigned)*str;
}
extern struct pas_file fil_input, fil_output;
unsigned char inp_buf[MAX_LEN_LINE+1] = "";
unsigned char *inp_bp = inp_buf;

scanin( buf, func, filedesc )
char *buf;
int (*func)();
fileptr filedesc;
{
	register char c;
	register int dex;
	extern int turbo_io;
	int valid;
	int firstone = TRUE;			/* first char read? */

startnum:
	valid = FALSE;
	/* if eol true at start of read in turbo mode, no effect */
	if( last_ioerr || (turbo_io && filedesc->f_flags & FIL_KEYBOARD &&
				eolcheck(filedesc) ) )
		return FALSE;
	for(;;) {
		if( filedesc->f_flags & FIL_EOF || last_ioerr ) {
			buf[0] = 0;
			return FALSE;
			}
		c = inp_char( filedesc, FALSE );
		if( c == ' ' || c == '\n' || c == '\r' )
			_a_get( filedesc, FALSE );
		 else
			break;
		firstone = FALSE;

	} 

	/* whatever non space character will now be in the buffer */

	for( dex = 0 ; !valid ; dex++ ) {
		if( dex >= MAX_NUM_LEN ) {
			tp_error( 0x10, FALSE );
			last_ioerr = 0;
			message( ER(229,"numin`Input Error - Number too long") );
			return FALSE;
			}
		/* ask that it not do the get */
		c = inp_char( filedesc, FALSE );
		buf[dex] = c;
		buf[dex+1] = 0;
		/* if we are still going, do the get we asked not to be done */
		valid = (*func)(buf);
		EDEBUG( 8, "Number input buffer %s, validity %d\n", buf,valid);
		if(  valid == 0 )
			_a_get( filedesc, FALSE );
		 else if( valid < 0 ) {
			if( filedesc->f_flags & FIL_LAZY ) {
				char badnum[80];
				/* no turbo error here */
				strcpy( badnum, buf );
				inp_bp = inp_buf;
				inp_buf[0] = 0;
				tp_error( 0x10, FALSE );
#ifdef SAI
				{
				char pbuf[80];
				sprintf( pbuf, "Sorry, '%s' isn't a valid number, please try again", badnum );
				pure_message( pbuf );
				/* Read the rest of the bad line */
				do_lnread( 0, NIL, NIL, filedesc );
				}
#else
				warning( ER(230,"bnum`Sorry, '%s' isn't a valid number, please try again"), badnum );
#endif SAI
#ifdef SAI
				if( sai_out == SAI_SCREEN ) {
#endif
				curOn( pg_out );
				wrefresh( pg_out );
#ifdef SAI
					}
#endif SAI
				last_ioerr = 0;
				_a_get( filedesc, TRUE );
				clear_em(TRUE);
				goto startnum;
				}
			 else {
				tp_error( 0x10, FALSE );
				run_error( ER(94,"bnum`Badly formed number %s on input"), buf );
				}
			}
		
		}
	return TRUE;
}


/* I want a character, and I want it now */

inp_char( filedesc, dtget )
fileptr filedesc;
int dtget;		/* do the get at the end */
{

	char c;

	if( filedesc->f_flags & FIL_NREAD )
		/* force the get */
		_a_get( filedesc,  TRUE );
	
	c = filedesc->f_buf;
	EDEBUG(8, "Input char filedesc %x returns char %d\n", (int)filedesc,c);

	if( dtget )
		_a_get( filedesc, FALSE );

	return c;
}

_a_get( xwhfile, realget )
fileptr xwhfile;		/* file to get from */
int realget;			/* get it, lazy or not */
{
	int datsiz, c, i;
	short flstore;
	register fileptr whfile = xwhfile;
	register pointer rdbuf;
#ifdef TURBO
	extern int turbo_io;

#endif

	flstore = whfile->f_flags;
	if( !(flstore & FIL_READ ) ) {
		tp_error( 2, 0 );
		run_error( ER(95,"notread`READ/GET - file not reset for reading") );
		}
	if( flstore & FIL_EOF ) {
		tp_error( 0x99, 0 );
		run_error( ER(96,"pasteof`READ/GET - tried to get past end of file") );
		}
	rdbuf = &(whfile->f_buf);

	/* if input, set the flag saying that the buffer should be present, */
	/* Chopped? */
	datsiz = whfile->f_size;
	if( !datsiz )
		/* untyped file has zero size, get does nothing */
		return ;

	/* IS THERE IS A BUG HERE FOR TWO GETS IN A ROW? */
	if( !realget && flstore & FIL_LAZY ) {
		whfile->f_flags |= FIL_NREAD;
		EDEBUG(9, "get sets unread bit on file %x was%x\n", (int)whfile,flstore );
		/* if bit was aready set, do a read and leave it set */
		if( !(flstore & FIL_NREAD) ) 
			return;
		}
	if( realget )
		whfile->f_flags &= ~FIL_NREAD;
	
	do_undef( rdbuf, U_SET, datsiz );
	for( i = 0; i < whfile->f_size; i++ ) {
#ifdef TURBO
		/* no i/o with pending error */
		if( last_ioerr )
			c = '\r';
		 else
#endif
		if (flstore & FIL_KEYBOARD ) {
			extern FILE	*LogFile;
			
			if( flstore & FIL_RAWKEY ) {
#ifdef SAI
				if( sai_out == SAI_SCREEN ) {
#endif
				extern WINDOW *pg_out;
				curOn( pg_out );
				wrefresh(pg_out);
#ifdef SAI
				}
#endif

#ifdef KEYFILE
				{
				extern FILE *keyfile;
				extern int keyfwrite;
				if( keyfile ) {
					if( keyfwrite ) {
						c = _rchar();
						fputc( c, keyfile );
						}
					 else
					 	c = fgetc( keyfile );
					}
				 else
					c = _rchar();
				}
# else
				c = _rchar();
# endif
				if( !c )
					c = 27;
#ifdef SAI
				if( sai_out == SAI_SCREEN )
#endif
					curOff(pg_out);
				}
			 else
				c = get_bufch(whfile->desc.f_window);
/*
 * The SAI doesn't have a logfile option
 */
#ifndef SAI
			if (LogFile && c >= 0)
				putc(c, LogFile);
#endif SAI
			}
		else {
			if( got_break() )
				return;
			c = fgetc( whfile->desc.f_stream );
			}
		EDEBUG( 8, "_a_get - gets char %d '%c'\n", c, c );
		if( c < 0 ) {
			whfile->f_flags |= FIL_EOF;
			whfile->f_flags &= ~FIL_EOL;
			do_undef( rdbuf, U_CLEAR, datsiz );
			c = ' ';
			break;
			}
		rdbuf[i] = c;
		}
	if( flstore & FIL_TEXT ) {
#ifdef unix
		if( c == '\n' || c == '\r' ) {
#else
		if( (!turbo_io && c == '\n') || c == '\r' ) {
#endif
			if( !(turbo_io || flstore & FIL_RAWKEY) )
				rdbuf[0] = ' ';
			whfile->f_flags |= FIL_EOL;
			}
		 else
			whfile->f_flags &= ~FIL_EOL;
		if( turbo_io && (whfile->f_flags & FIL_EOF || c == CNTL('Z') )){
			rdbuf[0] = CNTL( 'Z' );
			whfile->f_flags |= (FIL_EOL|FIL_EOF);
			/* direct contradict of standard */
			do_undef( rdbuf, U_SET, 1 );
			}

		}
		
}

wrput( type, xfiledesc )
nodep  type;
fileptr xfiledesc;
{
	register fileptr filedesc = xfiledesc;
	register pointer popptr;

	if( (is_stringtype(type) && type != Basetype(BT_char))
			|| ntype(type) == N_TYP_RECORD 
			|| ntype(type) == N_TYP_ARRAY ) {
		/* str_spop must handle normal pointers */
		popptr = str_spop();
		}
	 else  {
		popptr = WB_ADJUST(filedesc->f_size) + sp.st_generic;
		unload_stack( size_adjust(filedesc->f_size) );
		}
	EDEBUG( 7, "Put from Write address %x, file %x\n", (int)popptr, (int)filedesc);
	putout( filedesc, popptr, FALSE );
	do_undef( &(filedesc->f_buf), U_CLEAR, filedesc->f_size );
}
getread( xfiledesc, loc )
fileptr xfiledesc;
pointer loc;
{
	register fileptr filedesc = xfiledesc;
	/* will never get here for text or lazy files */
	/* copy the data over */
	EDEBUG(7, "Get from read address %x from file %x\n", (int)loc, (int)filedesc );
	/* NREAD set if we just did a turbo style seek */
	if( filedesc->f_flags & FIL_NREAD )
		_a_get( filedesc, TRUE );
	blk_move( loc, &(filedesc->f_buf), filedesc->f_size );
	do_undef( loc, U_SET, filedesc->f_size );
	/* pass anything, so long as it isn't text */
	_a_get( filedesc, FALSE );
}

static char filbuf[MAX_FN_LEN];

char *
filevarname(whfile)
fileptr whfile;
{
	register nodep filvar;
	extern int fv_count;
		
	/* if a name was assigned, use that */
	if( whfile->f_name )
		return whfile->f_name;

#ifndef SAI
	filvar = kid1( kid2( ex_loc ) );
	while( kid_count( ntype(filvar) ) > 0 )
		filvar = kid1(filvar);
	if( ntype(filvar) != N_ID )
		return "foobar";
	sprintf( filbuf, "%0.*s.%x",MAX_FN_LEN-1,sym_name(kid_id(filvar)),
			fv_count++ );
#else SAI
	sprintf( filbuf, "ap%x", (int) filvar );
#endif SAI
	return filbuf;
}

pop_susp()
{
	extern int count_suspend;
	struct susp_block *upper;
	/* assumes checking already done */
#ifndef SAI
	upper = curr_suspend->susp_prev;
	closeall(FALSE, curr_suspend, upper );
	curr_suspend = upper;
	redraw_status = TRUE;
	if( --count_suspend ) {
		call_depth = sp.st_susp->susp_cdepth;
		call_current = 0;
		tracepop();
		}
	 else {
		finishGraphics();
		closeall(TRUE,(pointer)0,(pointer)0);
		}
#endif SAI
}

stack_pointer *tb_frame;	/* track frame */

	/* pop the traceback stack, moving the cursor to the caller */
tracepop()
{
	register nodep cloc;
#ifndef SAI

	redraw_status = TRUE;
	if( --call_current < 0 ) {
		call_current = call_depth;
		if( cloc = curr_suspend->susp_todo )
			cursor = cloc;
		tb_frame = curr_suspend->susp_display;
		}
	 else {
		cursor = get_fpointers( tb_frame )[0];
		tb_frame = up_frame( tb_frame );
		}
#endif SAI
}

static char ordbuf[20];		/* big enough for longest integer + 6 */
char *
ordname( xtyp, ord )
nodep xtyp;		/* enumerated type guy */
int ord;		/* ordinal value */
{
	register nodep typ = comp_type( xtyp );

	if( is_char(typ) )
		sprintf( ordbuf, "'%c'", ord );
	 else if( ntype(typ) == N_TYP_ENUM ) {
		if( ord >= typ_bound( typ, 0 ) && ord <= typ_bound(typ,1) )
			return sym_name((symptr)node_kid(kid1(typ), ord));
		 else
			sprintf( ordbuf, "Enum:%d", ord );
		}
	 else
		sprintf( ordbuf, "%d", ord );
	return ordbuf;
}

#ifdef IBMPC
int	EOFChar = KEY_F(6);
#define MAGIC_SPLAT 0x8a	/* upside-down '?' */
#else

#ifdef QNX
#define MAGIC_SPLAT 0x0f
extern int	EOFChar;	/* defined and set in curses */
#else

#define MAGIC_SPLAT '?'
int	EOFChar = CNTL('D');
#endif QNX
#endif msdos

/* NEEDS WINDOW ARGUMENT replace pg_out */
get_bufch()
{
	int dex;
	ErrorMsg lastmess;		/* last status message */
	extern ErrorMsg last_slmsg;
	int y, x;
	int c;
	if( *inp_bp )
		return (unsigned)*inp_bp++;

	checkScreen();

	curOn( pg_out );
	lastmess = last_slmsg;
#ifdef msdos
	slmessage( ER( 192, "Awaiting Keyboard Input - F6 for EOF" ) );
#else
	slmessage( ER( 192, "Awaiting Keyboard Input - %s for EOF" ),
				unctrl(EOFChar) );
#endif
	wrefresh( pg_out );

	inp_buf[0] = 0;
	for( dex = 0; dex < MAX_LEN_LINE; ) {
#ifdef SAI
		c = wgetch( pg_out );
#else
		c = keyget();
#endif SAI
		if (c != EOFChar ) {
			if( c == KEY_BACKSPACE ) {
				if( dex > 0 ) {
					inp_buf[--dex] = 0;
					getyx( pg_out, y, x );
					wmove( pg_out, y, x-1 );
					waddch( pg_out, ' ');
					wmove( pg_out, y, x-1 );
					wrefresh(pg_out);
				   	}
				continue;
				}
			 else {
				if( c >= 128 )
					waddch( pg_out, MAGIC_SPLAT );
				 else
					waddch( pg_out, c == '\r' ? '\n' : c );
				}

			inp_buf[dex++] = c;
			inp_buf[dex] = 0;
			}
		 else {
			waddstr( pg_out, "<EOF>\n" );
			}
		wrefresh( pg_out );

		if( c == '\r' || c == '\n' || c == EOFChar || got_break()) {
			break;
			}
		}
	slmessage( lastmess );
	curOff(pg_out);
	wrefresh( pg_out );

	inp_bp = inp_buf;
	if (!*inp_bp && c == EOFChar)
		return -1;
	return (unsigned)*inp_bp++;
}

#ifdef TURBO
	/* add two strings, which may or may not be on the stack */
string_add()
{
	register unsigned char *st1, *st2;
	int comblen;
	unsigned int len1, len2;
	char tempbuf[MAX_STR_LEN];


	/* the strings are in the wrong order if on already */

	st2 = str_spop();	/* second string */
	st1 = str_spop();	/* first string */

	/* get lengths and increment strings */
	comblen = (len1 = *st1++) + (len2 = *st2++);
	if( comblen > MAX_STR_LEN )
		run_error( ER(291,"strplus`Concatenated string has length %d (greater than 255)"), comblen );
	rstackov( comblen + 2 );
	do_undef( st1, U_CHECK, len1 );
	do_undef( st2, U_CHECK, len2 );
	blk_move( tempbuf, st1, len1 );	/* copy string 1 over */
	bump_stack( len2+1 );	/* add one for the zero */
	blk_move( sp.st_generic, st2, len2+1 ); /* include zero */
	bump_stack( len1+1 );	/* add one for new length */
#ifdef WORD_ALLIGNED
	if( ((len1 + len2) & 1) == 0 )
		bump_stack( 1 );
#endif
	blk_move( sp.st_generic+1, tempbuf, len1 );
	*sp.st_generic = comblen;
	do_undef( sp.st_generic, U_SET, comblen+2 );
	p_spush( (pointer)0 );	/* null pointer for stack string */


}

#if defined(POINT32) && !defined(msdos)
unsigned char *
str_spop(){
	register unsigned char *res;
	register to_unload;

	if( res = p_spop() )
		return res;
	res = sp.st_generic;
	to_unload = res[0] + 2;
	unload_stack( word_allign(to_unload) );
	return res;
}
#endif

#endif TURBO
