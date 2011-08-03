/*
 *DESC: Interpreter init and declaration 'compile' and processing.
 */

#define INTERPRETER 1

#include "alice.h"


#include "interp.h"
#include "flags.h"
#include "typecodes.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#ifdef DEBUG
# define PRINTT
#endif
#include "printt.h"

stackloc *exec_stack = 0;
unsigned sc1_size;		/* size of global stack frame */
#ifdef LARGE
unsigned SSeg;
#endif

#ifdef CHECKFAR
farcheck(filename, line)
char	*filename;
int	line;
{
	if (tracing)
		fprintf(dtrace, "farcheck [line %d in %s]\n", line, filename);
}
#endif

#ifndef HYBRID
# ifdef QNX
	extern unsigned malloc_total;
	extern long MemUsed;
# define malloc_left() (malloc_total-MemUsed)
# else
#  ifdef msdos
    extern long malloc_left();
#  else
#   define malloc_left() 1000000
#  endif
# endif
#define malloc_divide 3
#else
extern long malloc_left();
#define malloc_divide 2
#endif

extern char foref_str[];
#ifdef QNX
int
num_handler()
{
	run_error( ER(314,"math`Floating point error (overflow)"));
}
#endif

#ifndef PROC_INTERP
init_interp()
{
	extern int count_suspend;
#ifdef QNX
	/* disable breaks we check them ourselves */

	/* exception handler in first segment */
#define EXC_FLOAT 0x80
#define EXC_DIV_Z 0x8000
	exc_handler( EXC_FLOAT | EXC_DIV_Z, 0, &num_handler );
	/* disable breaks we check them ourselves */
	unbreakable();
#endif
	curr_suspend = (struct susp_block *)NULL;
	count_suspend = 0;
	if( exec_stack )
		dmfree( exec_stack );
	exec_stack = 0;
	if( loc_stack )
		dmfree( loc_stack );
	loc_stack = 0;
	if( undef_bitmap )
		dmfree( undef_bitmap );
	undef_bitmap = 0;
	if( !max_stack_size ) {
		long mlcalc;
#ifdef ATARI520ST
		mlcalc = 5000L;
#else
		mlcalc = (malloc_left() - 4096 ) / malloc_divide;
#endif
		if( mlcalc < 0 )
			max_stack_size = 1024;
		 else if ( mlcalc > 1000000L )
			max_stack_size = 1000000L;
		 else
			max_stack_size = (unsigned)mlcalc;
		
		}
	if( !lc_stack_size ) 
		lc_stack_size = min( max_stack_size/8, 2000 );
	printt2( "max stack size %d, lc_stack_size %d\n", max_stack_size,
					lc_stack_size );
	exec_stack = (stackloc *) checkalloc( max_stack_size );
#ifdef LARGE
	SSeg = (unsigned)((long)exec_stack >> 16);
#endif
	loc_stack = (estackloc *) checkalloc(lc_stack_size*sizeof(estackloc) );

#ifdef UDEBUG
	undef_bitmap = (bits8 *) checkalloc( 1+max_stack_size/BITS_IN_BYTE );
#endif
	EDEBUG(1, "Exec stack at %x, loc_stack at %x\n", exec_stack, loc_stack);
	EDEBUG(1, "Undef bitmap at %x, size of exec stack %d\n", undef_bitmap,
				max_stack_size );

}
#endif

extern int seterrbit;
extern int tc_interactive;

