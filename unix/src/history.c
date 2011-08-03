#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "class.h"
#include "typecodes.h"
#include "flags.h"

/*
 *DESC: This file contains routines that must be used for all changes to the
 *DESC: tree.  It supports a history mechanism to allow undo/redo
 */

#ifdef DB_HISTORY
static	char	*actions[] = { "mark", "swap", "exp_list", "del_list", "change_ntype", "prune_decl_id", "graft_decl_id", "changeFlags" };
# define	actionname(i)	(i < 1 || i > H_CHG_FLAGS? "?" : actions[i-1])
#define	PRINTT
#endif
#include "printt.h"

#define MAX_HIST	HIST_SIZE	/* max no. of history entries */
#define	ORPHAN		-1	/* kid number when no parent */

/* 
 * The primitive operations in the history list (action field in
 * struct hist_entry).  The name is always suggestive of the forward
 * moving operation, e.g. in the redo direction, not of the inverse
 * operation (the undo direction).
 */
#define H_MARK		1	/* mark */
#define H_SWAP		2	/* swap */
#define H_EXP_LIST	3	/* expand */
#define H_DEL_LIST	4	/* delete */
#define H_CHNG_NTYPE	5	/* change node type */
#define H_PRN_DECID	6	/* prune decl id */
#define H_GFT_DECID	7	/* graft decl id */
#define H_CHG_FLAGS	8	/* change node flags */

typedef struct hist_entry {
	bits8	action;
	bits8	kid;
	nodep from;
	nodep to;
} Hist_ent;

static	Hist_ent history[MAX_HIST];
static	int	hist_last	= -1;
static	int	hist_cur	= 0;
static	int	hist_going	= 0;

static canGoToZero();
static _swap();
static detach();
static attach();
static _exp_list(); static _del_list();
static _chng_ntype(); static hist_add();
static free_ancient_hist();  static free_undone_hist();

/*
 * Graft the tree 'what' onto the tree 'where'.
 */
graft(what, where)
nodep what;
nodep where;
{
	printt2("graft(%x, %x)\n", where, what);

	hist_add(H_SWAP, where, what, 0);
	_swap(where, what);
}

/*
 * prune, checking for whole program deletion 
 */

chprune(what)
nodep what;
{
	if (what == cur_work_top)
		error(ER(49,"pruneall`You can't delete an entire program!"));
	prune(what);
}
	
#if defined(HYBRID) && defined(ES_TREE)
extern nodep stub_for_prune();
#else
#define stub_for_prune(arg) make_stub(arg)
#endif
/*
 * Prune the tree 'what'.  Replace it with a stub.
 */

prune(what)
nodep what;
{
	register nodep stub;
	
	
#ifdef notdef
	if( no_tree_mem() && (ntype_info(ntype(what)) & F_DECLARE ||
				highType(ntype(what)) ) )
		memerr(SEG_TREE);
#endif

	stub = stub_for_prune((int)ex_class(what));

	printt1("prune(%x)\n", what);


	hist_add(H_SWAP, what, stub, 0);
	_swap(what, stub);
}

/*
 * Expand the list above the cursor.
 */
exp_list(dir)
int	dir;		/* direction.  0 => before, 1 => after */
{
	register nodep np;
	register listp lp;
	int		kn;
	nodep stub;

	printt1("exp_list(%s)\n", (dir == 0 ? "before" : "after"));

	/* find the first list above the cursor */
	for (np = cursor; !c_at_root(np); np = tparent(np))
		if (is_a_list(tparent(np)))
			break;
	if (c_at_root(np))
		error(ER(50,"nolist`Cannot find a list to expand"));

	lp = tparent(np);
	if (listcount(lp) == MAX_LIST_KIDS)
		error(ER(51,"fulllist`Can't expand list any further"));

	kn = get_kidnum(np) + dir;
	stub = make_stub((int)ex_class(kid1(lp)));

	_exp_list(lp, kn, stub);
	/* use parent of stub since list may move */
	hist_add(H_EXP_LIST, tparent(stub), stub, kn);
}

/*
 * Open up this (presumably) zero element list.
 */
open_list(_lp)
listp _lp;
{
	register listp lp = _lp;
	register nodep stub;

	printt1("open_list(%x)\n", lp);

	stub = make_stub((int)ex_class(FNCAST lp));
	_exp_list(lp, 0, stub);
	hist_add(H_EXP_LIST, tparent(stub), stub, 0);
}

/*
 * Delete a range of children of a list.
 */
