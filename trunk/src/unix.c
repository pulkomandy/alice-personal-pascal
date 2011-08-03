/*
 * Routines that depend on Unix or the unix curses.  Loaded in for
 * running on unix with unix curses.  Other files for other systems
 */
#include "alice.h"
#include <curses.h>
#ifdef ATARI520ST
#include <osbind.h>
#endif

#ifndef ATARI520ST
/*
 * clear from beginning of line routine
 */

wclrfrombeg( win )
WINDOW *win;
{
	register int i;
	int y;	/* not that important */

	getyx( win, y, i );
	wmove( win, y, 0 );
	while( i-- )
		waddch( win, ' ' );
}

wattrstr( win, ip )
WINDOW *win;
char *ip;
{
	waddstr( win, ip );
}

#endif

int mono_adapter, mon_is_colour, screen_mem_addr, does_flicker;

#ifndef USG
char *strchr( str, chr )
char *str;
char chr;
{
	extern char *index();
	return index(str,chr);
}
char *strrchr( str, chr )
char *str;
char chr;
{
	extern char *rindex();
	return rindex(str,chr);
}
#endif USG

#ifndef ATARI520ST
beep(){
	putchar( 7 );
}
#endif

#ifdef ATARI520ST
int
_rchar()
{
	return gem_getc();
}
#endif

#ifndef ECURSES

#include "keycurs.h"

#define _KEYPAD 0
#define ESC 033
curOn(){}
curOff(){}


keypad(win, bf)
WINDOW *win;
bool bf;
{
	if (bf) {
		win->_flags |= _KEYPAD;
		printf( "\033=");
		}
	else
		win->_flags &= ~_KEYPAD;
}


wgetch(win)
WINDOW	*win;
{
	int	c;

	c = _rchar();
	if (c != ESC)
		return c;
	/*
	 * Got an escape.  It might be the first char of a function key,
	 * or it might be the escape button.  No easy way to tell.
	 */
	c = _rchar();
	if (c == ESC) {
		/* User pressed escape twice - treat as one escape */
		return ESC;
	} else if (c == 'O') {
		/* VT100 and clones send \EOx in certain modes */
		c = _rchar();
		switch(c) {
		/* Ambassador, PF1-PF12 */
		case 'A': case 'B': case 'C': case 'D':
		case 'E': case 'F': case 'G': case 'H':
		case 'I': case 'J': case 'K': case 'L':
		/* Ambassador, Shift PF1-PF3 */
		case 'M': case 'N': case 'O':
			return KEY_F(c - 'A' + 1);

		/* VT100, PF1-PF4 */
		case 'P': case 'Q': case 'R': case 'S':
			return KEY_F(c - 'P' + 1);

		/* VC keypad specific */
		case 'l': case 'm': case 'o':
		case 'q': case 'r': case 's':
		case 't': case 'u': case 'v':
		case 'w': case 'x': case 'y':
			return KEY_F(c - 'l' + 5);

		/* For compatibility with Ambassador */
		case 'n': return KEY_DL;
		case 'p': return KEY_IL;

		default: return KEY_CLEAR;
		}
	} else if (c != '[') {
		/* Not a function key, return the escape */
		setla(c);
		return ESC;
	}
	c = _rchar();
	switch(c) {
	/* The following are true of many ANSI terminals */
	case 'A': return KEY_UP;
	case 'B': return KEY_DOWN;
	case 'C': return KEY_RIGHT;
	case 'D': return KEY_LEFT;
	case 'F': return KEY_END;
	case 'H': return KEY_HOME;
	case 'I': return KEY_PPAGE;
	case 'G': return KEY_NPAGE;
	case 'J': return KEY_CLEAR;
	case 'K': return KEY_CLEAR;
	case 'L': return KEY_IL;
	case 'M': case 'N': case 'O': case 'P': case 'Q':
	case 'R': case 'S': case 'T': case 'U': case 'V':
		return KEY_F( c - 'M' );
	case '@': return KEY_IL;
	case 'Z': return KEY_BTAB;
	default: return KEY_CLEAR;
	}
}

static int _lachar = 0;	/* lookahead char */

/* Read a character from the keyboard, checking for pushed input */
int
_rchar()
{
	int c;
	char reader;

	if (_lachar) {
		c = _lachar;
		_lachar = 0;
		return c;
	}

#ifdef unix
	if (read(0, &reader, 1) == 1)
		return reader;
	else
		return EOF;
#endif /*unix*/
#ifdef QNX
# ifdef ICON
	/* this loop eliminates the key released codes from the function
	   keys */
	do
		c = getbc();
	while( c >= 0xa0 && c <= 0xad );
	return c;
# else /* ibm qnx then */
	return fgetc( stdin );
# endif /*ICON*/
#endif QNX
#ifdef msdos
	/* dos function call #7, MWC compiler syntax */
	return dos(7<<8, 0, 0);
#endif msdos

#ifdef ATARI520ST
	return Cconin();
#endif

}

/* Set the lookahead char to c (e.g. push it back onto the input) */
setla(c)
int c;
{
	_lachar = c;
}

wattron( win, aton )
WINDOW *win;
int aton;
{
	wstandout( win );
}
wattroff( win, aton )
WINDOW *win;
int aton;
{
	wstandend( win );
}


#endif

/*
 *
 * Main for runtime process.  Uses message communication with editor process
 * to save space.
 *
 */
#define INTERPRETER 1


int is_cooked = FALSE;
extern WINDOW *pg_out;

go_cooked() {
}

go_raw() {
}

get_inp_char() {
#ifdef SAI	
	return getchar();
#else
	return keyget();
#endif
}

in_char_waiting() {
	return TRUE;
}

change_pg_out(win)
WINDOW *win;
{
	extern WINDOW *pg_out;

	pg_out = win;
}

inp(port)
unsigned port;
{
	printf( "input from port %x\n", port );
	return port & 0xff;
}
outp( port, val )
{
	printf( "output to port %x with %d\n", port, val );
}
getCDir(){}
selDisk(){}

mkdir(str)
char *str;
{
	return Dcreate( str );
}

rename( old, new )
char *old;
char *new;
{
	return Frename( 0, old, new );
}

rmdir( dir )
char *dir;
{
	return Ddelete( dir );
}

filelength()
{
	return 7200;
}

getFreeParas(){
	return 1000;
}

char *
mlstring()
{
	return "Mem x%";
}

meminit()
{}
#ifndef A_ATTRIBUTES
#define A_ATTRIBUTES  0x80
#endif

reswflags( win, newflgs )
WINDOW *win;
short newflgs;
{
	win->_flags &= ~A_ATTRIBUTES;
	win->_flags |= (newflgs & A_ATTRIBUTES );
}

curfilename(){}

int dir_current;

dofirst( pattern, attr )
char *pattern;
int attr;
{
	dir_current = FALSE;

	/* Do the FsFirst Call on the specified pattern */
	if( FsFirst( pattern, attr ) == E_OK )
		dir_current = TRUE;

}

int
findNext()
{
	return Fsnext() == E_OK;

}

comload(){}
getDisk(){ return 1; }
