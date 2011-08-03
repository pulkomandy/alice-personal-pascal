/*
 * Yet more commands for Alice
 */
#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "keys.h"
#include "flags.h"
#include "dbflags.h"
#include "class.h"
#include "menu.h"
#include "typecodes.h"
#include "extramem.h"

#ifdef DB_CMDS2
# define PRINTT
#endif
#include "printt.h"

#ifdef GEM
extern int MvSlider;
#endif

tryhide()
{
#ifdef HAS_COM
	register nodep hidenode;
	register nodep wheretopatch;

	if(is_a_hide(cursparent) )
		cursor = cursparent;

	grab_range(R_FORCE|R_FLIST);
	if( c_at_root(sel_node)) {
		error( ER(10,"hideprogram`Can't hide the whole program") );
		}
	wheretopatch = node_kid(sel_node,sel_first);
	if( !(ntype_info(ntype(wheretopatch)) & F_LINE ) )
		error( ER(11,"badhide`You must select at least a whole line for hiding") );
	if( sel_first == sel_last  && is_a_hide(wheretopatch) ) {
		change_ntype(wheretopatch, N_HIDE);
		cursor = wheretopatch;
		}
	 else {
		hidenode = l_lower(N_HIDE, 1);
		cursor = kid1(hidenode);	/* the hide comment */
		}
			
#endif HAS_COM
}

curspos
ins_node( place, newtype, whatkid )
nodep place;			/* where insertion takes place */
NodeNum newtype;		/* node to be inserted */
int whatkid;			/* which kid we insert stuff into */
{
	register nodep savedpl;		/* save our place */
	register curspos retval;
	reg curspos savcur;		/* save cursor ? */

	savcur = cursor;

	savedpl = place;
	prune(place);
	do_exp_prod(newtype,NULL);
	retval = cursor;
	/* restore what we clipped */
	graft( savedpl, realcp(node_kid(cursor, whatkid )) );
	cursor = savcur;
	return retval;
}
	

tryreveal( )
{
#ifdef HAS_COM
	if( ntype(cursparent) == N_HIDE )
		cursor = cursparent;
	else if( ntype( cursor ) != N_HIDE ) {
		error( ER(12,"badreveal`Not a hidden part of the program") );
		}
	change_ntype( cursor, N_REVEAL );

#ifdef GEM
	count_lines();
#endif

#endif HAS_COM
}

/* Routine to build a range from two points in the tree */

nodep sel_node;			/* selected node */
nodep anchor = 0;		/* range select anchor */
int sel_first;			/* first in a selected list */
int sel_last;			/* last in a selected list */

bld_range( p1, p2 )
nodep p1; /* first point */
nodep p2; /* second point */
{
	/* first find the common ancestor */
	register nodep up1;
	register nodep up2;
	reg nodep old1;
	reg nodep old2;
	
	for( up1 = p1, old1 = p1; up1; up1 = tparent(up1) ) {

		for( up2 = p2, old2= p2; up2; up2 = tparent(up2) )
			if( up2 == up1 ) /* Got a common ancestor */
				goto gotancest;
			 else
				old2 = up2;
		/* for failed */
		old1 = up1;
		}
	/* we terminated */
gotancest:
	sel_last = -1;		/* indicates pure node*/
	sel_node = up1;
	if( up1 ) {
		/* got a real ancestor */
		if( is_a_list(up1) ) {
			sel_first = get_kidnum( old1 );
			sel_last = get_kidnum( old2 );
			if( sel_last < sel_first ) {
				int temp;
				temp = sel_last;
				sel_last = sel_first;
				sel_first = temp;
				}
			}
		 else {
			NodeNum part;
			nodep snp;

			snp = tparent(sel_node);
			/* otherwise already setup */
			part = ntype(snp);
			if( (part == N_ST_CALL || part == N_EXP_FUNC ) &&
					listcount( kid2(snp) ) == 0 )
				sel_node = snp;
			}
		}
	 else 
		sel_node = cur_work_top;	/* program node */
#ifdef DB_CMDS2
	if(tracing)fprintf(trace,"Range from %x to %x is %x start=%d, end=%d\n",
		p1, p2, sel_node, sel_first, sel_last );
#endif
}

/* routine to grab a range for a command that wants one */
/* the selflag sets certain error checks */
/*
	R_FORCE - insist on selection
	R_LEAF - while insisting, allow a leaf without selection
	R_NOLIST - can't apply this command to a list
	R_FLIST - insist on a list
		R_LEAF and R_FLIST give the special delete feature
				of giving a single list element if needed.
*/

bool noforce = FALSE;

