/*
 * Routines to do type comparisons, checks and manipulations
 */

#define INTERPRETER 1

#include "alice.h"
#include "flags.h"
#include "interp.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "typecodes.h"


/*
 * Routine gets the type pointer associated with a declaration
 * subtree.  It scans up until it finds something with a type
 * child and points to that
 * returns nil if nothing can be found or if given nil to start
 * This routine is used when a symbol is declared 
 */


#ifndef OLMSG
char foref_str[] = "foref`Symbol used before it is defined";
#endif


nodep 
decl_typetree(xdecl )
symptr xdecl;		/* where we begin */
{
	register symptr decl = xdecl;
	register nodep dparent;
	nodep ret;

#ifdef DB_TYPES
	if(tracing)fprintf(trace, "Look for type tree of %x\n", decl );
#endif
	dparent = tparent(decl);	/* we'll be using this a bit */
	if( decl ) switch( sym_dtype(decl) ) {
		case T_VAR:
		case T_FORM_VALUE:
		case T_FORM_REF:
		case T_ABSVAR:
		case T_FIELD:	 /* will this work for tag-fields? */
			if( is_a_list( dparent ) ) {
				ret = kid2(tparent(dparent));
				break;
				}
		case T_TYPE:
		case T_INIT:
		case T_CONST:
			ret = kid2(dparent);
			break;
		case T_BTYPE:
			ret = NCAST decl;
			break;
		case T_BBTYPE: /* built in constructed type (boolean) */
		case T_BFUNC:
		case T_BCONST:
		case T_BENUM:
			ret = sym_type(decl);
			break;
			
		case T_FUNCTION:
		case T_FORM_FUNCTION:
			ret = kid3(dparent);
			break;
		case T_UNDEF:
			return Basetype(BT_integer);
		case T_FORM_PROCEDURE:
		case T_PROCEDURE:
			return Basetype(SP_proc);
		case T_FILENAME:
			ret = Basetype(BT_text);
			break;
		case T_ENUM_ELEMENT:
			return tparent(dparent);
		default:
			error( ER(166,"Type tree of unknown symbol type %d"), sym_dtype(decl) );
		}
	return ntype(ret) == N_ID ? NCAST kid_id( ret ) : ret;
	
}

nodep 
find_realtype(xstart )
nodep xstart;		/* again where we begin */
{
	register nodep f_real_node;
	NodeNum sntype;
	type_code sdtype;
	int kflags;

	f_real_node = xstart; 
	while( TRUE ) {
		sntype = ntype(f_real_node); 
		if( sntype == N_ID ) {
			symptr kidguy = kid_id(f_real_node);
			kflags = sym_mflags(kidguy);
			clr_node_flag(f_real_node,NF_ERROR);
			if( kflags & SF_DEFOERR || ( !(kflags & SF_DEFYET ) &&
					sym_dtype(kidguy) != T_UNDEF) ){
				or_sym_mflags(kidguy,SF_DEFOERR);
				type_error( f_real_node, ER(167,foref_str) );
				break;
				}
			f_real_node = NCAST kidguy;
			}
		else if( sntype == N_DECL_ID  &&
			(sdtype = sym_dtype((symptr)f_real_node)) != T_BTYPE  &&
				sdtype != T_UNDEF && !( sym_mflags(
				(symptr)f_real_node) & SF_DEFOERR ) )
			f_real_node = decl_typetree( (symptr)f_real_node );
		else if( sntype == N_TYP_PACKED )
			f_real_node = kid1( f_real_node );
		else
			break;
		}
#ifdef DB_TYPES
	if(tracing)fprintf(trace, "Find_realtype returns %x from %x\n", f_real_node, xstart );
#endif
	return f_real_node;
}

/* get the bounds of a type */

#ifdef OLMSG
char bsmsg[] = "bigstruct`Array too large -- shrink the bounds";
#endif

/* This routine returns the ordinal number of the first or last elements
   of a type if this is appropriate.  0 if not defined or infinite */

