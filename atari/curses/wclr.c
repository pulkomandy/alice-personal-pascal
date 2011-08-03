#include <whoami.h>
#include <curses.h>
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

#define attributes(X)	(X & A_ATTRIBUTES)

/*
 * curscr and stdscr are the same, wclear and werase are the same
 */
wclear(xwin)
WINDOW  *xwin;
{
	register WINDOW *win = xwin;
	register int y, x;
	int maxx, maxy;
	unsigned blank = SPACE | attributes(win->_flags);

	maxx = win->_maxx;
	maxy = win->_maxy;

	for(y=0; y<maxy; y++) {
		for (x=0; x<maxx; x++)
			win->_y[y][x] = blank;
		win->_firstch[y] = TRUE;
	}
	win->_cury = 0;
	win->_curx = 0;
}


wclrtoeol(xwin)
WINDOW  *xwin;
{
	register WINDOW *win = xwin;
	register int i, r, maxv;

	unsigned blank = SPACE | attributes(win->_flags);

	r = win->_cury;
	maxv = win->_maxx;
	for (i=win->_curx; i<maxv; i++)
		win->_y[r][i] = blank;
	win->_firstch[r] = TRUE;
}

#ifndef GEM
/* 
 * wclrtoeol's opposite number
 */
wclrfrombeg(xwin)
WINDOW *xwin;
{
	register WINDOW *win = xwin;
	register int i,r;
	unsigned blank = SPACE | attributes(win->_flags);

	r = win->_cury;
	for (i= win->_curx - 1; i >= 0; i-- )
		win->_y[r][i] = blank;
	win->_firstch[r] = TRUE;
}
#endif

/* clear to bottom of screen */

wclrtobot(xwin)
WINDOW  *xwin;
{
	register WINDOW *win = xwin;
	register int y, x;
	int maxy, maxx;
	unsigned blank = SPACE | attributes(win->_flags);

	wclrtoeol(win);
	maxx = win->_maxx;
	maxy = win->_maxy;
	for (y=win->_cury+1; y<maxy; y++) {
		for (x=0; x<maxx; x++)
			win->_y[y][x] = blank;
		win->_firstch[y] = TRUE;
	}
}

/* screens don't mean anything in this version, so same as wclear */

werase(win)
WINDOW  *win;
{
	wclear(win);
}

