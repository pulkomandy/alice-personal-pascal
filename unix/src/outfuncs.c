
/*
 *DESC: Routines associated with the display of the program on the screen.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "class.h"

#include "flags.h"

#ifdef DB_OUTFUNCS
#define PRINTT
#endif
#include "printt.h"

/*  routine to find the next line for printing */

go_to_next(xline, how_many, last_printed )
struct scr_line *xline;		/* information about a printing line */
int how_many;			/* how many fold lines long this line is */
int last_printed;		/* the last fold line we printed */
{
	register struct scr_line *line;
	register nodep outnode;	
#ifdef LOADABLE_TEMPLATES
	char far *npc;		/* new print code */
#else
	char *npc;
#endif
	curspos newline;	/* new list pointer */
	int kid_index;		/* which child in descend */
	int nlisted;		/* number of elements in list */
	nodep noutnode;		/* kludge case outnode */
	int nt, pc;		/* temps to get around mwc cc bug */
	line = xline;
		

	/* if on a blank line, give up */

	if( !line->listptr )
		return( FALSE );
	outnode = line_headnode( (*line) );
	line->sub_line = 0;	/* we want the first line now */
	if( not_a_stub(outnode) ) {
		if( last_printed+1 < how_many ) {
			line->sub_line = last_printed+1;
			return(TRUE);
			}
		if( ntype_info(ntype(outnode)) & F_PMULTI &&
			(noutnode = look_multi(outnode)) ) {
			outnode = noutnode;
			line->pcode = 0;	/* start fresh */

			/* WARNING
			 * At this point, to be general, the code should
			 * initialize line->listptr and line->listindex
			 * to point at outnode, but as we know there will
			 * be more print codes, and that in all the cases
			 * for Pascal the next code is a descend, we get
			 * away with this
			 */
			 }
real_gonext:
		nt = ntype(outnode);
		/*{
		extern int ascii_output;
		if( ascii_output && nt == N_HIDE )
			nt = N_REVEAL;
		} */
		pc = ++line->pcode;
		if( npc = print_codes(nt)[pc] ) {
			/* check for !l print code */
			while( is_descend( npc ) ) {
				kid_index = npc[1] - '1';
					line->listptr = node_kid(outnode,kid_index);
					line->listindex = 0; /* first in list */
					if( ntype_info(ntype(outnode)) & F_INDENT )
						line->ind_level++;
					line->pcode = 0;
					outnode = line_headnode((*line));
					npc = print_codes(ntype(outnode))[0];
				}
			return(TRUE);
			} 
		}
	line->pcode = 0;
	/* don't check for end of list if it isn't one.  damn RECORD */
	if( is_a_list( line->listptr ) ) {
		nlisted = n_num_children(line->listptr);
		if( line->listindex < nlisted-1 ) { /* if any more siblings */
			line->listindex++; /* go to the next one */
			line->pcode = -1;
			goto real_gonext;
			}
		}
	/* otherwise up we go */
	newline = tparent( line->listptr );
	/* under current assumptions, newline is NOT a list nodep */
	if( !newline || tparent(newline) == NIL ) {
		/* no parent, must be the root node itself */
		line->listptr = NIL;
		return( FALSE );
		}
	/* we now look for our node.
	 * the get_kidnum will be done only on a regular node, so don't
	 * worry about speed. If not a list must be something funny like
	 * the dreaded RECORD and we go up to the top of the line
	 */
	if( is_a_list( line->listptr ) )
		/* which child in my parent? */
		kid_index = get_kidnum( line->listptr ); 
	 else
		newline = up_to_line( line->listptr, &kid_index );
	line->pcode = kcget( kid_index, newline );
	line->listptr =  tparent( newline );
	/* this will search a list, but there is little we can do */
	line->listindex = get_kidnum( newline ); 
	/* set indentation level */
	line->ind_level = calc_indent( line->listptr );
	outnode = newline;
	/* now loop, but this time don't check for multi line kids */
	goto real_gonext;
}

kcget( kid_index, lnode )
int kid_index;		/* what kid, -1 for none */
nodep lnode;		/* node to find kid in */
{
#ifdef LOADABLE_TEMPLATES
	bits8 far *kcvec;
#else
	register bits8 *kcvec;
#endif

	kcvec = kid_codes(ntype(lnode));
	return kcvec ? kcvec[1+kid_index] : 0;
}
/* check for record and set to end of necessary */