int
typ_bound(typetree, lastflag )
nodep typetree;
register int lastflag;	/* one if want last, zero if first */
{
	register nodep tree;

	tree = find_realtype( typetree );
	if( is_a_symbol(tree) )
		return lastflag ? sym_ivalue((symptr) tree) : 
		  (is_integer(tree) ? -MAXINT : 0) ;

	switch( ntype(tree) ) {
		case N_TYP_ENUM :
			return lastflag ? listcount( FLCAST kid1(tree) ) - 1: 0;
		case N_TYP_SUBRANGE :
			return (int)eval_const(node_kid( tree, lastflag ), FALSE );
		case N_TYP_PACKED:
			return typ_bound( kid1(tree), lastflag );
#ifdef DEBUG
		case N_TYP_ARRAY:
#ifdef TURBO
		case N_TYP_STRING:
#endif
		case N_TYP_SET:
		case N_TYP_FILE:
		case N_TYP_POINTER:
		case N_TYP_RECORD:
		case N_STUB:
			return 0;
		default:
			error( ER(168,"typ_bound:Funny node %d in typetree %x"), ntype(tree), tree);
#else
		default: return 0;
#endif
		}

	
	return 0;
}

/* figure the size of the type described by a typetree */

/* later on, this routine can be optimized into two, one that calls realtype,
   and the other that grabs the pre-calculated size out of named types */