grab_range(selflag)
reg int selflag;		/* ok if nothing selected */
{
	if( anchor ) {
		bld_range( cursor, anchor );
		if( sel_last >= 0  && selflag & R_NOLIST ) {
			error( ER(13,"badsellist`This command can't be applied to a list") );
			}
		anchor = NIL;
		/* turn off the selected stuff */
		clr_oldlines();
		vid_alter(FALSE, 1);
		}
	 else {
		if( (selflag & R_LEAF && kid_count(ntype(cursor)) == 0 )
				|| !(selflag & R_FORCE ) || noforce ) {
			sel_node = cursor;
			sel_last = -1;
			}
		 else {
			error( ER(14,"needrange`You must select an item or range first - see SELECT command") );
			}
		}
	/* force a list even if they didn't mean it */
	if( selflag & R_FLIST && sel_last < 0) {
		if( not_a_list(tparent(sel_node)) ) {
			if( !(selflag & R_LEAF) )
				error( ER( 15,"needlist`Selected item must be an element in a list") );
			}
		 else {
			sel_first = sel_last = get_kidnum(sel_node);
			sel_node = tparent(sel_node);
			}
		}
	return TRUE;
}

	/* list lowering routine.  Takes a selected and puts it in
	   a new list */
listp 
list_range()
{
#ifdef HAS_COM
	register listp madelist;
	register int index1;
	reg int index2;

	if( sel_last >= 0) {
		madelist = sized_list(sel_last - sel_first + 1);
		index2 = 0;
		for( index1 = sel_first; index1 <= sel_last; index1++ )
			s_node_kid(madelist,index2++,node_kid(sel_node,index1));
		}
	 else {
		/* if(is_a_list(tparent(sel_node))) {
			madelist = newlist( sel_node );
			sel_first = sel_last = get_kidnum(sel_node);
			sel_node = tparent(sel_node);
			}
		 else */
			error(ER(15,"needlist`Your selection must be an item in a list") );
		}
	return madelist;
#endif HAS_COM
}

/* command to lower a specified list inside a node of appropriate type */

nodep 
l_lower( newnode, lkid )
NodeNum newnode;	/* node code is to be lowered into */
int lkid;		/* which kid it is to be placed in */
{
#ifdef HAS_COM
	register listp nlist;
	register nodep created;
	register int index;
	int	nk;

	/*
	 * This routine works by deleting selected elements, creating
	 * a 'newnode' in their place, and reattaching the
	 * deleted elements somewhere on a list child of 'newnode'.
	 *
	 * If a node is removed with prune/dellist, it *must* be
	 * reattached with graft/explist if undo and redo are to work
	 * properly.
	 *
	 * First, build a new list to hold the elements which are
	 * going away.  (There must be a better way to remember
	 * deleted elements...)
	 */
	 /* first check history chances */

	hist_room( 4 * (sel_last - sel_first + 1) );
	nlist = list_range();
	
	/*
	 * Then delete the selected elements.  Just prune the first one,
	 * which leaves a stub to build on.
	 */
	if (++sel_first <= sel_last)
		del_range(FALSE);
	prune(cursor = node_kid(sel_node, --sel_first));

	/*
	 * Create the newnode and graft it to the tree.
	 */
	created = tree(newnode, NIL, NIL, NIL, NIL, NIL);
	nk = reg_kids(created);
	graft(created, cursor);
	for (index = 0; index < nk; index++)
		fresh_stub(created, index);

	/*
	 * Graft each previously deleted node onto the newnode's list,
	 * which is its 'lkid'th child.  This assumes node_kid(created,lkid)
	 * is a list.
	 */
	cursor = kid1(node_kid(created, lkid));
	for (index = listcount(nlist) - 1; /* test in middle */; index--) {
		graft(node_kid(nlist, index), cursor);
		if (index == 0)
			break;
		exp_list(0);
	}
#ifdef old
	graft(kid1(nlist), cursor);
	for (index = 1; index < listcount(nlist); index++) {
		exp_list(1);
		graft(node_kid(nlist, index), cursor);
	}
#endif

	/*
	 * Free the storage used by the list, its not used any more.
	 */
	tfree(nlist);		/* NOT treefree! */

	return created;
#endif HAS_COM
}

/* command to begin the selection of a range */

do_select()
{
	bld_range( anchor = oldcur = cursor, cursor );
	clr_oldlines();
	vid_alter( TRUE, 1 );
}

static unsigned char curs_keys[] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 0 };