rec_check( xline )
struct scr_line *xline;
{
	register struct scr_line *line = xline;
	register nodep outnode;
	nodep recnode;

	outnode = line_headnode( (*line) );

	if( ntype_info( ntype(outnode) ) & F_PMULTI &&
			(recnode = look_multi( outnode )) ) {
		
		line->listptr = tparent(recnode);
		line->listindex = get_kidnum(recnode);
		/* pcode already set to last */
		}
}




/* find the previous line to the one we are on */

go_to_prev(xline )
struct scr_line *xline;		/* descriptor of line */
{
	register struct scr_line *line;
	register nodep outnode;	
#ifdef LOADABLE_TEMPLATES
	char far *npc;		/* new print code */
#else
	reg char *npc;
#endif
	curspos newline;	/* new list pointer */
	int kid_index;		/* which kid we are of parent */
	int nt, pc;		/* temps to get around mwc cc bug */
	line = xline;

	stackcheck();
#ifdef DB_OUTFUNCS
if (tracing) {
	fprintf(dtrace, "in go_to_prev(%x) ", line);
	fprintf(dtrace, "with line->listptr=%x, line->listindex=%x, line->sub_line=%x, line->pcode=%d\n",
		line->listptr, line->listindex, line->sub_line, line->pcode);
}
#endif DB_OUTFUNCS
	/* if on a blank line, give up */

	if( !line->listptr )
		return( FALSE );

	outnode = line_headnode( (*line) );
	/* if set to MAX_LINES, we don't decrement it */
	if( line->sub_line != MAX_LINES && line->sub_line-- ) {
		return(TRUE);
	}
	line->sub_line = MAX_LINES;
	if( not_a_stub(outnode) ) {
		if( line->pcode ) {
			nt = ntype(outnode);
			pc = --line->pcode;
			npc = print_codes(nt)[pc];
			/* check for !l print code */
			if( is_descend( npc ) ) {
				kid_index = npc[1] - '1';
				line->listptr = node_kid(outnode, kid_index );
				line->listindex = n_num_children(line->listptr)-1;
				if( ntype_info(ntype(outnode)) & F_INDENT )
					line->ind_level++;
				line->pcode = MAX_PCODE;
				rec_check( line );
				}
			 else if( !(line->pcode || ntype_info(nt)
						& F_LINE ) ) {
				/* not line level.  must go up */
				outnode = up_to_line( outnode, &kid_index );
				line->listptr = tparent( outnode );
				line->listindex = get_kidnum(outnode);
				line->pcode = kcget( kid_index, outnode );
				}
			return(TRUE);
			} 
		}
	line->pcode = MAX_PCODE;
	if( line->listindex > 0 ) {
		line->listindex--;
		rec_check( line );
		return(TRUE);
		}

	if( c_at_root( outnode ) ) {
		line->listptr = NIL;
		return(FALSE);
		}
	newline = tparent( line->listptr );

	kid_index = get_kidnum( line->listptr );

	line->pcode = kcget( kid_index, newline );
	line->listptr = tparent( newline );
	line->listindex = get_kidnum( newline );
	/* set indentation level */
	line->ind_level = calc_indent( line->listptr );
	return( go_to_prev( line ) );
}
/* calculate the indent level for a node by scanning up */

calc_indent(anode )
nodep anode;
{
	register int indent;
	register nodep moveup;	

	/* will this routine ever get called on a non list node?  If so
	   it will be one too high.  fix by setting moveup to tparent(anode)
	   at the start */
	indent = 0;
	for( moveup = anode;moveup ; moveup = tparent(moveup) )
		if( ntype_info( ntype(moveup) ) & F_INDENT )
			++indent;
	return(indent);
}


/*
 *  Routines to insert and delete physical lines on the screen
 */

/*
 * scr_insert.  - open a space at the indicated line.
 */

scr_insert(xwin, line_no )
alice_window *xwin;		/* the window in which to do it */
int line_no;			/* line for the space to appear */
{
	register alice_window *win;
	register char *ldaddr;		/* line descriptor address */
	win = xwin;

	/* move all the line table entries down one */
	ldaddr = (char *) &(win->w_lines[line_no]);
	blk_move( ldaddr + sizeof(struct scr_line), ldaddr,
		(win->w_height - ( line_no + 1 )) * sizeof(struct scr_line) );
	/* now use curses routine to open area */
	wmove( win->w_desc, line_no, 0 );
	winsertln( win->w_desc );
}

