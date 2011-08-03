#include "alice.h"
#include "action.h"
#include "typecodes.h"
#include "class.h"
#include "alctype.h"

/*
 *DESC: Handles 'actions' which are keyed by the typing of tokens on nodes
 *DESC: and placeholders.
 */

extern char *safeprint();
extern char tokentext[];

/* Tests for pending input either stuffed in the special input buffer by
 * the scanner or text routines that call it.  This does not refer to
 * the keyboard buffer
 */

input_pending()
{
	extern int nptok;
	return (lookaheadchar || (nptok>0));
}

/* Carry out the action indicated by act */

/*
 * This routine looks at the action specified.  If it is in the range 128..255,
 * it is considered an EXPAND action for act-255.  Otherwise it should be
 * passed through the big CASE in actrout
 */
do_action(act,toknum,foundnode)
ireg2 int act;		/* action specified */
ireg3 int toknum;		/* token that prompted the action */
preg1 nodep foundnode;	/* node where action was found */
{
	ireg1 int nt;

	if (act == 0) {
		if( find_class( toknum, ex_class(cursor ) )) {
			/* a stub would have allowed this! */
			ncerror( ER(1,"badtok2`'%s' isn't valid here.  You could type '%s' if you did a delete"), safeprint(tokentext), tokname(toknum) );
			}
		 else
			ncerror(ER(2,"badtoken`The input '%s' is not valid at the cursor.  Try the HELP key"), safeprint( tokentext ), tokname(toknum) );
	} else if (act >= EXPAND) {
		nt = act - EXPAND;
#ifdef DB_ACTION
		if (tracing) fprintf(dtrace, "expand node type %d\n", nt);
#endif
		exp_prod(nt);
	} else {
		actrout(act,toknum,foundnode);
	}
}

/*
 * Expand production #nt on cursor.
 */

exp_prod(nt)	/* first routine expands and skips */
reg NodeNum	nt;
{
	do_exp_prod(nt, NIL);
	skip_down();
}
/* 
 * This does the general expansion, but doesn't move the cursor afterwards.
 * Essentially it tells the difference beteween symbols and normal nodes
 */
/* returns the new expanded node */

do_exp_prod(nt,intype)	/* this does the actual expansion */
ireg2 NodeNum nt;		/* what kind of node to expand */
type_code intype;	/* what type variable gets if declaring one */
{
	preg1 nodep nnode;		/* pointers to created nodes */
	ireg1 int i;			/* to loop through kids making stubs */
	extern char tokentext[];	/* text of the token typed */
	listp newlist();

	/* Build the subtree (just a node of stubs for now) */
	switch (nt) {
	case N_DECL_ID:		/* an ID for declaration */
	case N_HIDECL_ID:	/* a special id for declaration one symbol table
					upwards */
		nnode = NCAST s_declare(tokentext, intype);
		break;
	case N_ID_FIELD:	/* special id for declaring field names */
		nnode = NCAST s_declare(tokentext, T_FIELD);
		break;	
	case N_ID:		/* an ID for use */
		nnode = symref(tokentext);
		break;
	default:
		nnode = tree(nt, NIL,NIL,NIL,NIL,NIL,NIL,NIL,NIL);
		/* now fill in all the stubs */
		for( i = 0; i < kid_count(nt); i++ ) {
			fresh_stub( nnode, i );
			}

	}

#ifdef DB_ACTION
	if (tracing) fprintf(dtrace, "build node %x\n", (int)nnode);
#endif

	graft(nnode, cursor);
	cursor = nnode;
}

/*
 * Advance the cursor to the next unfilled-in stub.
 * CONSOLIDATE ALL THESE SKIPPING ROUTINES SOON
 */