extern alice_window main_win;
extern nodep  ocursor;
int phys_pcode;		/* actual pcode cursor is on */
new_phys(code)
int code;		/* cursor direction */
{
	struct scr_line cline;
	extern int inbuf_count;
	int ola;
#ifdef GEM
	int skip_count = 0;
#endif

	phys_row = srch_row;
	phys_column = srch_real;

	while( TRUE ) {

		switch( code ) {
			case KEY_LEFT:
				if( phys_column )
					phys_column--;
				
				break;
			case KEY_DOWN:
				if( phys_row < curr_window->w_height - 1 )
					phys_row++;
		 		else {
					bot_cdown( curr_window );
#ifdef GEM
					if( MvSlider )
						SetLine( 1 );
#endif
				}
				break;
			case KEY_UP:
				if( phys_row )
					phys_row--;
		 		else {
					top_cup( curr_window );
#ifdef GEM
					if( MvSlider )
						SetLine( -1 );
#endif
				}
				break;
			case KEY_RIGHT:
				if( phys_column < curr_window->w_width - 1 )
					phys_column++;
				break;	
#ifdef HAS_POINTING
			case KEY_COORD:
				{
				/* Handle mouse movement */
#ifdef GEM
				GMouse( srch_row, srch_real );
#endif
				return;
				}
				break;
#endif HAS_POINTING
			case 0:
				break;
			default:
				/* stuff in lookahead buffer and quit */
				setlachar( code );
				find_phys_cursor( curr_window, cursor, TRUE );
				srch_row = phys_row;
				srch_real = phys_column;
				ocursor = cursor;	/* where we left it */
				return;
			}

		/* make all this conditional on type-ahead */
		cursor = find_alice_cursor( curr_window, phys_row, phys_column );
		scrl_copy( cline, curr_window->w_lines[phys_row] );
		if( cursor == line_headnode( cline ))
			phys_pcode = cline.pcode;
		 else
			phys_pcode = 0;

		newrange(TRUE);
		wmove(curr_window->w_desc, phys_row, phys_column );

#ifdef GEM
		ola = get_lookahead();
		/*
		 * If we have a cursor key in the lookahead and we haven't
		 * skipped 3 lines so far, then just skip updating this
		 * time
		 */
		if( ola && strchr( curs_keys, ola ) && (skip_count < 3))
			skip_count++;
		else {
			skip_count = 0;
			wrefresh( curr_window->w_desc );
		}
#else GEM

#ifndef ICON
		if( !((ola = get_lookahead()) && strchr( curs_keys, ola ) ) ||
			inbuf_count > KBUF_SIZE - 2 )
#endif
			wrefresh(curr_window->w_desc);

#endif GEM
		/* now read in a new character */

		code = readkey();

		} /* end while */
}

#ifdef GEM

/*
 * Handle a mouse press
 */

GMouse( row, col )
int row, col;
{
	WINDOW *win;
	int new_x, new_y;
	int in_selection = FALSE;
	int acceleration = 0;
	int num_scrolls = 0;
	int i;

	win = curr_window->w_desc;

	/*
	 * Set where the cursor is currently
	 */
	phys_column = col;
	phys_row = row;

	/*
	 * If there already is a selection, clear it
	 */
	if( anchor ) {
		grab_range( 0 );
		display( TRUE );
		wrefresh( win );
	}

	/*
	 * Set the column to whatever we are at
	 */
	new_x = gptr_xp( win, FALSE );

	if( new_x >= 0 && (new_x < win->_maxx) )
		phys_column = new_x;


	new_y = gptr_yp( win, FALSE );

	if( new_y >= 0 && (new_y < win->_maxy) ) {
		phys_row = new_y;
	}

	/*
	 * Go and move the cursor, etc
	 */
	GUpdate( phys_row, phys_column );

	for( ;; ) {

		/*
		 * While the mouse is down, and we haven't dragged anywhere,
		 * just wait...
		 */

		for( ;; ) {
			new_x = gptr_xp( win, FALSE );
			new_y = gptr_yp( win, FALSE );

			if( new_x != phys_column ) break;
			if( new_y != phys_row ) break;

			if( new_y < 0 || new_y == win->_maxy ) break;

			if( !m_down() ) break;
		}

		printt3( "Mouse: %d, X: %d, Y: %d\n", m_down(), new_x, new_y );
		printt2( "PhysRow: %d, PhysColumn: %d\n", phys_row, 
			phys_column );

		/*
		 * If the mouse came up, exit
		 */
		if( !m_down() ) break;

		/*
		 * If this is the first time through the loop, then
		 * start the selection process
		 */
		if( !in_selection ) {
			do_select();
			display( TRUE );
			in_selection = TRUE;
		}

		/*
		 * Set the column to whatever we are at
		 */
		if( new_x >= 0 && (new_x < win->_maxx) )
			phys_column = new_x;
		
		/*
		 * If we are above or below the window, scroll
		 */
		if( new_y < 0 ) {
			num_scrolls++;
			acceleration = (num_scrolls / 2) + 1;
			if( phys_row ) {
				phys_row--;
			} else
				for( i=0; i<acceleration; i++ ) {
					top_cup( curr_window );
					if( MvSlider )
						SetLine( -1 );
				}
		}
		else if( new_y >= win->_maxy ) {
			num_scrolls++;
			acceleration = (num_scrolls / 2) + 1;
			if( phys_row < (win->_maxy - 1) ) {
				phys_row++;
			} else
				for( i=0; i<acceleration; i++ ) {
					bot_cdown( curr_window );
					if( MvSlider )
						SetLine( 1 );
				}
		}
		else {
			num_scrolls = 0;
			phys_row = new_y;
		}

		/*
		 * Update the cursor position, etc
		 */
		GUpdate( phys_row, phys_column );

	}

	/*
	 * The mouse came up, so we are finished
	 */
	find_phys_cursor( curr_window, cursor, TRUE );
	srch_row = phys_row;
	srch_real = phys_column;
	ocursor = cursor;

}