c_comp_decls( scope, loc, interactive )
int scope;
nodep loc;
int interactive;
{
	extern int had_type_error;
#ifdef DEBUG
#ifndef SAI
	extern int CheckSpeed;

	if( CheckSpeed < -2 )
		return;
#endif
#endif
	here(1);

	had_type_error = FALSE;
	seterrbit = NF_ERROR;		/* clear and set this bit */	
	comp_decls( scope, loc, FALSE, interactive, FALSE );

	if( ntype(loc) == N_IMM_BLOCK ) {
		stack_pointer *gotact, *find_active(), *holdact = 0;
		/* clear immediate block only */
		/* loop up through active frames for this block */
		while( gotact = find_active( loc ) ) {
			if( gotact == holdact )
				bug( ER(266,"Bad suspension stack clearing immediate") );
			/* while still suspened, if suspended below active, pop
			   the states until no state exists with this block */
				
			while( curr_suspend  &&
					(stack_pointer *)curr_suspend < gotact )
				pop_susp();
			/* if we popped, we can't find the same frame */
			holdact = gotact;
			}
		}
	 else {
		extern int count_suspend;
#ifndef SAI
		if( count_suspend )
			redraw_status = TRUE;
#endif
		curr_suspend = 0;
		count_suspend = 0;
		}
}

comp_decls( scope, xblock, isdirty, inter, ishide )
int scope;		/* what scope to set for these guys */
nodep xblock;		/* which block we are compiling for */
int isdirty;		/* set if block above wasn't checked */
int inter;		/* interactivity */
int ishide;		/* is a hidden block */
{
	register nodep block;
	int cursize;		/* how much of the stack frame we have so far*/
	reg listp dlist;	/* list of decls in the block */
	NodeNum btype;		/* type of block */
	int bflag;		/* flag for program block */
	register int bigdec;		/* indexes into declaration lists */
	register nodep dblock;		/* block of declarations or routine */
	symptr ourdec;		/* declaration to be compiled */
	int vard_comp(), cond_comp(), typd_comp();	/* compiling routines */
	int kc;

#ifndef SAI
	stackcheck();
#endif SAI

	here(2);
	block = xblock;
	btype = ntype(block);
	seterrbit = NF_ERROR;		/* clear and set this bit */	
	tc_interactive = inter;

	if( is_a_hide(block) ) {
		dlist = FLCAST kid2( block );
		bflag = FALSE;
		}
	 else {
		symptr ptr;
		dlist = FLCAST decl_kid( block );
		bflag = TRUE;
		isdirty = isdirty || !(node_flag(block) & NF_TCHECKED);
		/* scan symbol table, clearing defined bits */
		for (ptr = (symptr)kid1(sym_kid(block)); ptr; ptr = sym_next(ptr)) {
			clr_sym_mflags(ptr,SF_DEFYET|SF_DEFOERR|SF_NEGREAL);
			if( sym_dtype( ptr ) == T_UNDEF )
				s_sym_type( ptr, Basetype(BT_integer) );
			}
		}
	printt2( "Descend in block %lx, isdirty is %d\n", block, isdirty);
	cursize = 0;	/* in case no variables */

	/* if it has parameters, check those */
	if( btype == N_DECL_PROC || btype == N_DECL_FUNC ) {
		symptr routname;
		if( not_a_stub( (routname = (symptr)kid1(block)) ) ) {
			if( ishide )
				or_sym_mflags(routname, SF_HIDDEN );
			 else
				clr_sym_mflags( routname, SF_HIDDEN );
			}
		scan_decl( FLCAST kid2(block), vard_comp, 0, TRUE,
					NF_ERROR, (pint)scope );
		}
	/* loop through the declaration list of the block, dealing with
	   all the blocks of declarations, or the proc/func decls */

	for( bigdec = 0; bigdec < listcount( dlist ); bigdec++ ) {
		dblock =  node_kid( dlist, bigdec );
		switch( ntype( dblock ) ) {
		case N_DECL_VAR:
			cursize = scan_decl(FLCAST kid1(dblock), vard_comp,
				cursize, TRUE, NF_ERROR,(pint)scope);
			break;
		case N_DECL_TYPE:
			scan_decl( FLCAST kid1(dblock), typd_comp,  0, TRUE,
				NF_ERROR, (pint)0 );
			break;
		case N_DECL_CONST:
			scan_decl( FLCAST kid1(dblock), cond_comp, 0, TRUE,
				NF_ERROR, (pint)0 );
			break;
		case N_DECL_LABEL:
			break;	/* no init for labels */
		case N_DECL_FUNC:
			ourdec = (symptr)kid1( dblock );
			if( not_a_stub( ourdec ) ) {
				nodep functype;
				functype = find_realtype( kid3(dblock) );
				s_sym_type(ourdec,functype );
#ifdef DB_TYPES
				if(tracing)fprintf(dtrace,"Setting type for function %x to %x\n",
						ourdec, functype );
#endif
				if( ntype(functype) == N_TYP_RECORD ||
				    ntype(functype) == N_TYP_ARRAY )
					type_error( dblock, ER(117,"sttype`Can't return a complex type from a function") );
				s_sym_size(ourdec, calc_size(functype, FALSE) );
				}
			/* activation record size will be set later */
		case N_DECL_PROC:
			ourdec = (symptr)kid1( dblock );
			comp_decls( scope + 1, dblock, isdirty, inter, ishide );
			if( not_a_stub(ourdec) ) {
				s_sym_scope( ourdec, scope+1 );
				s_sym_dtype( ourdec, ntype(dblock) ==
					N_DECL_PROC ?T_PROCEDURE:T_FUNCTION );
				}
			break;
		case N_IMM_BLOCK:
			comp_decls( scope +1, dblock, isdirty, inter, ishide );
			break;
		case N_HIDE:
		case N_REVEAL:
			cursize += comp_decls( scope, dblock, isdirty, inter, 
						ntype(dblock) == N_HIDE );
			break;

		case N_STUB:
		case N_FORWARD:
		case N_ST_COMMENT:
		case N_NOTDONE:
			break;
		default:
			error( ER(118,"comp_dec: odd block %d"), ntype(dblock));
		 }
		}
	if( !bflag )
		return cursize;

	/* now set the activation record size */
	kc = kid_count(btype);

	s_int_kid( kc+1, block,  cursize );

	/* now typecheck the code if it isn't slow */
	if( inter & TC_FULL ||(isdirty&&(inter & TC_CHECKDIRTY))) {
		tc_interactive = inter;
		typecheck( node_kid(block, kc - 1 ), 0 );
		/* mark block as checked */
		or_node_flag(block, NF_TCHECKED);
		/* restore error setter */
		seterrbit = NF_ERROR;		/* clear and set this bit */	
		}
	clr_node_flag( block, NF_ERROR);
	return cursize;
}

