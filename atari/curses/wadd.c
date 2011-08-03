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

/* the big character adder */

waddch(xwin, c)
WINDOW  *xwin;
int c;
{
	register WINDOW *win = xwin;
	int at;
	chtype res;
	int y = win->_cury;
	int x = win->_curx;

	/* modified here to do a cr if x >= max if you want it to wrap */
	if( y >= win->_maxy || x > win->_maxx )
		return ERR;
	if( x == win->_maxx && c >= ' ' ) {
		/* if it would cause a nasty scroll */
		if( y == win->_maxy - 1 && !win->_scroll )
			return ERR;
		/* do a cr */
		waddch( win, '\n' );
		x = 0;
		y = win->_cury;
		}

	/* is it a tab? */

	if (c == '\t') {
		do {
			waddch(win, ' ');
			} while (win->_curx & 7);
		return OK;
		}
	 else if (c == '\n' ) {
#ifdef RELEASE
		wclrtoeol( win );
#endif
		if (win->_cury + 1 >= win->_maxy ) {
			int savy;
			if( !win->_scroll )
				return ERR;
			savy = win->_cury;
			/* do a newline */
			wmove( win, 0, 0 );
			wdeleteln( win );
			wmove( win, savy, 0 );
			}
		else {
			win->_cury++;
			win->_curx = 0;
			}
		return OK;
		}
	 else if ( c == '\r' ) {
		win->_curx = 0;
		return OK;
		}
	 else if( c == 8 ) {
		if( win->_curx )
			win->_curx--;
		return OK;
		}
	 else
		win->_curx++;

	at = c&A_ATTRIBUTES | (win->_flags & A_ATTRIBUTES);
	c &= A_CHARTEXT;
#ifdef msdos
	if( at & A_REVERSE )
		at = (at & 0x8800) |((at << 4) & 0x7000);
	 else if( at & A_STANDOUT )
		at = 0x7000;
	 else if( at & A_BACKGROUND )
		at = ( at & 0x8f00 ) | win->c_background;

	if( win->_flags & _DOSWIN ) {
		dosPutAttr( c, at >> 8, win->_begx + x, win->_begy + y, 1 );
		return OK;
		}
#endif

	res = c | at;

	if( win->_y[y][x] != res ) {
		win->_y[y][x] = c | at;
		win->_firstch[y] = 1;   /* mark line "dirty" */
		}
	win->_flags |= _WRITTENTO;

	return OK;
}

waddstr(win, xs)
WINDOW  *win;
unsigned char *xs;
{
	register unsigned char *s = xs;
	while (*s)
		if( waddch(win, *s++) == ERR )
			return ERR;
	return OK;
}

/*
 * special routine to add a vector of characters already set with
 * attributes.  Not as checked as it should be
 */

wattrstr(win, _ip)
WINDOW  *win;
chtype   *_ip;	/* a real chtype * */
{
	register chtype *ip = _ip;	/* a real chtype * */
	register cbufptr rsar;
	register int	column;
	int	maxtogo;

	column = win->_curx;
	maxtogo = win->_maxx;
	rsar = win->_y[win->_cury]+column;

	while (column < maxtogo && *ip) {
		*rsar++ = *ip++;
		++column;
		win->_curx++;
	}
	win->_flags |= _WRITTENTO;

	return column >= maxtogo ? ERR : OK;
}
/* printw assumes the Mark williams %r feature, which originated at Waterloo
 * This feature is the only way to get a portable printw over large and
 * small models.  What this feature does is tell printf that the argument
 * that cooresponds to %r is a pointer to some arguments on the stack that
 * are more printf arguments
 */


/* here is printw without %r - not as elegant */
printw( format, a, b, c, d, e, f, g, h, i )
char *format;
{
	return wprintw( stdscr, format, a, b, c, d, e, f, g, h, i );
}

#define BUFSIZE	 256



/* here is a wprintw if you don't have %r */
wprintw( win, format, a, b, c, d, e, f, g, h, i )
WINDOW *win;
char *format;
{
	char buf[BUFSIZE];

	sprintf( buf, format, a, b, c, d, e, f, g, h, i );
	return waddstr( win, buf );
}
