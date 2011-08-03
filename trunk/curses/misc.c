

#include <whoami.h>
#include <curses.h>
#include <osbind.h>

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

/* min and max macros */

#define min(a,b) ((a>b) ? (b) : (a))
#define max(a,b) ((a>b) ? (a) : (b))

/* This routine draw a box around the inside of the window 
 * If the characters hor and vert are zero or '-' and '|' it uses the
 * fancy ibm box characters
 */ 
box(xw,vert,hor)
WINDOW *xw;
int vert, hor;
{

/* ignore arguments, use nice ibm characters */
	register WINDOW *w = xw;
	register int i;
	int hvcs;
	int endy = w->_maxy-1;
	int endx = w->_maxx-1;

	hvcs = (vert == 0 || vert == '|' ) && ( hor == 0 || hor == '-');
	if( hvcs ) {
#ifdef ATARI520ST
		vert = '|';
		hor = '-';
#endif
		}
	/* make it white if no colour given */

	for (i=0; i<=endy; i++) {
		w->_y[i][0] = vert;
		w->_y[i][endx] = vert;
		w->_firstch[i] = 1;
	}
	for (i=0; i<=endx; i++) {
		w->_y[0][i] = hor;
		w->_y[endy][i] = hor;
	}

	if( hvcs ) {
#ifdef ATARI520ST
		/* Put the corners in */
		w->_y[0][0] = '+';
		w->_y[0][endx] = '+';
		w->_y[endy][0] = '+';
		w->_y[endy][endx] = '+';
#endif
		}

}


/*
 * Make the terminal beep.
 */
beep()
{
#ifdef ATARI520ST
	Bconout( 2, 7 );
#endif
}


/* turn on and off block cursors */

curOn(win)
WINDOW *win;
{
	if( !win->_hascur ) {
		win->_hascur = TRUE;
		/* dirty line to show cursor */
		win->_firstch[win->_cury] = TRUE;
		}
}
curOff(win)
WINDOW *win;
{

	if( win->_hascur ) {
		win->_hascur = FALSE;
		/* dirty the line to erase cursor */
		win->_firstch[win->c_oldy] = TRUE;
		}

}

/*
 * standard curses debug function. some people use it
 */
char *
unctrl( ch )
char ch;
{
	static char cstr[3];

	if( ch < ' ' ) {
		cstr[1] = ch | '@';
		cstr[0] = '^';
		cstr[2] = 0;
		}
	 else {
		cstr[0] = ch;
		cstr[1] = 0;
		}
	return cstr;
}


