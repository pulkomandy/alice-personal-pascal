/*
 *
 *DESC:  Routines to handle windows of various sorts for Alice
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "dbflags.h"
#ifndef A_ATTRIBUTES
#define A_ATTRIBUTES 0x80
#endif
#ifdef DB_WIN
#define PRINTT
#endif
#include "printt.h"

WINDOW *pg_out;		/* program output */
WINDOW *db_out;		/* debug output here */
WINDOW *savemain, *savecmd; /* holding pointers for old cmd and main
			window structures */

int	SaveLines;	/* Number of lines in the old window */

	/* setup pg_out window to get ready to run */
prep_to_run()
{
#ifdef HAS_INTERP
#ifdef QNX
	/* flush input buffer of characters on icon */
	while( getbc(TRUE) )
		getbc(FALSE);
#endif
	if( work_debug(curr_workspace) & DBF_ON ) {
		change_pg_out(work_owindow(curr_workspace));
		printt1( "Setting program output to window %x\n", pg_out );
		}
	 else {
		change_pg_out(curWindows[MAINWIN]); 
		werase(  pg_out );
		wmove( pg_out, 0, 0);
		printt0( "Using main win for program output\n" );
		wrefresh( pg_out );
		}
	scrollok( pg_out, TRUE );
	curOff( pg_out );
#endif HAS_INTERP
}

#ifdef DEMO
extern int demo_mode;
#endif

prep_r_window()
{
#ifdef HAS_INTERP
	int cwlines;
	register int newlines;
	int statrows;		/* how many rows for status lines */
	WINDOW *newerr, *newedit, *newout, *newcmd;

	if( work_debug(curr_workspace) & DBF_ON )
		return;		/* already set up */
#ifdef Notdef
	/* This code is for a fancier windowing system with an output
	   window always present */
	if( curr_window->out_split ) {
		work_owindow(curr_workspace) = curr_window->out_split;
		work_debug(curr_workspace) |= DBF_ON;
		return;
		}
#endif
	SaveLines = cwlines = curr_window->w_height;
	newlines = ( cwlines * 2 ) / 3;
	statrows = COLS < 64 ? 2 : 1;
	if( newlines < 3 )
		error(ER(179,"noroom`Not enough room to set up program output window"));

	savemain = curWindows[MAINWIN];
	savecmd = curWindows[CMDWIN];

	/* 
	 * clear the entire screen, so that we can recreate the windows in
	 * new positions 
	 */
	
	/*  (is thes needed?)
	wclear( stdscr );
	wrefresh( stdscr );
	*/

#ifdef ICON_WIN_MGR
	newerr = pnewwin( statrows,  curr_window->w_width,
			(1+DEMO_BORD)*BORDER_WIDTH+DEMO_LINES*CHAR_HEIGHT,
			CHAR_WIDTH/2 - 1, FALSE);
	newedit = pnewwin( newlines,  curr_window->w_width,
		(statrows+DEMO_LINES)*CHAR_HEIGHT + (3+DEMO_BORD)*BORDER_WIDTH,
		CHAR_WIDTH/2 - 1, TRUE );
	newcmd = pnewwin( statrows, curr_window->w_width,
		(newlines + statrows + DEMO_LINES)*CHAR_HEIGHT +
				(6+DEMO_BORD)*BORDER_WIDTH,
		CHAR_WIDTH/2 - 1, FALSE );
	newout = pnewwin( cwlines - newlines, curr_window->w_width,
		(newlines+2*statrows+DEMO_LINES)*CHAR_HEIGHT+(9+DEMO_BORD)*BORDER_WIDTH,
		CHAR_WIDTH/2 - 1, FALSE );
#else
	newerr = subwin( stdscr, statrows,  curr_window->w_width, DEMO_LINES,
				savemain->_begx );
	curOff( newerr );
	newedit = subwin( stdscr, newlines, curr_window->w_width, savemain->_begy,
		savemain->_begx);
	newcmd = subwin( stdscr, statrows, curr_window->w_width,
		savemain->_begy + newlines, savemain->_begx);
	newout = subwin( stdscr, cwlines - newlines, curr_window->w_width,
		savemain->_begy + newlines + statrows, savemain->_begx);
	curOff( newcmd );
#endif ICON_WIN_MGR

	printt2( "New editor window %d high, %d wide\n", 
			newlines, curr_window->w_width );

	work_debug(curr_workspace) |= DBF_ON;

	dmfree(curr_window->w_lines);
	setwlines(curr_window, newlines );

	delwin( curWindows[ERRWIN] );

	curWindows[CMDWIN] = newcmd;
	curWindows[ERRWIN] = newerr;
	curWindows[MAINWIN] = curr_window->w_desc = newedit;
	curWindows[EXTRAWIN] = work_owindow(curr_workspace) =
				curr_window->out_split = newout;

	wclear(newout);
	wrefresh(newout);

	show_whole_doda();
#endif HAS_INTERP
}

	/* delete the user output window */

	/* The implicit assumption is made here that del_r_window is */
	/* called only after prep_r_window */
	/* This doesn't SEEM dangerous ... */

