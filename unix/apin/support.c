#include "alice.h"
#include <curses.h>
#include "typecodes.h"
#include "input.h"
#include "class.h"

#ifndef RELEASE
# define PRINTT
#endif
#include "printt.h"

/* and now the symbol lookup routines */

nodep GlobalSymtab = NIL;
nodep CurSymtab = &master_table;
nodep CurBlkSymtab = &master_table;
SymtabStackEntry SymtabStack[MAX_BLOCK_DEPTH];/* stack of scopes */
int scope_level = 0;		/* current scope level */

extern int yylineno;
extern char *yyfilename;

int	BlockFileMade	= FALSE;

/*
 *    Declaration routines.
 *	These routines declare the appropriate item in the local symbol
 *	table and return a pointer to the tree showing the declaration
 */


 /*   makeprog - make the program node */


nodep
comstub()
{
	return make_stub( C_COMMENT );
}

makeprog( pnode )
nodep pnode;
{
	listp filenames;
	int i;


	printt1("makeprog %lx\n", (long)pnode);
#ifdef notdef
	/* FILENAME are now comments */
	filenames = FLCAST kid2( pnode );

	for( i = 0; i < listcount(filenames); i++ )
		s_sym_dtype( (symptr)node_kid(filenames, i), T_FILENAME );

	s_kid1( root, pnode );
	s_tparent(pnode, root);
#endif
	root = pnode;
}


listp	prevTypes();
Boolean	refsSomething();
listp	insertType();
symptr	inventType();

/*
 * Recursively fix up declarations:
 *	1. Constant initializers must have named types.  Move
 *	   structured types in constant initializers to 'type'
 *	   declarations, and invent names for them.
 *	2. Constant field initializers are loaded as N_NAMEs.
 *	   Convert them to correctly linked up N_IDs.
 *	3. Absolute variables must have named types.  Move structured
 *	   types in absolute variable declarations to 'type'
 *	   declarations, and invent names for them.
 *
 * 1 and 3 can use the previous type tree iff it immediately precedes
 * the const or var block, and iff the type doesn't reference anything declared
 * in the const or var block (i.e. consts in const blocks, and enumerated
 * types in var blocks).  Otherwise we must insert a type tree smack-dab
 * in the middle of the const or var block.
 */

/* This code should also work for the not-yet-added N_VAR_ABSOLUTE node. */
#ifndef N_VAR_ABSOLUTE
# define N_VAR_ABSOLUTE	N_CONST_INIT
#endif

fixupDecls(decls, symtab)
listp	decls;				/* declaration list for this block */
nodep	symtab;				/* symbol table for this block */
{
	nodep	decl;
	int	i;

	printt2("fixupDecls(%lx, %lx)\n", decls, symtab);

	/* Scan over each declaration in the decl list */
	for (i = 0; i < listcount(decls); i++) {
	    decl = node_kid(decls, i);
	    printt3("i=%d, decl=%lx, ntype=%d\n", i, decl, ntype(decl));

	    switch (ntype(decl)) {
	    case N_DECL_VAR:
	    case N_DECL_CONST:
		{
		listp	dlist	= kid1(decl);
		nodep	d;
		int	nt;
		int	j;
		listp	types;

		/* Look for absolute variables or structured typed constants */
		for (j = 0; j < listcount(dlist); j++) {
		    d = node_kid(dlist, j);
		    nt = ntype(d);
		    if (nt == N_CONST_INIT || nt == N_VAR_ABSOLUTE) {
			if (ntype(kid2(d)) != N_ID) {
			    if ((types = prevTypes(decls, i)) == NIL ||
				refsSomething(kid2(d), decl)) {
				decls = insertType(decls, i, j);
				break;
				}
			    linkup(d, 1,
				   tree(N_ID,
					inventType(types, kid2(d), symtab)));
			    }
			if (nt == N_CONST_INIT)
			    fixupFieldInit(kid2(d), kid3(d));
			}
		    }
		}
		break;
	    case N_DECL_PROC:		/* recurse */
	    case N_DECL_FUNC:
		fixupDecls(decl_kid(decl), sym_kid(decl));
		break;
	    default:
		break;
	    }
	    } /* brace style collision */
}


