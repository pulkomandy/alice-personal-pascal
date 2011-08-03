/*
 * display routines to look at the tree and put something on the screen.
 */

#include "alice.h"
#include "curses.h"
#include "workspace.h"
#include "window.h"
#include "jump.h"
#include "flags.h"

#ifdef DB_DISPLAY
# define PRINTT
#endif
#include "printt.h"

/*
 * If a list moves when grown, we must change references in the line
 * structures from the old location to the new one.  Called from growlist().
 */
line_refs_list(_oldlp, newlp)
listp _oldlp;
listp newlp;
{
	register listp oldlp = _oldlp;
	register alice_window	*win;
	register int		i;

	printt2("line_refs_list(%x, %x)\n", oldlp, newlp);

	win = curr_window;
	for (i = 0; i < win->w_height; i++)
		if (win->w_lines[i].listptr == oldlp)
			win->w_lines[i].listptr = newlp;
}

/*
 * Redisplay the buffer on the screen.
 */

display(redo)
int redo;		/* flag to say if we should redo */
{
	register alice_window *win;	/* window being checked */
	register unsigned int lno;		/* line number in scan */
	bits8 changeflag;	
	bits8 qcflag;			/* quick change flag store */
	struct scr_line thisline;	/* storage for local line */
	register nodep starthere;	/* where we start redisplay */
	int startpc;			/* start printing code */
	extern int cantredraw;		/* to redraw is futile */

	/* at this point we should loop through all the windows
	   and see what has changed and what needs a redisplay */

	if( cantredraw )
		return;
	/* clear_em(FALSE); */
	if( redraw_status )
		statusLine();

	win = curr_window;

#ifdef DB_DISPLAY
	startpc = win->big_change;	/* avoid Onyx compiler bug */
	if (tracing) fprintf(trace, "display, redo %d, big_change %d\n", redo, startpc);
#endif
	curOn( win->w_desc );

	if( redo || win->big_change ) {
#ifdef DB_DISPLAY
		if( tracing ) fprintf( trace, "Display all of window\n" );
#endif
#ifdef msdos
		msdoscursor( FALSE );	/* turn off blink cursor */
#endif
		starthere = win->start_cursor;	
		startpc = win->start_pcode;
		disp_lines( win, 0, 0, starthere, startpc, win->start_sline );
		win->big_change = FALSE;
		/* should be a break but for now it is a.... */
#ifdef GEM
		/* Update the slider position */
		count_lines();
#endif GEM
		goto donewindow;
		}
		 
	for( lno = 0; lno < win->w_height; lno++ ) {
		scrl_copy( thisline, win->w_lines[lno] );
		if( thisline.listptr == NIL )
			break;
		changeflag = thisline.sc_change;
		if( changeflag & BIG_CHANGE ) {
			wmove( win->w_desc, lno, 0 );
#ifdef DB_DISPLAY
			if(tracing) fprintf(trace,"Display down from line %d\n", lno );
#endif
			disp_lines( win, lno, 0, line_headnode( thisline ),
				thisline.pcode, thisline.sub_line );
			break;
			}
		/*qcflag = changeflag & (LINE_INVERT | LINE_OFF ); */
		if( changeflag & SMALL_CHANGE /* ||
				(qcflag && qcflag !=(LINE_INVERT|LINE_OFF))*/) {
#ifdef DB_DISPLAY
			if(tracing)fprintf(trace,"Display change line %d\n", lno );
#endif
			cur_line = lno;
			out_line( &thisline, win, FALSE, win->w_height );
			}
		/* if both the on and off bits were set we didn't redraw the
		   line so we must be sure to clear the change bits as though
		   we did call outline */
		win->w_lines[lno].sc_change = NULL;
		}
	donewindow: ;

}

/* mark that a node has been changed */
/*
	This routine marks changes and how they affect the display.
	Single line changes are flagged in the line tables, anything
	else causes a redraw
 */