GUpdate( row, col )
int row, col;
{
	struct scr_line cline;

	/*
	 * Now go and find the cursor
	 */
	cursor = find_alice_cursor( curr_window, row, col );

	scrl_copy( cline, curr_window->w_lines[row] );

	if( cursor == line_headnode( cline ) )
		phys_pcode = cline.pcode;
	else
		phys_pcode = 0;

	newrange( TRUE );

	wmove( curr_window->w_desc, row, col );
	wrefresh( curr_window->w_desc );
}

#endif

/*
 *  DELETE function - on the DEL key 
 *  This function deletes selected items.
 *  If the selected item is a list, it del_ranges.
 *  if it is a subtree who's parent is a list, it del_ranges.
 *  otherwise it prunes
 *  You can delete a leaf without selection
 */

do_delete(arg)
int arg;	/* true if we want to leave a placeholder */
{
	grab_range(R_FORCE | R_LEAF | R_FLIST );

	if( sel_last < 0 )
		chprune( sel_node );
	 else
		del_range(arg);
}

menuact( hdir, menucom )
char *hdir;		/* help directory */
char *menucom;		/* command from menu */
{
#ifdef HAS_COM
#ifdef DB_CMDS2
	if(trace)fprintf(trace, "Menu action %s in directory %s\n", menucom, hdir );
#endif
	if( menucom[0] == 'H' )
#ifdef GEM
		helpfile( menucom+1, "" );
#else
		helpfile( hdir, menucom );
#endif
 	else {
		int comnum;
	
		comnum = look_cmd(menucom);
		if( comnum )
			bi_command( comnum, (pointer)0 );
	 	else
			error( ER(17,"unfinished`We haven't implemented %s yet"), menucom);
		}
#endif HAS_COM
}

do_errhelp(args)
char *args;
{
#ifdef HAS_COM

	extern char last_ecode[];

	if( args )
		helpfile( "perror/", args );
	 else {

		if( *last_ecode )
			helpfile( "perror/", last_ecode );
	 	else
			pure_message( LDS( 408, 
		"Sorry, can't elaborate any further on the last message") );
		}
#endif HAS_COM
}

do_elsemap( ifnode )
nodep ifnode;
{
	/* if the cursor is on an empty statement stub, delete it */
	if( is_a_stub( cursor ) && int_kid( 0, cursor ) == C_STATEMENT &&
							!anchor )
		do_delete();
	cursor = realcp( kid3( transmog( ifnode, N_ST_ELSE) ) );
}

#define TRSMALL 0
#define TRREP 1
#define TRFOR 2
#define TRWSPC 3
#define TRSIMPLE 4
#define TRROUTINE 5
#define TRVAR 6

#define STATCLASS 0
#define ROUTCLASS 1
#define VARCLASS 2

bits8 mapclass[] = { STATCLASS, STATCLASS, STATCLASS, STATCLASS, STATCLASS,
ROUTCLASS, VARCLASS };

bits8 tr_table[] = {
N_ST_IF, TRSMALL,
N_ST_ELSE, TRSMALL,
N_ST_WHILE, TRSMALL,
N_ST_REPEAT, TRREP,
N_ST_FOR, TRFOR,
N_ST_DOWNTO, TRFOR,
/* N_ST_WITH, TRWSPC, */
N_ST_SPECIAL, TRWSPC,
N_ST_BLOCK, TRSIMPLE,
N_ST_COUT, TRSIMPLE,
N_HIDE, TRWSPC,
N_REVEAL, TRWSPC,
N_DECL_PROC, TRROUTINE,
N_DECL_FUNC, TRROUTINE,
#ifdef N_VAR_ABSOLUTE
N_VAR_ABSOLUTE, TRVAR,
N_VAR_DECL, TRVAR,
#endif
0 };