/*
 * Return the immediately previous types list in a declaration list, or NIL.
 */
listp
prevTypes(decls, i)
listp	decls;
int	i;
{
	nodep	type;

	printt2("prevTypes(%lx, %d)\n", decls, i);

	if (i > 0 && ntype((type = node_kid(decls, i-1))) == N_DECL_TYPE)
		return kid1(type);
	else
		return NIL;
}

/*
 * Return TRUE if a type subtree references something in its parent declaration
 */
Boolean
refsSomething(type, decl)
nodep	type;
nodep	decl;
{
	printt2("refsSomething(%lx, %lx)\n", type, decl);

	if (ntype(type) == N_ID) {
		symptr	ref		= kid_id(type);
		int	refType		= sym_dtype(ref);
		int	nt		= ntype(decl);

		if (((nt == N_DECL_VAR && refType == T_ENUM_ELEMENT) ||
		     (nt == N_DECL_CONST && refType == T_CONST)) &&
		    is_ancestor(decl, ref))
			return TRUE;
		}
	else {
		int	i;
		int	nkids;

		nkids = reg_kids(type);
		for (i = 0; i < nkids; i++)
			if (refsSomething(node_kid(type, i), decl))
				return TRUE;
		}

	return FALSE;
}

/*
 * Insert a type tree in the declaration list, possibly between one
 * const or var declaration and the next one.  It may be necessary to
 * create a second const or var declaration list to hold the remaining
 * consts or vars.
 *
 * Returns the declaration list (which may have moved).
 */
listp
insertType(decls, i, j)
listp	decls;				/* declaration list */
int	i;				/* const or var kid of decl list */
int	j;				/* insertion pos in const or var list */
{
	nodep	rest;			/* rest of const or var decls */
	listp	exp_list();

	printt3("insertType(%lx, %d, %d)\n", decls, i, j);

	if (j > 0) {
		/*
		 * Split the const or var list at j.
		 *
		 * Create a new list, and copy the remaining kids from j on
		 * into it, and insert it into the declaration list.
		 */
		nodep	decl	= node_kid(decls, i);
		listp	oldlist	= kid1(decl);
		int	newkids	= listcount(oldlist) - j;
		listp	newlist	= sized_list(newkids);
		int	k;

		for (k = 0; k < newkids; k++)
			linkup(newlist, k, node_kid(oldlist, j+k));
		s_listcount(oldlist, j);
		rest = tree(ntype(decl), newlist);
		i++;			/* insert AFTER the first decls */
		}

	decls = exp_list(decls, i, tree(N_DECL_TYPE, sized_list(0)));

	if (j > 0)
		decls = exp_list(decls, ++i, rest);

	return decls;
}

symptr
inventType(types, t, symtab)
listp	types;				/* types list */
nodep	t;				/* type subtree which needs a name */
nodep	symtab;				/* symbol table to put new type in */
{
	int	lastKid;		/* last kid in type decl tree */
	symptr	newSym;			/* new type's symbol */
	static int nameNumber	= 1;	/* new type name sequencer */
	char	buf[20];

	printt3("inventType(%lx, %lx, %lx)\n", types, t, symtab);

	/* Expand the list so we can add the new type to the end */
	types = growlist(types);
	lastKid = listcount(types) - 1;

	/* Create a new name */
	sprintf(buf, "NamedType%d", nameNumber++);
	newSym = s_declare(allocstring(buf), T_TYPE, symtab);

	/* Build a type tree and link it into the type list */
	linkup(types, lastKid,
	       tree(N_TYPE_DECL, newSym, t,
		    tree(N_T_COMMENT, "apin-generated named-type")));

	return newSym;
}

/*
 * Fix field identifiers -- At parse time it is not possible to link the field
 * identifiers to their declarations (in a record symbol table), so we
 * build N_NAMEs to hold the name.  We must now recursively lookup the
 * name and replace the N_NAME with an N_ID pointing to the declaration.
 */
