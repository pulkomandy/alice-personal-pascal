/*
 * Routines that depend on Unix or the unix curses.  Loaded in for
 * running on unix with unix curses.  Other files for other systems
 */
#include "alice.h"
#include <curses.h>
#include <osbind.h>


int mono_adapter, mon_is_colour, screen_mem_addr, does_flicker;

extern char *index();
extern char *rindex();

char *
strchr( str, c )
char *str;
char c;
{
	return index( str, c );
}

char *
strrchr( str, c )
char *str;
char c;
{
	return rindex( str, c );
}

int rcharbuf = -1;

int
_rchar()
{
	int c;

	/*
	 * Return keycodes - anything greater than 128 return
	 * an escape (1BH) followed by the value - 128
	 */
	if( rcharbuf != -1 ) {
		c = rcharbuf;
		rcharbuf = -1;
		return c;
	}
	c = runkeyget();
	if( c > 127 ) {
		rcharbuf = c - 128;
		return 0x1b;
	}
	return c;
}

/*
 *
 * Main for runtime process.  Uses message communication with editor process
 * to save space.
 *
 */
#define INTERPRETER 1


int is_cooked = FALSE;

extern WINDOW *pg_out;

inp(port)
unsigned port;
{
	return port & 0xff;
}

outp( port, val )
{
}

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

long
filelength( fp )
FILE *fp;
{
	long pos;
	long len;

	pos = ftell( fp );
	fseek( fp, 0L, 2 );	/* seek to the end */
	len = ftell(fp);
	fseek( fp, pos, 0 );
	return len;
}

getFreeParas(){
	return 1000;
}


meminit()
{}
#ifndef A_ATTRIBUTES
#define A_ATTRIBUTES  0x80
#endif

#ifndef SAI
reswflags( win, newflgs )
WINDOW *win;
short newflgs;
{
	win->_flags &= ~A_ATTRIBUTES;
	win->_flags |= (newflgs & A_ATTRIBUTES );
}
#endif