/*
 * scr_delete() - somewhat more difficult, this closes a line and
 * must bring up another line at the bottom.  This routine can only
 * be called after all printing associated with a given deletion is
 * done, as it calls the somewhat non-reentrant printing routines
 * to add the new line at the bottom
 */

scr_delete(xwin, line_no )
alice_window *xwin;		/* the window in which to do it */
int line_no;			/* line for the space to appear */
{
	register alice_window *win;
	struct scr_line lastline;	/* last line on window */
	win = xwin;

	printt2( "scr_delete(%x,%d)\n", (int) win, line_no );

	/* get a copy of the last line of the screen */
	scrl_copy( lastline, win->w_lines[win->w_height-1] );
	go_to_next( &lastline, (int)lastline.sc_size, (int)lastline.sub_line );

	do_del_line( win, line_no, &lastline );
}


/* top_cup
 * go up a line from the top.  Return FALSE if you can't.
 */

top_cup(xwin )
alice_window *xwin;		/* window to do this in */
{
	register alice_window *win;
	struct scr_line topline;		/* what it was */
	win = xwin;

#ifdef DB_OUTFUNCSVERBOSE
	if (tracing) {
		int	i;

		fprintf(dtrace, "in top_cup with win=%x,mainwin=%x\n",win,curWindows[MAINWIN]);
		fprintf(dtrace, "win->start_cursor=%x\n", win->start_cursor);

		for (i = 0; i < win->w_height; i++)
			fprintf(dtrace, "w_lines[%d]->listptr = %x, w_lines[%d]->listindex = %d\n", i, win->w_lines[i].listptr, i, win->w_lines[i].listindex);
	}
#endif

	scrl_copy( topline, win->w_lines[0] );
#ifdef DB_OUTFUNCS
	if (tracing) fprintf(dtrace, "topline.listptr=%x, topline.listindex=%d, topline.sub_line=%d\n",  topline.listptr, topline.listindex, topline.sub_line);
#endif
	if( go_to_prev( &topline ) ) {
#ifdef DB_OUTFUNCS
		if (tracing) fprintf(dtrace, "topline.listptr=%x, topline.listindex=%d, topline.sub_line=%d\n",  topline.listptr, topline.listindex, topline.sub_line);
#endif

		/* create a new space at the top */
		scr_insert( win, 0 );
		cur_line = 0;
		out_line( &topline, win, TRUE, 1 );
		set_sc_fline( win, &topline );
#ifdef DB_OUTFUNCS
		if (tracing) fprintf(dtrace, "win->start_cursor = %x\n", win->start_cursor);
#endif

		return( TRUE );
		}
	 else {
#ifdef DB_OUTFUNCS
		if (tracing) fprintf(dtrace, "go_to_prev returned false\n");
#endif
		return(FALSE);
	}
}

/* bot_cdown - scroll down one */

bot_cdown(xwin )
alice_window *xwin;
{
	register alice_window *win;
	struct scr_line lastline;	/* put last line of screen here */
	win = xwin;

	/* get a copy of the last line of the screen */
	scrl_copy( lastline, win->w_lines[win->w_height-1] );
	if( go_to_next( &lastline, lastline.sc_size, lastline.sub_line ) ) {
		do_del_line( win, 0, &lastline );
		set_sc_fline( win, &(win->w_lines[0]) );
		}
	/* cursor down ahead type-ahead check */
	/*wrefresh( win->w_desc ); */
}

	/* routine to do the deletion of a screen line */

do_del_line(xwin, line_no, xlastline )
alice_window *xwin;		/* window to do the delettion in */
reg int line_no;			/* which line to delete */
struct scr_line *xlastline;	/* pointer to prepared next line */
{
	register alice_window *win;
	register struct scr_line *lastline;
	char *ldaddr;		/* line descriptor address */
	int save_cl;		/* save cur_line */
	win = xwin;
	lastline = xlastline;

