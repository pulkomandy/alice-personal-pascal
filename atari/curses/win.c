#include <stdio.h>
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

#define REAL_MAX_LINES 66

#ifndef MX_WIN_WIDTH
# define MX_WIN_WIDTH 80
#endif

#ifdef ATARI520ST
# define MAX_LINES	25
#endif

#ifdef ATARI520ST
extern int charHeight, charWidth;
#endif

extern char *malloc();
extern char *calloc();
extern WINDOW *winbody();

int LINES   = 0;   
int COLS    = 0;

WINDOW  *stdscr = 0, *curscr = 0;

/* start everything the same */

WINDOW *
initscr()
{
	if( COLS == 0 )
		COLS = 80;
	if( LINES == 0 )
		LINES = MAX_LINES;
	init_tty();

#ifdef ATARI520ST
	init_draw();
#endif

#if !defined(ATARI520ST)

	if( stdscr )
		delwin( stdscr );

	/* curscr has no special meaning.  It is stdscr */
	stdscr = curscr = newwin(LINES, COLS, 0, 0);

#endif

	return stdscr;

}

endwin()
{
	end_tty();
#if !defined(ATARI520ST) || CURSES_VERSION
	/* There is no stdscr in the ST version */
	delwin(stdscr);
	curscr = stdscr = (WINDOW *)0;
#endif

}

WINDOW *
newwin(lines, cols, begin_y, begin_x)
int lines;
int cols;
int begin_y;
int begin_x;
{
	register WINDOW *win;
	register int i;
	register cbufptr sp;
	cbufptr recbuf;
	int bbsize;

	if( lines == 0 )
		lines = LINES;
	if( cols == 0 )
		cols = COLS;
	win = winbody( lines, cols, begin_y, begin_x );

	if( !win )
		return win;
#ifndef MIN_OUT
	if ((win->_firstch = calloc(FIRARS*lines, sizeof (short))) == NULL) {
		free(win->_y);
		free(win);
		return (WINDOW *)NULL;
		}
#endif

	/* allocate a big buffer */
	bbsize = cols * lines * sizeof(chtype);
#ifdef HYBRID
	recbuf = (cbufptr ) choosealloc( bbsize );
#else
	recbuf = (cbufptr ) calloc( bbsize, 1 );
#endif

	if( recbuf == (cbufptr )NULL ) {
		free( win->_firstch );
		free( win->_y );
		free( win );
		return (WINDOW *)NULL;
	}
	/* make pointers into it and clear it */
	for (i = 0; i < lines; i++) {
		cbufptr maxv;
		win->_y[i] = &recbuf[i*cols];
		maxv = win->_y[i] + cols;
		for (sp = win->_y[i]; sp < maxv; )
				*sp++ = SPACE;
#ifdef MIN_OUT
		win->_lastnw[i] = win->_firstch[i] = 0
		win->_firstnw[i] = win->_lastch[i] = cols;
#else
		win->_firstch[i] = TRUE;
#endif
		}
	return win;
}


delwin(xwin)
WINDOW  *xwin;
{
	register WINDOW *win = xwin;
	int j;
	cbufptr minp;

	/* do not free the memory pointed to by a subwin */
	/* although perhaps we should copy it on in proper order */
	if( !( win->_flags & _SUBWIN ) ) {
		minp = win->_y[0];
		for( j = 1; j < win->_maxy; j++ )
			if( win->_y[j] < minp )
					minp = win->_y[j];
		free( minp );

		}

	if( !( win->_flags & _SUBWIN ) )
		free(win->_firstch);

	free(win->_y);
	free(win);

}

/* mark everything touched for full refresh */

touchwin(win)
WINDOW  *win;
{
	register int y;

	for( y = 0; y < win->_maxy; y++ ) {
#ifdef MIN_OUT
		win->_firstnw[y] = win->_firstch[y] = 0;
		win->_lastnw[y] = win->_lastch[y] = win->_maxx;
#else
		win->_firstch[y] = TRUE;
#endif
		}
}


#ifndef GEM
/*
 * Force output to the screen.
 * We output whatever's been changed.
 */