asize
calc_size( typetree, runtime )
reg nodep typetree;
int runtime;		/* is this run or "compile" time */
{
	register nodep tree;
	asize stemp;		/* temporary in getting compound type size */
	int qindex;		/* for looping through lists */
	extern int tc_interactive;

	listp compound;
	framesize fd_func();
#ifndef SAI
	stackcheck();
#endif
	
	if( runtime )
		tree = typetree;
	 else
		tree = find_realtype( typetree );

	clr_node_flag(tree, NF_ERROR);

	switch( ntype(tree) ) {
		case N_ID:
			tree = (nodep )kid_id(tree);
			if( ntype( typetree) != N_DECL_ID ) {
				if( sym_dtype( (symptr)tree ) == T_UNDEF )
					type_error( typetree, ER(169,"undeft`Undefined type name") );
				if(!(sym_mflags((symptr)tree) & SF_DEFYET ) ) {
					type_error( typetree, ER(167,foref_str) );
					or_sym_mflags((symptr)tree, SF_DEFOERR);
					}
				}
#ifdef DB_TYPES
			if(tracing)fprintf(trace,"Calc-size of id %x\n", tree);
#endif
		case N_DECL_ID : /* base type, size stored inside it */
			return sym_size( (symptr) tree );
		case N_TYP_ENUM :
			if( !runtime ) {
			   compound = FLCAST kid1(tree);
			   for( qindex = 0; qindex<listcount(compound);qindex++) {
				symptr ourdecl;
				ourdecl = (symptr) node_kid(compound,qindex);
				if( not_a_stub(ourdecl) ) {
					s_sym_value(ourdecl, (pint)qindex );
					s_sym_type(ourdecl, tree );
					or_sym_mflags(ourdecl,SF_DEFYET);
					s_sym_size(ourdecl, sizeof(char) );
					/* allow decl */
					s_sym_scope(ourdecl,1);
					}
				}
			   }
			return sizeof(char);/* only 256 elements per enum */
		case N_TYP_SUBRANGE :
			/* use the size of the master type of subrange,
			   as detirmined by the type of the first constant */
			/* check to see types match etc */
			if( !runtime ) {
				int lowbound;
				unsigned rsize;
				int srbytes;
				rsize = range_check( tree, &lowbound );
				if( lowbound >= 0 && (unsigned)lowbound + rsize <= 256 )
					srbytes = sizeof(char);
				 else
					srbytes = calc_size( gexp_type(
						kid1( tree ) ), runtime ); 
				s_int_kid( 2, tree, srbytes );
				}	
			return int_kid(2,tree);
		case N_TYP_PACKED:
			return calc_size( kid1(tree), runtime );
#ifdef TURBO
		case N_TYP_STRING:
			if( runtime )
				return 2+int_kid( 1, tree );
			 else {
				register int arsize;
				arsize = (int)eval_const( kid1(tree), FALSE );
				if( arsize < 1 || arsize > MAX_STR_LEN )
					type_error( tree, ER(290, "bigstruct`String length out of bounds 1..255"));
				s_int_kid( 1, tree, arsize );
				return 2+arsize;
				}
#endif
		case N_TYP_ARRAY:
			{
			register long arsize;
			int isstr = FALSE;
			/* multiply size of constituent by cardinality of
			   index type */
			/* also store bounds and size information */
			if( !runtime ) {
				int lowbound;
				nodep component = kid2(tree);
				int upperb;
				/* do calc size in case of enum **/
				calc_size( kid1(tree), runtime );
				upperb = range_check( kid1(tree), &lowbound );
				if( upperb < 0 )
					type_error( tree, ER(277,bsmsg) );
				s_int_kid(4, tree, upperb );
				stemp = calc_size( component, runtime );
				s_size_kid( 2, tree, stemp );
					/* set multiplier */
				if( lowbound == 1 && ntype(tparent(tree)) ==
					N_TYP_PACKED && ntype(component) == N_ID
					&&kid_id(component)==Basetype(BT_char)){
					/* BUG, should check for integer 
					 * subrange type
					 */

					isstr = TRUE;
					or_node_flag( tree, NF_STRING );
#ifdef TURBO
					lowbound  = 0;
					s_int_kid(4, tree, 1 + int_kid(4, tree));
#endif
					}
				 else
					clr_node_flag( tree, NF_STRING );
				s_int_kid(3, tree, lowbound );
				}
			 else {
				stemp = size_kid( 2, tree );
				isstr = (node_flag(tree) & NF_STRING) != 0;
				}
			/* add one to size of array of bytes for zero in str */
			/* also add one for size byte */

			arsize = isstr + (long)stemp * (long)int_kid(4,tree);

			if( arsize > MAX_OBJ_SIZE ) {
				type_error( tree, ER(277,bsmsg) );
				arsize = 2;
				}
			return (asize) arsize;
			}

			/* this will return 0 in an error */
		case N_TYP_SET:
			/*sets are always 256 elements.  Period */
			/*stemp = 1 + typ_bound(kid1(tree),1)
				- typ_bound(kid1(tree),0);
			return ( stemp + BITS_IN_BYTE - 1 ) / BITS_IN_BYTE;
			*/
			if( !runtime ) {
				int low, high;
				register nodep settype = kid1(tree);
				if( not_a_stub(settype) &&
					  !is_ordinal(find_realtype(settype)) )
					type_error( tree, ER(127,"setord`Set elements must be of ordinal type") );
				calc_size( settype, runtime );
				high = range_check( settype, &low );
				if( low < 0 || low + high > 256 )
					type_error( tree, ER(170,"badset`Set elements must be in the range 0..255") );
				}
		case N_EXP_SET:	/* type of built on the fly set */
			return SET_BYTES;
		case N_TYP_POINTER:
			{
			type_code std;
			nodep idn = kid1(tree);
			symptr kidn;

			if( not_a_stub( idn ) ) {
				kidn = kid_id(idn);
				std = sym_dtype( kidn );
				if( !(std == T_TYPE || std == T_BTYPE || std == T_BBTYPE))
					type_error(tree,ER(171,"nottype`%s must be a type name"), sym_name(kidn) );
				}
			}
		case N_CON_STRING:	/* passed to "pointer" */
			return sizeof(pointer);
		case N_TYP_FILE:
			{
			nodep btype;
			nodep comtyp;
			asize ctsize;
			comtyp = kid1(tree);
			if( is_a_stub(comtyp) )
				return sizeof(struct pas_file);
			btype = find_realtype( comtyp );
			if( ntype(btype) == N_TYP_FILE || btype == Basetype(BT_text))
				type_error(tree, ER(319,"fof`Can't have a file of files" ) );
			ctsize = calc_size( comtyp, runtime );
			if( ctsize > MAX_OBJ_SIZE - sizeof(struct pas_file) ) {
				type_error( tree, ER(277,bsmsg) );
				ctsize = 2;
				}
			return sizeof(struct pas_file) - sizeof(char) + ctsize;
			}
		case N_TYP_RECORD:
			if( !runtime && kid1(tree) ) {
				framesize recsize;
				recsize = scan_decl( FLCAST kid1(tree), fd_func,
					(framesize)0,FALSE,NF_ERROR, (pint)0 );
				if( recsize > MAX_OBJ_SIZE ) {
					type_error( tree, ER(277,bsmsg) );
					recsize = 2;
					}
				s_size_kid( 2, tree, (asize)recsize );
				}
			return size_kid(2, tree );
		case N_STUB:
			type_error( tree, ER(172,"tystub`Placeholder in type declaration") );
			break;
		default:
			error( ER(173,"bug`Calc_size:Bad Declaration %d in typetree %x"), ntype(tree), tree);
		}
	
	return 0;
}