fixupFieldInit(type, init)
nodep	type;
nodep	init;
{
	int	k;
	listp	init_list;
	symptr	field;
	char	*name;
	extern struct symbol_node null_undef;

	printt4("fixupFieldInit(%lx(%d), %lx(%d)\n", type, ntype(type),
		init, ntype(init));

	if (ntype(type) == N_ID) {
		type = get_type(kid_id(type));
		printt2("now type is %lx(%d)\n", type, ntype(type));
		}

	switch (ntype(init)) {
	case N_INIT_STRUCT:
		{
		nodep artype;
		init_list = kid1(init);
		yylineno = (int)kid2(init);	/* hack! */
		if( ntype(type) == N_TYP_ARRAY )
			artype = kid2( type );
		 else
			artype = type;
		for (k = 0; k < listcount(init_list); k++)
			fixupFieldInit(artype, node_kid(init_list, k));
		}
		break;
	case N_FLD_INIT:
		name = (char *)kid1(kid1(init));
		printt1("looking up %s...\n", name);
		if (ntype(type) != N_TYP_RECORD) {
			error("field '%s' is not in a record", name);
			return;
			}
		field = slookup(name, NOCREATE, sym_kid(type), NOMOAN, NIL);
		printt2("field = %lx(%s)\n", field, sym_name(field));
		if (field == &null_undef) {
			error("Bad constant: field '%s' doesn't exist", name);
			return;
			}

		linkup(init, 0, tree(N_ID, field));
		fixupFieldInit(get_type(field), kid2(init));
		break;
	}
}

/*
 * Hide a block in an existing hide/reveal node, and return it
 */
nodep
hide_block(hide, block)
nodep	hide;
nodep	block;
{
	s_kid2(hide, block);
	return hide;
}

 /*
  * 	label_declare - declares a label.  Takes a name pointer pointing
  *	to the text of the label number
  *	This routine does not return a DECL tree
  */

nodep
label_declare( lname )
char	*lname;
{
	return NCAST s_declare(lname, T_LABEL, CurSymtab);
}



/*
 *	
 *	const_declare - declares a constant.
 *	The constant tree should be evaluated at some point.
 *	What we need is a flag saying if the constant has been
 *	evaluated, so that we only do it the first time through
 *	at run-time
 */

nodep
const_declare( lname, ctree )
symptr lname;		/* the name of the constant */
nodep ctree;		/* the tree defining it */
{
	nodep np;
	nodep comstub();

	/* we should evaluate the type of the constant, and determine
	   if it is a fixed value or not.  The type, from among
	   CHAR, STRING, INT and REAL, should be place in the symbol
	   table node.  If the constant is fixed, it should be
	   placed in the symbol table node, and the flag marked that
	   it has been evaluated.
	*/

	s_sym_dtype( lname, T_CONST );

	scoop_comments();
	/* Only use up one comment, since that's all we have room for */
	if (iscombuf())
		np = tree(N_T_COMMENT, nextcomment());
	else
		np = comstub();

	return( tree(N_CONST_DECL, lname, ctree, np ) );
}

nodep
const_init_declare( lname, typetree, inittree )
symptr	lname;		/* the name of the constant */
nodep	typetree;	/* type of the constant */
nodep	inittree;	/* initializer of the constant */
{
	nodep np;
	nodep comstub();

	s_sym_dtype( lname, T_CONST );

	scoop_comments();
	/* Only use up one comment, since that's all we have room for */
	if (iscombuf())
		np = tree(N_T_COMMENT, nextcomment());
	else
		np = comstub();

	return( tree(N_CONST_INIT, lname, typetree, inittree, np ) );
}


/*
	type_declare

	This routine adds a type name to the symbol table.
	The type will also be given a new unique "type_code".
	An entry should be made in the type table for that type_code
	That gives general information about the type and a pointer
	to the definition node for the type.
	We'll worry about this later
*/