del_range(leave)
int leave;			/* leave behind a placeholder */
{
	register int	kn;
	register listp lp = FLCAST sel_node;	/* save these since we */
	nodep pkid;
	int		first = sel_first;	/* might grab_range(0)... */
	int		last = sel_last-1;

	printt0("del_range()\n");
	hist_room( 2 * ( 1+sel_last - sel_first) );


	for (kn = first; kn <= last; kn++)
		del_list(lp, first);
	pkid = node_kid(lp,first);
	if( leave && (last >= first || not_a_stub(pkid)) )
		prune( pkid );
	 else
		del_list(lp,first);
}

/*
 * Prune the kn'th kid from the list, and delete it if the list is allowed
 * to have listcount-1 elements.
 */
del_list(_lp, kn)
listp _lp;		/* list to delete kid from */
int		kn;		/* kid number in list */
{
	register listp lp = _lp;

	printt2("del_list(%x, %d)\n", lp, kn);

	prune(node_kid(FNCAST lp, kn));

	if (listcount(lp) > 1 ||
	    (listcount(lp) == 1 && canGoToZero(kid1(lp)))) {
		hist_add(H_DEL_LIST, lp, node_kid(lp, kn), kn);
		_del_list(lp, kn);
		}
}

/*
 * Return TRUE if the list whose stub is stubp can have 0 kids.
 * This is true for formal parameters, expressions, set items, and
 * (a kludge) for the filename parameters of the program.
 */
static int
canGoToZero(stubp)
nodep stubp;
{
	int	class = (int)kid1(stubp);
	nodep p;

	if (class == C_FORMAL || class == C_EXP ||
	    (class == C_DECL_ID && (p=tparent(stubp)) && (p=tparent(p)) &&
	     ntype(p) == N_PROGRAM))
		return TRUE;
	else
		return FALSE;
}

/*
 * Change the node type.  Save the old type on the history list.
 */
change_ntype(np, newtype)
nodep np;
int	newtype;
{
	hist_add(H_CHNG_NTYPE, np, NCAST newtype, ntype(np));
	_chng_ntype(np, newtype);
}

/*
 * Change a declaration ID at location cp to newid.
 * This is similar to a prune and graft, but it needs a special
 * operation because it's a nameptr node that's being replaced,
 * and these nodes aren't full fledged nodes.  (They don't have
 * parent fields, for example, and graft/prune update the parent
 * fields.)  Since all the ID references to this ID point at the
 * parent (cp), we can't just prune and replace cp.
 */
chg_decid(cp, newid)
nodep cp;
char	*newid;
{
	char *allid;
	/*
	 * This is split into two operations because we really need
	 * three parameters: the parent cp, the old id sym_pname(cp),
	 * and the new id newid.  So we represent it as a pair of
	 * operations: a prune and graft.  (There is only room for
	 * two node parameters in the history list.)  Swap only stores
	 * two parameters, but the current parent of one of them is an
	 * implicit parameter, and nameptr's don't have parents stored.
	 */

	/*
	 * Do nothing if new id is the same as the old id...more cruft!
	 */
	if (!strcmp(sym_name((symptr)cp), newid))
		return;

	/*
	 * Refuse to let the user change the name to a name which is
	 * already in use.  This is gross.
	 */
	if (inUse(newid) && !case_equal(sym_name((symptr)cp), newid)) {
		mark_line(cp, SMALL_CHANGE);
		error(ER(52,"chg_decid`\"%s\" in use already"), newid);
	}
	allid = allocstring( newid );

	hist_add(H_PRN_DECID, cp, NCAST sym_name((symptr) cp), 0);
	hist_add(H_GFT_DECID, cp, NCAST allid, 0);
	s_sym_name((symptr) cp, allid);
	s_sym_nhash((symptr) cp, hash(allid));
}

/*
 * On node 'np', set 'set' bits and clear 'clear' bits
 */
changeFlags(np, set, clr)
nodep	np;
bits8	set;
bits8	clr;
{
	hist_add(H_CHG_FLAGS, np, NCAST (int)set, clr);
	s_node_flag(np, (node_flag(np) | set) & ~clr);
	mark_line( np, SMALL_CHANGE );
}

/*
 * Swap the trees 'from' and 'to'.  Assumes a direction: if the cursor
 * was on 'from' it will end up on 'to'.
 */
