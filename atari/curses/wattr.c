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

	/* turn on or off reverse video */
wstandout(win)
WINDOW  *win;
{
    wattron(win, A_STANDOUT);
}

wstandend(win)
WINDOW  *win;
{
    wattroff(win, A_STANDOUT);
}


attron(attrs)
short   attrs;
{
    wattron(stdscr, attrs);
}

#ifdef msdos
# define COL_ATRS 0x0700
#endif

/* turn on attributes and/or set the colour */

wattron(xwin, xattrs)
WINDOW  *xwin;
short   xattrs;
{
#ifdef ATARI520ST
	xwin->_flags |= xattrs;
#endif
#ifdef msdos
	register WINDOW *win = xwin;
	register short attrs = xattrs & 0xff00;

	/* if any colour is set, set it totally */

	if( attrs & COL_ATRS )
        	win->_flags = (win->_flags & ~COL_ATRS ) | attrs;
	 else
		win->_flags |= attrs;
	if( xattrs & A_BACKGROUND )
		win->c_background = xattrs << 12;
#endif
}

attroff(attrs)
short   attrs;
{
    wattroff(stdscr, attrs);
}

/* Turn attributes off, and/or set colour back to white */

wattroff(xwin, xattrs)
WINDOW  *xwin;
short   xattrs;
{
#ifdef ATARI520ST
	xwin->_flags &= ~xattrs;
#endif
#ifdef msdos
	register WINDOW *win = xwin;
	register short attrs = xattrs;

	/* if any colour (or underline) is going off, set to white */
	/* NO NO NO!  How do you turn on black letters? */
        win->_flags &= ~attrs;
	if( attrs & COL_ATRS )
		win->_flags |= AC_WHITE;
#endif
}

