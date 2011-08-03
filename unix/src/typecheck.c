/*
 *DESC: typecheck a subtree, and perhaps the whole program.
 *
 * This assumes the declarations have already been compiled.
 * it stores calculated type tags and other type related information
 * such as type bounds etc. within certain nodes
 * we do a post-order traversal
 *
 */
#define INTERPRETER 1

/*#define XDEBUG 1*/

#include "alice.h"
#ifdef HAS_TYPECHECK
#include "typecodes.h"
#include "class.h"
#include "flags.h"
#define kid_type(num) kt_array[num-1]
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "interp.h"

#ifdef DB_TYPECHECK
# define PRINTT
#endif
#include "printt.h"

extern int seterrbit;
extern nodep get_safe_exp_type();
extern nodep recheck();

static str_coerce(), assig_compat(), callcheck();

int tc_interactive;		/* typecheck is in interactive mode */
static bits16 ourflag;
#ifndef OLMSG
static char too_many[] = "manyargs`Too many arguments, must be at most %d";
#endif
static nodep base1temp;		/* temporary base type */
static nodep base2temp;		/* other base type */
static nodep kidt2;		/* kid type 2 */
static nodep tkid1, tkid2;	/* kids of tree */

static nodep tchtemp;		/* temp for type check */
static type_code rcheck_type;	/* type of routine for param check */
static int asg_result;		/* compat code for assignment */
static NodeNum ret_type;	/* node type of return type */
static listp ourargs;	/* an argument list */

extern int ccheck_func();	/* case check function */

int had_type_error = FALSE;
static NodeNum treetype;
#ifndef OLMSG
char bseterr[] = "mixedset`A set value is required here";
#endif

#define MX_USED_TYPE 3		/* only first three kid types ever used */

