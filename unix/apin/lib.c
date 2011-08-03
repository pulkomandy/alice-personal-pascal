#include "alice.h"
#include "flags.h"
#include "typecodes.h"

#ifndef RELEASE
# define PRINTT
#endif
#include "printt.h"

Boolean	LibRefd		= FALSE;
nodep	LibTree;
nodep	LibSymtab;
static listp	LibDecls;
static listp	NewLibDecls;
static int	LibTreesUsed;
static nodep	ProgSymtab;

linkLib()
{
	symptr	s;
	int	i;
	int	kid;
	nodep	declTree;
	nodep	hide;

	if (!LibTree)
		return;

	if (LibRefd)
		LoadRestOfLibrary();
	else {
		AbandonRestOfLibrary();
		return;
		}

	LibDecls = kid4(LibTree);
	LibTreesUsed = 0;
	ProgSymtab = sym_kid(root);

	/* If all the decls are hidden under a hide, look under *that* */
	if (listcount(LibDecls) == 1 && ntype(kid1(LibDecls)) == N_HIDE)
		LibDecls = kid2(kid1(LibDecls));

	/* Mark the decl subtrees which are directly or indirectly referenced */
	for (s = (symptr)kid1(LibSymtab); s; s = sym_next(s))
		if (sym_mflags(s) & SF_LIBREF)
			markDeclParent(s);

	/*
	 * Merge the referenced library decl subtrees with the parsed
	 * program's subtrees.  The library subtrees have go *before*
	 * the program's subtrees in a special hide.
	 */
	NewLibDecls = sized_list(LibTreesUsed);
	hide = tree(N_HIDE, tree(N_T_COMMENT, "apin-loaded library"),
			    NewLibDecls);
	or_2flag(hide, NF2_KEEP_HIDDEN);
	exp_list(kid4(root), 0, hide);

	kid = 0;
	for (i = 0; i < listcount(LibDecls); i++) {
		declTree = node_kid(LibDecls, i);
		if (node_flag(declTree) & NF_TREEUSED) {
			linkup(NewLibDecls, kid++, declTree);
			clr_node_flag(declTree, NF_TREEUSED);
			}
		}

	/* Now walk the library subtrees adding symbols to the symbol table. */
	addSyms(NewLibDecls, FALSE);

	/* Fix any dangling T_UNDEF references */
	s_kid1(LibSymtab, NIL);
	fixUndefs(NewLibDecls);
}

/* Mark the declaration subtree of this parent as used */
markDeclParent(s)
symptr	s;
{
	register nodep	p;

	printt2("markDeclParent(%lx==%s)\n", s, sym_name(s));

	if (sym_saveid(s) < 0 /* builtin symbol */ ||
	    sym_dtype(s) == T_UNDEF ||
	    sym_mflags(s) & SF_MARKED)
		return;
	or_sym_mflags(s, SF_MARKED);

	/* Walk up to the top of the declaration subtree */
	for (p = tparent(s); p && tparent(p) != LibDecls; p = tparent(p))
		;
	if (p && tparent(p) == LibDecls) {
		/*
		 * This subtree is referenced.  Descend it for additional
		 * (N_ID) references if required.
		 */
		if (!(node_flag(p) & NF_TREEUSED)) {
			or_node_flag(p, NF_TREEUSED);
			LibTreesUsed++;
			refIDs(p);
			}
		}
	else
		printt1("panic: tparent(%lx) == NIL!\n", p);
}

/*
 * Walk a decl subtree, looking for ID references in other subtrees
 *
 * This routine is *heavily* recursive -- since it calls markDeclParent()
 * which may call refIDs() on other subtrees, it may have a worst case
 * depth of roughly the longest path of references -- if subtree A
 * references subtree B which references subtree C which references
 * subtree D, etc.  This situation is pretty unlikely.
 */
refIDs(subtree)
nodep	subtree;
{
	printt1("refIDs(%lx)\n", subtree);

	if (ntype(subtree) == N_ID) {
		if (!(sym_mflags(kid_id(subtree)) & SF_MARKED))
			markDeclParent(kid_id(subtree));
		}
	else
		forAllKidsDo(subtree, refIDs);
}

/*
 * Walk a declTree, adding N_DECL_IDs to the symbol table
 * ...similar to if_sym_call()
 */
addSyms(declTree, inRec)
nodep	declTree;
int	inRec;
{
	register int	i;
	register int	nkids;

	printt2("addSyms(%lx, %d)\n", declTree, inRec);

	switch (ntype(declTree)) {
	case N_DECL_ID:
		clr_sym_mflags((symptr)declTree, (SF_MARKED|SF_LIBREF));
		if (!inRec || sym_dtype((symptr)declTree) == T_ENUM_ELEMENT)
			addSym(declTree);
		break;
	case N_DECL_PROC:
	case N_DECL_FUNC:
	case N_FORM_PROCEDURE:
	case N_FORM_FUNCTION:
		addSyms(kid1(declTree), FALSE);
		break;
	case N_TYP_RECORD:
		inRec = TRUE;
		/* fall through to */
	default:
		if (ntype_info(ntype(declTree)) & F_DECLARE) {
			nkids = reg_kids(declTree);
			for (i = 0; i < nkids; i++)
				addSyms(node_kid(declTree, i), inRec);
			}
		break;
	}
}

static nodep	FromID;
static nodep	ToID;

/* Add a symbol to the symbol table -- similar to sym_link/add_decl */
addSym(sym)
symptr	sym;
{
	char	*name	=	sym_name(sym);
	symptr	scan;

	printt1("addSym(%lx)\n", sym);

	if (kid1(ProgSymtab) == NIL) {
		kid1(ProgSymtab) = (nodep)sym;
		sym_next(sym) = NILSYM;
		return;
		}
	for (scan = (symptr)kid1(ProgSymtab); scan; scan = sym_next(scan))
		if (case_equal(name, sym_name(scan))) {
			extern char rdmsg[];

			printt1("there already was a %s\n", name);
			FromID = (nodep)sym;
			ToID = (nodep)scan;
			rerefLibIDs(NewLibDecls);

			or_node_flag((nodep)sym, NF_REDECLARED);
			clr_node_flag((nodep)sym, NF_IS_DECL);
			nonfatal(rdmsg, name);
			break;
			}
	sym_next(sym) = (symptr)kid1(ProgSymtab);
	kid1(ProgSymtab) = (nodep)sym;
}

rerefLibIDs(subtree)
nodep	subtree;
{
	printt1("rerefLibIDs(%lx)\n", subtree);

	if (ntype(subtree) == N_ID && kid1(subtree) == FromID) {
		printt0("kid1 changed\n");
		kid1(subtree) = ToID;
		}
	else
		forAllKidsDo(subtree, rerefLibIDs);
}

fixUndefs(subtree)
nodep	subtree;
{
	if (ntype(subtree) == N_ID) {
		symptr	sym	= kid_id(subtree);
		if (sym_dtype(sym) == T_UNDEF)
			s_kid_id(subtree,
				 slookup(sym_name(sym), CREATE, ProgSymtab,
					 MOAN, NIL));
		}
	else
		forAllKidsDo(subtree, fixUndefs);
}

forAllKidsDo(subtree, func)
nodep	subtree;
funcptr	func;
{
	register int	i;
	register int	nkids	= reg_kids(subtree);

	for (i = 0; i < nkids; i++)
		(*func)(node_kid(subtree, i));
}
