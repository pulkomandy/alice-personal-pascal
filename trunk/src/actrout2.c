/*
 * Action routines, part 2
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "token.h"
#include "typecodes.h"
#include "class.h"
#include "flags.h"

#ifdef DB_ACTROUT
#define PRINTT
#endif
#include "printt.h"

extern char tokentext[];

/*
 * Handle a constant typed on an absolute memory address placeholder
 */

static type_code Can_Abs[] = { T_VAR, T_FORM_VALUE, T_FORM_REF,
		T_ABSVAR, T_INIT, T_FILENAME, T_UNDEF,0};

abscon(tn)
bits8 tn;
{
	register symptr typedname;
	register type_code idtype;
	/* look up tokentext in the symbol table. */
	typedname = slookup(namestring(tokentext), NOCREATE, (symptr)NULL, MOAN, NIL);

	/* if it isn't an ID that can be referenced absolute, make a colon
	   absolute address node */
	if( !(tn == N_ID && strchr( Can_Abs, sym_dtype(typedname) ) ) )
		exp_prod( N_OEXP_2 );	/* colon node */
	contok(tn);
}
	
/* We're on a constant and the user typed various things */
/* 
 * This routine is also used to handle comments.
 */
contok(tn)
ireg1 bits8 tn;
{
	
	switch(tn) {
	case TOK_INT:
	case TOK_NUMB:
	case TOK_CHAR:
	case TOK_STRING:
	case TOK_CMNT:		/* not a constant but treated like one */
		do_const(tn);
		break;
	case TOK_PLUS:
		exp_prod(N_CON_PLUS);
		break;
	case TOK_MINUS:
		exp_prod(N_CON_MINUS);
		break;
	case TOK_ID:
		exp_prod(N_ID);
		break;
	default:
		bug( ER(241,"contok: funny constant token %d"), tn );
	}
}

/* This is like do_exp_prod, except it knows enough to call s_conval on
 * the constant to set up what values are required 
 */
do_const(cntype)
ireg1 bits8 cntype;
{
	do_exp_prod( cntype, 0 );
	/* 0 probably should really depend on something somewhere */
	s_conval( cursor, cntype, tokentext );
	skip_down();
}

/* s_conval moved to common.c */

/*
 * This routine is called when we are on a general <<TYPE>> placeholder and
 * and ID is typed.  Based on the nature of the ID, we must expand different
 * forms of types.  For example, constants require subranges etc.
 */
typid()
{
	preg1 symptr typedname;
	/* look up tokentext in the symbol table. */
	typedname = slookup(namestring(tokentext), NOCREATE, (symptr)NULL, MOAN, NIL);
	switch(sym_dtype(typedname)) {
	case T_TYPE:
	case T_BTYPE:
	case T_BBTYPE:
		exp_prod(N_ID);
		break;
	case T_CONST:
	case T_BCONST:
	case T_ENUM_ELEMENT:
	case T_BENUM:
		exp_prod(N_TYP_SUBRANGE);
		exp_prod(N_ID);
		break;
	case T_UNDEF:
		exp_prod(N_ID);
		break;
	default:
		error( ER(4,"nottype`%s is not a type name, you can't put it here"),
			tokentext );
	}
}

/* We're on a type and the user typed something that indicates a subrange */
typsr(tn)
int tn;
{
	exp_prod(N_TYP_SUBRANGE);
	contok(tn);
}

/*
 * This is a common routine that is called whenever and ID is typed on
 * a statement placeholder.  We do all sorts of productions based on the
 * type of the ID.
 * Most common is an assignment statement, but others include procedure calls,
 * labels etc.
 */