nodep 
typecheck( _tree, theclass )
nodep _tree;
ClassNum theclass;		/* expected class for this subtree */
{
	register nodep tree;
	register nodep kidt1;	/* type of left (first) child */
	int k_ind;		/* k_ind into children of node */
	int maxkids;		/* how many children for the node */
	nodep kt_array[MX_USED_TYPE];	/* types of children */
	nodep ret = NIL;	/* what type we return */

	tree = _tree;
#ifndef SAI
	stackcheck();
#endif


	maxkids = reg_kids( tree );
	EDEBUG(5, "typecheck(tree=%x, theclass=%d)\n", tree, theclass);
	EDEBUG(5, "Type of tree %d, max kids %d\n", ntype(tree), maxkids );

	if( is_a_list( tree ) ) {
		for( k_ind = 0; k_ind < maxkids; k_ind++ )
			typecheck( node_kid(tree,k_ind), theclass & CLASSMASK);
		printt0("returning NIL\n");
		return NIL;
		}
	 else {
		for( k_ind = 0; k_ind < maxkids; k_ind++ ) {
			static NodeNum ntt;
			static ClassNum kclass;
			int inter_save;	 /* save inter flag to turn desc off */

			ntt = ntype(tree);
			kclass = kid_class(ntt,k_ind);
			inter_save = tc_interactive;
			if( ntt == N_ST_WITH )
				tc_interactive |= TC_DESCEND;
			/* don't go down statements on interactive check */
			if( tc_interactive & TC_DESCEND || kclass !=
					CLIST(C_STATEMENT)) {
				kidt1 = typecheck( node_kid(tree,k_ind),kclass);
				}
			 else
				kidt1 = NIL;
			tc_interactive = inter_save;
			if( k_ind < MX_USED_TYPE )
				kt_array[k_ind] = kidt1;
			}
		}

	treetype = ntype(tree);
	/* save important child types in externals */
	kidt1 = kid_type(1);
	kidt2 = kid_type(2);
	/* the following code is (mildly) risky because it accesses kids
	 * who might not be there.  On a machine without memory checking
	 * this is harmless.  On one with such features, it will trap
	 * *very* rarely  (only when accessing the last node in memory)
	 * Thus the ifdefs
	 */
#ifdef unix
	if( maxkids )
#endif
		tkid1 = kid1(tree);
#ifdef unix
	if( maxkids > 1 )
#endif
		tkid2 = kid2(tree);

	/* now all the children are type checked, and in the case of the
	   list, we have already returned to our caller.  So now we do
	   the individual type checking.  The types of all our kids are
	   currently in the kt_array, which by the way, if we really want
	   to, we can make external by adding a quick loop to grab the
	   info from the stored locations */

	   /* first set the rvalue bit if we want that */
	   /* we'll have to clear it with a var parameter */
	   /* also clear type errors - what about syntax errors? */

	ourflag = node_flag(tree) &~(NF_PASSFUNC|NF_ISTRING|NF_CHRSTR|NF_VALMASK|seterrbit);
	s_node_flag(tree, 0);		/* we will get this back again*/

	if( theclass == C_EXP )
		ourflag |= NF_RVALUE;

	switch( treetype ) {

	 case N_CON_INT:
		if( ourflag & NF_BADCONST )
			type_error(tree, ER(275,
			 "badint`Integer is not in range %d..%d"), -MAXINT, MAXINT);
		ret = Basetype(BT_integer);
		break;
	 case N_CON_REAL:
		ret = Basetype(BT_real);
		break;
	 case N_CON_CHAR:
		ret = Basetype(BT_char);
		break;
	 case N_CON_STRING:
		/* the constant node with length it its own type */
		ret = tree;
		break;
	 case N_VAR_ARRAY:
#ifdef TURBO
		if( ntype( kidt1 ) == N_TYP_STRING ) {
			ret = Basetype(BT_char);
			if( comp_type(kidt2) != Basetype(BT_integer) )
				goto badinderr; /* save a call? How low... */
			break;
			}
#endif
		if( ntype( kidt1 ) != N_TYP_ARRAY ) {
			type_error( tkid1, ER(123,"needarray`Indexing requires an array") );
			ret = Basetype(BT_integer);
			}
		 else {
			type_code ardt;
#ifdef FULL
			if( ntype(tkid1) == N_ID && ((ardt =
					sym_dtype(kid_id(tkid1))) == T_MEMVAR
					|| ardt == T_PORTVAR ) ){
				or_node_flag( tree, NF_FUNNYVAR );
				}
#endif
			ret = find_realtype(kid2( kidt1 ));
			if( !assig_compat( NIL, find_realtype(kid1(kidt1)),
					tkid2, kidt2 ) ) {
			badinderr:
				type_error(tkid2, ER(124,"badindex`This index is not of the appropriate type") );
				}
			}
		break;
	 case N_VAR_POINTER:
		/*at runtime we want the value of the pointer, not its address*/
		ret = Basetype(BT_integer);

		or_node_flag(tkid1, NF_RVALUE);
		if( kidt1 == Basetype( BT_pointer ) ) {
			ret = Basetype(BT_char);
			break;
			}
		if( ntype(kidt1)!= N_TYP_POINTER && ntype(kidt1) != N_TYP_FILE )
			type_error( tkid1,
			   ER(125,"needpointer`Pointer ^ requires a pointer or file variable") );
		 else
			ret = find_realtype( kid1( kidt1 ) );
		break;
	 case N_VAR_FIELD:
		/* we should check for N_NAME on the right, and look it up
		   if possible.  If N_ID, we should confirm identity, and
		   if wrong, announce it.  For now we don't correct it because
		   it's easier and because the user very likely wants a warning
		   here that they changed the whole record */
		/* these guys are currently a little kludged */
		tchtemp = tkid2;
		ret = kidt2;
		if( ntype(kidt1) != N_TYP_RECORD )
			type_error( tkid1, ER(126,"baddot`dot (.) requires a record variable on the left") );
		else if( ntype(tchtemp) == N_ID ) 
			ret = recheck(tree, tchtemp, kidt1 );
				
		break;
	 case N_EXP_NIL:
		ret = Basetype(BT_pointer); 
		break;
	 case N_EXP_PAREN:
		ret = kidt1;
		break;
#ifdef notdef
	 case N_EXP_ESET:
		ret = Basetype(BT_set);
		break;
#endif
	 case N_EXP_SET:
		/* loop through kids, make sure they are all of compatible
		   types and mark the set type */
		ourargs = FLCAST tkid1;
		ret = tree;
		if( listcount(ourargs) == 0 ) {
			ret = Basetype(BT_set);
			break;
			}
		kidt1 = comp_type( get_safe_exp_type( kid1(ourargs) ) );
		if( !is_ordinal(kidt1) ) {
			type_error( kid1(ourargs), ER(127,"setord`Set elements must be of ordinal type") );

			break;
			}
		maxkids = listcount( ourargs );
		for( k_ind = 1; k_ind < maxkids; k_ind++ ) {
			tchtemp = node_kid(ourargs,k_ind);

			/* two parts of subrange already checked.  only
			   check the first one with the rest */

			if( ntype( tchtemp ) == N_SET_SUBRANGE ) 
				tchtemp = kid1( tchtemp );
			if( comp_type(get_safe_exp_type( tchtemp )) != kidt1)
				type_error( tchtemp, ER(128,"setel`This set element doesn't match the first one") );
			}
		/* save away the component base type */
		break;
	 case N_SET_SUBRANGE:
		ret = Basetype(BT_integer);
		if( ntype(tparent(tparent(tree) )) != N_EXP_SET )
			type_error( tree, ER(129,"setrange`Subranges (..) may only be placed in sets") );
		else if( (ret = comp_type( kidt1 )) != comp_type( kidt2 ) )
			type_error( tree, ER(130,"setrange`Two ends of subrange are of differing types") );
		break;
	 case N_EXP_NOT:
		{
		extern int turbo_flag;
		if( !(is_boolean( kidt1 ) || (turbo_flag && is_integer( comp_type(kidt1)) ) ) )
			type_error( tkid1,
				ER(131,"needbool`Operand of NOT must be boolean or integer") );
		ret = kidt1;
		}
		break;

	 case N_EXP_ERROR:
		type_error(tree, ER(121,"syntax`Expression syntax error") );
		ret = Basetype(BT_integer);
		break;

	 case N_ST_IF:
	 case N_ST_ELSE:
	 case N_ST_WHILE:
		if( !is_boolean( kidt1 ) )
			type_error( tkid1,
			ER(133,"needbool`A Boolean (true/false) value is required here") );
		break;
	 case N_ST_FOR:
	 case N_ST_DOWNTO:
		if( ntype(tkid1) != N_ID )
			type_error( tkid1, 
				ER(134,"loopvar`Loop counter must be a simple variable") );
		 else if( !is_ordinal( kidt1 ) )
				type_error( tkid1,
				  ER(135,"loopvar`Loop variable must be of ordinal type") );
			/* somewhere in here we should check local scope */
		if( !assig_compat(tkid1, kidt1, tkid2, kidt2) )
			type_error( tkid2,ER(136,"loopbound`Start not of appropriate type"));
		if( !assig_compat(tkid1, kidt1, kid3(tree), kid_type(3)))
			type_error( kid3(tree),
				ER(137,"loopbound`Finish is not of appropriate type") );
		break;
	 case N_ST_REPEAT:
		if( !is_boolean( kidt2 ) )
			type_error( tkid2,
				ER(138,"repeatbool`Looping condition requires boolean value") );
		break;

	 case N_ST_WITH:
		if( ntype(kidt1) != N_TYP_RECORD )
			type_error( tkid1,ER(311,"needrec`WITH requires a record variable") );
		break;

	 case N_ST_CASE:

		if( is_ordinal(kidt1) )
			scan_decl( FLCAST tkid2, ccheck_func, 0, FALSE, 0,
						(pint)comp_type(kidt1));
		 else
			type_error(tkid1, ER(139,"caseexpr`Case selector must be of ordinal type") );
		break;

	 case N_TYP_SUBRANGE:	/* case subrange */
		range_check( tree, (int *)0 );
		break;

	 case N_EXP_FUNC:
		/* we must type check the parameters */
		if( is_a_stub( tkid1 ) ) {
			ret = Basetype(BT_integer);
			break;
			}

		if( kidt1 == Basetype(SP_passtype) ) {
			ret = gexp_type( tree );
			}
		 else {
			/* functions can be used before declared, so we
			   look up the type if it isn't already set */
			ret = kidt1 ? kidt1 : find_realtype(
				kid3(tparent(kid_id(tkid1))) );
			}
		if( listcount(FLCAST tkid1) == 0 ) {
			/* got a func with zero args */
			static nodep callparent;

			callparent = find_true_parent(tree);
			if( ntype(callparent) == N_ST_CALL || ntype(callparent)
						== N_EXP_FUNC ) {
				/* ok, the parent is a call, now look at
				   the arg */
				static nodep temp1;
				static bits8 temp5;
				static symptr temp2;
				static int temp3;
				static nodep temp4;

				if( not_a_stub((temp1 = kid1( callparent ))) ) {
					temp2 = kid_id( temp1 );
					temp3 = get_kidnum( tree );
					temp4 = arg_indtype( temp2, temp3 );
					temp5 = ntype( temp4 );

					if( temp5 == N_FORM_FUNCTION ){
					
						/* set as passed function */
						or_node_flag(tree, NF_PASSFUNC);
						break;
						}
					}
				}

			}
	 case N_ST_CALL:
		callcheck( tree );
		break;

	 case N_EXP_UPLUS:
	 case N_EXP_UMINUS:
		check_numeric( ret = kidt1, tree );
		break;

	 case N_EXP_SLASH:
		ch_real_coerce( 1, tkid1 );
		ret = ch_real_coerce( 1, tkid2 );
		break;

	 case N_EXP_PLUS:
		/* allow for string concat */
#ifdef TURBO
		if(str_coerce(kidt1,tkid1) &&str_coerce(kidt2,tkid2)){
			ret = Basetype( SP_string );
			break; 
			} 
#endif
	 case N_EXP_MINUS:
	 case N_EXP_TIMES:
		/* we can have int, real or set types */
		if( ret = check_setop( tkid1, kidt1, tkid2,
					kidt2 ) )
			break;
		ret = ch_real_coerce( 2, tkid1, tkid2 );
		break;

#ifdef TURBO
		 
	 case N_EXP_SHL:
	 case N_EXP_SHR:
#endif
	 case N_EXP_DIV:
	 case N_EXP_MOD:
		if( !is_integer(comp_type(kidt1)) ||
				!is_integer(comp_type(kidt2)) )
			type_error( tree, ER(140,"integerop`Operands of DIV/MOD must be integers") );
		ret = Basetype(BT_integer);
		break;
	
#ifdef TURBO
	 case N_EXP_XOR:
#endif
	 case N_EXP_AND:
	 case N_EXP_OR:
		{
		extern int turbo_flag;
		ret = Basetype(BT_integer);
		if( turbo_flag && is_integer(comp_type(kidt1))
					&& is_integer(comp_type(kidt2))) {
			;
			}
		else if( !is_boolean(kidt1) || !is_boolean(kidt2) ) 
#ifdef TURBO
			type_error( tree, ER(141,"boolop`Operands of AND/OR must be boolean or integer") );
#else
			type_error( tree, ER(141,"boolop`Operands of AND/OR must be boolean") );
#endif
		else
			ret = Basetype(BT_boolean);
		}
		break;
		
		 
	 case N_ST_ASSIGN:
		/* check assignment compat */
		{
		if( !(asg_result = assig_compat( tkid1, kidt1, tkid2,
							kidt2 ) ) )
			type_error( tree, ER(142,"assign`Types on := don't match") );

#ifdef FULL
		/* support assignment into the port arrays */
		if( node_flag(tkid1) & NF_FUNNYVAR &&
				sym_dtype(kid_id(kid1(tkid1))) == T_PORTVAR )
			/* code must be CC_BYTE or CC_INTEGER */
			asg_result += CC_PORT;
#endif

		s_int_kid(2, tree, asg_result );
		if(kidt1 && (ntype(kidt1) == N_TYP_FILE ||
			kidt1==Basetype(BT_text)|| kidt1 == Basetype(SP_file)) )
			type_error(tree, ER(143,"fileassign`Files can't be assigned") );
		/* check for function assign */
		}
		break;

	 case N_ST_LABEL:
		/* store the pointer to here in the symbol value  */
		if( not_a_stub( tkid1 ) )
			s_sym_value( kid_id(tkid1), (pint) tree );
		break;

	 case N_VF_WITH:
		{
		/* nodep wnode;

		wnode = kid2( tree ); */
		ret = recheck(tree, tree, /*wnode?gexp_type(kid1(wnode)):*/NIL);

		}
		break;
	 case N_ID:
		/* pass up the type */
		{
		symptr thesym;
		type_code idsym;	/* dtype of symbol */
		extern bits8 typestoget[];

		thesym = kid_id(tree);
		ret = sym_type( thesym );
		if( (idsym = sym_dtype(thesym)) == T_UNDEF ) {
			type_error( tree, ER(190,"undef`Symbol '%s' has not been declared"),
						sym_name(thesym));
			ret = Basetype(BT_integer);
			clr_node_flag(tree,NF_ERROR);
			}
		else if( !ok_sym( thesym, theclass, tree ) )
			type_error( tree, ER(145,"badsymbol`A symbol of this kind can't be placed here" ) );
		else if( idsym == T_PROCEDURE || idsym == T_FORM_PROCEDURE ) {
			ourflag |= NF_PASSFUNC;
			ret = Basetype(SP_proc);
			}
		if( !ret && typestoget[sym_dtype(thesym)] )
			ret = find_realtype( thesym );
		/* do not have rvalue structure types to start with */
		}
		break;


	 case N_OEXP_3:
		kidt1 = ch_real_coerce( 1, tkid1 );
#ifdef notdef
		if( !is_real(kidt1) )
			type_error( tkid1, ER(146,"nrprec`Precisions (2 colons) may only be applied to real numbers") );
#endif
		if( !is_integer(comp_type(kid_type(3))) )
			type_error( kid3(tree), ER(147,"badprec`Output precision must be integer") );

	 case N_OEXP_2:
		ret = kidt1;
		{
		static nodep colparent;
		colparent = find_true_parent(tree);
		switch( ntype( colparent ) ) {
		 case N_ST_CALL:
			if( !is_integer(comp_type(kidt2)) )
				type_error( tkid2, ER(148,"badwidth`Field width must be integer") );
				/* should check for write, writeln, str */
			break;
#ifdef FULL
		 case N_VAR_ARRAY:
			if( ntype(kid1(colparent)) != N_ID || sym_dtype(kid_id(
					kid1(colparent))) != T_MEMVAR )
				type_error( tree, ER(317,"badmem`Colon array index valid only with Mem array" ) );
			  else
				if( !(is_integer(comp_type(kidt1))&&is_integer(
						comp_type(kidt2)) ) )
					type_error( tree, ER(318,"badmem`Absolute memory references must be made with integers" ) );
			break;
#endif
		 default:
			type_error( tree, ER(149,"wrformat`Improper use of colon specifier") );
		 }
		}
		break;

	 case N_EXP_EQ:
	 case N_EXP_NE:
		/* these guys allow anthing at all of basic nature */
		/* I think we store how to compare in an extra word too */
		/* the order we use here reduces code size but might make these
		   checks inordinately long.  In the future, experiment with
		   a master type check routine that takes information saying
		   what types to allow */
		ret = Basetype(BT_boolean);
		if( (ntype(kidt1) == N_TYP_POINTER ||
				kidt1 == Basetype(BT_pointer)) &&
				(kidt1 == kidt2
				|| (kidt1 == Basetype(BT_pointer) &&
				    ntype(kidt2) == N_TYP_POINTER)
				|| kidt2 == Basetype(BT_pointer) ) ) {
			s_int_kid(2, tree, CC_POINTER );
			break;  /* identical pointer types */
			}
	 case N_EXP_GE:
	 case N_EXP_LE:	
		ret = Basetype(BT_boolean);
		if( check_setop( tkid1, kidt1, tkid2,
					kidt2 ) ) {
			s_int_kid(2, tree, CC_SET );
			break;	/* compatible set types */
			}
	 case N_EXP_LT:
	 case N_EXP_GT:
		ret = Basetype(BT_boolean); /* we're doing a lot of this */
		/* check the one byters */
		base1temp = comp_type(kidt1);
		base2temp = comp_type(kidt2);
		if( base1temp == base2temp && ( base1temp == Basetype(BT_char) 
				|| ntype(base1temp) == N_TYP_ENUM) ) {
			s_int_kid(2, tree, CC_BYTE );
			break;	/* compatibile enum or character types */
			}
		if( str_coerce(kidt1,tkid1)&&str_coerce(kidt2,tkid2)){
			s_int_kid(2, tree, CC_STRING );
			break; /* two string types */
			}
		/* so check the numerics */
		/* BUG:THIS MAKES A CONFUSING ERROR MESSAGE */
		s_int_kid(2, tree,
			(is_real(ch_real_coerce(2,tkid1,tkid2)) ?
			CC_REAL : CC_INTEGER) );
		break;
	 case N_EXP_IN:
		{
		extern nodep set_base_type();
		if( ntype(kidt1) == N_LIST|| (kidt2 != Basetype(BT_set) && 
			comp_type(kidt1) != set_base_type(kidt2,tkid2) ) ) 

			type_error( tree, ER(150,"in2`IN: Types of set and member don't match") );
		ret = Basetype(BT_boolean);
		}
		break;

	 case N_STUB:
		{
		extern ClassNum Stub_Exec[];
		if( !strchr( Stub_Exec, (bits8)int_kid(0,tree) ) ) {
			type_error( tree,
			 ER(151,"stub`Error - program is not finished yet") );
			}
		ret = Basetype(BT_integer);
		break;
		}
		 }
	if( ntype_info(treetype) & F_TYPE ) {
		int fkids = full_kids(treetype);
		s_node_kid( tree, fkids - 1, ret );
		}
	/* if it is a big type, turn off rvalue bit */
	/* not needed for CON_STRING as it does its own push */
	if( ret != NIL ) {
		ret_type = ntype(ret);
		if( ret_type == N_TYP_ARRAY || ret_type == N_TYP_RECORD
#ifdef TURBO
			|| ret_type == N_TYP_STRING
#endif
				)
			ourflag &= ~NF_RVALUE;
		}

	or_node_flag(tree, ourflag);
	printt1("returning %x\n", ret);
	return ret;
}