nodep
type_declare( lname, ttree )
symptr lname;			/* the identifier for the type */
nodep ttree;			/* tree defining the type */
{
	nodep np;
	nodep comstub();

	s_sym_dtype( lname, T_TYPE );

	scoop_comments();
	/* Only use up one comment, since that's all we have room for */
	if (iscombuf())
		np = tree(N_T_COMMENT, nextcomment());
	else
		np = comstub();
	return( tree(N_TYPE_DECL, lname, ttree, np ) );
}


/*
	var_declare

	This routine adds a new variable to the symbol table.
	The type will also be given a new unique type_code as above.
	The variables will actually be in a list.
*/

nodep
var_declare( nodetype, sttype, namelist, ttree )
int nodetype;			/* type of node to create */
int sttype;			/* type to set symtab entry to */
listp namelist;		/* list of identifiers */
nodep ttree;			/* type they are to be given */
{
	int i;
	nodep np;
	nodep comstub();

	/* we must change the name pointers in the list to symbol ones */

	for( i = 0; i < listcount(namelist); i++ )
		if (ntype(node_kid(namelist,i)) == N_DECL_ID)
			s_sym_dtype( (symptr)node_kid(namelist,i), sttype );

	scoop_comments();
	/* Only use up one comment, since that's all we have room for */
	if (iscombuf())
		np = tree(N_T_COMMENT, nextcomment());
	else
		np = comstub();
	return( tree( nodetype, namelist, ttree, np ) );
}

nodep
abs_declare( namelist, ttree, abstree )
nodep namelist;		/* list of identifiers */
nodep ttree;			/* type they are to be given */
nodep abstree;
{
	int i;
	nodep np;
	nodep comstub();
	symptr thename;

	/* parser routine assures only one in list */

	thename = (symptr)kid1(namelist);
	/* we must change the name pointers in the list to symbol ones */

	if( ntype(abstree) == N_ID )
		s_sym_dtype( thename, sym_dtype( kid_id(abstree) ) );
	 else
		s_sym_dtype( thename, T_ABSVAR );

	scoop_comments();
	/* Only use up one comment, since that's all we have room for */
	if (iscombuf())
		np = tree(N_T_COMMENT, nextcomment());
	else
		np = comstub();
	return( tree( N_VAR_ABSOLUTE, thename, ttree, abstree, np ) );
}

/* enum_declare - list of enumerated type elements */

nodep
enum_declare( enumlist )
listp enumlist;		/* list of identifiers */
{

	int i;
	symptr ptr;
	/* change name pointers in list to symbol pointers */

	for(i = 0; i < listcount(enumlist); i++ ) {
		ptr = (symptr)node_kid( enumlist, i );
		if (ntype(ptr) == N_DECL_ID) {
			s_sym_value( ptr, i );
		}
	}
	return( tree(N_TYP_ENUM, FNCAST enumlist ) );
}



/*
	proc_declare

	This routine takes an already created node for the procedure,
	declares it and corrects the pointer

*/

proc_declare( pnode )
nodep pnode;		/* pointer to procedure node */
{
	s_sym_dtype( (symptr)kid1(pnode), T_PROCEDURE );
}

/*
	func_declare

	This routine takes an already created node for the function,
	declares it and corrects the pointer

*/

func_declare( pnode )
nodep pnode;		/* pointer to function node */
{
	s_sym_dtype( (symptr)kid1(pnode), T_FUNCTION );
	
}

/*
	param_declare

	This routine takes a list of formals and declares them, much
	like var_declare
*/

nodep
param_declare( ptype, namelist, ttree )
bits8 ptype;			/* type of parameter */
listp namelist;		/* list of identifiers */
nodep ttree;			/* type they are to be given */
{
	int i;

	/* we must change the name pointers in the list to symbol ones */

	for( i = 0; i < listcount(namelist); i++ )
		s_sym_dtype( (symptr)node_kid(namelist,i), 
			ptype == N_FORM_VALUE ? T_FORM_VALUE : T_FORM_REF );

	return( tree( ptype, namelist, ttree ) );
}