/* get the type of an expression or constant tree */

nodep
get_damn_exp_type(etree)
nodep	etree;
{
	nodep	ret	= gexp_type(etree);

	if (!ret)
		ret = c_typecheck(etree, 0, TC_QUICKIE);

	return ret;
}

nodep
get_safe_exp_type(etree)
nodep etree;
{
	nodep ret = gexp_type( etree );

	return ret ? ret : Basetype(BT_integer);
}
			

nodep 
gexp_type(xetree )
nodep xetree;
{
	register nodep etree = xetree;
	NodeNum ontype;
	register nodep tret;

	ontype = ntype(etree);
	if( ntype_info(ontype) & F_TYPE )
		return node_kid( etree, full_kids(ontype)-1 );
	switch( ontype ) {

		case N_EXP_ERROR:	/* nothing better to do */

		case N_EXP_DIV:
		case N_EXP_MOD:
		case N_CON_INT:
#ifdef TURBO
		case N_EXP_SHL:
		case N_EXP_SHR:
#endif
		case N_STUB:
			return Basetype(BT_integer);
		case N_CON_REAL:
		case N_EXP_SLASH:
			return Basetype(BT_real);
		case N_CON_CHAR:
			return Basetype(BT_char);
		case N_CON_STRING:
			return etree;
		case N_EXP_SET:
			/* the constant node with length it its own type */
			/* set constructor nodes are their own type */
			if( listcount( FLCAST kid1(etree) ) )
				return etree;
			 else
				return Basetype(BT_set);
		case N_EXP_NIL:
			return Basetype(BT_pointer);
		/* case N_EXP_ESET:
			return Basetype(BT_set); for later deletion */
		case N_CON_PLUS:
		case N_CON_MINUS:
		case N_OEXP_2:
		case N_OEXP_3:
		case N_EXP_PAREN:
		case N_EXP_UPLUS:
		case N_EXP_UMINUS:
		/* case N_SET_ELEMENT: */
			return gexp_type( kid1(etree) );
		case N_EXP_FUNC:
			{
			nodep thekid;
			thekid = kid1(etree);
			if( not_a_stub(thekid) ) {
				tret = sym_type( kid_id(kid1(etree)) );
				if( tret == Basetype(SP_passtype) ) {
#ifdef TURBO
					extern int turbo_flag;
					/* KLUDGE, -54 is random() */
					if( sym_saveid( kid_id(kid1(etree)) ) == -54 ) {
						if( turbo_flag && !listcount(kid2(etree)) )
							tret = Basetype(BT_real);
				 		 else
							tret = Basetype(BT_integer);
						}
			 	 	 else
#endif
						return gexp_type(kid1(kid2(etree)));
					}
				return tret;
				}
			 else
				return Basetype(BT_integer);
			}
		case N_ID:
		case N_VF_WITH:
			return sym_type( kid_id(etree) );
#ifndef TURBO
		case N_EXP_NOT:
		case N_EXP_OR:
		case N_EXP_AND:
#endif
		case N_EXP_EQ:
		case N_EXP_NE:
		case N_EXP_LT:
		case N_EXP_LE:
		case N_EXP_GT:
		case N_EXP_GE:
		case N_EXP_IN:
			return Basetype(BT_boolean);
		/* the rest of the cases are not important here */
		}
	return NIL;
}