bits8 typestoget[] =  { FALSE,
FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, TRUE, FALSE,
TRUE, TRUE, FALSE, TRUE, FALSE, TRUE, TRUE, FALSE, FALSE, FALSE,
FALSE, FALSE, FALSE, TRUE };

nodep 
c_typecheck( xtree, xtheclass, interactive )
nodep xtree;
ClassNum xtheclass;
register int interactive;
{
	int save_type_error;
	int saveerrbit;
	nodep ret;
#if defined(DEBUG) && !defined(SAI)
	extern int CheckSpeed;

	if( CheckSpeed < 0 )
		return Basetype(BT_integer);
#endif
	/* no errors at all if interactive is negative */
	printt3("c_typecheck(%x, %d, %d)\n", xtree, xtheclass, interactive);
	if( interactive & TC_QUICKIE ) {
		save_type_error = had_type_error;
		saveerrbit = seterrbit;
		}
	had_type_error = interactive & TC_QUICKIE;
	seterrbit = (interactive & TC_QUICKIE )? 0 : NF_ERROR;
	tc_interactive = interactive;
	ret = typecheck(xtree, xtheclass);
	if( interactive & TC_QUICKIE ) {
		had_type_error = save_type_error;
		seterrbit = saveerrbit;
		}
	return ret;
}