int
typd_comp(thedecl, so_far, type_parent )
reg symptr thedecl;		/* what symbol is declared */
int so_far;		/* how many bytes so far */
reg nodep type_parent;	/* the N_TYP_DECL node */
{
	register nodep stype;
	stype = find_realtype( kid2(type_parent) );
	s_sym_scope( thedecl, 1 );	/* 1 so it can be go_to_decl */
	s_sym_size(thedecl, calc_size(stype, FALSE) );
	s_sym_type(thedecl, stype );

	or_sym_mflags(thedecl, SF_DEFYET);
}

int got_minus;

int
cond_comp( thedecl, so_far, type_parent )
reg symptr thedecl;		/* what symbol is declared */
int so_far;		/* how many bytes so far */
reg nodep type_parent;	/* the N_TYPE_DECL node */
{
	register nodep ctype = NIL; 
	register nodep ctree;
	register int realcon = FALSE;

	ctree = kid2(type_parent);
	s_sym_scope( thedecl, 1 );	/* 1 so it can be go_to_decl */
#ifdef TURBO
	if( ntype(type_parent) == N_CONST_INIT ) {
		s_sym_dtype(thedecl, T_INIT);
		if( not_a_stub( ctree ) ) {
			register nodep destype;
			symptr ctype;
			extern pointer newAlloc();
			pointer nitmem;
			extern int tc_interactive;
			int typsize;
			ctype = kid_id(ctree);
			if( !(sym_mflags(ctype) & SF_DEFYET )) {
					type_error( ctree, ER(167,foref_str) );
					or_sym_mflags( ctype, SF_DEFOERR );
					return;
					}
			destype = sym_type( ctype );
			s_sym_type( thedecl, destype );
			typsize = sym_size( ctype );
			s_sym_size( thedecl, typsize );
			if( tc_interactive & TC_INIT ) {
				if( !(nitmem = newAlloc( typsize ) ) )
					type_error( type_parent, ER(277,"bigstruct`Array much too large -- shrink the bounds") );
				 else {	/* mark undefined */
					do_undef( nitmem, U_CLEAR, typsize );
					}
				s_sym_value( thedecl, (pint)nitmem );
				}
			 else 
				nitmem = (pointer) 0;
			/* when allocating, NEW enough and mark it defined */
			descend_init( kid3(type_parent), destype, nitmem);
			}
		or_sym_mflags(thedecl, SF_DEFYET);
		return;
		}
	 else
#endif
		{
		s_sym_dtype( thedecl, T_CONST );
		ctype = gexp_type(ctree);
		/* say we are runtime even though we aren't because it
		 * is such a simple size
		 */
		s_sym_size( thedecl, calc_size(ctype, TRUE ));
		}



	got_minus = 0;

	if( ctype )
		realcon = ctype == Basetype(BT_real) || (is_stringtype(ctype)
					&& ctype != Basetype(BT_char) );
	s_sym_value( thedecl, eval_const(ctree, realcon ) );
	or_sym_mflags(thedecl, SF_DEFYET);
	s_sym_type( thedecl, ctype );
	if( got_minus )
		or_sym_mflags( thedecl, SF_NEGREAL);
	 else
		clr_sym_mflags( thedecl,SF_NEGREAL);
}