mark_change( whatsnew, whatsold, guy_on_screen, important )
nodep whatsnew;			/* new thing that has been put there */
nodep whatsold;			/* what was there before */
nodep guy_on_screen;		/* somebody we can use to find the line */
int important;			/* is the new line a must */
{
	printt3("mark_change(%x, %x, %x)\n", whatsnew, whatsold, guy_on_screen);


	/* should loop through all active edit windows to find lines
	 * that the change is made on and tag them.  for now just
	 * do the main window 
	 */
	if( !mark_line( guy_on_screen, 
			size_in_lines(whatsnew)==size_in_lines(whatsold) ?
			SMALL_CHANGE : BIG_CHANGE ) && important )
		mark_line( NIL, HUGE_CHANGE );

}

mark_line( anode, thechange )
nodep anode;
int thechange;
{
	register int changeline;	/* line that is to be changed */
	register alice_window *win;
	int lastline;
	reg int countline;

	printt2("mark_line(%x, %d)\n", anode, thechange);

	win = curr_window;
	/* beware of return in multiple-window version */


	if( thechange == HUGE_CHANGE ) {
		win->big_change = TRUE;
		}
	 else {
		/*if( thechange >= LINE_INVERT ) {
			changeline = find_line_number(win, anode, (int *)NULL,
					&lastline );
			if( changeline < 0 )
				changeline = 0;
			}
		 else */ {
			if( (lastline = changeline =
				find_line_number(win, anode, (int *)NULL, (int *)NULL )) < 0 )
				/* if looping, change this around */
				return FALSE;

			}
#ifdef DB_DISPLAY
		if(tracing)fprintf(trace,"Marking Lines from %d to %d with %d\n",
			changeline, lastline, thechange );
#endif

		for( countline = changeline; countline <= lastline;
					countline++ )
			win->w_lines[countline].sc_change |= thechange;
		}
	return TRUE;
}

check_curgone(going, newtop)
nodep going;
curspos	newtop;
{
	printt3("check_curgone(%x, %x), cursor=%x\n", going, newtop, cursor);

	if (is_ancestor(going, cursor)) {
		printt1("going is ancestor of cursor, cursor now %x\n", newtop);
		cursor = newtop;
	}
}

check_topgone( going, newtop )
nodep going;		/* node that is going */
curspos newtop;		/* new top to use if deleted */
{
	register alice_window *win;

	printt2("check_topgone(%x, %x)\n", going, newtop);

	win = curr_window;
	if( is_ancestor( going, win->start_cursor ) ) {
		printt1("win->start_cursor now %x\n", newtop);
		set_wscur( win, newtop );
		win->big_change = TRUE;
	}
}

page_up(){
	struct scr_line line;
	extern Boolean anywflag;
	int i;
	int max;

	max = curr_window->w_height - 3;

	scrl_copy( line, curr_window->w_lines[0] );
	for( i = 0; i < max; i++ ) {
		if( !go_to_prev( &line ) ) {
			/* oh no, hit top */
			set_wscur( curr_window, cursor = cur_work_top );
			display(TRUE);
			return;
			}
		line.sub_line = 0;	/* always line 0 */
		pcod_adjust( &line );
		}
	set_sc_fline( curr_window, &line );
	cursor = curr_window->start_cursor;
	/* allow the cursor to find on any printcode */
	anywflag = TRUE;
	display(TRUE);
	return;
}
		
		

page_down()
{
	register int lastline;

	register alice_window *win = curr_window;
	extern Boolean anywflag;


	lastline = win->w_height - 1;
	if( win->w_lines[lastline].listptr )  {
		set_sc_fline( win, &(win->w_lines[lastline]) );
		cursor = win->start_cursor;
		/* allow the cursor to find on any printcode */
		anywflag = TRUE;
		win->big_change = TRUE;
		}
}


/* how many lines does a node take on the screen?
   0 - for nodes that are part of lines
   0 - for the null code
   1 - for nodes that take up a line to themselves
   2 or more for nodes that take up several lines
*/