wrefresh(win)
register WINDOW  *win;
{
	register int y;
	register int maxx, maxy;
	register cbufptr *winy;

	short cy = win->_cury;
	short cx = win->_curx;

	/* Only msdos implemented with SCREENIMAGE for now. */
	maxx = win->_maxx;
	maxy = win->_maxy;
	winy = win->_y;

#ifndef HARD_CURSOR
	/* line where old cursor was is touched */
	if( win->_hascur ) {	 /* has a cursor */
# ifdef MIN_OUT
		dirty_loc( win, cy, cx );
		dirty_loc( win, win->c_oldy, win->c_oldx );
# else
		if( win->c_oldy != cy ) {
			win->_firstch[win->c_oldy] = TRUE;
			win->_firstch[cy] = TRUE;
			}
	 	else if( win->c_oldx != cx )
			win->_firstch[cy] = TRUE;
		
# endif MIN_OUT
	
#ifdef ATARI520ST
		winy[cy][cx] ^= REV_BIT;	/* reverse it */
#endif
		}
#endif /* HARD_CURSOR */
	
#ifdef ATARI520ST

	/* Set up the VDI to do regular text drawing */
	startDraw();
	for (y = 0; y < maxy; y++) {
		/* Output all the dirty lines */
		/* Clipping rectangle set previously */
		if( isdirty( win, y ) ) {
			/* Draw line, starting at _begx, _begy */
			/* for a maximum of maxx chars */
#ifdef CURSES_VERSION
			draw_char( win->_begx * charWidth,
				(win->_begy + y) * charHeight,
				winy[y], maxx );
#else
			draw_char(win->_begx, win->_begy + (y * charHeight),
				   winy[y], maxx);
#endif
			/* Line isn't dirty anymore */
			clean_line( win, y );
		}
	}
#endif

	if( win->_hascur ) {

#ifdef ATARI520ST
		win->_y[cy][cx] ^= REV_BIT;	/* un-reverse it */
#endif
		win->c_oldy = cy;
		win->c_oldx = cx;
		}
}

#endif GEM

	/* this will not move an associated subwin (or will it?) */
#ifndef GEM

mvwin(win, y, x )
WINDOW *win;
int y;
int x;
{
#if !defined(ATARI520ST) || CURSES_VERSION
	if( win->_maxy + y > LINES || win->_maxx + x > COLS )
		return ERR;
#endif
	win->_begx = x;
	win->_begy = y;
	return 0;
}

#endif

/* common routine used by subwin and newwin */

WINDOW  * 
winbody( lines, cols, begin_y, begin_x )
int lines;
int cols;
int begin_y;
int begin_x;
{
	WINDOW *win;

	if ((win = (WINDOW *) malloc(sizeof(WINDOW))) == 0)
		return (WINDOW *)NULL;


	win->_cury  = 0;
	win->_curx  = 0;
	win->_maxy  = lines;
	win->_maxx  = cols;
#ifdef ATARI520ST
	win->_flags = AC_BLACK;
#endif
	win->c_background = 0;	/* black background to start */
	win->_clear = 0;
	win->_hascur = TRUE;	/* cursor on or off */
	win->_begx  = begin_x;
	win->_begy  = begin_y;
	win->_scroll = FALSE;
	win->c_oldy = 0;
	win->c_oldx = -1;
	if ((win->_y = (cbufptr *) calloc(lines, sizeof (cbufptr ))) == (cbufptr *)NULL) {
		free(win);
		return (WINDOW*)NULL;
	}
	if ((win->_firstch = calloc(FIRARS*lines, sizeof (colnum))) == NULL) {
		free(win->_y);
		free(win);
		return (WINDOW *)NULL;
		}
	return win;
}


	/* here curses differs from the document.  We follow the program */