/* what kid the lines are on for a class  */

static bits8 lkid[] = { 1, 0, 3, 1, 0 };

static bits8 ckid[] = { 0, 1, MAX_NODE_KIDS+1,MAX_NODE_KIDS+1,MAX_NODE_KIDS+1};

static bits8 proctofun[] = { 1, 2, 4, 5, 6, 0, 0, 0 };
static bits8 funtoproc[] = { 1, 2, 0, 3, 4, 5, 0, 0 };
#ifdef FULL
static bits8 vartoabs[] = { 1, 2, 4, 0, 0, 0, 0, 0 };
static bits8 abstovar[] = { 1, 2, 0, 3, 0, 0, 0, 0 };
#endif


extern nodep raw_transmog();

nodep 
transmog(_oldnode, newtype)
nodep _oldnode;	/* node to be transformed */
NodeNum	newtype;	/* new type for it */
{
#ifdef HAS_COM
	nodep newguy;
	register int	newcount;
	register nodep oldnode	= _oldnode;
	bits8	*tp;				/* table pointer */
	int	ckiddex;
	int	i;
	int	oldclass, newclass;
	bits8	trmap[MAX_NODE_KIDS];
	int	oldckid;

	if( ntype(oldnode) == newtype )
		return;
	tp = table_lookup(tr_table, ntype(oldnode), 2);
	for( i = 0; i < MAX_NODE_KIDS; i++ )
		trmap[i] = 0;
#ifdef DEBUG
	if (!tp)
		error(ER(18,"transmogrify attempted on %d node"), ntype(oldnode));
#endif
	oldclass = *tp;
	tp = table_lookup(tr_table, newtype, 2);
#ifdef DEBUG
	if (!tp)
		error(ER(18,"transmogrify attempted to %d node"), newtype);
#endif
	newclass = *tp;

	switch( mapclass[oldclass] ) {

	 case STATCLASS:
		if (oldclass == TRFOR && newclass == TRFOR) {
			change_ntype(oldnode, newtype);
			return oldnode;
			}


		trmap[lkid[oldclass]] = lkid[newclass] + 1;

		oldckid = ckid[oldclass];
		ckiddex = ckid[newclass];

		if (oldckid <= MAX_NODE_KIDS && ckiddex <= MAX_NODE_KIDS ) 
			trmap[oldckid] = ckiddex+1;

		return raw_transmog( oldnode, trmap, newtype );
	case ROUTCLASS:
		newguy = raw_transmog( oldnode, newtype == N_DECL_PROC ?
			funtoproc : proctofun, newtype );

		if( not_a_stub( kid1(newguy) ) )
			s_sym_dtype( (symptr)kid1(newguy), newtype ==
				N_DECL_PROC ? T_PROCEDURE : T_FUNCTION );
		return newguy;
#ifdef FULL
	case VARCLASS:
		{
		NodeNum tynum;

		tynum = ntype(kid2(oldnode));
		if( tynum != N_ID && tynum != N_STUB )
			error( ER(18,"badspec`Type must be a named type or placeholder for absolute") );
		return raw_transmog( oldnode, newtype == N_VAR_DECL ? abstovar :
				vartoabs, newtype );
		}
#endif
	 }
#endif HAS_COM
}


nodep
raw_transmog( oldnode, wharray, newtype )
nodep oldnode;		/* node being altered */
bits8 *wharray;		/* array of mappings */
NodeNum newtype;	/* the new nodetype */
{
	int whindex;
	int newcount;
	int i;
	register nodep newguy;
	nodep keepers[MAX_NODE_KIDS];


	for( whindex = 0; whindex < MAX_NODE_KIDS; whindex++ ) {
		if( wharray[whindex] ) {
			keepers[whindex] = node_kid(oldnode,whindex);
			prune(keepers[whindex]);
			}
		 else
			keepers[whindex] = NIL;
		}

	prune(oldnode);

	if( ntype_info(newtype) & F_SYMBOL ) {
		dec_block( newtype );
		newguy = cursor = find_true_parent( cursor );
		}
	 else {
		newguy = tree(newtype, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL);
		newcount = reg_kids(newguy);
		graft(newguy, cursor);
		for (i = 0; i < newcount; i++)
			fresh_stub(newguy, i);
		}

	for( whindex = MAX_NODE_KIDS-1; whindex >= 0; whindex-- ) 
		if( wharray[whindex] ) 
			graft( keepers[whindex], node_kid(newguy,
					wharray[whindex]-1 ) );

	return newguy;
}
do_lower( prepared )
int prepared;		/* has the range been prepared */
{
#ifdef HAS_COM
	register nodep res;

	if( !prepared ) {
		grab_range( R_FORCE|R_FLIST);
		}
	sterr();
	res = l_lower( N_ST_SPECIAL, 1 );
	cursor = kid1( res );
#endif HAS_COM
}