/*VARARGS2*/
type_error( xtree, string, ar1, ar2 )
nodep xtree;		/* where to mark the error */
ErrorMsg string;		/* what's so bad? */
char *ar1, *ar2;
{
	register nodep tree = xtree;

	if( tc_interactive & TC_NOSTUBS && is_a_stub( tree ) ) {
		had_type_error = TRUE;
		seterrbit = FALSE;
		return;
		}
	if( !had_type_error ) {
#ifndef SAI
		if( tc_interactive & TC_MOVECURSOR )
			cursor = hidebound(tree);
#endif SAI
#ifdef MarkWilliams
		warning( "%r", &string );
#else
		warning( string, ar1, ar2 );
#endif
		}
#ifndef SAI
	if( seterrbit ) {
		or_node_flag(tree, NF_ERROR);
		mark_line( tree, SMALL_CHANGE );
		}
#else
	if( tracing ) fprintf( dtrace, "%s at %lx\n", getERRstr(string), (long)tree );
#endif SAI

	if( !had_type_error ) {
		had_type_error = TRUE;
		if( tc_interactive & TC_ONERROR )
			seterrbit = FALSE;
		}
}
/*VARARGS2*/
nodep 
ch_real_coerce( nkids, tree1, tree2 )
int nkids;	/* single or double coerce */
nodep tree1;	/* first tree to attack */
nodep tree2;	/* second if a pair */
{
	register nodep type1, type2;		/* types of input trees */
#ifndef SAI
	stackcheck();
#endif

	type1 = check_numeric(gexp_type( tree1 ), tree1);

	if( nkids > 1 ) {
		type2 = check_numeric(gexp_type( tree2 ), tree2);
		}
	 else
		type2 = Basetype(BT_real);

	if( type1 == type2 )
		return type1;
	if( is_integer(type1) && is_real(type2) ) {
		or_node_flag( tree1 , NF_RCONVERT);
		return Basetype(BT_real);
		}
	 else if( nkids > 1  && is_real( type1 ) && is_integer(type2) ) {
		or_node_flag( tree2, NF_RCONVERT);
		return Basetype(BT_real);
		}
	 else
		return type1;
}