WINDOW *
subwin(parent, lines, cols, begin_y, begin_x)
WINDOW *parent;
int lines;
int cols;
int begin_y;
int begin_x;
{
	register WINDOW *win;
	register int i;
	cbufptr *parbuf;


	/* does it fit */

#if !defined(ATARI520ST) || CURSES_VERSION

	if ( begin_y < parent->_begy || begin_x < parent->_begx ||
			begin_y + lines > parent->_maxy + parent->_begy ||
			begin_x + cols > parent->_maxx + parent->_begx) 
		return (WINDOW *)ERR;

	if( !cols )
		cols = parent->_maxx + parent->_begx - begin_x;
	if( !lines)
		lines = parent->_maxy + parent->_begy - begin_y;
#endif

	win = winbody( lines, cols, begin_y, begin_x );

	if( !win )
		return win;
	parbuf = parent->_y;
	/* In the minimal output scheme, touching a subwindow does not affect
	   the dirty status of the parent.  This would be a pain to manage
	   in our scheme.  If you need this, you can't use the minimal output
	   scheme.  You may not need it, as it only gives a real bonus on
	   flickering IBM CGAs
	 */
#ifndef MIN_OUT

	/* use memory from parent for touched flag */

	win->_firstch = &parent->_firstch[begin_y - parent->_begy];
#endif
#if !defined(ATARI520ST) || CURSES_VERSION
	for (i = 0; i < lines; i++) {
		
		win->_y[i] = &parbuf[begin_y-parent->_begy+i]
				    [begin_x-parent->_begx];
		win->_firstch[i] = TRUE;
		}
	win->_flags |= _SUBWIN;
#endif
	return win;
}

/*
 * Resize a curses window, saving as much of the info as possible
 * returns 0 if successfull, 1 if not
 */

int
wresize( win, new_width, new_height )
WINDOW *win;
int new_width;
int new_height;
{
	int bbsize;
	cbufptr *line_ptr;
	colnum *dirty_ptr;
	int old_width, old_height;
	cbufptr sp;
	int i, j;
	cbufptr recbuf;
	int hadcur;

	/*
	 * If there is no window, return it !
	 */

	if( !win )
		return win;

	hadcur = win->_hascur;

	if( hadcur )
		curOff( win );

	/*
	 * Allocate a new set of line pointers
	 */
	line_ptr = (cbufptr *) calloc( FIRARS * new_height, sizeof( cbufptr ) );

	if( line_ptr == NULL )
		return TRUE;

	/*
	 * Allocate a new 'dirty' vector
	 */
	dirty_ptr = (colnum *) calloc( FIRARS * new_height, sizeof( colnum ) );
	if( dirty_ptr == NULL ) {
		free( line_ptr );
		return TRUE;
	}

	/* allocate a big buffer */
	bbsize = new_width * new_height * sizeof(chtype);

#ifdef HYBRID
	recbuf = (cbufptr ) choosealloc( bbsize );
#else
	recbuf = (cbufptr ) calloc( bbsize, 1 );
#endif

	if( recbuf == (cbufptr )NULL ) {
		free( line_ptr );
		free( dirty_ptr );
		return TRUE;
	}

	/*
	 * We have successfully allocated all the memory
	 */
	old_width = win->_maxx;
	old_height = win->_maxy;

	/* make pointers into it and clear it */
	for (i = 0; i < new_height; i++) {
		cbufptr maxv;
		line_ptr[i] = &recbuf[ i * new_width ];
		maxv = line_ptr[i] + new_width;
		for (sp = line_ptr[i]; sp < maxv; )
				*sp++ = SPACE;
		dirty_ptr[i] = TRUE;
	}

	for( i=0; i < min( new_height, old_height ); i++ ) {
		for( j=0; j < min( new_width, old_width ); j++ ) {
			line_ptr[i][j] = win->_y[i][j];
		}
	}

	/*
	 * Free the old structs
	 */

	free( win->_y[0] );
	free( win->_y );
	free( win->_firstch );

	/*
	 * Store the new pointers
	 */
	win->_y = line_ptr;
	win->_firstch = dirty_ptr;

	win->_maxx = new_width;
	win->_maxy = new_height;
	win->_cury = new_height - 1;
	win->_curx = 0;
	win->c_oldy = 0;
	win->c_oldx = -1;

	if( hadcur )
		curOn( win );

	return FALSE; /* No error */
}