stid()
{
	ireg1 int c;
	preg1 symptr typedname;

	/* we must check all "with" record symbol tables too... */

	/* Look up tokentext in the symbol table */
	typedname = slookup(namestring(tokentext), NOCREATE, NIL, MOAN, NIL);
	switch(sym_dtype(typedname)) {
#ifdef Savecode
	case T_VAR:
	case T_INIT:
	case T_FUNCTION:
	case T_FORM_VALUE:
	case T_FORM_REF:
	case T_UNDEF:
	case T_FIELD:
	case T_ABSVAR:
#endif
	default:

		exp_prod(N_ST_ASSIGN);
		break;
	case T_BTPROC:
	case T_WRITELN:
	case T_READLN:
	case T_BPROC:
	case T_PROCEDURE:
	case T_FORM_PROCEDURE:
		expCall(typedname, N_ST_CALL);
		break;
	case T_LABEL:
		lblst();
		return;
	}
	exp_prod(N_ID);
}

/*
 * When you type a procedure ID, we want to build out the right number of
 * stubs for the parameters.  This does it
 */

expCall(sp, ctype)
NodeNum ctype;
symptr	sp;
{
	preg1 nodep np;
	listp paramStubs();

	np = tree(ctype, NIL, paramStubs(sp));
	fresh_stub(np, 0);
	graft(np, cursor);
	cursor = kid1(np);
}


/* 
 * double production for a label, when a label ID was typed in
 */
lblst()
{
	exp_prod(N_ST_LABEL);
	exp_prod(N_ID);
}
#ifndef OLMSG
char fld_msg[] = "badfield`%s is not a valid field of that record";
#endif
/* we have a field id */
do_fdid()
{
	preg1 symptr typedname;
	preg2 nodep rectype;

	if( ntype(cursparent) == N_FLD_INIT ) 
		rectype = kid2( find_true_parent(cursparent) );
	 else
		rectype = gexp_type( kid1(cursparent) );
	printt1("rectype=%x\n", rectype);
	if( ntype(rectype) != N_TYP_RECORD ) {
		error( ER(5,"baddot`field entry requires a record for context") );
		}
	typedname = slookup(namestring(tokentext), NOCREATE, sym_kid(rectype),
				NOMOAN, NIL);
	if( sym_dtype(typedname) == T_FIELD ) {
		exp_prod( N_ID );
		}
	 else {
		warning( ER(189,fld_msg), tokentext );
		}
}


/* We're on an expression and the user typed an ID */
/* This actually doesn't happen very much because the expression parser
 * handles most of these things
 */
expid()
{
	preg1 symptr typedname;
	ireg1 type_code dtype;
	/* Look up tokentext in the symbol table. */

	/* we must check all "with" record symbol tables too... */
	typedname = slookup(namestring(tokentext), NOCREATE, NIL, MOAN, NIL);

	dtype =sym_dtype(typedname);
	if( dtype == T_FUNCTION || dtype == T_BFUNC || dtype == T_BTFUNC
		|| dtype == T_FORM_FUNCTION )
		expCall( typedname, N_EXP_FUNC );
	exp_prod(N_ID);

}


/* realcp - the first child of a list if on a list */

curspos
realcp(xcp)
curspos	xcp;
{
	preg1 curspos cp;
	cp = xcp;

	while( ntype_info(ntype(cp)) & F_NOSTOP )
		cp = kid1(cp);
	return cp;
}

/* routine to handle a typed in declaration word */
/* 
 * This is the routine that knows where to put a "var", "const" or "procedure".
 * It scans up for the declaration block and figures a good place to put
 * your new declaration.  If you type your keyword on a declaration placeholder,
 * the new block always goes there.  Otherwise, we search to see if there
 * already is a block of the type required.  If not, we make one in the place
 * that Pascal would insist.
 */
static bits8 decl_map[] = {
TOK_LABEL,  255,
TOK_CONST,  N_CONST_DECL,
TOK_TYPE,  N_TYPE_DECL,
TOK_VAR,  N_VAR_DECL,
TOK_PROCEDURE,  0,
TOK_FUNCTION,  0,
N_IMM_BLOCK, 0,
0 };