nodep 
check_numeric( btype, tree )
nodep btype;
nodep tree;
{
	nodep base;
	base = comp_type( btype );

	if( !is_integer( base ) && !is_real(base) )
		type_error( tree, ER(152,"neednumber`Operand must be integer or real") );
	return base;
}

	/* test if a type is indeed a string type */

is_stringtype( _atype )
nodep _atype;
{
	register NodeNum sntype;
	register nodep atype = _atype;

	sntype = ntype( atype );
	return sntype == N_CON_STRING || 
#ifdef TURBO
		sntype == N_TYP_STRING || atype == Basetype(BT_char) ||
#endif
		(sntype == N_TYP_ARRAY && node_flag(atype) & NF_STRING)
		|| atype == Basetype(SP_string);
}

static
str_coerce( atype, treenod )
nodep atype;
nodep treenod;
{
	if( !atype )
		return FALSE;
	if( comp_type(atype) == Basetype(BT_char) ) {
		or_node_flag( treenod, NF_CHRSTR );
		return TRUE;
		}
	return is_stringtype( atype );
}

/* get the "n" in packed array [1..n] of char, assumes it is already a
   string type */

get_stsize( strtype, thestr )
nodep strtype;
unsigned char *thestr;			/* optional argument with the string */
{
	/* this works for turbo strings just like constants, as the length
	 * for them is in int_kid #1 (kid2)
	 */
	register NodeNum stntype;
	if( (stntype = ntype(strtype)) == N_DECL_ID )
		return thestr ? thestr[-1] + 2 : MAX_STR_LEN;
	 else if (stntype == N_TYP_ARRAY) {
#ifdef TURBO
		return int_kid(4, strtype) - 1;		/* we added 1 for [0] */
#else
		return int_kid(4, strtype);
#endif TURBO
		}
	 else
		return int_kid(1, strtype );
}

is_set_type( atype )
nodep atype;
{
	register NodeNum stype = ntype(atype);
	return stype == N_TYP_SET || stype == N_EXP_SET
		|| atype == Basetype(BT_set);
}

