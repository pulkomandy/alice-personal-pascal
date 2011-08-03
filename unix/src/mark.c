/*
 *DESC: pop back, mark and go -- remember and return to a node (if possible)
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "alctype.h"

#ifdef HAS_MARK

#ifdef DB_MARK
# define PRINTT
#endif
#include "printt.h"

#define	NBACKMARKS	5

#define FREED		(nodep )1
#define	NMARKS		26			/* one for each letter */

extern	workp	ws_list;

static	nodep BackMarks[NBACKMARKS];		/* starts off NULL */
static	int	BMPos = 0;

static	nodep Marks[NMARKS];			/* starts off NULL */

/*
 * The pop back routines allow the analogy of '' from vi ... return to
 * a previous point of work, if possible.  Unlike vi, it is possible
 * to go back more than one position.
 */
markPopBack(np)
nodep np;
{
	printt1("markPopBack(%x)\n", np);

	BMPos = (BMPos < NBACKMARKS - 1) ? BMPos + 1 : 0;
	BackMarks[BMPos] = np;
}

/*
 * "Pop back" to a previous cursor position, if possible..
 */
popBack()
{
	register int	i;
	register int	pos;

	printt0("popBack()\n");

	for (i = 0, pos = BMPos;
	     i < NBACKMARKS;
	     i++, pos = pos ? pos - 1 : NBACKMARKS-1)
		if (goTo(BackMarks[pos]))
			return;
	error(ER(74,"badmark`Nothing to go back to"));
}

/*
 * Send the cursor to a node.  The node might be in a different workspace, or
 * might even have been removed from the tree.
 */
goTo(where)
nodep where;
{
	register nodep	np;
	register workp	wp;

	printt1("goTo(%x)\n", where);

	if (where == NULL)
		return FALSE;

	for (np = where; tparent(np); np = tparent(np))		/* find root */
		;
	for (wp = ws_list; wp; wp = work_next(wp)) {
		if ((workp)np == wp) {
			if (wp != curr_workspace)
				go_to_workspace(work_name(wp));
			cursor = where;
			printt1("cursor=%x\n", cursor);
			return TRUE;
		}
	}
	return FALSE;
}

static
getMarkIndex(args)
char	*args;
{
	char	line[MX_WIN_WIDTH];
	register char	m;

	if (args && args[0])
		m = args[0];
	else
		m = getKeyPrompt("Mark? ");
	if (isupper(m))
		m = tolower(m);
	if (m < 'a' || m >'z')
		error(ER(75,"badmark`marks must be a letter"));

	return	(m - 'a');
}

mark(args)
char	*args;
{
	Marks[getMarkIndex(args)] = cursor;
}

go(args)
char	*args;
{
	register int	m;
	register nodep amark = Marks[m = getMarkIndex(args)];

	markPopBack(cursor);

	if (amark == NULL)
		error(ER(76,"badmark`mark '%c' hasn't been set"), m + 'a');

	if (amark == FREED || !goTo(amark))
		error(ER(77,"badmark`mark '%c' has been deleted"), m + 'a');
}

/*
 * marksGoing() is called to invalidate all the marks in a subtree,
 * since it is about to be treefree()'d.
 */

marksGoing(subtree)
nodep	subtree;
{
	register int	i;
	nodep	mark;

	for (i = 0; i < NBACKMARKS; i++) {
		mark = BackMarks[i];
		if (mark > FREED && is_ancestor(subtree, mark))
			BackMarks[i] = FREED;
		}
	for (i = 0; i < NMARKS; i++) {
		mark = Marks[i];
		if (mark > FREED && is_ancestor(subtree, mark))
			Marks[i] = FREED;
		}
}

markMoved(_from, _to)
nodep _from;
nodep _to;
{
	register nodep from = _from;
	register int	i;
	register nodep to = _to;

	for (i = 0; i < NBACKMARKS; i++)
		if (BackMarks[i] == from)
			BackMarks[i] = to;
	for (i = 0; i < NMARKS; i++)
		if (Marks[i] == from)
			Marks[i] = to;
}
#else HAS_MARK
go() {}
mark() {}
popBack() {}
markMoved(){}
marksGoing(deadNode) {}
markPopBack(np) {}
#endif HAS_MARK