/*
 * This has changed since we allow multiple consts, types, vars,
 * etc, in arbitrary order to compensate for problems with
 * hidden declarations and also libraries which may want their
 * own declarations.
 */
go_declare(tok, blocknode )
bits8 tok;		/* token the user typed */
nodep blocknode;	/* block to hang declaration off */
{
	bits8 *tokent;
	register listp decl_list;	/* list of declarations */
	register nodep decblock;    /* block of declarations as we like 'em */
	bits8 glob_decl;	/* what kind of declaration we look for */
	int num_decl;		/* number of declaration types */
	register int i;		/* loop through listed declaration classes */
	int decstub;		/* on a declarations stub */

	if ((tokent = table_lookup( decl_map, tok, 2)) != NULL)
		glob_decl = tok;
	else
		bug(ER(242,"Invalid declaration token %d"), tok);

	markPopBack(cursor);

	decl_list = decl_kid(blocknode);
	num_decl = listcount(decl_list);
	decstub = is_a_stub(cursor) && int_kid(0,cursor) == C_DECLARATIONS;

	if (glob_decl >= N_DECL_PROC) {
		/*
		 * Declaration of procedure or function.  If the cursor
		 * is on a declaration stub, add the function there, otherwise
		 * add it to the end of the declaration list.
		 */
		if (!decstub) {
			cursor = node_kid(decl_list, num_decl-1);
			if (not_a_stub(cursor))
				exp_list(1);
		}
		dec_block(glob_decl);
		return;
	}

	/*
	 * Declaration of a label, const, type or var.
	 * First sweep backwards over the existing declarations for a
	 * declaration of our type, in which case we add our
	 * declaration to the end of that declaration's list.
	 */
	 if( !decstub ) {
		for (i = num_decl - 1; i >= 0; i--) {
			decblock = node_kid(decl_list, i);
			if (is_a_stub(decblock) || is_a_hide(decblock))
				continue;
			if (ntype(decblock) == glob_decl) {
				/* found our type of declaration */
				decl_list = kid1(decblock);
				cursor = node_kid(decl_list,
						  listcount(decl_list)-1);
				exp_list(1);
				goto final_expand;
			}
		}
		/*
		 * The first sweep failed, so that there do not exist
		 * any declarations of our type.  Search the declarations
		 * backwards for a decl that should occur earlier than
		 * our declaration, and create our declaration immediately
		 * after it.
		 */
		for (i = num_decl - 1; i >= 0; i--) {
			decblock = node_kid(decl_list, i);
			if (is_a_stub(decblock) || is_a_hide(decblock))
				continue;
			if (ntype(decblock) < glob_decl)
				break;
		}
		cursor = decblock;
		printt2( "Decblock is %x, kid %d on prep to expand\n", decblock, i );
		if (not_a_stub(cursor)) {
			printt1( "Expanding list from %x", cursor );
			exp_list(i >= 0);
			}
		}
	exp_prod(glob_decl);

final_expand:
	/* labels are simple */
	if (glob_decl == N_DECL_LABEL)
		return;
	exp_prod(*tokent);
}

/*
 * When you type a comma in a list of things like arguments, and the next
 * item in the list is a stub, you don't want to make an extra stub, you
 * just want to go there.  This does that action.
 */

comma_expand(np)
nodep np;
{
	preg1 nodep ascend;
	preg2 listp ex_list = NIL;	/* list to expand in */
	ireg1 int	me;			/* my kid number */

	printt1("comma_expand(%x)\n", np);

	for( ascend = cursor; ascend != NIL && ascend != np; ascend = tparent(ascend)){
		if( is_a_list(tparent(ascend)) ) {
			ex_list = tparent(ascend);
			cursor = ascend;
			}
		}
	if( !ascend ) 
		error( ER( 6, "bug`BUG: appropriate list not found for comma") );
			


	if ((me = get_kidnum(cursor)) < (listcount(ex_list) - 1) &&
	    is_a_stub(node_kid(ex_list, me + 1)))
		cursor = c_rsib(cursor);
	else
		exp_list(1);
}

