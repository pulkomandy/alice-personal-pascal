#include <stdio.h>
#include <whoami.h>
#include <curses.h>

#include <linea.h>
#include <obdefs.h>
#include <osbind.h>
#include <gemdefs.h>
#include <vdibind.h>
#include <stwin.h>

extern FILE *trace;
extern MFDB *GemBits();
extern int charHeight;

/*
 *
 *  Looking Glass Software Limited
 *  "Curses" emulating windowing package for the IBM-PC
 *
 *  The following code is a trade secret of Looking Glass Software Limited
 *  and should be protected and considered proprietary, subject to the
 *  terms of the licence under which this code was provided
 *
 */



/* Be warned that insert and delete line can't be done on the parent of a
 * subwindow
 */

winsertln(xwin)
WINDOW  *xwin;
{
	register WINDOW *win = xwin;
	register int y;
	register int lcount;
	int maxx, maxy;
	register cbufptr holdend;
	register colnum *first;
	int	dirty;
	int	hadCursor;
	int	startLine;
	register int	cury;
	int	i;

	graf_mouse( M_OFF, 0L );
	wind_update( BEG_UPDATE );

	maxy = win->_maxy;
	maxx = win->_maxx;
	cury = win->_cury;

	first = win->_firstch;

	hadCursor = win->_hascur;
	if( win->_hascur ) {
		first[cury] = TRUE;
		first[win->c_oldy] = TRUE;
	}

	/* This is the line that is disappearing */
	holdend = win->_y[maxy-1];

	/*
	 * Insert quickly by block moving the lines down the screen.
	 */
	for( y = maxy-1; y > win->_cury; y-- ) {
		win->_y[y] = win->_y[y-1];
	}

	win->_y[y] = holdend;
	first[y] = TRUE;
	lcount = win->_maxx;
	while( lcount-- )
		*holdend++ = SPACE;

	/* If we are scrolling the whole window, and the screen is
	 * just one box that is not obscured, then we can go and blit
	 * the screen and clear the window
	 */
	if( cury == 0 && OneBox( win ) ) {
		ScrollWin( win, charHeight );
		/* Scroll the dirty bits, and dirty the cursor line */
		first[0] = TRUE;
		for( y = maxy-1; y > 0; y-- ) {
			first[y] = first[y-1];
		}
		first[0] = TRUE;
	} else {
		/* Mark all the scrolled lines as dirty */
		for( y = maxy - 1; y >= cury; y-- )
			first[y] = TRUE;
	}

	wind_update( END_UPDATE );
	graf_mouse( M_ON, 0L );
}

/* just does a delete of line 0 to scroll screen */

scroll(win)
WINDOW *win;
{
	int savy = win->_cury;
	win->_cury = 0;
	wdeleteln(win);
	win->_cury = savy;
}

wdeleteln(win)
register WINDOW  *win;
{
	register int y;
	register int lcount;
	register cbufptr holdend;
	register colnum *first;
	int maxy, cury;
	int	dirty;
	int	hadCursor;
	int	startLine;
	int	i;

	graf_mouse( M_OFF, 0L );
	wind_update( BEG_UPDATE );

	first = win->_firstch;

	hadCursor = win->_hascur;
	cury = win->_cury;
	if( hadCursor ) {
		first[cury] = TRUE;
		first[win->c_oldy] = TRUE;
	}

	maxy = win->_maxy - 1;
	holdend = win->_y[cury];

	/*
	 * Change the lines around to scroll the curses window
	 */
	for( y = cury; y < maxy; y++ ) {
		win->_y[y] = win->_y[y+1];
	}

	win->_y[maxy] = holdend;
	first[maxy] = TRUE;
	lcount = win->_maxx;
	while( lcount-- )
		*holdend++ = SPACE;

	/* If we are scrolling the whole window, and the screen is
	 * just one box that is not obscured, then we can go and blit
	 * the screen and clear the window
	 */
	if( cury == 0 && OneBox( win ) ) {
		ScrollWin( win, -charHeight );
		for( y = 0; y < maxy; y++ )
			first[y] = first[y+1];
		first[maxy] = TRUE;
	} else {
		for( y = cury; y <= maxy; y++ )
			first[y] = TRUE;
	}

	wind_update( END_UPDATE );
	graf_mouse( M_ON, 0L );
}


#ifndef GEM
/* insert char */
winsch(win, c)
WINDOW  *win;
char	c;
{
	cbufptr opos;
	int curx = win->_curx;
	int i;

	opos = win->_y[win->_cury];

	for( i = win->_maxx - 2; i >= curx; i-- )
		opos[i+1] = opos[i];
	waddch( win, c );

}

/* delete char */
wdelch(win)
WINDOW  *win;
{
	cbufptr opos;
	int maxx = win->_maxx;
	int i;

	opos = win->_y[win->_cury];

	for( i = win->_curx + 1; i < maxx-1; i++ )
		opos[i] = opos[i+1];
	opos[maxx-1] = SPACE;
}
#endif