#ifdef notdef
/* These are now just done in pas.y directly. AND A GOOD THING TOO*/
/* 
	fparam_declare

	Declares a function parameter

	This will be resolved later
*/

nodep 
fparam_declare( lname, ttree, params )
nameptr lname;		/* name of parameter */
nodep ttree;		/* type tree for return type */
listp params;	/* list of the parameters to the routine */
{
	type_code rettype;	/* type returned by function */

	rettype = T_INTEGER;
	return( tree(N_FORM_FUNC,s_declare(NIL,rettype,
		lname, T_FORM_FUNCTION ), params, ttree ) );
}


/* 
	pparam_declare

	Declares a procedure parameter

	This will be resolved later
*/

nodep
pparam_declare( lname, params )
nameptr lname;		/* name of parameter */
listp params;	/* list of the parameters to the routine */
{
	return( tree(N_FORM_PROC,
			s_declare(NULLWORD,NULLTYPE,lname, T_FORM_PROCEDURE ),
			params ) );
}
#endif


extern int	Errors;

nonfatal(str, a1, a2, a3)
char	*str;
char	*a1, *a2, *a3;
{
	message(str, a1, a2, a3);
	Errors--;
}

fatal(str, a1, a2, a3)
char	*str;
char	*a1, *a2, *a3;
{
	message(str, a1, a2, a3);
#ifdef GERMAN
	fprintf(stderr, "Abbruch Fehler\n");
	fprintf(stderr, "APIN uebernimmt nur fehlerfreie Programme und ueberprueft nicht\n" );
	fprintf(stderr, "den Syntax.\n" );

#else
	fprintf(stderr, "Input aborted due to errors\n");
	fprintf(stderr, "The APIN program is intended for Pascal programs that are already\n" );
	fprintf(stderr, "correct.  It was not designed to properly handle syntax errors in your program.\n" );
#endif

	exit(1);
}


#ifdef TURBO

static int HadUnsupp = FALSE;

/* are we parsing for the small model alice? */
#ifdef msdos
int smallflag = TRUE;
#else
int smallflag = FALSE;
#endif

smallerror( str, a1, a2, a3 )
char *str, *a1, *a2, *a3;
{
	if( smallflag ) {
		unsupported( str,a1,a2,a3 );
#ifdef GERMAN
		fprintf( stderr, "Fuer LARGE ALICE +L Option benutzen\n" );
#else
		fprintf( stderr, "Try the +L option if using LARGE model ALICE\n" );
#endif
		}
	
}

unsupported(str, a1, a2, a3)
char	*str;
char	*a1, *a2, *a3;
{
	message(str, a1, a2, a3);
	Errors--;

	HadUnsupp = TRUE;
}

checkUnsupported()
{
	if (HadUnsupp) {
#ifdef GERMAN
		fprintf(stderr, "\nProgramm enthaelt nicht unterstuezte Turbo Pascal Funktionen.\n");
		fprintf(stderr, "Modifikation dringend empfohlen.\n");
#else
		fprintf(stderr, "\nProgram converted despite use of unsupported Turbo Pascal features.\n");
		fprintf(stderr, "It may be necessary to modify the program before use.\n");
#endif
		}
}

#endif

message( str, a1, a2, a3, a4, a5, a6 ) 
char *str;
char *a1, *a2, *a3, *a4, *a5, *a6;
{
	char *mb;

	Errors++;

	/* Skip over "why" part of message */
	for (mb=str; *mb; mb++)
		if (*mb == '`') {
			str = ++mb;
			break;
		}

	if (yyfilename)
		fprintf( stderr, "(%s)%d: ", yyfilename, yylineno );
	fprintf( stderr, str, a1, a2, a3, a4, a5, a6 );
	if (str[strlen(str)-1] != '\n')
		fprintf(stderr, "\n");
#ifndef RELEASE
	if (dtrace) {
		fprintf( dtrace, "(%s)%d: ", yyfilename, yylineno );
		fprintf( dtrace, str, a1, a2, a3, a4, a5, a6 );
		if (str[strlen(str)-1] != '\n')
			fprintf(dtrace, "\n");
	}
#endif
}