nodep
set_base_type( stype, stree )
register nodep stype;
register nodep stree;		/* tree where error might be */
{
	if( stype == Basetype(BT_set) )
		return Basetype(BT_integer);
	else if( ntype(stype) == N_TYP_SET ) 
		return comp_type( find_realtype( kid1(stype) ) );
	 else if( ntype( stype ) == N_EXP_SET ) {
		return comp_type( get_safe_exp_type( kid1(kid1(stype) ) ) );
		}
	 else if( stree )
		type_error( stree, ER(153,bseterr) );
	return Basetype(BT_integer);	/* what else to do */
}

nodep 
check_setop( leftkid, _lefttype, rightkid, _righttype )
nodep leftkid;		/* left child of possible set operator */
nodep _lefttype;	/* already calculated type */
nodep rightkid;		/* right child of possible set operator */
nodep _righttype;	/* already calculated type */
{
	register nodep lefttype = _lefttype;
	register nodep righttype = _righttype;
	int nsetleft, nsetright;

	/* we might call is_set_type here if we want to be les efficient */
	nsetleft = !is_set_type(lefttype);
	nsetright = !is_set_type(righttype);

	if( nsetleft && nsetright )
		return NIL;
	/* ok, we have some set types */
	if( nsetleft ) {
		type_error( leftkid ? leftkid : rightkid, ER(153,bseterr) );
		return NIL;
		}
	if( nsetright ) {
		type_error( rightkid, ER(153,bseterr) );
		return NIL;
		}
	/* so we must have dual set types */

	/* first check for "[]" of type BT_set */
	if( lefttype == Basetype(BT_set) )
		return righttype;
	if( righttype == Basetype(BT_set) )
		return lefttype;
	if( set_base_type(lefttype, leftkid) !=
			set_base_type(righttype, rightkid) )
		type_error( tparent(rightkid), ER(154,"incompatset`Set types are not compatible") );
	return lefttype;
}

	/* test for assignment compatability and return size for copying */

static
assig_compat( treelhs, _typelhs, treerhs, _typerhs )
nodep treelhs;		/* left hand side tree */
nodep _typelhs;		/* left hand side type */
nodep treerhs;		/* right hand side tree */
nodep _typerhs;		/* right hand size type */
{
	nodep compright;
	NodeNum rightcode;		/* right hand side type nodetype */
	register nodep typelhs = _typelhs;
	register nodep typerhs = _typerhs;
	int stright;	/* was string on right */


	if( !(typelhs && typerhs) )
		return FALSE;

	if( typelhs == typerhs )
		return asg_code( typelhs );


	compright = comp_type( typerhs );


	if( comp_type( typelhs ) == compright )
		return asg_code( typelhs );

	EDEBUG(5, "is_stringtype(%x)=%d\n", typelhs, is_stringtype(typelhs));
	EDEBUG(5, "is_stringtype(%x)=%d\n", typerhs, is_stringtype(typerhs));

	if( (stright = is_stringtype( typerhs )) && is_stringtype( typelhs ) ) {
		if( typelhs == Basetype(BT_char) ) {
			type_error( treerhs, ER(293,"strcast`Strings may not be assigned to \"char\"s") );
			return FALSE;
			}
		str_coerce( typerhs, treerhs );
		return CC_STRING;
		}


	if( is_integer( compright ) && is_real( typelhs ) ) {
		or_node_flag( treerhs, NF_RCONVERT);
		return CC_REAL;
		}
	rightcode = ntype(typerhs);

	/* special type checks for built in routines and NIL */

	if( typelhs == Basetype(SP_file) &&
			(rightcode == N_TYP_FILE || typerhs==Basetype(SP_file)))
		return CC_GENERIC;
	if( typelhs == Basetype(SP_ordinal) && is_ordinal(compright) )
		return CC_GENERIC;
	if( typelhs == Basetype(SP_number) && (is_integer(compright) ||
			is_real(compright) ) )
		return CC_GENERIC;
	/* pointer type compatible with all pointers and files */
	if( (ntype(typelhs)==N_TYP_POINTER && typerhs == Basetype(BT_pointer))||
		(typelhs==Basetype(BT_pointer) && (rightcode == N_TYP_POINTER ||
						rightcode == N_TYP_FILE)))
		return CC_POINTER;
	/* special case for strings */
	if( stright && typelhs == Basetype(BT_pointer) )
		return CC_STRPOINT;

	if( check_setop( treelhs, typelhs, treerhs, typerhs ) != NIL )
		if( ntype(find_realtype(kid1(typelhs))) == N_TYP_SUBRANGE )
			return CC_SET | CC_SUBRANGE;
		 else
			return CC_SET;
	if( typelhs == Basetype(SP_variable) )
		return CC_GENERIC;
	return FALSE;
}
/* get the assignment code for this guy */
int
asg_code( atype )
nodep atype;
{
	register nodep otype;
	register NodeNum ttype;
	int orval;

	orval =  ntype(atype) == N_TYP_SUBRANGE ? CC_SUBRANGE : 0;

	otype = comp_type( atype );
	ttype = ntype(otype);

	if( ttype == N_DECL_ID ) {
		if( is_integer(otype) ) {
			if( orval && int_kid(2,atype) == sizeof(char) )
				return CC_BYTE | CC_SUBRANGE;
			return orval | CC_INTEGER;
			}
		if( is_real(otype) )
			return CC_REAL;
		if( is_char(otype) )
			return orval | CC_BYTE;
		if( otype == Basetype(BT_pointer) )
			return CC_POINTER;
		}
	else {
		if( ttype == N_TYP_ENUM )
			return orval | CC_BYTE;
		if( ttype == N_TYP_SET )
			return CC_SET |
				((ntype(find_realtype(kid1(otype))) == N_TYP_SUBRANGE) ?
					CC_SUBRANGE : 0);
		if( ttype == N_TYP_POINTER )
			return CC_POINTER;
		if( is_stringtype( otype ) )
			return CC_STRING;
		 else /* must be record or array */
			return - calc_size( atype, TRUE );
		}
}