del_r_window()
{
#ifdef HAS_INTERP
	if( !(work_debug(curr_workspace) & DBF_ON) )
		error( ER(180,"nodebug`Debug window does not exist") );

	work_debug(curr_workspace) = 0;
	delwin( curr_window->out_split );
	delwin( curr_window->w_desc );
	delwin( curWindows[CMDWIN] );


	curr_window->w_desc = curWindows[MAINWIN] = savemain;
	curr_window->out_split = (WINDOW *)0;
	curWindows[EXTRAWIN] = (WINDOW *)0;
	curWindows[CMDWIN] = savecmd;
	redrw_screen(TRUE);
#endif HAS_INTERP
}

refresh_scr()
{
#ifdef SCREENIMAGE
	register int i;
	WINDOW *win;

	for( i = 0; win = curWindows[i]; i++ ) {
		if( win != stdscr ) {
			touchwin( win );
			wrefresh( win );
			}
		}
# ifdef DEMO
	if( demo_mode ) {
		extern WINDOW *demo_win;
		touchwin( demo_win );
		wrefresh( demo_win );
		}
# endif
#else
	redrw_screen(FALSE);
#endif
}

redrw_screen( recreate )
int	recreate;
{
	register int	i;
	register WINDOW *win;
#ifdef DEMO
	extern WINDOW *demo_win;
#endif

	/* clear the screen, so that we can recreate the windows */

	/* wclear( stdscr ); */
#if BORD
	if ( !curWindows[EXTRAWIN] ) 
		box( stdscr, ACS_VLINE, ACS_HLINE );
	wrefresh(stdscr);
#endif
	
	for( i = 0; (win = curWindows[i]) != (WINDOW *)NULL; i++ ) {
		if (win == stdscr) 
			continue;

		touchwin( win );
		wrefresh( win );

/*#ifdef SCREENIMAGE  */
		if (recreate)
/* #endif SCREENIMAGE */
		    {	
		    curWindows[i] = subwin(stdscr, win->_maxy, win->_maxx,
			win->_begy, win->_begx);
		    delwin( win );
		    }
		}
		curOff( curWindows[ERRWIN] );
		curOff( curWindows[CMDWIN] );
#ifdef DEMO
		if( demo_mode ) {
			touchwin( demo_win );
			wrefresh( demo_win );
#ifdef NOTDEF
			if( recreate ) {
				win = demo_win;
				demo_win = 
					newwin( demo_win->_maxy,demo_win->_maxx,
						demo_win->_begy, demo_win->_begx );
				delwin( win );
				curOff( demo_win );
				scrollok( demo_win, TRUE );
				}
#endif
			}

			
#endif /* DEMO */

	main_win.w_desc = curWindows[MAINWIN];
	if( recreate ) {
		dmfree(curr_window->w_lines);
		setwlines( curr_window, curWindows[MAINWIN]->_maxy);
		}

	if ( curWindows[EXTRAWIN] ) 
		work_owindow(curr_workspace) = curr_window->out_split
						= curWindows[EXTRAWIN];

	show_whole_doda();
}

show_whole_doda()
{
	extern int cantredraw;
	extern int error_present;

	if( cantredraw )
		return;
	statusLine();
	if( error_present > 0 )
		repeat_err();
	 else
		pfkeyslabel();
	display(TRUE);
}

	/* setup the lines table for a window */