par_error(str)
char *str;
{
	message(str);
}

/* Make a statement comment node */
nodep 
makecom(str)
char *str;
{
	return tree(N_ST_COMMENT, tree(N_T_COMMENT, str));
}

listp 
exp_list(_lp, kn, kid)
listp _lp;
int	kn;
nodep kid;
{
	register listp lp = _lp;
	register char	*insertion_pt;

	printt3("exp_list(%lx, %d, %lx)\n", lp, kn, kid);

	/* increase the size of the list, leaves an empty slot at the end. */
	lp = growlist(lp);

	/* move later list elements over one slot */
	insertion_pt = kn * sizeof( nodep ) + (char*)kid1adr(lp);
	printt1("insertion_pt=%lx\n", insertion_pt);

	t_blk_move(insertion_pt + sizeof(nodep),		/* to, */
		   insertion_pt,				/* from, */
		   ((int)listcount(lp) - kn - 1) * sizeof(nodep ));/* nbytes */

	linkup(lp, kn, kid);

	return lp;
}

/* tree segment block move routine. */
t_blk_move( to, from, bytes )
reg char *to;		/* destination block start */
reg char *from;		/* source block start */
reg int bytes;		/* how many to move */
{
#ifdef QNX
	copy( to, from, bytes );
#else
	if( to < from )
		while( bytes-- )
			*to++ = *from++;
	 else {
		to += bytes;
		from += bytes;
		while( bytes-- )
			*--to = *--from;
		}
#endif
}

/*
 * Routines for backpatching forward pointers.
 */
struct bp {
	nodep node_to_patch;
	nameptr sname;
	struct bp *next;
};

struct bp *cur_backpatch;		/* current "top of backpatch stack" */
struct bp *backpatch[MAX_BLOCK_DEPTH];/* parallel stack of things to backpatch*/

/*
 * Similar to symref, but if not declared, it's OK.  Put it on the
 * list of things to be backpatched later.
 */
nodep 
ptr_symref( nam )
nameptr nam;		/* name table entry for this string */
{
	nodep r;
	symptr sp;
	struct bp *b;

	printt1("ptr_symref(%s)\n", nam);
	sp = slookup(nam, NOCREATE /* was CREATE? */, NIL, NOMOAN, NIL);
	r = tree(N_ID, sp);
	if (sp != &null_undef && inThisScope(sp))
		return r;

	printt0("\tneeds to get patched\n");
	b = (struct bp *) checkalloc(sizeof (struct bp));
	b->node_to_patch = r;
	b->sname = nam;
	b->next = backpatch[scope_level];
	backpatch[scope_level] = b;
	printt2("forward ptr reference to %s, storing backpatch at scope level %d\n", nam, scope_level);
	return r;
}

inThisScope(sym)
symptr	sym;
{
	symptr	syms;

	for (syms = (symptr)kid1(CurSymtab); syms; syms = sym_next(syms))
		if (sym == syms)
			return TRUE;
	return FALSE;
}
/*
 * We are going into a new scope - push a new backpatch list.
 */
push_backpatch()
{
	printt1("push_backpatch, scope level %d\n", scope_level);
	backpatch[scope_level] = (struct bp *) NULL;
}

/*
 * We are leaving a scope - do all the backpatching for this
 * scope and pop the backpatch list.
 */
pop_backpatch()
{
	symptr sp;
	nameptr nam;
	struct bp *p;

	cur_backpatch = backpatch[scope_level];
	printt2("pop_backpatch, scope level %d, head %lx\n", scope_level, (long)cur_backpatch);
	while (cur_backpatch) {
		nam = cur_backpatch->sname;
		sp = slookup(nam, CREATE, NIL, MOAN, NIL);
		printt3("popping %lx %s, found at %lx\n",
			(long)cur_backpatch, cur_backpatch->sname, (long)sp);
		s_kid1(cur_backpatch->node_to_patch, sp);
		p = cur_backpatch;
		cur_backpatch = cur_backpatch->next;
		mfree(p);
	}
}