skip_to_next_stub()
{
	preg1 curspos holdcp;

	holdcp = cursor;
	do {
#ifdef DB_ACTION
		if(tracing) fprintf(dtrace, "cursor at %x\n", PCURS);
#endif
		cursor = c_nextpos(cursor);
	} while ( cursor != holdcp && not_a_stub( cursor ) && !c_at_root(cursor));
	if (c_at_root(cursor)) {
#ifdef DB_ACTION
		if(tracing) fprintf(dtrace,
			"cursor moved to %x, parent %x\n", PCURS, (int)node_parent(cursor));
#else
		;
#endif
	} while (not_a_stub( cursor ) && !c_at_root(cursor));
	if (c_at_root(cursor)) {
#ifdef DB_ACTION
		if (tracing) fprintf(dtrace, "cursor at root, reset to %x\n",
			PCURS);
#endif
		cursor = holdcp;
	}
#ifdef DB_ACTION
	else
		if (tracing) fprintf(dtrace, "cursor skipped to %x\n", PCURS);
#endif
}

/* skip down - goes down to empty stub kid */

skip_down()
{
#ifdef DB_ACTION
	if (tracing) fprintf(dtrace, "skip_down, cursor starts at %x stub not %d is %d\n", (int)cursor, not_a_stub(cursor), is_a_stub(cursor));
#endif
	if( not_a_stub(cursor) ) {
#ifdef DB_ACTION
	if (tracing) fprintf(dtrace, "skip_down, cursor type %d, kid_count %d, lookaheadchar %d\n", ntype(cursor), kid_count(ntype(cursor)), lookaheadchar);
#endif
		/* we must think on this, further */
		if( kid_count(ntype(cursor))||!input_pending()) {
			preg1 nodep crline;
			preg2 nodep moldcur;

			moldcur = cursor;
			crline = t_up_to_line(cursor);
			skip_to_next_stub();
			if( !is_ancestor( crline, cursor ) )
				cursor = moldcur;
			}
	 	 else 
		/* skip downwards */
			while( not_a_stub(cursor) && (is_a_list(cursor)
					|| kid_count(ntype(cursor)) ))
				cursor = realcp( node_kid( cursor, 0 ) );
		}
#ifdef DB_ACTION
	if (tracing) fprintf(dtrace, "skip_down, cursor ends at %x\n", (int)cursor);
#endif

}

/* test to see if one node is ultimately the parent of another */

is_ancestor( ancestor, _childnode )
preg2 nodep ancestor;
nodep _childnode;		/* the guy whose lineage we check */
{
	preg1 nodep childnode = _childnode;

	while( childnode != NIL /* && is_not_root(childnode) */ )
		if( childnode == ancestor )
			return TRUE;
		 else
			childnode = tparent( childnode );
	return FALSE;
}

/*
 * Place a stub on the specified kid of a specified node.  The class is
 * looked up and marked appropriately.  Not a history action
 */

fresh_stub( xnnode, i )
nodep xnnode;	/* where stub is going */
ireg1 int i;		/* which child within node is stub */
{
	preg1 nodep nstub;
	preg2 nodep nnode;
	ClassNum ce;
	Boolean	makeList;
	nnode = xnnode;

	ce = kid_class( ntype(nnode), i );

	makeList = ce >= CLIST(0);		/* fix for CLIST(C_PASSUP) */

	if( (ce & CLASSMASK) == C_PASSUP )
		ce = ex_class( nnode );
	/* assume stubs have ONE child */
	nstub = make_stub( ce & CLASSMASK );
#ifdef DB_ACTION
	if(tracing) fprintf( dtrace, "New stub of type %d - %s\n",
		ce, classname(ce) );
#endif
	if (makeList)
		nstub = FNCAST newlist( nstub ); /* sets parent*/
	linkup( nnode, i, nstub );
}

/* turn a buffer into something safe to print, max 16 chars */
char *
safeprint( txbuf )
char *txbuf;
{
	char srcbuf[20];
	preg1 char *sp;
	preg2 char *dp;

	strncpy( srcbuf, txbuf, 16 );

	dp = txbuf;
	sp = srcbuf;

	for( sp = srcbuf; *sp; sp++ ) {
		if( isprint( *sp ) ) {
			*dp++ = *sp;
			continue;
		}
		if( *sp < ' ' ) {
			*dp++ = '^';
			*dp++ = *sp + '@';
			continue;
		}
		*dp++ = '?';
	}
	*dp++ = 0;
	return txbuf;
}