/* declaration scan routine to check types on a call */
int
argcheck_func( thedecl, parm_no, type_parent, xplist )
symptr thedecl;		/* what symbol is declared */
int parm_no;		/* which parameter */
nodep type_parent;	/* the formal node */
pint xplist;	/* actual parameter list in pint form*/
{
	/* we must set ref bit and so on */

	NodeNum partype;
	register nodep actual;		/* the actual parameter */
	listp plist = LCAST xplist;	/* parameter list */
	nodep actual_type;	/* type of actual parameter */
	type_code rout_type;	/* type of routine */
	nodep formtype;		/* type of formal param */
	symptr pname;		/* name of routine */
	extern int turbo_relax;
#ifndef OLMSG
	static char argerror[] = "argtype`This argument is not type-compatible with the formal parameter";
	static char nvstr[] = "needvar`This parameter must be a variable";
#endif


	/* if this is a builtin, thedecl will be the string and 
			type_parent will be the type */
	pname = kid_id(kid1(tparent(plist)));
	rout_type = sym_dtype( pname );
	if( parm_no >= listcount(plist) ) {
		if( rout_type <= T_USERCALL )
			type_error( tparent(plist),
				ER(155,"fewargs`Routine %s has too few arguments"), 
					sym_name(pname) );
		return ++parm_no;
		}

	actual = node_kid( plist, parm_no );
	actual_type = get_safe_exp_type(actual);
	/* this won't work quite for routine parameters */


	clr_node_flag( actual ,NF_VALMASK|NF_ISTRING);
	clr_2flag( actual, NF2_GENERIC );

	if( !thedecl || not_a_stub( thedecl ) ) {
		partype = ntype(type_parent);
		formtype = find_realtype( kid2( type_parent ) );
		if( partype == N_FORM_REF ) {

			if( not_variable( actual ) ) {
				type_error(actual,ER(156,nvstr) );
				}
			else if( rout_type <= T_USERCALL ) {
				if( actual_type != formtype && not_a_stub(formtype) ) {
					if( !(turbo_relax && is_stringtype(actual_type) && is_stringtype(formtype) ) )
					type_error( actual, ER(157,"varptype`Var actual parameter must be of identical type to the formal") );
					}
				}
			  else {
				/* built in routine will REF parameter */
				if( !assig_compat( NIL, formtype, actual,
						actual_type) )
					type_error( actual, ER(158,argerror) );
				}
			
			}
		else if( partype == N_FORM_PROCEDURE ) {
			int qdtype;
			if( ntype(actual) != N_ID ||
				((qdtype = sym_dtype(kid_id(actual)))
				!= T_PROCEDURE && qdtype != T_FORM_PROCEDURE))
				type_error(actual, ER(159,"procparam`This parameter must be a procedure") );
			}
		else if( partype == N_FORM_FUNCTION ) {
			if( ntype(actual) != N_EXP_FUNC ||
					listcount(FLCAST kid2(actual)) )
				type_error( actual, ER(160,"funcparam`This parameter must be the name of a function") );
			if( actual_type != find_realtype(kid3(type_parent)) )
				type_error( actual, ER(158,argerror) );
			}
		 else {
			register NodeNum type_ntype;
			int comtype;
			int actflags;
			/* no deals with proc params yet */
			actflags = node_flag(actual) & ~(NF_VALMASK|NF_ISTRING);
			if( formtype == Basetype(BT_anytype) ) {
				if( not_variable( actual ) && !is_ordinal(
					 actual_type) &&!is_real(actual_type)){
					type_error(actual,ER(312,"genparam`This parameter must be real, ordinal or a variable") );
					}
				or_2flag( actual, NF2_GENERIC );
				return ++parm_no;
				}
			if( comtype = assig_compat( NIL, formtype, actual,
						actual_type) ) {
				type_ntype = ntype( find_realtype(actual_type));

				/* check to see if we have a big type */
				/* sets are not included as they are
				   treated like scalars */
				if( (type_ntype == N_TYP_ARRAY ||
#ifdef TURBO
				     type_ntype == N_TYP_STRING ||
				     actual_type == Basetype(SP_string) ||
#endif
				     type_ntype == N_TYP_FILE ||
				     type_ntype == N_TYP_RECORD ||
				     type_ntype == N_CON_STRING) ) {
					if( formtype != Basetype(BT_pointer) )
					 s_node_flag(actual, actflags | NF_STPUSH);
					else
						if( comtype == CC_STRPOINT )
							s_node_flag(actual,
							 actflags|NF_ISTRING);
					}
				 else
					or_node_flag(actual,  NF_RVALUE);
				}
			 else
				type_error( actual, ER(158,argerror) );
					
			}

		}
	return ++parm_no;
}

/* check to see if of ordinal type */

is_ordinal( ttree )
register nodep ttree;
{
	register NodeNum actarg;

	actarg = ntype(ttree);
	return is_integer(ttree) || ttree == Basetype(BT_char) ||
		actarg == N_TYP_ENUM || actarg == N_TYP_SUBRANGE;
}

/* check the arguments for read */