#ifdef TURBO

/* scan down initializer, checking, setting types and calculating if
 * necessary
 */

#ifndef OLMSG
static char nitmatch[]= "nitmatch`Initializer types don't match";
#endif

descend_init( inittree, typetree, ptradr )
register nodep inittree;		/* initializer tree */
nodep typetree;		/* related tree in type declaration */
pointer ptradr;		/* location for initializer storage, or zero */
{
	nodep realttree;
	register nodep stlist;
	NodeNum mtype;
	int asgt;

#ifndef SAI
	stackcheck();
#endif

	if( typetree ) {
		realttree = comp_type(find_realtype( typetree ));
		mtype = ntype( realttree );
		}

	clr_node_flag( inittree, NF_ERROR );
	switch( ntype( inittree ) ) {
		case N_INIT_STRUCT:
			s_kid2( inittree, realttree );
			stlist = kid1( inittree );
			if( mtype == N_TYP_RECORD ) {
				int i;
				/* descend record */
				for( i = 0; i < listcount(stlist); i++ ) {
					nodep rekid;
					rekid = node_kid(stlist,i);
					if( ntype(rekid) != N_FLD_INIT ) {
						type_error( rekid, ER(302,"needfi`Need field initializer here") );
						}
					 else
						if( ntype(kid1(rekid)) == N_ID)
							descend_init( rekid,
								NIL,
								ptradr );
					}
				}
			 else if( mtype == N_TYP_ARRAY ) {
				int numels;
				int i;

				numels = int_kid(4,realttree);
				if( numels != listcount(stlist) ) {
					type_error( inittree, ER(301,"wsai`Should be %d elements in array initializer"), numels );
					break;
					}
				for( i = 0; i < numels; i++ ) {
					nodep arkid;

					arkid = node_kid(stlist,i);
					if( ntype(arkid) == N_FLD_INIT ) {
						type_error( arkid, ER(300,nitmatch) );
						continue;
						}
					descend_init( node_kid(stlist,i),
							kid2(realttree),
							ptradr );
					/* add size to ptr if necessary */
					if( ptradr )
						ptradr += int_kid(2,realttree);
					}

				}
			 else
				goto init_matcherr;
			break;
		case N_FLD_INIT:
			if( ntype(kid1(inittree)) == N_ID ) {
				register symptr fldid;
				fldid = kid_id(kid1(inittree));
				if( !(sym_mflags(fldid) & SF_DEFYET )) {
					type_error( inittree, ER(167,foref_str) );
					or_sym_mflags( fldid, SF_DEFOERR );
					return;
					}
				descend_init( kid2(inittree), sym_type(fldid),
					ptradr +(ptradr ? sym_offset(fldid):0));
				}
			break;
				
		case N_EXP_SET:
			if( mtype != N_TYP_SET )
				goto init_matcherr;
			 else {
				nodep setlist;
				nodep sbtype;
				int i;

				typecheck( inittree, (ClassNum)0 );
				setlist = kid1( inittree );
				if( ptradr ) {
					zero( ptradr, SET_BYTES );
					do_undef( ptradr, U_SET, SET_BYTES );
					}
				for( i = 0; i < listcount(setlist); i++ ) {
					nodep setel;
					int lower, upper;

					setel = node_kid(setlist,i);
					if( ntype(setel) == N_SET_SUBRANGE ){
						lower = eval_const(kid1(setel), FALSE );
						upper = eval_const(kid2(setel), FALSE );

						}
					 else 
						lower = upper = eval_const(
								setel, FALSE );
					if( lower > upper ) {
						type_error( setel, ER(58,"setrange`Invalid subset range %d to %d"), lower, upper );
						break;
						}
					if( lower < 0 || upper > 255 ) {
						type_error( setel,ER(170, "badset`Set elements must be in the range 0..255") );
						break;
						}
					if( ptradr ) {
						int j;
						for( j = lower; j <= upper;j++)
							ptradr[j>>3] |=1<<(j&7);
						}
					}
				}
			break;
		default:
			{
			pint cval;
			nodep contype;

			contype = gexp_type( inittree );
			if( is_ordinal( realttree ) &&
					comp_type(contype) == realttree ) {
				if(ptradr) {
					rint result;
					int isbyte;
					result=(rint)eval_const(inittree,FALSE);
					isbyte = asg_code(realttree) == CC_BYTE;
					if( isbyte )
						*ptradr = (char)result;
					 else
						*(rint *)ptradr = result;
					/* is this a waste of a ? */
					do_undef(ptradr, U_SET, 
					  isbyte ? sizeof(char) :sizeof(rint) );
					}
				}
			else if( is_stringtype( realttree ) ) {
				if( is_stringtype( contype ) ) {
					cval = eval_const(inittree, TRUE);
					if( ptradr ) {
						if( contype==Basetype(BT_char)){
							ptradr[0] = 1;
							ptradr[1] = (char)cval;
							ptradr[2] = 0;
							do_undef(ptradr,U_SET,3);
							}
						 else
							str_assign(ptradr,
							 (char *)cval,
							 get_stsize(realttree));
						}
					}
				 else
					goto init_matcherr;
				}
			else if( realttree == Basetype(BT_real) ) {
				if( contype != Basetype(BT_real) )
					goto init_matcherr;
				if( ptradr ) {
					rfloat rresult;
					got_minus = FALSE;
					cval = eval_const(inittree, TRUE);
#ifdef QNX
					esdsblk( &rresult, cval,
						sizeof(rfloat), FALSE );
#else
# ifdef ES_TREE
					rresult = STAR((rfloat far *)
						tfarptr(cval));
# else
					rresult = *(rfloat *)cval;
# endif ES_TREE
#endif QNX
					*(rfloat *)ptradr = got_minus ?
					   -rresult : rresult;
					do_undef(ptradr,U_SET,sizeof(rfloat));
					}
				}
			 else {
				goto init_matcherr;
				}
			}
		}
	return;
	init_matcherr:
		type_error( inittree, ER(300,nitmatch) );

}