pint
eval_const(xcontree, xrealcon )
nodep xcontree;
int xrealcon;		/* flag that says we may have string or real constant */
{
	register nodep contree = xcontree;
	register int realcon = xrealcon;
	extern int got_minus;
#ifndef SAI
	stackcheck();
#endif
#ifdef DB_TYPES
	if(tracing)fprintf(trace,"Evaluate constant at %x\n", contree );
#endif
	clr_node_flag(contree,NF_ERROR);
	switch( ntype( contree ) ){
		case N_CON_INT:
			if( node_flag(contree) & NF_BADCONST )
				type_error( contree, ER(275,
				"badint`Integer is not in range -32768..32767"));

			return (pint)int_kid(1, contree);
		case N_CON_MINUS:
		case N_EXP_UMINUS:
			if( realcon ) {

				got_minus ^= 1;
				return eval_const( kid1(contree), realcon );
				}
			 else
				return -eval_const( kid1(contree ), realcon );
		case N_CON_CHAR:
			return (pint)(unsigned)int_kid(0,contree);
		case N_CON_PLUS:
		case N_EXP_UPLUS:
			return eval_const( kid1(contree), realcon );
		case N_CON_STRING:
			if( realcon )
				return (pint) (1+(pointer)kid1(contree));
		case N_CON_REAL:
			/* THIS CODE IS NOT VERY PORTABLE */
			/* IT ASSUMES WE CAN HIDE OUR POINTER IN A PINT */
			/* it also assume the float starts at kid2 */
			if( realcon )
				return sizeof(nodep) + (pint)(nodep)kid1adr(contree);
		default:
			type_error( contree, ER(174,"badconst`An ordinal constant is expected here") );
			return (pint)0;
		case N_ID:
			/* of T_CONST only */
			{
			type_code idtype;
			register symptr consym;

			clr_node_flag(contree, NF_ERROR);
			consym = kid_id(contree);
			if( !(sym_mflags(consym) & SF_DEFYET )) {
				type_error( contree, ER(167,foref_str) );
				or_sym_mflags( consym, SF_DEFOERR );
				return (pint)0;
				}
			idtype = sym_dtype(consym);

			/* if( idtype == T_CONST ) return eval_const( find_realtype(NCAST consym ),
						realcon ); else */
			if( idtype == T_ENUM_ELEMENT ||idtype == T_BCONST||
					idtype == T_CONST || idtype == T_BENUM){
				if( sym_mflags(consym) & SF_NEGREAL )
					got_minus ^= 1;
				return (pint)sym_value( consym );
				}
			else
				type_error(contree, ER(175,"needconstname`This name is not a constant") );
			}
		}
	return (pint)0;
}
/* scanning function for a field.  Evaluates the size AND compiles
   the declarations  MUST BE REENTRANT! */