sterr() {
	if( ex_class( sel_node ) != C_STATEMENT )
		error( ER(20,"statonl`The selected item must be a list of statements") );
}

#ifndef WRONG

do_raise()
{
#ifdef HAS_COM
	register nodep kid;
	register int i;
	register listp listToRaise;
	listp list1, list2;
	int iselse;
	int	pass;
	bits8	*p;

	grab_range(R_FORCE|R_NOLIST);
	p = table_lookup( tr_table, ntype(sel_node), 2);
	if( !p || mapclass[*p] != STATCLASS )
		error(ER(21,"needblock`You may only raise a block statement"));

	/*
	 * If we are raising an if-then-else block then raise both the
	 * then-code and the else-code, by raising the then-code in the
	 * first pass and the else-code in the second pass.  (Other
	 * nodes will only take one pass to do the raise.)
	 */

	iselse = ntype(sel_node) == N_ST_ELSE;
	list1 = node_kid( sel_node, lkid[*p] );
	if( iselse )
		list2 = node_kid( sel_node, lkid[*p] + 1 );

	hist_room( 2 + 3 * (listcount(list1)+(iselse ? listcount(list2) : 0)) );
	for (pass = 0; pass < (1 + iselse); pass++) {
		listToRaise = pass ? list2 : list1;
		/*
	 	 * For each kid in the list, prune it from the original
		 * list and insert it immediately before the block node.
		 */
		for (i = 0; i < listcount(listToRaise); i++) {
			kid = node_kid(listToRaise, i);
			prune(kid);
			cursor = sel_node;
			exp_list(0);
			graft(kid, cursor);
		}
	}

	/*
	 * And finally delete the original block node
	 */
	del_list(tparent(sel_node), get_kidnum(sel_node));
#endif HAS_COM
}

#else WRONG

do_raise()
{
#ifdef HAS_COM
	register bits8 *p;
	register int i;
	register listp ltoraise;

	grab_range( R_FORCE|R_NOLIST );
	if( !(p = table_lookup( tr_table, ntype(sel_node), 2 ) ) )
		error( ER(21,"needblock`You may only raise a block statement") );

	ltoraise = node_kid( sel_node, lkid[ *p ] );
	cursor = sel_node;
	prune( sel_node );
	/* ASSUME NO NULL LISTS OF STATEMENTS */
	graft( kid1(ltoraise), cursor );
	for( i = 1; i < listcount( ltoraise ); i++ ) {
		exp_list(1);
		graft( node_kid( ltoraise, i ), cursor );
		}
#endif HAS_COM
}
#endif WRONG

do_comout() {
#ifdef HAS_COM
	grab_range(R_FORCE | R_FLIST );
	sterr();
	cursor = l_lower( N_ST_COUT, 0 );
#endif HAS_COM
}
	/* transmogrify command */
do_transmog()
{
	struct menu_data m;
	extern char menu_nop[];
	register int i;
	NodeNum selected;
	int ret;
	bits8 *maptype;
	bits8 menitems[20];	/* enough to hold all things in tmenu*/
	int micount = 0;	/* menu item count */
#ifdef GEM
	char *str;
#endif

#ifdef HAS_COM

	grab_range( 0 );

	if( is_a_list(sel_node) ) {
		do_lower( TRUE );
		sel_node = tparent( cursor );
		sel_last = -1;
		}
	selected = ntype(sel_node);

	if( !( maptype = table_lookup( tr_table, ntype(sel_node), 2 ) ) )
		error( ER(23,"badspec`Can't make a specialty change to a '%s' node"),
			 NodeName( ntype(sel_node)) );
	/* pop up a menu of transformations */
	new_menu( &m, LDS( 409, "Change this to:" ) );
#ifndef GEM
	add_menu_item( menu_nop );
#endif
	for( i = 0; tr_table[i]; i += 2 )
		if( mapclass[*maptype] == mapclass[tr_table[i+1]] ) {
			add_menu_item( NodeName( tr_table[i] ) );
			menitems[micount++] = tr_table[i];
			}
#ifdef GEM
	pop_menu( &m, (funcptr)0, FALSE, &str, &ret );
	if( str ) {
		ret++;
	} else
		ret = 0;
#else
	ret = pop_menu( curr_window->w_desc, &m, (funcptr)0 );
#endif
	if( ret > 0 )
		transmog( sel_node, menitems[ret-1] );
#endif HAS_COM
}