#endif TURBO

int
vard_comp( xthedecl, so_far, xtype_parent, scope )
symptr xthedecl;		/* what symbol is declared */
reg int so_far;		/* how many bytes so far */
nodep xtype_parent;	/* the N_VAR_DECL node */
pint scope;		/* general argument, scope in this case */
{
	register symptr thedecl;
	register nodep type_parent;
	register int newsize;
	NodeNum ptype;

	thedecl = xthedecl;
	type_parent = xtype_parent;

	ptype = ntype( type_parent );
	/* This needs some proper turbo ifdefs */
	if( ptype == N_FORM_REF && is_a_stub(kid2(type_parent)) ) {
		/* special case, untyped var parameter */
		s_sym_type( thedecl, Basetype(BT_anytype));
		newsize = sizeof(struct anyvar);
		}
	 else {
		
		/* frame and argument pointers point one beyond the data */
		if( ptype == N_FORM_PROCEDURE )
			newsize = 0;
	 	else {
			newsize = calc_size(ptype == N_FORM_FUNCTION ?
				kid3(type_parent) : kid2(type_parent), FALSE );
			}
		/* different amounts of memory are needed for different kinds of parms*/
	
		s_sym_type(thedecl,find_realtype( NCAST thedecl ));
		}
	s_sym_size(thedecl,newsize);
	if( ptype == N_FORM_PROCEDURE || ptype == N_FORM_FUNCTION )
		newsize = sizeof(pointer) + sizeof(nodep ); /*passed proc */
	 else if( ptype == N_FORM_REF ) {
		newsize = sizeof(pointer);
		s_sym_dtype( thedecl, T_FORM_REF );
		}
	 else if( ptype == N_FORM_VALUE ) {
		s_sym_dtype( thedecl, T_FORM_VALUE );
		}
	 else if( ptype == N_VAR_DECL )
		s_sym_dtype( thedecl, T_VAR );
	s_sym_scope( thedecl, (int)scope );
	or_sym_mflags(thedecl, SF_DEFYET);
	if( ptype == N_VAR_ABSOLUTE ) {
		nodep absarg;

		absarg = kid3( type_parent );
		if( ntype(absarg) == N_ID ) {
			symptr copyid;

			copyid = kid_id(absarg);
			s_sym_dtype( thedecl, sym_dtype( copyid ) );
			s_sym_value( thedecl, sym_value( copyid ) );
			}
# ifdef LARGEPTR
		 else if( ntype(absarg) == N_OEXP_2 ) {
			/* must be an absolute memory ref */
			s_sym_value( thedecl, (eval_const( kid1(absarg), FALSE )
				<< 16) + eval_const( kid2(absarg), FALSE ) );
			s_sym_dtype( thedecl, T_ABSVAR );

			}
# else
		 else {
			type_error( absarg, ER(327,"Segmented absolute address not allowed in regular ALICE") );
			}
# endif
		}
	 else
		{
		so_far += size_adjust( newsize );
		s_sym_offset(thedecl, so_far - WB_ADJUST(newsize));
		}
	return so_far;
}

int
ccheck_func( theconst, dummy, thecase, xswtype )
nodep theconst;		/* what constant to check */
int dummy;		/* useless total */
nodep thecase;		/* check for else */
pint xswtype;		/* type of switch argument */
{
	nodep swtype = NCAST xswtype;
	if( ntype(thecase) == N_CASE ) {
		if( ntype(theconst) == N_TYP_SUBRANGE )
			theconst = kid1(theconst);
		if( gexp_type(theconst) != swtype )
			type_error(theconst, ER(119,"caseconst`Constant isn't the same type as case value") );
		}
	return 0;
}
/* adjust sizes so one byters use two bytes on the stack */
/* also deal with allignment problems on the z-8000 */

int
size_adjust( size )
int size;
{
#ifdef QNX
	return size > 1 ? size : 2;
#else
# ifdef POINT32
	return size < sizeof(pint) ? sizeof(pint) : (size+(size&1));
# else
	return size + (size & 1);
# endif
#endif
}