setwlines( xalwin, nlines )
alice_window *xalwin;
int nlines;
{
	register int lin;
	register alice_window *alwin = xalwin;

	alwin->w_lines = (struct scr_line *)checkalloc(nlines * sizeof(struct scr_line) );	
	alwin->big_change = TRUE;
	alwin->w_height = nlines;
	for( lin = 0; lin < nlines; lin++ ) {
		(alwin->w_lines[lin]).sc_size = (bits8) 0;
		(alwin->w_lines[lin]).listptr = NIL;
		}
}

/* set start cursor to a node */

set_wscur( xalwin, newtop )
alice_window *xalwin;
nodep newtop;
{
	register alice_window *alwin = xalwin;

	alwin->start_cursor = newtop;
	alwin->start_sline = 0;

	alwin->start_pcode = kcget( -1, newtop );
}

/* set top of screen to a given line */

set_sc_fline( xwin, line ) 
alice_window *xwin;		/* window we are moving */
struct scr_line *line;		/* pointer to line struct */
{
	register alice_window *win = xwin;

	win->start_cursor = line_headnode( *line );
	win->start_pcode = line->pcode;
	win->start_sline = line->sub_line;
}

initWindows()
{
	int	i;

	for (i=0; i<MAXWINDOWS; i++)
		curWindows[i] = NULL;
	curWindows[STDWIN] = stdscr;
	curOff( stdscr );

#ifdef DEMO
	init_demo();
#endif
	make_std_windows();

	scrollok(main_win.w_desc, TRUE);
	crmode();
	noecho();
#ifdef ECURSES
	keypad(stdscr, TRUE);
#endif

	werase(main_win.w_desc);
	werase(curWindows[CMDWIN]);
	werase(curWindows[ERRWIN]);
}

int windows_made = FALSE;

make_std_windows() 
{
	register int 	mainLines;
	register int statrows;
#ifdef DEMO
	extern WINDOW *demo_win;
#endif

	statrows = COLS < 64 ? 2 : 1;

#ifdef ICON_WIN_MGR

	mainLines = (SCREEN_YPIXELS - 2*statrows*CHAR_HEIGHT -
			(6+DEMO_BORD)*BORDER_WIDTH) / CHAR_HEIGHT - DEMO_LINES;

	curWindows[ERRWIN] = pnewwin( statrows , COLS - 1, BORDER_WIDTH +
				DEMO_LINES*CHAR_HEIGHT, 
				CHAR_WIDTH/2 - 1, FALSE);
	main_win.w_desc = curWindows[MAINWIN] = pnewwin(mainLines, COLS - 1,
		(statrows+DEMO_LINES)*CHAR_HEIGHT + (4+DEMO_BORD)*BORDER_WIDTH,
		CHAR_WIDTH/2 - 1, TRUE);
	curWindows[CMDWIN] = pnewwin(statrows, COLS - 1,
			(DEMO_LINES+mainLines + statrows)*CHAR_HEIGHT +
			(7+DEMO_BORD)*BORDER_WIDTH, CHAR_WIDTH/2 - 1, FALSE);
	main_win.w_width = COLS - 1;
# ifdef DEMO
	if( demo_mode ) {
		demo_win = pnewwin( DEMO_LINES, COLS-1, BORDER_WIDTH,
			CHAR_WIDTH/2-1, TRUE);
		scrollok( demo_win, TRUE );
		}
# endif
#else
	mainLines = LINES - 2*statrows - DEMO_LINES;
	curOff( curWindows[ERRWIN] = subwin( stdscr, statrows, COLS - 2*BORD,
		DEMO_LINES, BORD) );
	main_win.w_desc = curWindows[MAINWIN] = subwin(stdscr, mainLines,
		COLS - 2*BORD, statrows+DEMO_LINES, BORD);
	curOff( curWindows[CMDWIN] = subwin(stdscr, statrows, COLS - 2*BORD,
			(DEMO_LINES+mainLines + statrows), BORD) );
	main_win.w_width = COLS - 2*BORD;
# ifdef DEMO
	if( demo_mode ) {
		curOff( demo_win = newwin( DEMO_LINES, COLS-2*BORD, 0, BORD ));
		scrollok( demo_win, TRUE );
		}
# endif
#endif

	windows_made = TRUE;
	setwlines( &main_win, mainLines);
}