framesize
fd_func(xthedecl, so_far, xtype_parent )
symptr xthedecl;		/* what symbol is declared */
framesize so_far;		/* how many bytes so far */
nodep xtype_parent;	/* the N_FIELD node */
{
	register symptr thedecl = xthedecl;
	register nodep type_parent = xtype_parent;
	register asize howbig;
	nodep ptt, rptt;	/* parent type tree */
	listp varlist;
	framesize nsf;	
	int i;
	extern framesize do_allign();
	extern asize calc_size();

	if( is_a_stub( type_parent ) )
		return so_far;
	ptt = kid2(type_parent);
	rptt = find_realtype(ptt);

	if( not_a_stub( thedecl ) ) {
		howbig = calc_size( ptt, FALSE );
		so_far = do_allign( so_far, howbig );
		s_sym_size( thedecl, howbig );
		or_sym_mflags(thedecl, SF_DEFYET);
		s_sym_type( thedecl, rptt );
		s_sym_offset( thedecl, so_far );
		/* for decl */
		s_sym_scope( thedecl, 1 );
		so_far += howbig;
		if( so_far > MAX_OBJ_SIZE ) {
			type_error( type_parent, ER(277,bserr) );
			so_far -= howbig;
			}
		}
	if( ntype(type_parent) != N_VARIANT )
		return so_far;
	varlist = FLCAST kid3(type_parent);
	rptt = comp_type(rptt);

	/* now scan the variant and take the max size */
	nsf = so_far; /* current max size */
	for( i = 0; i < listcount(varlist) ; i++ ) {
		nodep varnode;
		listp conlist;
		framesize thisone;	/* size with this variant */
		int conscan;

		varnode = node_kid(varlist, i );
		/* We should check that all constants match the tag type */
		/* also that there are no duplications etc. */
		if( is_a_stub( varnode ) || is_a_comment(varnode) )
			continue;
		conlist = FLCAST kid1(varnode);
		for( conscan = 0; conscan < listcount(conlist); conscan++ ) {
			nodep ctree;
			if( gexp_type( ctree = node_kid(conlist,conscan) ) != rptt )
				type_error(ctree, ER(176,"varcon`Constant is not of the right type") );
			}
		/* what about overflow? */
		thisone = scan_decl( FLCAST kid2(varnode), fd_func, so_far,
				FALSE, NF_ERROR, (pint)0 );
		if( thisone > nsf )
			nsf = thisone;

		}
		
	return nsf;
}

	/* get the type used for compat checks */

/* for subranges, it is the master base type
 * for sets it is the master base type again
 * for enumerateds,
 * and all other types it is the type itself 
 */

nodep 
comp_type(xatype )
nodep xatype;
{
	register nodep atype = xatype;

	if( atype ) {
		if( ntype(atype) == N_TYP_SUBRANGE )
			return gexp_type( kid2(atype) );
		else if( ntype(atype) == N_LIST ) /* constructed set */
			return comp_type(gexp_type( kid1(atype) ));
		}
	return atype;
}

/* check to see if a subrange is ok */
range_check(xtree, lower )
nodep xtree;		/* subrange tree to check */
int *lower;		/* lower bound of range if interested */
{
	register nodep tree = xtree;
	int mlow, mup;	/* bounds */
	clr_node_flag(tree, NF_ERROR);

	if( ntype(tree) != N_TYP_SUBRANGE ) {
		nodep rtree;
		register NodeNum actarg;
		rtree = find_realtype(tree);
		actarg = ntype(rtree);
		if( !(rtree == Basetype(BT_char) || actarg == N_TYP_ENUM
					|| actarg == N_TYP_SUBRANGE) ) {
			type_error( tree, ER(292, "badrtype`This is not a valid type for a range" ) );
			return 1;
			}
		if(lower)
			mlow = *lower = typ_bound( tree, 0 );
		return 1+typ_bound( tree, 1 ) - mlow;
		}

	mlow = eval_const( kid1(tree), FALSE );
	mup = eval_const( kid2(tree), FALSE );
	if(gexp_type(kid1(tree))!=gexp_type(kid2(tree))) {
		type_error( tree, ER(177,"rangebounds`The two bounds of this range don't match") );
		return 0;
		}
	if( mlow > mup )
		type_error( tree, ER(178,"rangeorder`Range is in the wrong order") );
	if( lower )
		*lower = mlow;
	return 1 + mup - mlow;
}
	/* adjust allignment for processor */

framesize
do_allign( offset, objsize )
framesize offset;
asize objsize;		/* how big object is */
{
#ifdef QNX		/* actually 8086 */
	return offset;
#else
	return offset + (objsize > 1 ? (offset & 1) : 0);
#endif
}