static
_swap(from, to)
nodep from;
nodep to;
{
	register nodep from_parent	= tparent(from);
	register int	from_kn		= from_parent ? get_kidnum(from) : ORPHAN;

	printt2("_swap(%x, %x)\n", from, to);

	if (anchor)			/* turn off selection, lest */
		grab_range(0);		/* we delete an anchor */

	markMoved(from, to);

	mark_change(to, from, from, FALSE);
	check_topgone(from, to);
	check_curgone(from, to);

	detach(from_parent, from_kn, from);
	attach(from_parent, from_kn, to);
}

/*
 * Break any links between the kid and the parent, including declarations
 */
static
detach(parent, kn, kid)
nodep parent;
int	kn;
nodep kid;
{
	extern del_decl();		/* deletes declarations */

	printt3("detach(%x, %d, %x)\n", parent, kn, kid);

	if (!parent || kn == ORPHAN) {
		printt0("nothing to detach from!\n");
		return;
	}

	whatChanged(kid);

	if( is_not_root( parent ) )
		if_sym_call(kid, del_decl);

	s_tparent(kid, NIL);
	s_node_kid(parent, kn, NIL);
}

/*
 * Create links between the kid and the parent, including declarations
 */
static
attach(parent, kn, kid)
nodep parent;
int	kn;
nodep kid;
{
	extern	int add_decl();

	printt3("attach(%x, %d, %x)\n", parent, kn, kid);

	if (!parent || kn == ORPHAN) {
		printt0("nothing to attach to!\n");
		return;
	}

	linkup(parent, kn, kid);

	if( is_not_root(parent) )	/* graft onto root of workspace */
		if_sym_call(kid, add_decl);

	whatChanged(kid);
}

/*
 * Open list 'lp' before kid 'kn'.  Note that 'kn' might equal 'listcount(lp)'.
 * Store 'kid' into empty slot.
 */
static
_exp_list(lp, kn, kid)
listp lp;
int	kn;
nodep kid;
{
	register nodep line;

	printt3("_exp_list(%x, %d, %x)\n", lp, kn, kid);

	if (listcount(lp))
		line = node_kid(lp, kn > 0 ? kn-1 : 0);
	else
		line = tparent(lp);

	if( line != NIL ) /* is this needed, Jan? */
		mark_change(kid, NIL, line, TRUE);

	__exp_list(lp, kn, kid);

	cursor = kid;
	whatChanged(kid);
}

listp 
__exp_list(_lp, kn, kid)
listp _lp;
int	kn;
nodep kid;
{
	register listp lp = _lp;
	register char	*insertion_pt;


	printt3("__exp_list(%lx, %d, %lx)\n", lp, kn, kid);

	/* increase the size of the list, leaves an empty slot at the end. */
	lp = growlist(lp);

	/* move later list elements over one slot */
	insertion_pt = kn * sizeof( nodep ) + (char*)kid1adr(lp);
	printt1("insertion_pt=%lx\n", insertion_pt);

	t_blk_move(insertion_pt + sizeof(nodep ),		/* to, */
		   insertion_pt,				/* from, */
		   ((int)listcount(lp) - kn - 1) * sizeof(nodep ));/* nbytes */

	linkup(lp, kn, kid);

	return lp;
}

/*
 * Remove kid kn from list lp.
 */
static
_del_list(_lp, kn)
listp _lp;
int		kn;
{
	register listp lp		= _lp;
	register char	*deletion_pt;
	register int	lcount		= listcount(lp) - 1;	/* note "- 1" */
	nodep np		= node_kid(lp, kn);
	nodep line;		/* screen line affected */

	deletion_pt = kn * sizeof(nodep) + (char *)kid1adr(lp);
	printt2("_del_list(%x, %d)\n", lp, kn);

	whatChanged(np);

	/*
	 * Mark the change.  If the cursor was on the last element
	 * in the list, we must mark the previous one, unless the list
	 * has only one element in it, in which case we must mark the
	 * list's parent's line.
	 */
	if (lcount > 0)
		line = node_kid(lp, (kn != lcount) ? kn : kn-1);
	else
		line = tparent(lp);
	mark_change(NIL, line, line, TRUE);

	/*
	 * If the cursor or the top of the screen is going to be deleted,
	 * change it to the position of the next kid (or the previous one
	 * if this is the last one (or the parent if this is the only kid)).
	 */
	if (lcount > 0)
		cursor = node_kid(lp, (kn != lcount) ? kn+1 : kn-1);
	else
		cursor = tparent(lp);
	check_topgone(np, cursor);

	/* shrink the list */
	s_tparent(np, NIL);
	t_blk_move(deletion_pt,					/* to, */
		   deletion_pt + sizeof(nodep ),		/* from, */
		   (lcount - kn) * sizeof(nodep ));		/* nbytes */
	s_listcount(lp, lcount);
}