	printt3( "do_del_line(%x,%d,%x)\n", (int) win, line_no, (int) lastline);
	/* move all the line table entries up one */
	ldaddr = (char *) &(win->w_lines[line_no]);
	printt3( "do_del_line: blk_move( %x, %x, %d )\n",
		(int) ldaddr, (int) (ldaddr + sizeof(struct scr_line )),
		(win->w_height - (line_no+1) ) * sizeof(struct scr_line ));
	printt1( "window height = %d\n", win->w_height );
	
	blk_move( ldaddr, ldaddr + sizeof(struct scr_line), 
		(win->w_height - (line_no+1) ) * sizeof(struct scr_line) );
	
	/* now delete the line.  There should probably be an optimizing
	   check of some sort for line 0, in which case we scroll the
	   screen.
	   */
#ifdef KLUDGE_CURSES
	if( line_no == 0 ) {
		scroll( win->w_desc );
		/* kludge it to handle curses bug */
		mvcur( 0, COLS-1, LINES-1, 0 );
		write( 1, "\n", 1 );
		}
	 else
#endif
		{
		wmove( win->w_desc, line_no, 0 );
		wdeleteln( win->w_desc );
		}

	save_cl = cur_line;	/* not sure we have to */
	cur_line = win->w_height-1;

	/* now we fill in the line at the bottom of the screen. */
	if( lastline ) {
		if( lastline->listptr == NIL ) 
			win->w_lines[cur_line].listptr = NIL;
		else 
			/* full speed option just as well at bottom.  It will stop */
			out_line( lastline, win, TRUE, win->w_height );
		}
	cur_line = save_cl;
}

	/* given a tree location, find the phsical location on the
	   screen for it, if any */

static ClassNum LineClasses[] = {
C_STATEMENT,C_PROGRAM, C_VARIANT, C_DECLARATIONS,
C_CONST_DECL, C_TYPE_DECL, C_VAR_DECL, C_BLCOMMENT,
C_FIELD, C_CASE, C_PASSUP,
C_ROOT,	/* must be the last class as it is NULL */
0	/* isn't necessary now, but might be if C_ROOT changes */
};

find_phys_cursor(xwin, xcurs, anywhere )
alice_window *xwin;		/* window to look in */
curspos xcurs;			/* tree position we look for */
int anywhere;			/* find it anywhere */
{
	register alice_window *win;
	register curspos curs;
	register int i;
	int pc_index = 0;		/* which print code to look for */
	int dir;		/* direction to scan for line */
	int retry = FALSE;
	struct scr_line crline; /* to find node within */
	curspos ncurs;		/* line stature cursor */
	win = xwin;
	curs = xcurs;
	stackcheck();

	/* if the cursor is somehow on a list node, descend to the
	   zeroth child */
	curs = realcp( curs );
	/* first put the cursor on a "line level" spot */

find_again:
	i = find_line_number( win, curs, anywhere ? &pc_index : (int *)0,
					(int *)0 );

	/* we might have found any particular line, if we found one */

	if( i >= 0 ) {
		scrl_copy( crline, win->w_lines[i] );
		if( pc_index ) {
			srch_row = i;
			srch_real = calc_main_indent( crline.ind_level,
							win->w_width);
			return;
			}
		 else {
			/* got it! now outline to find it */

			/* set externals for search */
			srch_cursor = curs;
			cur_line = i;
#ifdef DB_OUTFUNCS
			if(tracing)fprintf(dtrace,"Looking for %x on line %d\n", PCP(srch_cursor), i );
#endif
			out_line( &crline, win, LOOK_CURSOR, 1 );
			/* hopefully we now found the cursor */
			/* externals have been set with position */
			if( srch_row >= 0 && srch_row < win->w_height ) { /*yes*/
				return;
				}
			}
		}


	

	/* oh no, it was not on the screen.  will have to look */
	/* search down for it */
#ifdef DB_OUTFUNCS
	if(tracing)fprintf(dtrace, "Scanning down for cursor line\n" );
#endif
	/* if the bottom is blank, don't bother searching beyond it */
	ncurs = up_to_line(curs, &pc_index);

	pc_index = kcget( pc_index, ncurs );
	if( !retry )for( dir = 1; dir >= 0; dir-- ) {

		scrl_copy( crline, win->w_lines[dir ? win->w_height-1 : 0]);
		if( !crline.listptr )
			break;
		for( i = 0; i < MAX_LSEARCH; i++ ) {
			int mvret;
			if( dir )
				mvret = go_to_next( &crline, crline.sc_size, crline.sub_line ); 
			 else {
				if( mvret = go_to_prev( &crline ) ) {
					if( crline.sub_line >= MAX_LINES )
						crline.sub_line = 0;
					pcod_adjust( &crline );
					}
				}

			if( !mvret )
				break;
			if( ncurs == line_headnode(crline) && crline.pcode == pc_index) {
#ifdef DB_OUTFUNCS
				if(tracing)fprintf(dtrace, "Found it %d lines down\n", i);
#endif
				while( i-- >= 0 )
					if( dir )
						bot_cdown(win);
					 else
						top_cup( win );
				/* TAIL RECURSION */
				retry = TRUE;
				goto find_again;
			}
		    }

		}
	/* do a redisplay */
	win->start_cursor = ncurs;
	win->start_sline = 0;
	win->start_pcode = pc_index;

	display(TRUE);	/* force a redisplay */
	find_phys_cursor( win, curs, FALSE );
	/* boy aren't these recursions risky */

}

