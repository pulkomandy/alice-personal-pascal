/*
 *DESC: General subroutines for tree creation an manipulation at the low
 *DESC: level.  Moving around the tree is supported here, too
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "class.h"
#include "typecodes.h"
#include "flags.h"

#ifdef DB_TREESUBR
# define PRINTT
#endif
#include "printt.h"

#if defined(SAI) || defined(MAP)
# define NOT_EDITOR
#endif

/*
 *  tree building routines
 */

/*
 * tree - builds a standard node with a variable number of kids
 */
/*VARARGS2*/
nodep 
tree(type, a1, a2, a3, a4, a5, a6, a7, a8)
bits8 type;		/* node type */
nodep a1, a2, a3, a4, a5, a6, a7, a8;	/*args*/
{
	register nodep	newnode;	/* pointer to built node */
	register nodep	thekid;		/* for loop setting parents */
	register int	kcount;		/* for loop setting parents */
	int		num;

	num = full_kids(type);

	newnode = (nodep)talloc(sizeof(node) + sizeof(nodep)*(num-1));
	s_ntype(newnode, type);
	s_node_flag(newnode, 0);
	s_2flag(newnode, 0);
	s_node_parent(newnode, NIL);

	printt6("New node, type %d(%s) at %lx, %d kids, %lx %lx\n",
		type, NodeName(type), (long)newnode, num, (long)a1, (long)a2);

	if( dtrace != NULL )
		fflush(dtrace);

	/* ASSUME: s_kid# is appropriate, we have a non-list nodep */
	switch (num) {
	case 8:
		s_kid8(newnode, a8);
	case 7:
		s_kid7(newnode, a7);
	case 6:
		s_kid6(newnode, a6);
	case 5:
		s_kid5(newnode, a5);
	case 4:
		s_kid4(newnode, a4);
	case 3:
		s_kid3(newnode, a3);
	case 2:
		s_kid2(newnode, a2);
	case 1:
		s_kid1(newnode, a1);
	case 0:
		break;
	default:
#ifndef NOT_EDITOR
		bug(ER(271,"Can't have %d kids\n"), num);
#endif
		break;
	}

	num = reg_kids(newnode);
	for (kcount = 0; kcount < num; kcount++) {
		thekid = node_kid(newnode, kcount);
		if (thekid) {
#ifndef PARSER
			printt4("Kid #%d, at %x type %d(%%s)\n",
				kcount+1, (int)thekid, ntype(thekid),
				Node_Names[ntype(thekid)]);
#endif
			s_tparent(thekid, newnode);
		}
	}
	return newnode;
}

linkup( parent, kidnum, kidnode )
nodep parent;		/* top guy in link */
int kidnum;		/* which kid he is */
nodep kidnode;		/* who gets put there */
{
	s_tparent(kidnode, parent);
	s_node_kid(parent, kidnum, kidnode);
}

listp 
sized_list(size)
register int	size;
{
	register listp lp;

	lp = LCAST talloc(sizeof(node) + (size-1)*sizeof(nodep));
	s_ntype(lp, N_LIST);
	s_tparent(lp, NIL);
	s_listcount(lp, size);
	s_listacount(lp, size);

	return lp;
}

#ifdef PARSER
# undef NOT_EDITOR
#endif

#ifndef NOT_EDITOR

listp 
newlist(_thenode)
nodep	_thenode;
{
	register nodep	thenode	= _thenode;
	register listp	nlist;

	/* first check to see if it is already a list */
	if (_thenode && is_a_list(thenode))
		return FLCAST thenode;
	nlist = sized_list(LIST_CHUNK);
	if (_thenode) {
		s_listcount(nlist, 1);
		linkup(nlist, 0, thenode);
	} else
		s_listcount(nlist, 0);

	printt2("newlist(%lx) returns %lx\n", thenode, nlist);
	return nlist;
}

#define	ORPHAN		-1

