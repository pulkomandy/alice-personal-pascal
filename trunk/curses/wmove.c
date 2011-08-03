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
int realY = -1;	 /* physical cursor is here */
int realX = -1;

wmove(xwin, y, x)
WINDOW  *xwin;
short   y;
short   x;
{
	register WINDOW *win = xwin;
	if (y < 0 || y >= win->_maxy || x < 0 || x >= win->_maxx)
		return ERR;

	win->_cury = y;
	win->_curx = x;
#ifdef msdos
	if( win->_flags & _DOSWIN ) {
		_poscurs( win->_begy + win->_cury, win->_begx + win->_curx );
		}
#endif

	return OK;

}

#ifdef msdos
mvcur(oldy, oldx, y, x)
int oldy;
int oldx;
int y;
int x;
{
	_poscurs( y, x );
}
#endif