/* do a single step.  this needs work */

sing_step(steptype)
int steptype;		/* true for single step, false for continue */
{
	extern int count_suspend;
	if( steptype )
		prep_r_window();
	if( count_suspend ) {
		res_check( (pointer)0 );
		prep_to_run();
		resume( steptype );
		}
	else {
		do_run_prog( cur_work_top, steptype );
		}
	
}
#ifdef Notdef

/* do a cr on a stub */
do_stubcr(xendstub)
nodep xendstub;	
{
	register int ourkid;
	register nodep endstub = xendstub;

	ourkid = get_kidnum(endstub);
	if( ourkid == listcount(sel_node = tparent(endstub)) - 1
		&& is_a_list(tparent(tparent(sel_node))) ) {
		sel_first = sel_last = ourkid;
		del_range(FALSE);	/* delete the stub */
		cursor = tparent(sel_node);
		exp_list(1);	/* expand after */
		}
	 else
		if( is_not_root(tparent(find_true_parent(endstub)))  )
			cursor = c_rightpos(endstub);
}

#endif
do_shell(args, exec)
char	*args;
int exec;
{
	char shbuf[MX_WIN_WIDTH];
	int ret;

	getIfNull(args, LDS( 410, "System Command: " ), shbuf, MX_WIN_WIDTH );

#ifdef GEM
	clr_screen();
#endif

	if( ret = safeshell( shbuf, TRUE, exec ) ) {/* pause at end */
#ifdef msdos
		extern char *dosexecerr();
		message( ER(193,"shell`Error executing %s - cause: %s"),
				shbuf, dosexecerr(ret) );
#else
		message( ER(193,"shell`Error executing %s"), shbuf );
#endif
		}

#ifdef GEM
	draw_all();
#endif

	redrw_screen(FALSE);
}

do_immediate()
{
	register listp declist;		/* list with procs and funs */
	register nodep ascend;			/* scan up to block */
	register int dkid;
	/* scan up from cursor for immediate loc */
	for( ascend = cursor; ascend; ascend = tparent(ascend) )
		if( ntype_info(ntype(ascend)) & F_SCOPE ) {
			if( ntype(ascend) == N_IMM_BLOCK )
				continue;
			declist = decl_kid(ascend);
			for( dkid = 0; dkid < listcount(declist); dkid++ ) {
				nodep thedec;
				thedec = node_kid(declist,dkid);
				if(ntype(thedec) == N_IMM_BLOCK) {
					markPopBack(cursor);
					cursor = node_kid(kid2(thedec),
					    listcount(kid2(thedec))-1);
					if( not_a_stub(cursor) )
						exp_list(1);
					return;
					}
				}
			/* didn't find imm block in the declarations */
			go_declare( N_IMM_BLOCK, ascend );
			cursor = uc_nextpos(cursor);	/* move to stats */
			return;
					
			}
	error( ER(24,"badimm`Can't enter immediate mode at this point") );

}

/* set breakpoint */

set_bpoint()
{
	grab_range(R_FORCE|R_NOLIST);
	if( ex_class(sel_node) == C_STATEMENT  ) {
		if (!(node_flag(sel_node) & NF_BREAKPOINT))
			changeFlags(sel_node, NF_BREAKPOINT, 0);
	} else
		error( ER(25,"badbp`Breakpoints should only be put on statements") );
}

/* clear debug points  */

clr_point()
{
	register int i;
	grab_range(R_FORCE|R_LEAF);
	if( sel_last >= 0 ) {
		for( i = sel_first; i <= sel_last; i ++ )
			rem_points(node_kid(sel_node,i));
		}
	 else
		rem_points(sel_node);
}

rem_points( nod )
nodep nod;
{
	register nodep nptr = nod;
	register int i;
	register int maxkid;
	stackcheck();

	if( not_a_list( nptr ) ) {
		if( !(ntype_info(ntype(nptr)) & F_LINE) )
			return;
		if (node_flag(nptr) & NF_BREAKPOINT)
			changeFlags(nptr, 0, NF_BREAKPOINT);
		}
	maxkid = regl_kids( nptr );

	for( i = 0; i < maxkid; i++ )
		rem_points( node_kid( nptr, i ) );
}

/*
 * Search for the appropriate action, given that "toknum" was typed
 * with the cursor on an expanded node of type "nodetype".
 */
unsigned
find_type(toknum, nodetype, upkid, pcode, menuadd)
unsigned int toknum;
NodeNum nodetype;
int upkid;		/* what kid we came up from, -1 if none */
int pcode;		/* if upkid is -1, what printcode we were on */
struct menu_data *menuadd;	/* a menu if we are adding to one */
{
	register bits8 far *p;