/*
 * An identifier has just been defined - check the current scope level's
 * backpatch to see if we've been waiting for it.
 */
check_backpatch(np)
nameptr np;
{
	symptr sp;
	nameptr nam;
	struct bp *p;
	struct bp *prev = NULL;

	cur_backpatch = backpatch[scope_level];
	printt3("check_backpatch, scope level %d, head %lx, name '%s'\n",
		scope_level, (long)cur_backpatch, np);
	while (cur_backpatch) {
		nam = cur_backpatch->sname;
		printt5("check, nam %lx %s, np %lx %s, prev %lx\n",
			(long)nam, nam, (long)np, np, (long)prev);
		if (case_equal(nam, np)) {
			sp = slookup(nam, CREATE, NIL, MOAN, NIL);
			printt3("popping %lx %s, found at %lx\n",
			    (long)cur_backpatch, cur_backpatch->sname, (long)sp);
			s_kid1(cur_backpatch->node_to_patch, sp);
			printt0("here is the fixed decl tree:\n");
			dump(cur_backpatch->node_to_patch);

			printt2("remove %lx from list, prev %lx\n",
				(long)cur_backpatch, (long)prev);
			if (prev == NULL)
				backpatch[scope_level] = cur_backpatch->next;
			else
				prev->next = cur_backpatch->next;

			prev = cur_backpatch;
			cur_backpatch = cur_backpatch->next;
			mfree(prev);
		}
		else {
			prev = cur_backpatch;
			cur_backpatch = cur_backpatch->next;
		}
	}
}

/*
 * Figure out (recursively) the type of v.  We don't have to be exact, we
 * just have to get records correctly.  This is used for record fields.
 * The only reason this isn't trivial is that we don't necessarily have
 * the type fields filled in yet, since this is the parser.
 */
nodep 
var_type(v)
nodep v;
{
	nodep a;
	symptr b;
	nameptr n;
	nodep get_type();

	printt2("var_type(%lx)(%d)\n", (long)v, ntype(v));
	switch (ntype(v)) {
	case N_ID:
	case N_VF_WITH:
		return get_type(kid_id(v));

	case N_DECL_ID:
		return get_type((symptr) v);

	case N_VAR_ARRAY:
		a = kid2(var_type(kid1(v)));
		printt2("var_array, a=%lx, type %d\n", (long)a, ntype(a));
		/* a is now a pointer to kid2 of an N_TYP_ARRAY node */
		if (ntype(a) == N_ID)
			return get_type(kid_id(a));
		else	/* it must be a real type */
			return a;

	case N_VAR_POINTER:
		a = kid1(var_type(kid1(v)));
		printt2("var_pointer, a=%lx, type %d\n", (long)a, ntype(a));
		/* a is now a pointer to kid1 of an N_TYP_POINTER node */
		if (ntype(a) == N_ID)
			return get_type(kid_id(a));
		else
			return a;

	case N_VAR_FIELD:
		a = var_type(kid1(v));
		b = (symptr) kid1(kid2(v));
		printt4("var_field, a=%lx, type %d, b=%lx, type %d\n",
			(long)a, ntype(a), (long)b, ntype(b));
		/* a is now a pointer to an N_TYP_RECORD node */
		/* b is now a pointer to a symbol under an N_FIELD node */
		return get_type(slookup(sym_name(b), NOCREATE, kid2(a), MOAN, NIL));

	default:
		printt2("default in var_type, node %lx, type %d\n",
			(long)v, ntype(v));
		abort();
	}
}

/* 
 * Figure out the type of node d, which is a N_DECL_ID.  Ordinarily you
 * would just take sym_type(d), but chances are good that isn't filled
 * in yet so we have to wing it.
 */