static
_chng_ntype(np, type)
nodep np;
int	type;
{
	whatChanged(np);

	mark_change(np, NIL, np, FALSE);
	s_ntype(np, type);
}

extern	nodep WhereDeclsChanged;
extern	Boolean CodeChanged;
extern	workspace *myWS();

/*
 * Find what changed occured at this node and set appropriate flags
 * and pointers for later typechecking and declaration compiling.
 */
whatChanged(np)
nodep np;
{
	register nodep line;
	register nodep block;
	extern nodep CheckAt;
	register int	lineType;

	printt1("whatChanged(%x) ", np);

	if (myWS(np) != curr_workspace) {
		printt0("not my ws\n");
		return;
	}

	line = t_up_to_line(np);
	if( !(CheckAt && is_ancestor( CheckAt, line )) ) {
		CheckAt = line; 
		printt1( "Checkat set to %x\n", CheckAt );
		}
	block = my_block(line);
	lineType = ntype(line);
	printt4("line=%x, block=%x, lineType=%d(%s)\n", (int)line, (int)block,
		lineType, Safep(NodeName(lineType)));

	if (ntype_info(lineType) & F_DECLARE) {
		printt0("decl changed\n");
		WhereDeclsChanged = block;
	}
	else if (highType(lineType)) {
		printt0("higher up decl changed\n");
#ifdef DEBUG
		if (is_root(block))
			bug(ER(253,"whatChanged/proc at root"));
#endif
		WhereDeclsChanged = my_block(tparent(block));
	}
	else if ( block != NIL && ntype(block) != N_IMM_BLOCK) {
		printt0("code changed\n");
		CodeChanged = TRUE;
	}
}

#ifdef HAS_UNDO

/*
 * Called from growlist(), this routine looks through the history list to
 * change references of oldlp to newlp instead.  (This is necessary when
 * the list has moved in memory).
 */
hist_refs_list(_oldlp, newlp)
listp _oldlp;
listp newlp;
{
	register int		pos;
	register Hist_ent	*hp;
	register listp oldlp = _oldlp;

	printt2("hist_refs_list(%x, %x)\n", oldlp, newlp);

	for (pos = 0; pos <= hist_last; pos++) {
		hp = &history[pos];
		if (hp->from == FNCAST oldlp)
			hp->from = FNCAST newlp;
		if (hp->to == FNCAST oldlp)
			hp->to = FNCAST newlp;
	}
}

zap_history()
{
	printt0("zap_history()\n");

	free_undone_hist();
	while (hist_last > 0)
		free_ancient_hist();

	hist_cur = 0;
	hist_last = -1;
	history[0].action = 0;
	hist_mark();
}
/*
 * check if there is room to do a given editing operation 
 */