/* routines to handle a cr */
/* As you know, in ALICE a CR creates or goes to a new empty line.
 * This routine is for a CR on a normal level line like an assignment
 * statement.  If in an immediate block, it also executes the line.
 */

do_cr( linenode )
preg1 nodep linenode;		/* line the cr was hit on */
{
	ireg1 int whoami;	/* which kid am I? */
	ireg2 int parkids;		/* how many kids our parent has */

	/* check to see if we have a rightsib */
	if( ntype(find_true_parent(linenode)) == N_IMM_BLOCK ) {
		prep_r_window();
		prep_to_run();
		ex_immediate(linenode,work_debug(curr_workspace));
		}
	cursor = linenode;
	parkids = n_num_children( tparent(cursor) );
	if( (whoami = get_kidnum(cursor)) < parkids-1 ) {
		/* we have one is it empty? */
		if( is_a_stub( node_kid( tparent(cursor), whoami+1 ) ) ){
			cursor = c_rsib( cursor );
			return;
			}
		}
	exp_list(1);		/* add new blank unit */
}

/* This is the CR routine for block statements, notably for the first
 * print code of such a block statement.  Here we don't want to expand
 * at the level of the statement, but rather inside it.
 */

block_cr(xlinenode, whichkid )
nodep xlinenode;		/* which block we are on */
int whichkid;		/* first line or declarations */
{
	ireg1 int kidn;	/* kid number finding kids */
	ireg2 bltype;
	preg1 nodep linenode;
	ClassNum qkcls;
	linenode = xlinenode;

	bltype = ntype(linenode);
	for( kidn = 0; kidn < kid_count(bltype); kidn++ )
		if( (qkcls = kid_class(bltype,kidn)) >= CLIST(0)
				&& is_line_class(qkcls) && !whichkid-- )
			break;
	/* kidn now gives the kid we want unless error */
	if( kidn >= kid_count(bltype) )
		bug( ER(243,"wrong linelist kid") );
	cursor = realcp(node_kid( linenode, kidn ));
	if( not_a_stub(cursor) )
		exp_list(0);
}

/*
 * This inserts a node between a child and its parent.  Useful for adding
 * an "^" to an ID
 */

op_insert( where, type, tok )
nodep where;		/* new child of inserted node */
int type;		/* what kind of node is inserted */
int tok;		/* the token */
{
	preg1 nodep created;

	if( not_variable( where ) )
		in_token( tok, tparent(cursor) );
	 else {
		int nkids;

		created = ins_node( where, type, 0 );
		cursor = kid_count(type) > 1 ? kid2(created) : created;
		}
}

/* turn and undefined assignment into an undefined call */
/* 
 * When you type an undefined ID on a statement placeholder you get an
 * assignment statement.  If you type a "(" right away, it calls this action
 * which turns it into the procedure call you probably want.
 */

do_mcall(xassig)
nodep xassig;			/* assignment statement */
{
#ifdef HAS_ACT
	preg1 nodep assig;
	preg2 nodep savedID;		/* save id to graft onto call */
	assig = xassig;

	savedID = kid1(assig);
	if(!is_undid(savedID) || not_a_stub(kid2(assig)) ){
		error( ER(7, "procmap`You can't turn this into a procedure call") );
		}

	prune(savedID);
	prune(assig);
	exp_prod(N_ST_CALL);	/* move to appropriate kid */
	graft(savedID, cursor);
	cursor = realcp(kid2(cursparent));	/* move to parameter */
#endif HAS_ACT
}

	/* is the loc an undefined symbol */

is_undid(loc )
preg1 nodep loc;
{
	
	return ntype(loc) == N_ID && sym_dtype(kid_id(loc)) == T_UNDEF;
}