static
rd_check( arglist, isread )
listp arglist;
int isread;		/* true if for READLN, false for write */
{
	register nodep ourfile;
	nodep inpexpr;			/* input expression tree */
	int k_ind, maxkids;		/* looping in list */
	int istext = FALSE;		/* is a text file */
	maxkids = listcount( arglist );

	/* the following code makes file of char as
	  powerful as "text".  It's a transgresion, who cares */
	/* actually, it's a nasty bug because we don't catch it writing */

	if( maxkids && is_a_file((ourfile =
			get_safe_exp_type(kid1(arglist)))) ){
		k_ind = 1;
		istext =  ourfile == Basetype( BT_text );
		ourfile = comp_type(find_realtype(kid1(ourfile)));
		}
	 else {
		k_ind = 0;
		istext = TRUE;
		ourfile = Basetype(BT_char);
		}
	for( ; k_ind < maxkids; k_ind++ ) {
		register nodep vtype;		/* variable type */

		inpexpr = node_kid( arglist, k_ind );
		if( isread )
			clr_node_flag(inpexpr,NF_VALMASK); /* not rvalues */
		vtype = comp_type(get_safe_exp_type( inpexpr ));
		/* a constant string can be an argument to readln */
		if( isread && not_variable(inpexpr) &&
				ntype(inpexpr) != N_CON_STRING )
			type_error(inpexpr,ER(161,"needvar`Read parameter must be a variable"));
		if( istext )
			if( !isread || is_integer(vtype) || is_real( vtype )
						|| is_stringtype(vtype))
				continue;
		if( vtype != ourfile )
			type_error( inpexpr, ER(162,"iotype`I/O argument is not of the same type as given file") );
		}
}

	/* check a record for consistency and correct it if possible */


	/* BUG
	 * The problem here is that the changes to kid_id are not done
	 * in a history mechanism fashion.  That means an undo without a
	 * typecheck doesn't undo these changes.  No worse than it used
	 * to be, mind you.  If we do put in a history change, we run the
	 * risk of having a big change overflow the history list with no
	 * real way to predict this in advance.  To be nice, workspace
	 * GET should typecheck any code it brings in
	 */

nodep
recheck(tree, idnode, rtype )
nodep tree;
nodep idnode;		/* id node that has a field in it */
nodep rtype;		/* record type that this is supposedly for */
{
	symptr fielddec;
	symptr newsym;
	register nodep treeup;
	register nodep recascend;

	/* scan up for record */

	fielddec = kid_id( idnode );

	for( recascend = NCAST fielddec ;
				recascend && ntype(recascend) != N_TYP_RECORD;
						recascend = tparent(recascend) )
					;
	/* we now have the first record parent */
	if( ntype(tree) == N_VF_WITH ) {
		nodep oldtree;
		nodep recst;
		oldtree = tree;
		for( treeup = tree; treeup; treeup = tparent(treeup) ) {
			if( ntype(treeup) == N_ST_WITH && kid1(treeup)
							!= oldtree ) {
#ifdef SAI
				if( recascend==gexp_type(kid1(treeup))){
					s_kid2( tree, treeup );
					return sym_type( fielddec );
					}
#else
				recst = find_r_symtab(treeup);
				newsym = fielddec;
				if( recst == recascend || (newsym =
					  search_symtab( recst,
					  sym_name(fielddec),
					  sym_nhash(fielddec),FALSE )) ) {
					s_kid_id( idnode, newsym );
					s_kid2( tree, treeup );
					return sym_type(newsym);
					}
#endif
				}
			oldtree = treeup;
			}
		}
	 else {
#ifndef SAI
		newsym = fielddec;
		if( rtype == recascend || (newsym = search_symtab( tree,
			sym_name(fielddec), sym_nhash(fielddec), FALSE ) ) ) {

			s_kid_id( idnode, newsym );
			return sym_type(newsym);
			}
#else
		if( rtype == recascend )
			return sym_type(fielddec);
#endif	
		}
	
	type_error( tree, ER(163,"recmatch`Field name %s no longer matches the record"),
			sym_name( fielddec ) );
	return sym_type( fielddec );
}

static
callcheck( xtree )
nodep xtree;
{
	nodep thedec;
	register nodep tree = xtree;
	listp arglist;
	symptr therot;
	int argcheck_func();

	if( is_a_stub( kid1(tree) ) )
		return;
	therot = kid_id(kid1(tree));
	thedec = tparent( therot );
	rcheck_type = sym_dtype( therot );
	arglist = FLCAST kid2(tree);
	if( rcheck_type == T_READLN || rcheck_type == T_WRITELN ) {
		rd_check( arglist, rcheck_type == T_READLN );
		}
	else if( rcheck_type > T_USERCALL ) {
		/* built in routine */
		rout_np built_rot;
		built_rot = (rout_np )thedec;
		if( built_rot ) {
			/* PTS is a macro for -> or -BRC under qnx */
			if( listcount(arglist)< PTS(built_rot, min_params,realrout_np))
				type_error(tree, ER(164,"fewargs`Too few arguments, should be at least %d"), PTS(built_rot , min_params, realrout_np) );
			else if( listcount(arglist) > PTS(built_rot,max_params,realrout_np))
				type_error(tree, ER(165,too_many), PTS(built_rot,max_params,realrout_np));
			else if( PTS(built_rot, r_decls,realrout_np) )
				scan_decl( FLCAST PTS(built_rot, r_decls,rout_node far *),
				 argcheck_func, 0, FALSE, 0, (pint)arglist );
			}
		}
		else if( thedec ) {
			static int formcount;
			formcount = scan_decl( FLCAST kid2(thedec), argcheck_func,
				0, FALSE, 0, (pint)arglist );
			if( formcount < listcount(arglist) )
				type_error( tree, ER(165,too_many), formcount );
			}
}
/*
 * This is here because the #include "class.h" was preventing runtime.c
 * from compiling.  It was probably in runtime.c because it was preventing
 * interp.c from compiling...
 */
 /*
  * Table of stub classes that can exist without hurting the executability
  * of the program
  */
ClassNum Stub_Exec[] = { C_STATEMENT, C_COMMENT, C_CASE, 0 };
		
#else

nodep 
typecheck( tree, theclass ) {}
type_error() {}
#endif HAS_TYPECHECK