nodep 
get_type(decl)
symptr	decl;
{
	nodep	ret;
	nodep	parent;

	printt2("get_type(%lx)(%s)\n", (long)decl, sym_name(decl));

	if (sym_dtype(decl) == T_UNDEF)
		return Basetype(BT_integer);

	ret = sym_type(decl);
	printt2("sym_type(%lx)=%lx  ", (long)decl, (long)ret);

	if (ret == NIL) {
		parent = tparent(decl);
		if (is_a_list(parent))		/* var decls are in a list */
			parent = tparent(parent);
		printt2("parent=%lx(%d)  ", (long)parent, ntype(parent));

		ret = kid2(parent);
		printt2("ret=%lx(%d)  ", (long)ret, ntype(ret));

		if (ntype(ret) == N_ID) {	/* follow an ID */
			printt0("an id, calling get_type again\n");
			ret = get_type(kid_id(ret));
			printt2("back, ret=%lx(%d)  ", (long)ret, ntype(ret));
			}

		if (ntype(ret) == N_TYP_PACKED) {/* follow a packed */
			printt0("packed\n");
			ret = kid1(ret);
			}

		printt2("s_sym_type(%lx, %lx)\n", (long)decl, (long)ret);
		s_sym_type(decl, ret);
		}
	printt1("returning %lx\n", (long)ret);
	return ret;
}

/* trim off leading zeros from a label, so that 007 will compare equal to 7 */
char *
trimzero(p)
char * p;
{
	while (*p == '0')
		p++;
	if (*p == '\0')
		--p;
	return p;
}

/*
 * Given a "variable tree" (ie a tree containing IDs, vt.foo, vt[foo] or vt^),
 * insert "0 argument" function calls where necessary.
 */
nodep
fix0ArgFuncs(vt)
nodep	vt;
{
	register int	t	= ntype(vt);
	nodep		parent;
	nodep		func;
	static char	funcs[] = { T_FUNCTION, T_BFUNC, T_BTFUNC,
				    T_FORM_FUNCTION, 0 };

	printt2("fix0ArgFuncs(%lx), type=%d\n", vt, t);

	/* If it's an ID of type "function" ... */
	if (t == N_ID && strchr(funcs, sym_dtype((symptr)kid_id(vt)))) {
		printt1("%s is a function\n", sym_name((symptr)kid_id(vt)));
		parent = tparent(vt);
		func = tree(N_EXP_FUNC, vt, sized_list(NIL));
		if (parent)
			linkup(parent, 0, func);
		return func;
	}
	return vt;
}


nodep
pushSymtabStack(symtab, isBlock)
nodep	symtab;
Boolean	isBlock;
{
	++scope_level;
	changeSymtabStack(symtab, isBlock);
	return symtab;
}

changeSymtabStack(symtab, isBlock)
nodep	symtab;
Boolean	isBlock;
{
	SymtabStack[scope_level].symtab = CurSymtab = symtab;
	SymtabStack[scope_level].isBlock = isBlock;

	if (isBlock)
		CurBlkSymtab = symtab;
}

popSymtabStack()
{
	register int	i;

	if (SymtabStack[scope_level].isBlock) {
		/* Find previous block symbol table, if there is one */
		for (i = scope_level-1; i >= 0; i--) {
			if (SymtabStack[i].isBlock) {
				CurBlkSymtab = SymtabStack[i].symtab;
				break;
				}
			}
		}
	--scope_level;
	CurSymtab = SymtabStack[scope_level].symtab;
}

nodep
pack(type)
nodep	type;
{
	if (ntype(type) == N_TYP_ARRAY && ntype(kid2(type)) == N_TYP_ARRAY)
		linkup(type, 1, pack(kid2(type)));
	return tree(N_TYP_PACKED, type);
}

is_ancestor( ancestor, _childnode )
preg2 nodep ancestor;
nodep _childnode;		/* the guy whose lineage we check */
{
	preg1 nodep childnode = _childnode;

	while( childnode != NIL && is_not_root(childnode) )
		if( childnode == ancestor )
			return TRUE;
		 else
			childnode = tparent( childnode );
	return FALSE;
}
