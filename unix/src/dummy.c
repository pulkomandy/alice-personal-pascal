/*
 *DESC: Some dummy definitions for versions missing graphics tools and other
 *DESC: runtime tools
 */

typedef long WINDOW;
#define ERR (-1)
#include <io.h>
#include <fsys.h>
#ifdef NOT
do_draw() {}
do_plot() {}
do_window() {}
do_grmode() {}
do_textmode() {}
do_colour() {}

initGraphics() {}
resetGraphics() {}
finishGraphics() {}
suspendGraphics() {}
resumeGraphics() {}
checkGraphics() {}
viewGraphics() {}
checkScreen() {}
char RunModeInfo;
pg_outChanged(){}
okayWithGraphics(){}
do_vid(){}
char ModesInfo;
change_pg_out(win)
WINDOW *win;
{
	extern WINDOW *pg_out;

	pg_out = win;
}
#endif

inp(port)
unsigned port;
{
	return io_in(port);
}
outp( port, val )
{
	io_out( port, val );
}
mkdir(str)
char *str;
{
	FILE *dirfile;
	char buf[5*48];

	zero( buf, sizeof(buf) );
	if( dirfile = fopen( str, "wdn" ) ) {
		fput( buf, sizeof(buf), dirfile );
		fclose( dirfile );
		return 0;
		}
	 else
		return ERR;
}
rename(oldname, newname)
char *oldname;
char *newname;
{
	FILE *oldnfp;
	FILE *newexist;
	struct dir_entry buf;
	int ret;


	if( strchr( newname, '/' ) || strlen( newname ) > 16 )
		return ERR;
	if( oldnfp = fopen( oldname, "mo" ) ) {
		if( newexist = fopen( newname, "ro" ) ) {
			fclose( newexist );
			return ERR;
			}
		ret = get_dir_entry( oldnfp, &buf );
		strcpy( buf.fname, newname );
		ret |= set_dir_entry( oldnfp, &buf );
		fclose( oldnfp );
		return ret;
		}
	 else
		return ERR;
}
rmdir(dirname)
char *dirname;
{
	char shbuf[100];

	sprintf( shbuf, "drel %s", dirname );
	return shell( shbuf );
}

unlink(fname)
char *fname;
{
	FILE *thefile;

	if( thefile = fopen( fname, "w" ) ) {
		/* zero size deletes file */
		fclose( thefile );
		return 0;
		}
	 else
		return ERR;
}
#ifndef A_ATTRIBUTES
#define A_ATTRIBUTES  0x80
#endif

char *
strrchr( str, chr )
char *str;
char chr;
{
	char *ret;
	register char *stp = str;

	ret = 0;
	while( *stp ) {
		if( *stp == chr )
			ret = stp;
		++stp;
		}
	return ret;
}
clear_screen()
{
	extern char *stdscr;
	werase(stdscr);
	wrefresh(stdscr);
}
viewGraphics(){}

int mon_is_colour = 0;

tdelay( hundredths )
unsigned int hundredths;
{
	unsigned get_ticks();
	unsigned start;
	/* assumes clock tick is exactly 50 msec.  Doubt it is */

	set_timer( 0 /* wakeup */, 1 /* relative */, hundredths/5, 0,0,0);
}

#include <speech.h>

int sndtab[] = {
500, 471, 444, 421, 400, 381, 364, 348, 333, 320,
308, 296, 286, 276, 267, 258, 250, 242, 235, 229,
222, 216, 211, 206, 200, 195, 190, 186, 178, 170,
163, 156, 151, 148, 140, 136, 131, 127, 121, 116,
113, 110, 104, 101,  99,  94,  92,  87,  84,  81,
78, 75, 73, 70, 67, 65, 63, 60, 58, 56,
54, 52, 50, 50
};

static unsigned char frame[] = { 8, 1, 24, 8, 10, 5, 5, 4, 4, 3, 2, 1 };

static int any_speech = 0;
soundon( hz )
int hz;
{

	if( !any_speech ) {
		if( init_speech() || flush_speech(SPEECH_ADMIN) == SPEECH_BUSY )
/*			run_error( ER(332,"speech`Sound generating task is not available") ); */
			run_error( 332 );
		speech_timeout_period( 600 );
		/* 30 second timeout in case they don't turn it off */
		}
	if( hz == 0 ) {
		silence();
		}
	 else {
		int code;
		if( hz >= 500 ) {
			code = 0;
			}
		 else if ( hz <= 50 )
			code = 63;
		 else for( code = 0; code < 63; code++ ) {
			if( sndtab[code] < hz ) {
				if( hz - sndtab[code] > sndtab[code-1] - hz )
					code--;
				break;
				}
			}
		code++;
		frame[1] = code;
		put_frame( frame, SPEECH_ADMIN );
		flush_speech( SPEECH_ADMIN );
		}
}