int
size_in_lines(xanode )
nodep xanode;		/* node to get the size of */
{
	register nodep anode;
	anode = xanode;

	if( !anode )
		return 0;
	if( is_a_stub( anode ) )
		/* following query is actually not necessary */
		return( is_line_class( int_kid(0,anode) ) ? 1 : 0 );
	 else
		/* if it is a line and more than one print code, return 2 */
		if( print_codes(ntype(anode))[1] || ntype_info(ntype(anode))
						& F_PMULTI )
			return 2;
		 else
			return (ntype_info( ntype(anode) ) & F_LINE ) ? 1 : 0;
		
}

int sought_column;		/* column treeprint looks for */

int
find_line_number(xwin, curs, rpc_index, fullrange )
alice_window *xwin;	/* window we look in */
reg curspos curs;	/* cursor we are finding */
int *rpc_index;		/* what print code in the line we return or null */
int *fullrange;		/* get the whole range for the line */
{
	register alice_window *win;
	register curspos ncurs;	/* line level cursor we look for */
#ifdef LOADABLE_TEMPLATES
	bits8 far *kcvec;	/* array of kid code vectors */
#else
	bits8 *kcvec;
#endif
	register int lno;	/* line number */
	int kid_index;	/* which kid we came via */
	int pc_index;	/* for our own use */
	int realline;	/* last line with this on it too */
	win = xwin;


	printt2("find_line_number(xwin=%lx, curs=%lx, ...)\n", win, curs);

	ncurs = up_to_line( curs, &kid_index );

	/* now scan the window for this line */

	/* find the print code associated with the line the cursor is on */

	/* if rpc_index is set we find any printcode we want */

	if( kid_index < 0 && rpc_index )
		pc_index = (-1);
	 else
		pc_index = kcget( kid_index, ncurs );
#ifdef DB_DISPLAY
	if(tracing)fprintf(trace, "Line level cursor %x from print code %d\n", (int)PCP( ncurs ), pc_index );
#endif

	realline = -1;


	for( lno = 0; lno < win->w_height; lno++ ) {
		int lpcode;
		printt4("lno=%d, listptr=%x, listindex=%d, pcode=%d\n", lno,
			(int)win->w_lines[lno].listptr,
			(int)win->w_lines[lno].listindex,
			win->w_lines[lno].pcode );
		if (win->w_lines[lno].listptr &&
		    ncurs == line_headnode(win->w_lines[lno]) &&
		    ((lpcode = win->w_lines[lno].pcode) == pc_index ||
		     pc_index < 0)) {
			/* return > 0 only if we found a line not the first */
			if( rpc_index )
				*rpc_index = pc_index < 0 ? lpcode : 0 ;
			realline = lno;
			break;	
			}
		}
	lno = realline;
	if( fullrange ) {
#ifdef LOADABLE_TEMPLATES
		bits8 far *nvec;
#else
		bits8 *nvec;
#endif
		nodep ngtcheck;
		if( ntype_info(ntype(curs)) & F_PMULTI &&
				(ngtcheck = look_multi(curs)) ) 
			curs = ngtcheck;
		if( nvec = kid_codes(ntype(curs)) ) {
			kcvec = nvec;
			ncurs = curs;
			*fullrange = win->w_height - 1;
			while( ++lno < win->w_height ) {
				/* if on the line and on last pcode */
				if( ncurs == line_headnode(win->w_lines[lno]) &&
					print_codes(ntype(ncurs))[
						win->w_lines[lno].pcode+1]== 0)
					*fullrange = lno;
				}
			}
		 else {
			*fullrange = realline;
#ifdef DB_DISPLAY
			if(tracing)fprintf(trace,"Setting fullrange to line number %d due to kcvec=0\n", lno );
#endif
			}
		}
	return realline;
}

calc_main_indent( ind_level, scr_width )
int ind_level;		/* how many tabs */
int scr_width;		/* how wide the screen */
{
	register int main_indent;

	main_indent = TABSIZE * ind_level;
	if( main_indent > scr_width - TABSIZE * 4 )
		main_indent = scr_width - TABSIZE * 4;
	return main_indent;
}


/*
 * get the Alice node cursor position for row-column coordinates on
 * the screen
 */


curspos pointed_node;		/* what we were pointing at */
int point_offset;		/* offset into what we pointed at */
	