	printt4("find_type(toknum %d=%s, nodetype %d=%s) ", toknum,
		  tokname(toknum), nodetype, NodeName(nodetype));

	for( p = node_actions(nodetype); *p; p += 3 )
		if( *p == toknum || menuadd ) {
			register bits8 modcode;
			modcode = p[1];
			if( !modcode ||
				modcode & UT_EXACT && upkid < 0 ||
				modcode & UT_UPKID && (modcode & 15) == upkid ||
				modcode & UT_PCODE && (modcode & 15)== pcode )
			
				if( menuadd )
					m_token_add( *p, menuadd );
				 else
					return (unsigned )p[2];
			}
	/* otherwise not found at all */
	return 0;
}

/*
 * Search for the appropriate action, given that "toknum" was typed
 * with the cursor on an unexpanded node of class "childclass".
 */
unsigned
find_class(toknum, childclass)
unsigned toknum;
ClassNum childclass;
{
	register ActionNum *p;
#ifdef DB_COMMAND
	if (tracing)
		fprintf(trace, "find_class(toknum %d=%s, childclass %d=%s) ",
			toknum, tokname(toknum),
			childclass, classname(childclass));
#endif
	p = table_lookup( Class_Actions[childclass], toknum, 2 );
	return p ? (unsigned)*p : 0;
}


/*
 * The user just typed in this token.  Find the appropriate action
 */
get_action(toknum, actloc, startloc)
unsigned toknum;
nodep *actloc;
nodep startloc;		/* where we start */
{
	register unsigned act;
	register nodep ascend;
	register nodep old_ascend;
	reg ClassNum childclass;
	int upkid;			/* which kid we came up from */
	int cpcode;			/* print code cursor is on */
	extern char tokentext[];

#ifdef DB_COMMAND
	if (tracing) fprintf(trace, "\nin_token %s '%s'\n",
			tokname(toknum), tokentext);
#endif
	if (is_a_stub((ascend = startloc))) {
		childclass = int_kid(0,startloc);
#ifdef DB_COMMAND
		if (tracing) fprintf(trace,
			"cp (%x), basic class %d=%s\n",
			(int)PCP(cursor), childclass, classname(childclass));
#endif

		if (childclass >= CLIST(0))
			childclass -= CLIST(0);
		act = find_class(toknum, childclass);

#ifdef DB_COMMAND
		if (tracing) fprintf(trace,
			"toknum %d, cursor (%x), ntype %d, class %d\n",
			toknum, startloc, ntype(cursparent),ex_class(startloc));
#endif
		ascend = cursparent;	
		upkid = get_kidnum( startloc );

	} else {
		/* cursor is on a filled-in node in the tree */
		act = 0;
		ascend = startloc;
		upkid = -1;
		}
	cpcode = phys_pcode;
	old_ascend = NIL;
	if( act == 0 )
		while( ascend  ) {
			if( old_ascend )
				upkid = get_kidnum( old_ascend );
			if( act = find_type(toknum, ntype(ascend), upkid,
					cpcode, (struct menu_data *)0 ) )
				break; 
			old_ascend = ascend;
			cpcode = -1;
			ascend = tparent(ascend);
			}
		

#ifdef DB_COMMAND
	if (tracing) fprintf(trace, "action %d\n", act);
#endif
	/* may want to do sanity checks, move cursor, etc first */
	*actloc = ascend;
	return act;
}
in_token(toknum,stloc)
unsigned toknum;
nodep stloc;		/* starting loc */
{
	nodep ascend;
	register unsigned action;

	action = get_action( toknum, &ascend, stloc );
	do_action( action, toknum, ascend );
}

	/* run program, doing check first */
do_run_prog( loc, steptype )
nodep loc;
int steptype;
{
	extern int had_type_error;
	extern int win_default;

	/* If in immediate mode, give an error */
	imm_abort();

	/* free all new/dispose memory */
	newClear();
	prep_stacks(TRUE);
	/* compile declarations and clear had error flag */
	/* this also calculates initializers */
	c_comp_decls( 1, loc, TC_INIT|TC_DESCEND|TC_CHECKDIRTY|TC_MOVECURSOR );
	if( !had_type_error ) {
#ifndef SAI
		prep_to_run();
#endif
		run_prog( loc, steptype );
#ifndef SAI
		reswflags( curWindows[MAINWIN], win_default );
#endif
		}
	return had_type_error;
}

imm_abort()
{
	extern int imm_only;
	if( imm_only )
		error( ER(26,"runimm`Can't do that when experimenting in immediate mode") );
}