hist_room( size )
int size;
{
	char msgbuf[30];
	if( size > MAX_HIST - 3 ) {
#ifdef GEM
		if( form_alert( 1, 
	"[2][This operation is too involved|to UNDO|Do it anyway ?][OK|CANCEL]"
		) == 1 ) {
#else
		warning( ER(284, "nohist`This operation is too involved to UNDO.") );

		getprline( "Do you wish to do it anyway? (y/n) ", msgbuf, sizeof(msgbuf) );
		space_strip( msgbuf );
		if( msgbuf[0] == 'y' ) {
#endif
			/* clear what is there ! */
			zap_history();
			/* turn off recording of history */
			hist_going = FALSE;
			}
		 else
			ncerror( ER(285, "Editing operation aborted" ) );
		}
}

/*
 * Add a mark between user level commands, so that undo/redo work in units
 * of commands, not basic changes.
 */
hist_mark()
{
	printt2("hist_mark(), hist_cur=%d, cursor=%x\n", hist_cur, cursor);
	/*
	 * Assumption: the first time this is called, history[hist_cur].action
	 * is uninitialized.  We assume it contains zero, not H_MARK.
	 */
	hist_going = TRUE;
	if (history[hist_cur].action != H_MARK) /* 2 in a row is wasteful */
		hist_add(H_MARK, cursor, NIL, 0);
	else
		history[hist_cur].from = cursor;/* update last cursor pos */
}

static
hist_dump()
{
	register int	i;
	register Hist_ent *hp;

#ifdef DB_HISTORY
	if (tracing) {
		fprintf(dtrace, "History dump:\n");
		fprintf(dtrace, "hist_cur=%d, hist_last=%d\n", hist_cur, hist_last);
		fprintf(dtrace, "slot\taction\t\tfrom\tto\tkid\n");
		for (i = 0; i <= hist_last; i++) {
			hp = &history[i];
			fprintf(dtrace, "%d\t%d (%s)\t%x\t%x\t%d\n", i,
				hp->action, Safep(actionname(hp->action)),
				(int)hp->from, (int)hp->to, (int)hp->kid);
		}
		fprintf(dtrace, "\n");
	}
#endif
}

/*
 * Record a change on the history list.  Zeroes may be passed
 * for "don't care" arguments.
 */
static
hist_add(action, from, to, kid)
bits8	action;
nodep from;
nodep to;
bits8	kid;
{
	register Hist_ent *hp;

	printt4("hist_add(%d, %x, %x, %d)\n", action, from, to, kid);

	if (!hist_going)
		return;

	if (hist_last > hist_cur)
		free_undone_hist();
	else if (hist_last == MAX_HIST - 1)
		free_ancient_hist();

	++hist_last;
	hp = &history[hist_last];
	hp->action = action;
	hp->from = from;
	hp->to = to;
	hp->kid = kid;
	hist_cur = hist_last;

	if (action != H_MARK)
		dirty_curws();
}

/*
 * Called from hist_add, this routine frees the space in the history list
 * used by the first MARK delimited series of actions.  It also frees
 * the storage used by "dead" trees and stubs.
 */
static
free_ancient_hist()
{
	register Hist_ent *hp;
	register int	i;
	int	j;

	printt0("free_ancient_hist()\n");
	hist_dump();

	/* skip entry 0, which is always a MARK for some reason */
	for (i = 1; i <= hist_last && history[i].action != H_MARK; i++) {
		hp = &history[i];
		printt2("i=%d, action=%s\n", i, actionname(hp->action));
		switch (hp->action) {
		case H_MARK:
		case H_EXP_LIST:
		case H_CHNG_NTYPE:
		case H_CHG_FLAGS:
			break;
		case H_SWAP:
			/* Free tree only if it isn't referenced after. */
			for (j = i+1; j <= hist_last; j++)
				if (history[j].from == hp->from)
					break;
			if (j > hist_last) {
				printt1("freeing unref'd tree %x\n", hp->from);
				treefree(hp->from);
			} else
				printt1("can't free, ref at slot %d\n", j);
			break;
		case H_DEL_LIST:
			treefree(hp->to);	/* free pruned off kid */
			break;
		case H_PRN_DECID:
		case H_GFT_DECID:
/*
 * This is wrong, and it causes free: corrupt arena problems if, for
 * instance, you make two edits to a declaration.  This free would have
 * freed a name table entry!  The storage is recovered at save time,
 * where the unreferenced name is not saved.
 *			free(hp->to);
 */
			break;
		}
	}

	/*
	 * We have freed the appropriate trees from the MARKed region
	 * at the beginning of the history list.  Now move the list
	 * forward that many elements and adjust hist_cur and hist_last.
	 */
	blk_move((pointer)&history[0], (pointer)&history[i],
		 (hist_last + 1 - i) * sizeof(Hist_ent));

	hist_cur -= i;
	hist_last -= i;

	printt0("leaving free_ancient_hist with:\n");
	hist_dump();
}

/*
 * If an undo is immediately followed by a hist_add operation, the last few
 * history elements will be overwritten.  This routine frees the tree storage
 * referenced by these "about to be forgotten" history elements.
 */
static
free_undone_hist()
{
	register Hist_ent *hp;
	register int	i;

	printt0("free_undone_hist()\n");
	hist_dump();

	for ( ; hist_last > hist_cur; hist_last--) {
		hp = &history[hist_last];
		printt2("hist_last=%d, action=%s\n", hist_last,
			actionname(hp->action));
		switch (hp->action) {
		case H_MARK:
		case H_DEL_LIST:
		case H_CHNG_NTYPE:
		case H_CHG_FLAGS:
			break;
		case H_SWAP:
			/* Free tree only if it isn't referenced before. */
			for (i = 0; i < hist_last; i++)
				if (history[i].to == hp->to)
					break;
			if (i == hist_last) {
				printt1("freeing unref'd tree %x\n", hp->to);
				treefree(hp->to);
			} else
				printt1("can't free, ref at slot %d\n", i);
			break;
		case H_EXP_LIST:
			treefree(hp->to);	/* free stub */
			break;
		case H_PRN_DECID:
		case H_GFT_DECID:
/*
 * This is wrong, and it causes free: corrupt arena problems if, for
 * instance, you make two edits to a declaration.  This free would
 * have free a name table entry!  The storage is recovered at save
 * time, where the unreferenced name is not saved.
 *			free(hp->to);
 */
			break;
		}
	}

	printt0("leaving free_undone_hist() with:\n");
	hist_dump();
}

/* Undo one user level command */
undo(cleanup)
int cleanup;		/* true if a cleanup undo */
{
	register Hist_ent *hp;

	printt0("undo()\n");
	hist_dump();

	if (hist_cur == 0) {
		if( cleanup )
			return;
		 else
			error(ER(53,"undo`Nothing to undo"));
		}

	if (!cleanup && history[hist_cur--].action != H_MARK)
		bug(ER(254,"History list bad at start of undo"));

	do {
		hp = &history[hist_cur];
		printt4("\nundo, hist_cur=%d, action %d(%s), cursor=%x\n",
			hist_cur, hp->action, actionname(hp->action), cursor);
		switch (hp->action) {
		case H_MARK:
			/* It will fall out at the bottom of the loop */
			if( cleanup )
				hist_cur++; /* stops it in cleanup */
			break;
		case H_SWAP:
			_swap(hp->to, hp->from);
			break;
		case H_EXP_LIST:
			_del_list(hp->from, hp->kid);
			break;
		case H_DEL_LIST:
			_exp_list(hp->from, hp->kid, hp->to);
			break;
		case H_CHNG_NTYPE:
			_chng_ntype(hp->from, hp->kid);
			break;
		case H_PRN_DECID:
			s_sym_name((symptr) hp->from, (char *)hp->to);
			cursor = hp->from;
			break;
		case H_GFT_DECID:
			s_sym_name((symptr) hp->from, (char *)NULL);
			break;
		case H_CHG_FLAGS:
			s_node_flag(hp->from, (node_flag(hp->from) | hp->kid) &
						~(int)hp->to);
			mark_line( hp->from, SMALL_CHANGE );
			break;
		default:
			bug(ER(255,"Unknown history code %d found in history list"),
				hp->action);
			break;
		}
	} while (history[--hist_cur].action != H_MARK);

	cursor = history[hist_cur].from;
	if( cleanup )
		free_undone_hist();

	dirty_curws();

	grab_range(0);			/* clear selection */
}

/*
 * Redo one user level command.  This operation is the inverse of undo
 * and the code is nearly identical, only backwards.
 */
redo()
{
	register Hist_ent *hp;

	printt0("redo()\n");
	hist_dump();

	if (hist_cur == hist_last)
		error(ER(54,"redo`Nothing to redo"));

	if (history[hist_cur++].action != H_MARK)
		bug(ER(256,"History list bad at start of redo"));

	do {
		hp = &history[hist_cur];
		printt4("redo, hist_cur=%d, action %d(%s), cursor=%x\n",
			hist_cur, hp->action, actionname(hp->action), cursor);
		switch (hp->action) {
		case H_MARK:
			/* It will fall out at the bottom of the loop */
			break;
		case H_SWAP:
			_swap(hp->from, hp->to);
			break;
		case H_EXP_LIST:
			_exp_list(hp->from, hp->kid, hp->to);
			break;
		case H_DEL_LIST:
			_del_list(hp->from, hp->kid);
			break;
		case H_CHNG_NTYPE:
			_chng_ntype(hp->from, (int)hp->to);
			break;
		case H_PRN_DECID:
			s_sym_name((symptr) hp->from, (char *)NULL);
			break;
		case H_GFT_DECID:
			s_sym_name((symptr) hp->from, (char *) hp->to);
			cursor = hp->from;
			break;
		case H_CHG_FLAGS:
			s_node_flag(hp->from, (node_flag(hp->from) | (int)hp->to) &
						~hp->kid);
			mark_line( hp->from, SMALL_CHANGE );
			break;
		default:
			bug(ER(255,"Unknown history code %d found in history list"),
				hp->action);
			break;
		}
	} while (history[++hist_cur].action != H_MARK);

	cursor = history[hist_cur].from;

	dirty_curws();

	grab_range(0);			/* clear selection */
}

#else HAS_UNDO
hist_refs_list(){}
zap_history(){}
hist_mark(){}
hist_add(){}
undo(){}
redo(){}
#endif HAS_UNDO