curspos
find_alice_cursor(xwin, row, column )
alice_window *xwin;		/* window this row and column are in */
reg int row;
int column;
{
	register alice_window *win;
	struct scr_line ourline;
	int main_indent;
	int width;		/* how wide the screen is here */
	int indent;		/* real indent at this point */
	extern buf_el *lb_cursor;	/* cursor in line buffer */
	extern buf_el line_buffer[]; /* where treeprint outputs */
	win = xwin;

	width = win->w_width;
	if( row < 0 || row >= win->w_height )
		bug(ER(247,"find_alice_cursor: Row %d outside of window bounds"), row );
	if( column < 0 || column >= win->w_width )
		bug(ER(248,"find_alice_cursor: Column %d out of window bounds"), column);

	/* if this line is blank, move up until we find one */
	while( !win->w_lines[row].listptr && row >= 0 )
		row--;
	if( row < 0 )
		bug( ER(249,"find_alice_cursor : Screen has no lines on it!!!") );
	
	scrl_copy( ourline , win->w_lines[row] );

	main_indent = calc_main_indent( ourline.ind_level, win->w_width );
	indent = ourline.sub_line ? main_indent + TABSIZE/2 : main_indent;

	sought_column = column - indent;
	if( sought_column < 0 )
		if( sought_column > -3 )
			sought_column = 0;
		 else {
			/* it is far to the left.  take the whole line */
			return( line_headnode( ourline ) );
			}
	if( ourline.sub_line != 0 )	
		sought_column += width - main_indent
			+ (width - indent) * (ourline.sub_line - 1);

	/* we now know which column we seek.  now print the line in a
	   buffer to work out what node this really is */


	/* treeprint knows to set appropriate values */
	/* set up the externals */

	lb_cursor = line_buffer;	/* start things off at beginning,
					   which is after all a good place */

	out_flag = LOOK_COLUMN;		/* say we are looking for column */

	pointed_node = NIL;
	form_column = 0;		/* initial offset calculator */
	/* set up the node we get if at the very start */
	former_node = line_headnode( ourline );

	if( setexit( ) )
		treeprint( former_node, ourline.pcode, ' ', (char *)0 );

#ifdef DB_DISPLAY
	if(tracing) fprintf( trace, "find_alice_cursor:Found node at %x for row %d, column %d\n", pointed_node, row, column );
#endif


	/* if we did not find the sought column, return the line itself */

	return pointed_node ? pointed_node : line_headnode( ourline );
}

/* see treeprint for a little about this routine.  The external former_node
 * is set by treeprint whenever it prints a node in LOOK_COLUMN mode
 */ 

found_column() {

	/* OH BOY.  We are past the node
	   at the requested column */
	pointed_node = former_node;
	point_offset = sought_column - form_column;
#ifdef DB_DISPLAY
	if(tracing)fprintf(trace,
		"At column %d found node %x offset %d\n",
		form_column, pointed_node, point_offset );
#endif
	reset();
}

/* look_multi
 * routine used in pascalish output code to detect the presence of
 * a child that actually takes up several lines.  Shame on it.
 *
 * This routine just recursively descends the tree to find the culprit,
 * but is clever and searches children in reverse order since the nasty
 * culprit is bound to be closer to the right than the left.  In fact,
 * the only thing that could cleanly be to the right would be a comment.
 * It returns the node it found
 */

nodep 
look_multi(xanode )
nodep xanode;		/* subtree we look at */
{
	register nodep retval;	/* return value */
	register nodep anode;
	reg int kid;		/* how many kids */

	stackcheck();
	anode = xanode;

	/* terminate if we found it */
	if( print_codes(ntype(anode))[1] )
		return anode;

	kid = n_num_children( anode );
	while( kid-- )
		if( retval = look_multi( node_kid( anode, kid ) ) )
			return retval;
	/* did not find it */
	return FALSE;
}
/* check to see if the cursor has moved, and if so, handle the reverse
   video problems associated with anchor range sweeping */

curspos oldcur = NIL;