listp 
growlist(_lp)
listp	_lp;
{
	register listp	lp = _lp;
	register int	lcount;
	int	lacount;
	int	newsize;
	extern char	*trealloc();

	lcount = listcount(lp);
	lacount = listacount(lp);

	printt3("growlist(%lx), lcount=%d, lacount=%d\n", lp, lcount, lacount);
	if (lcount == MAX_LIST_KIDS)
#ifdef PARSER
		bug(
#else
		error(
#endif
			ER(272,"growlist(%x) on full list"), lp);

	if (lcount == lacount) {	/* is list full? */
		listp	newlp;
		int	which_kid_am_i;

		if (tparent(lp) != NIL)	/* list has a parent? */
			which_kid_am_i = get_kidnum(lp);
		else
			which_kid_am_i = ORPHAN;

		newsize = min(lacount + LIST_CHUNK, MAX_LIST_KIDS);

		/* Grow the list another LIST_CHUNK entries */
		newlp = LCAST trealloc(lp,
				sizeof(node) + sizeof(nodep)*(lacount - 1),
				sizeof(node) + sizeof(nodep)*(newsize - 1));

		s_listacount(newlp, newsize);

		/*
		 * Since checkrealloc always returns a new block of memory,
		 * this comparison will always be true.
		 */
		if (newlp != lp) {
			int	i;

			printt1("new list now at %x\n", newlp);

			/*
			 * The list moved in memory.  Make all references to the
			 * old list 'lp' now refer to the new list.
			 */
			if (which_kid_am_i != ORPHAN)
				s_node_kid(tparent(newlp), which_kid_am_i,
					   FNCAST newlp);

			for (i = 0; i < lcount; i++)
				s_tparent(node_kid(newlp, i), FNCAST newlp);
#ifndef PARSER
			hist_refs_list(lp, newlp);
			line_refs_list(lp, newlp);
#endif
			lp = newlp;
		}
	}
	s_listcount(lp, lcount + 1);

	printt3("growlist() returns %x, lcount=%d, lacount=%d\n",
			lp, listcount(lp), listacount(lp));
	return lp;
}

#ifndef PARSER

extern curspos infix_descend();

/*
 * infix nextpos - traverses tree in infix order according to the
 * kid_descend flag of each node -- if this flag is set visit the first
 * child of a node before visiting the parent.
 */
curspos
uc_nextpos(xcp)
curspos xcp;
{
	register curspos cp = xcp;
	register int	kd;	/* "descend to kid1 before parent" flag */
	int	kid_no;

	printt1("uc_nextpos(%x)\n", cp);

	kd = kid_descend(ntype(FNCAST cp));
	/*
	 * If at a node with children, go to the kd'th child (if the
	 * flag is set we will have already visited kid1).
	 */ 
	if (n_num_children(FNCAST cp) > kd) {
		printt1("follow child %d\n", kd);
		return infix_descend((curspos)node_kid(FNCAST cp, kd));
	}
	for ( ; !c_at_root(cp); cp = tparent(cp)) {
		kid_no = get_kidnum(FNCAST cp);
		/*
		 * If at kid1 of a node which hasn't been visited yet
		 * (because its flag indicated to visit kid1 first),
		 * go back up and visit it.
		 */
		if (kid_no == 0 && kid_descend(ntype(tparent(FNCAST cp)))) {
			printt0("back to parent\n");
			return tparent(cp);
		}
		/*
		 * If any more right siblings at this level, visit them.
		 */
		if (kid_no + 1 < n_num_children(tparent(FNCAST cp))) {
			printt0("here I come right bro\n");
			return infix_descend(c_rsib(cp));
		}
		printt1("go up, cp was %x\n", cp);
	}
	return infix_descend((curspos)cur_work_top);
}

/*
 * If visiting a node for the first time and its kid_descend flag is set,
 * visit its kid1 first.
 */
curspos
infix_descend(xcp)
curspos	xcp;
{
	register curspos cp = xcp;

	printt1("infix_descend(%x) ", cp);

	while (kid_descend(ntype(FNCAST cp)))
		cp = (curspos)kid1(cp);

	printt1("returns %x\n", cp);
	return cp;
}

/*
 * Go down and to the right as far as possible
 */
curspos
down_and_right(xcp)
curspos	xcp;
{
	register curspos cp = xcp;
	register int	n_kids;

	while ((n_kids = n_num_children(cp)) > 0)
		cp = node_kid(cp, n_kids - 1);		/* rightmost child */
	return cp;
}

/*
 * infix prevpos - traverses tree in infix order according to the
 * kid_descend flag of each node...see uc_nextpos above.
 */
curspos
uc_prevpos(xcp)
curspos xcp;
{
	register curspos cp = xcp;

	printt1("uc_nextpos(%x)\n", cp);

	if (c_at_root(cp))
		return down_and_right(cp);

	if (kid_descend(ntype(FNCAST cp)) && n_num_children(FNCAST cp) > 0)
		return down_and_right((curspos)kid1(FNCAST cp));

	while (kid_descend(ntype(tparent(FNCAST cp))) &&
	       get_kidnum(FNCAST cp) == 0)
		cp = tparent(cp);

	if (kid_descend(ntype(tparent(FNCAST cp))) == get_kidnum(FNCAST cp))
		return tparent(cp);

	return down_and_right(c_lsib(cp));
}

/*
 * Given a cursor position, this routine returns the next cursor
 * position in the tree, given a preorder depth first traversal.
 */
curspos
c_nextpos(xcp)
curspos xcp;
{
	register curspos cp = xcp;
	register curspos rv;

	if (n_num_children(cp) > 0) {
		/* We have a child here, follow it. */
		printt0("follow child\n");
		rv = node_kid(cp, 0);
	} else {
		rv = c_rightpos(cp);
	}
	printt2("c_nextpos returns %x: %x\n", PCP(rv), rv);
	return rv;
}

curspos
c_rightpos(xcp)
curspos xcp;		/* thing to go to the right of */
{
	register curspos cp = xcp;
	register curspos rv;	/* where we are going */

	/* Go up as far as necessary to find a right sibling */
	for (rv=cp; !c_at_root(rv) &&
		get_kidnum(rv)+1 >= n_num_children(tparent(rv));
	    		rv = tparent(rv)) {
#ifdef DB_TREESUBR
		if (tracing) fprintf(dtrace,
		"go up, rv=%x, at root %d, kidnum+1 %d, num_children %d\n",
		PCP(rv), c_at_root(rv), get_kidnum(rv)+1, n_num_children(tparent(rv)));
#endif /*DB_TREESUBR*/
	}
#ifdef DB_TREESUBR
	if (tracing) fprintf(dtrace,
	"popped out, rv=%x, at root %d, kidnum+1 %d, num_children %d\n",
	PCP(rv), c_at_root(rv), get_kidnum(rv)+1, n_num_children(tparent(rv)));
#endif /*DB_TREESUBR*/
	printt1("try right, rv=%x, root CENSORED\n", PCP(rv));
	if (c_at_root(rv)) {
		rv = cur_work_top;
	} else
		rv = c_rsib(rv);	/* go to right sibling  */
	return rv;
}

#ifdef NEVERCALLED
/*
 * This is the inverse function of c_nextpos.
 */
curspos
c_prevpos(xcp)
curspos xcp;
{
	register curspos cp = xcp;
	register curspos rv;
	register int nk;

	rv = c_at_root(cp) ? cp : c_lsib(cp);
	if (rv == 0) {	/* there is no left sibling */

		/* Leftmost child, must go up. */
#ifdef DB_TREESUBR
		if (tracing) fprintf(dtrace, "go up\n");
#endif /*DB_TREESUBR*/
		rv = tparent(cp);
	} else {
		/* Go down as far as possible. */
		while ((nk=n_num_children(rv)) > 0) {
			/* Take rightmost child */
			rv = node_kid(rv, nk-1);
#ifdef DB_TREESUBR
			if (tracing) fprintf(dtrace, "go down, rv=%x\n", PCP(rv));
#endif /*DB_TREESUBR*/
		}
	}
#ifdef DB_TREESUBR
	if (tracing)
		fprintf(dtrace, "c_prevpos returns %x: %x\n", PCP(rv), rv);
#endif /*DB_TREESUBR*/
	return rv;
}

#endif NEVERCALLED


/*
 * Return the left sibling of a curspos, or NIL if we are leftmost
 * of this parent.
 */
curspos
c_lsib(xcp)
curspos xcp;
{
	register curspos cp = xcp;
	register int kidnum;

	if( ( kidnum = get_kidnum( cp ) ) <= 0) {
		return NIL;
	} else
		return node_kid( tparent( cp ), kidnum - 1 );
}

/*
 * Return the right sibling of a curspos, or NIL if we are rightmost
 * of this parent.
 */
curspos
c_rsib(xcp)
curspos xcp;
{
	register curspos cp = xcp;
	register int kidnum;

	if( ( kidnum = get_kidnum( cp ) ) >= n_num_children(tparent(cp)) - 1) {
#ifdef DB_TREESUBR
		if (tracing) fprintf(dtrace, "node %x\n", PCP(cp) );
#endif /*DB_TREESUBR*/
		return cp;	/* might want to say bug */
	}
	return node_kid(tparent( cp ), kidnum + 1 );
}
#endif PARSER

#ifndef PARSER
/*
 * Return TRUE if cp is the root of the tree.
 */
int
c_at_root(cp)
curspos cp;
{
	return is_root(tparent(cp));
}
#endif PARSER
#endif /* not NOT_EDITOR */

#ifndef MAP

workp
myWS(_np)
nodep _np;
{
	register nodep np = _np;

	while (tparent(np))
		np = tparent(np);

	printt2("myWS(%x) returns %x\n", _np, np);
	return (workp)np;
}

nodep 
my_block(_np)
nodep _np;
{
	register nodep	np = _np;

	while (!is_root(np))
		if (ntype_info(ntype(np)) & F_SCOPE)
			return np;
		 else
			np = tparent(np);
	return NIL;
}

int
my_scope(_np)
nodep _np;
{
	register nodep	np	= _np;
	register int	scope	= 0;

	printt1("my_scope(%x) ", np);

	while (!is_root(np)) {
		if (ntype_info(ntype(np)) & F_SCOPE)
			scope++;
		np = tparent(np);
	}

	printt1("returns %d\n", scope);
	return scope;
}

#endif

#ifdef PARSER
/*
 * Add a node to an existing list.  Grow the list as necessary.  (Lists
 * are variable length arrays).
 */
listp 
addlist(_lp, _np)
listp _lp;			/* existing list */
nodep _np;			/* node to add to list */
{
	register listp	lp	= _lp;
	register nodep	np	= _np;
	register int	i;

	if (not_a_list(lp))
		lp = newlist(lp);

	if (is_a_list(np))
		for (i = 0; i < listcount(np); i++)
			lp = addlist(lp, node_kid(np, i));
	else {
		lp = growlist(lp);
		linkup(lp, listcount(lp) - 1, np);
		}

	return lp;
}

/*
 * Force n to be a list, by encapsulating it in one if necessary.
 */
listp 
forcelist(n)
nodep n;
{
	if (is_a_list(n))
		return n;
	return newlist(n);
}

listp 
conclist(a, b)
register listp	a;
register listp	b;
{
	if (b == NIL) 
		if (a == NIL)
			return NIL;
		else
			return forcelist(a);
	else if (a == NIL)
		return forcelist(b);
	else if (is_a_stub(b))
		return forcelist(a);
	else if (is_a_stub(a))
		return forcelist(b);
	else if (is_a_list(b) && listcount(b) == 1 && is_a_stub(kid1(b)))
		return forcelist(a);
	else if (is_a_list(a) && listcount(a) == 1 && is_a_stub(kid1(a)))
		return forcelist(b);
	else
		return addlist(a, b);
}
#endif PARSER