/*
 * Fast, tiny up to line
 */
nodep 
t_up_to_line(_np)
nodep _np;
{
	return up_to_line( _np, (int *)0 );
}
nodep 
up_to_line(_np, lchild)
nodep _np;
int *lchild;
{
	register nodep np = _np;
	register nodep oldret;

	oldret = NIL;

	if (!(is_a_stub(np) && is_line_class(int_kid(0,np)) )) {
		while (!is_root(np))
			if (ntype_info(ntype(np)) & F_LINE)  {
				break;
				}
		 	else {
				oldret = np;
				np = tparent(np);
				}
		}
	/* warn about root? */ 
	if( lchild ) {
		if( oldret )
			*lchild = get_kidnum( oldret );
		 else
			*lchild = -1;
		}
	return np;
}


#ifdef OLD
/* routine to move the cursor to a "line level" position */
curspos
up_to_line(xcurs, lchild )
curspos xcurs;		/* cursor to be scanned from */
int *lchild;		/* what child we came up from */
{
	/* we now have the problem that if the node is null we are not
	sure what kind it is, so we have to check the class to see if
	it is something that takes up a whole line. */

	register curspos curs;
	register ClassNum expect_class;
	register nodep ascend;		/* to find a line class parent */
	nodep oldkid;
	int i, imax;		/* counting to find where we came from */
	curs = xcurs;
	
	if( lchild )
		*lchild = -1;
#ifdef DB_OUTFUNCS
	if(tracing)fprintf(dtrace,"Asked to find line of %x\n", PCP(curs) );
#endif
	if( is_a_stub((ascend = curs)) ) {
		expect_class = int_kid(0,curs) & CLASSMASK;
		/* if it is a list stub of line class we stay right here,
		   otherwise, we go up the tree */
		if( is_a_list(tparent(curs)) && is_line_class( expect_class ) ){
#ifdef DB_OUTFUNCS
					if(tracing)
						fprintf(dtrace,"Is Line Class\n");
#endif
					return curs;
					}
		/* did not find it, must ascend */
		ascend = tparent( curs );
		if(lchild)
			*lchild = get_kidnum( curs );
		}
	/* must ascend to a line class parent */
	else if( !is_standard( ascend ) ) {  /* WHY IS THIS HERE ?*/
		ascend = tparent(curs);
		if(lchild)
			*lchild = get_kidnum(curs);
		}
	oldkid = NIL;
	while( !(ntype_info(ntype(ascend)) & F_LINE) ) {
		oldkid = ascend;
		ascend = tparent(ascend);
		}
	/* ascend now points to a line class node, thus not a list node */
	imax = n_num_children(ascend);
	if( oldkid != NIL ) {
		for( i = 0; i < imax; i++ )
			if( node_kid(ascend,i) == oldkid ) {
				if(lchild)
					*lchild = i;
				return ascend;
				}
		if(lchild)
			*lchild = -1;
#ifdef DB_OUTFUNCS
		if(tracing)fprintf(dtrace,"Can't find %x in %d kids of %x\n", oldkid, imax, ascend );
#endif
		error(ER(86,"Did not find child in parent in up_to_line"));
		}
	return ascend ;
}
#endif OLD

/* does a stub take up a whole line? */
is_line_class( stclass )
ClassNum stclass;
{
	return strchr( LineClasses, stclass  & CLASSMASK) != 0;
}