newrange(redisp)
int redisp;	/* should display be called */
{
	nodep savsnode;
	int ssel_first, ssel_last;
	if( cursor != oldcur ) {
		if( anchor ) {
			bld_range( oldcur, anchor );
			savsnode = sel_node;
			ssel_first = sel_first;
			ssel_last = sel_last;
			bld_range( cursor, anchor );
			if( sel_node != savsnode || sel_first != ssel_first ||
					sel_last != ssel_last ) {
				bld_range( oldcur, anchor );
				vid_alter(FALSE, 0);
				bld_range( cursor, anchor );
				vid_alter(TRUE, 1);
				}
			if( redisp )
				display(FALSE);
			}
		oldcur = cursor;
		}
}

static int osfirst[2];
static int oslast[2];
clr_oldlines() {
	osfirst[0] = oslast[0] = -1;
}
	

vid_alter(onoff, which)
int onoff;		/* turning on or off */
int which;		/* 0 for first range, 1 for second range */
{
	register int index;	/* loop through selected list */
		
	if( sel_last < 0 ) {
		show_subtree( sel_node, onoff, TRUE );
		osfirst[which] = find_line_number(curr_window, sel_node,
				(int *)NULL, &(oslast[which]) );
		}
	 else {
		int dummy;

		osfirst[which]=find_line_number( curr_window, node_kid(sel_node,
			sel_first), (int*)0, &dummy );
		dummy = find_line_number( curr_window, node_kid(sel_node,
			sel_last), (int*)0, &oslast[which] );
		for( index = sel_first; index <= sel_last; index++ )
			show_subtree(node_kid(sel_node,index), onoff, TRUE );
		}
	if( osfirst[which] < 0 )
		osfirst[which] = 0;
	/* if it is the second one, do the job */
	if( which ) {
		alice_window *win;

		win = curr_window;
		if( osfirst[0] < 0 ) {
			/* if nothing else, set to first line */
			osfirst[0] = oslast[0] = osfirst[1];
			}
		loopset( win, osfirst[0], osfirst[1] );
		loopset( win, oslast[0], oslast[1] );
		
		}
}

	/* set change bit in a given range any order */
loopset( win, bound1, bound2 )
alice_window *win;
int bound1, bound2;
{
	int maxr, minr, i;

	maxr = min( win->w_height-1, max( bound1, bound2 ) );
	minr = max( 0, min( bound1, bound2 ));
	for( i = minr; i <= maxr; i++ )
		win->w_lines[i].sc_change |= SMALL_CHANGE;


}
show_subtree(xthenode, mode, linemark )
nodep xthenode;		/* node to highlight */
int mode;		/* set or clear */
reg int linemark;		/* should we mark the change */
{
	register nodep thenode;
	register int i;
	int imax;
	int mtype = mode ? LINE_INVERT : LINE_OFF;


	stackcheck();
	thenode = xthenode;

	printt3("show_subtree(%x, %d, %d)\n", thenode, mode, linemark);

	if( not_a_list(thenode) ) {
		if( mode )
			s_node_flag(thenode, NF_STANDOUT | node_flag(thenode));
		 else
			s_node_flag(thenode, ~NF_STANDOUT & node_flag(thenode));
#ifdef OldSel
		 if( linemark ) {
			mark_line( thenode, size_in_lines(thenode) ?
				mtype : SMALL_CHANGE);
			linemark = FALSE;
			}
#endif OldSel
		}
	
		
	imax = n_num_children(thenode);

#ifdef OldSel
	if( ntype( thenode ) == N_TYP_RECORD ) {
		mark_line( thenode, mtype );
		/* make sure parenting line gets updated, no matter what */
		mark_line( tparent(thenode), SMALL_CHANGE );
		}
#endif Oldsel
	for( i = 0; i < imax; i++ ) {
		show_subtree( node_kid(thenode, i), mode, linemark );
		}
}


nodep 
l_hcalc( xwptr )
struct scr_line *xwptr;
{
	register struct scr_line *wptr = xwptr;
	register nodep thelist;

	thelist = wptr->listptr;

	return thelist ? node_kid( thelist, (unsigned)wptr->listindex ) : NIL;
}
