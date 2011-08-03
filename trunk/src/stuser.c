
#include "alice.h"
#include "interp.h"
#include <curses.h>
#include <obdefs.h>
#include <osbind.h>
#include <gemdefs.h>
#include "workspace.h"
#include "window.h"
#include "stwin.h"
#include "dbflags.h"
#include <ctype.h>
#include "alrsc.h"

extern WINDOW *pg_out;
extern struct pas_file fil_input, fil_output;

/*
 * This file contains the builtins for the user's GEM interface
 */


mkuserwin( whfile, x, y, w, h, features )
fileptr whfile;
int x, y, w, h;
int features;
{
	int flags;
	int slot;
	WINDOW *win;
	char *the_title;

	flags = FIL_WRITE | FIL_READ | FIL_OPEN | FIL_TEXT | FIL_EOF 
		| FIL_SCREEN | FIL_NREAD | FIL_LAZY | FIL_KEYBOARD;

	if( whfile == &fil_output || whfile == &fil_input ) {
		stclose( whfile );
	}

	/*
	 * Set the buffer size to 1
	 */
	whfile->f_size = 1;

	/*
	 * Mark the file var as defined
	 */
	do_undef( whfile, U_SET, sizeof( struct pas_file ) - 1 );

	/*
	 * Get a slot in the open array for the text file
	 */
	set_descrip( whfile, "", "", flags );

	/*
	 * mark the buffer as undefined
	 */
	do_undef( &(whfile->f_buf), U_CLEAR, whfile->f_size );

	slot = FindSlot();

	if( slot == -1 ) 
		run_error( ER(339,"manywindow`Too many windows open") );

	if( features & MOVER ) {
		features |= NAME;
	}

	if( whfile->f_name )
		the_title = whfile->f_name;
	else
		the_title = LDS( 370, " Output " );

	if( MkWindow( slot, the_title, features, x, y, w, h,
			TRUE ) )
		run_error( ER(344,"newwindow`Error while creating window") );

	whfile->desc.f_window = win = AW_WIN(slot);
	AW_KIND(slot) = WIN_OUTPUT;

	if( features & INFO ) {
		AW_INFO(slot) = (char *)checkalloc( AW_TLEN(slot) );
		AW_INFO(slot)[0] = 0;
	}

	AW_FILE(slot) = whfile;

	if( whfile == &fil_output || whfile == &fil_input ) {
		pg_out = win;
		fil_output.desc.f_window = pg_out;
		fil_input.desc.f_window = pg_out;
	}

	touchwin( win );
	wrefresh( win );
	flush_events();
}

/*
 * Create the user's window
 */

do_gcreate( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;
	int flags;
	int x, y, w, h;
	int features;
	int slot;
	WINDOW *win;

	h = int_spop();
	w = int_spop();
	y = int_spop();
	x = int_spop();

	WinMap( &x, &y );
	WinMap( &w, &h );

	features = int_spop();

	whfile = p_spop();

	mkuserwin( whfile, x, y, w, h, features );

}

#define PREDEF_WIN	9

int quickwin[PREDEF_WIN][4] = {
	1000, 2000, 30000, 30000,
	1000, 2000, 30000, 15000,
	1000, 15000, 30000, 15000,
	1000, 2000, 15000, 30000,
	15000, 2000, 15000, 30000,
	1000, 2000, 15000, 15000,
	15000, 2000, 15000, 15000,
	1000, 15000, 15000, 15000,
	15000, 15000, 15000, 15000,
	};

do_quickwin( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;
	int flags;
	int x, y, w, h;
	int features;
	int slot;
	WINDOW *win;
	int num;
	int sys;
	extern int _currSystem;

	/*
	 * Get the number of the window
	 */
	num = int_spop();

	if( num < 0 || num >= PREDEF_WIN )
		run_error( ER(350, "quickwindow`Illegal QuickWindow number") );

	x = quickwin[num][0];
	y = quickwin[num][1];
	w = quickwin[num][2];
	h = quickwin[num][3];

	sys = _currSystem;
	_currSystem = CO_STRETCH;
	
	WinMap( &x, &y );
	WinMap( &w, &h );

	_currSystem = sys;

	features = int_spop();

	whfile = p_spop();

	mkuserwin( whfile, x, y, w, h, features );
}

/*
 * Close the window
 */
do_gwinclose( win )
WINDOW *win;
{
	int slot;

	slot = WinSlot( win );

	if( slot != -1 ) 
		FreeSlot( slot );

	flush_events();

}

win_title( whfile, str )
fileptr whfile;
char *str;
{
	int slot;

	/*
	 * Set the title for window pointed to by the text file
	 * called by givename (which is called by Assign)
	 */
	slot = WinSlot( whfile->desc.f_window );

	if( slot == -1 ) return;

	if( AW_TITLE(slot) ) {
		strncpy( AW_TITLE(slot), str, AW_TLEN(slot));
		wind_set( AW_HANDLE(slot), WF_NAME, AW_TITLE(slot) );
	}
}

extern WINDOW *_currGraph;

do_SetGraph( argc, argv )
int argc;
fileptr *argv;
{
	fileptr whfile;
	whfile = p_spop();
	scr_check( whfile );
	_currGraph = whfile->desc.f_window;
}

scr_check( whfile )
fileptr whfile;
{
	if( whfile && (whfile->f_flags & FIL_SCREEN) && whfile->desc.f_window )
		return TRUE;

	run_error(ER(348,"notwindow`File variable does not refer to a window"));
}


do_gwinopt( argc, argv )
int argc;
rint *argv;
{
	int handle;
	WINDOW *win;
	extern WINDOW *pg_out;
	GRECT r;
	int slot;
	char title[100];

	int features = int_spop();
	fileptr whfile = p_spop();

	extern struct pas_file fil_input, fil_output;

	scr_check( whfile );

	win = whfile->desc.f_window;
	slot = WinSlot( win );

	handle = AW_HANDLE(slot);

	wind_get( handle, WF_CURRXYWH, &r.g_x, &r.g_y, &r.g_w, &r.g_h );

	if( AW_TITLE(slot) ) {
		strcpy( title, AW_TITLE(slot) );
	} else {
		strcpy( title, LDS( 370, " Output " ));
	}

	/* Free the current window */
	FreeSlot( slot );

	if( features & MOVER )
		features |= NAME;

	if( MkWindow( slot, title, features, r.g_x, r.g_y, r.g_w, r.g_h ) )
		run_error( ER(345,"nochange`Couldn't change window features") );

	if( win == pg_out ) {
		pg_out = AW_WIN( slot );
		fil_output.desc.f_window = pg_out;
		fil_input.desc.f_window = pg_out;
	}

	whfile->desc.f_window = AW_WIN(slot);

	AW_FEATURES(slot) = features;

}

do_gtopwin( argc, argv )
int argc;
rint *argv;
{
	fileptr whfile;
	int slot;

	whfile = p_spop();

	scr_check( whfile );

	slot = WinSlot( whfile->desc.f_window );

	wind_set( AW_HANDLE(slot), WF_TOP, 0, 0, 0, 0 );

	flush_events();

}

do_filesel( argc, argv )
int argc;
pointer *argv;
{
	/*
	 * Give the user a file-selection box
	 */
	unsigned char *filename;
	nodep filetype;
	unsigned char *directory;
	nodep dirtype;
	char *pattern;
	nodep pattype;
	char the_filename[13];
	char the_pattern[80];
	int len;
	int i;
	int exit_button;
	int mstate;
	extern int MenuState;

	extern char *strchr();

	filename = p_spop() + STR_START;
	filetype = p_spop();
	checksvar( filetype, LDS( 467, "Filename" ));

	directory = p_spop() + STR_START;
	dirtype = p_spop();

	checksvar( dirtype, LDS( 466, "Directory" ));

	pattern = p_spop() + STR_START;

	/* If there is a slash or a colon in the pattern, put the default
	 * pathname in front of it
	 */
	if( strchr( pattern, '\\' ) == (char *)0 &&
	    strchr( pattern, ':' ) == (char *)0 ) {
		the_pattern[0] = Dgetdrv() + 'A';
		the_pattern[1] = ':';
		Dgetpath( &the_pattern[2], 0 );
		if( strlen( the_pattern ) > 3 )
			strcat( the_pattern, "\\" );
		else
			the_pattern[2] = 0;
		strcat( the_pattern, pattern );
	} else {
		strncpy( the_pattern, pattern, 80 );
	}

	for( i=0; i<strlen(the_pattern); i++ ) {
		if( islower( the_pattern[i] ) )
			the_pattern[i] = _toupper( the_pattern[i] );
	}

	if( strlen( filename ) > 12 ) {
		run_error( ER(332,"deftoolong`Default filename too long") );
	}
	strcpy( the_filename, filename );

	mstate = MenuCtrl( TRUE );

	fsel_input( the_pattern, the_filename, &exit_button );

	MenuCtrl( mstate );

	if( exit_button == 0 ) {
		/* Cancel */
		filename[0] = 0;
		filename[-1] = 0;
		do_undef( filename - 1, U_SET, 2 );
		directory[0] = 0;
		directory[-1] = 0;
		do_undef( directory - 1, U_SET, 2 );

	} else {
		len = strlen( the_filename );
		if( len > get_stsize( filetype ) )
			run_error( ER( 356, 
			"catlong`Filename variable not big enough"));
		strcpy( filename, the_filename );
		filename[-1] = len;
		do_undef( filename-1, U_SET, len + 1 + STR_START );
		/* Now handle the directory */
		i = strlen( the_pattern ) - 1;
		while( (i>=0) && the_pattern[i] != '\\' && the_pattern[i] != ':' )
			i--;

		if( i >= 0 )
			the_pattern[i+1] = 0;

		len = strlen( the_pattern );
		if( len > get_stsize( dirtype )  )
			run_error( ER(357,
			"catlong`Directory string not big enough" ));

		strcpy( directory, the_pattern );
		directory[-1] = len;
		do_undef( directory-1, U_SET, len + 1 + STR_START );
	}

	/* Handle the refresh after the fsel_input */
	flush_events();

}

do_getprompt( argc, argv )
int argc;
pointer *argv;
{
	unsigned char *answer;
	nodep ans_type;
	nodep prmpt_type;
	char *prompt;
	char temp[100];
	int len;
	int mstate;
	extern int MenuState;

	answer = p_spop() + STR_START;
	ans_type = (nodep) p_spop();

	checksvar( ans_type, LDS( 377, "Answer string" ));
	
	prompt = str_spop() + STR_START;

	mstate = MenuState;
	MenuCtrl( TRUE );

	GemCommand( prompt, temp );

	MenuCtrl( mstate );
	

	len = strlen( temp );

	if( len > get_stsize( ans_type ) )
		run_error( ER(358, "catlong`Answer variable not big enough" ));

	strcpy( answer, temp );
	answer[-1] = len;

	do_undef( answer-1, U_SET, len + 1 + STR_START );

}

do_setinfo( argc, argv )
int argc;
rint *argv;
{

	char *str;
	fileptr whfile;
	int slot;

	str = str_spop() + STR_START;

	whfile = p_spop();

	scr_check( whfile );

	slot = WinSlot( whfile->desc.f_window );

	if( (AW_FEATURES( slot ) & INFO) && AW_INFO(slot) ) {
		strncpy( AW_INFO( slot ), str, AW_TLEN( slot ) );
		wind_set( AW_HANDLE( slot ), WF_INFO, AW_INFO(slot), 0, 0, 0, 0 );
	} else {
		run_error( ER(353,
		"noinfline`There is no information line in this window" ));
	}
}

do_setslider( argc, argv )
int argc;
rint *argv;
{

	int size = int_spop();
	int pos = int_spop();
	int flag = int_spop();
	fileptr whfile = p_spop();
	int slot, handle;
	int features;

	scr_check( whfile );

	slot = WinSlot( whfile->desc.f_window );

	features = AW_FEATURES(slot);
	handle = AW_HANDLE(slot);

	if( flag ) {
		if( (features & VSLIDE) == 0 )
			run_error( ER(354,
	"noslide`Window does not have a vertical slider" ));

		wind_set( handle, WF_VSLIDE, pos, 0, 0, 0 );
		wind_set( handle, WF_VSLSIZE, size, 0, 0, 0 );

	} else {
		if( (features & HSLIDE) == 0 )
			run_error( ER(354,
		 "noslide`Window does not have a horizontal slider"));

		wind_set( handle, WF_HSLIDE, pos, 0, 0, 0 );
		wind_set( handle, WF_HSLSIZE, size, 0, 0, 0 );
	}
}

do_movewin()
{

	int y = int_spop();
	int x = int_spop();
	fileptr whfile = p_spop();
	int w, h;
	int dummy;
	int slot;

	WinMap( &x, &y );

	scr_check( whfile );

	slot = WinSlot( whfile->desc.f_window );

	wind_get( AW_HANDLE(slot), WF_CURRXYWH, &dummy, &dummy, &w, &h );

	win_align( AW_FEATURES(slot), &x, &y, &w, &h );

	wind_set( AW_HANDLE(slot), WF_CURRXYWH, x, y, w, h );

	SetWork( slot );

	flush_events();
}

do_resizewin()
{
	int h = int_spop();
	int w = int_spop();
	int dummy;
	int slot;
	GRECT r;
	GRECT s;

	fileptr whfile;
	WinMap( &w, &h );

	whfile = p_spop();
	scr_check( whfile );

	slot = WinSlot( whfile->desc.f_window );
	wind_get( AW_HANDLE(slot),
		 WF_WORKXYWH, &r.g_x, &r.g_y, &dummy, &dummy );

	r.g_w = w;
	r.g_h = h;

	/*
	 * Now convert the WORK size into a CURRXYWH sort of size
	 */
	wind_calc( 0, AW_FEATURES( slot ), 
                      r.g_x,  r.g_y,  r.g_w,  r.g_h,
		     &s.g_x, &s.g_y, &s.g_w, &s.g_h );

	SizeWin( slot, &s );

	flush_events();
}

do_mousetype()
{
	int t = int_spop();

	if( t < 8 && t >= 0 ) 
		graf_mouse( t, 0L );
	else
		run_error( ER( 342, "mousetype`Illegal mouse type number" ));

}

do_wheremouse()
{
	int *buttons = p_spop();
	int *abs_y = p_spop();
	int *abs_x = p_spop();
	int *y = p_spop();
	int *x = p_spop();
	fileptr *whfile = p_spop();
	int mx, my, mm, mb;
	int handle, slot;
	GRECT rect;

	/*
	 * Get the location of the mouse
	 */
	graf_mkstate( &mx, &my, &mb, &mm );

	*abs_x = mx;
	*abs_y = my;

	/* Map absolute co-ordinates to current co-ordinate system */
	MapWin( abs_x, abs_y );

	handle = wind_find( mx, my );

	slot = HndlSlot( handle );

	/* If it wasn't on one of the output windows, set the pointer to nil */
	if( slot == -1 || AW_KIND(slot) == WIN_EDIT ) {
		*whfile = 0;
		*x = -1;
		*y = -1;
	} else {
		*whfile = AW_FILE(slot);
		GetWork( AW_WIN(slot), &rect );
		if( inside( mx, my, &rect ) ) {
			mx -= rect.g_x;
			my -= rect.g_y;
			*x = mx;
			*y = my;
			/* Map to current co-ordinate system */
			MapCo( slot, x, y );
		} else {
			*x = -1;
			*y = -1;
		}
	}
	*buttons = mb;
	do_undef( whfile, U_SET, sizeof( pointer ) );
	do_undef( x, U_SET, sizeof( rint ) );
	do_undef( y, U_SET, sizeof( rint ) );
	do_undef( abs_x, U_SET, sizeof( rint ) );
	do_undef( abs_y, U_SET, sizeof( rint ) );
	do_undef( buttons, U_SET, sizeof( rint ) );

}

do_gtopptr( argc, argv )
int argc;
fileptr *argv;
{
	int handle, dummy;
	int slot;
	fileptr whfile;

	wind_get( 0, WF_TOP, &handle, &dummy, &dummy, &dummy );

	slot = HndlSlot( handle );

	whfile = 0L;

	if( slot >= 0 && (AW_KIND(slot) == WIN_OUTPUT) )
		whfile = AW_FILE( slot );

	*argv = whfile;
}

do_dosound()
{
	char *ptr = p_spop();

	if( ptr )
		Dosound( ptr );

}

#ifndef SAI
do_cdir( str )
char *str;
{
	char the_dir[80];
	char the_filename[15];
	int  exit_button;
	char *path;
	char c;
	int i;

	if( str ) {
		/*
		 * Process the args...
		 */
		strcpy( the_dir, str );
		strcat( the_dir, "\\" );
	} else {
		/*
		 * Do a fsel_input sort of thing
		 */
		the_dir[0] = Dgetdrv() + 'A';
		the_dir[1] = ':';
		Dgetpath( &the_dir[2], 0 );
		if( strlen( the_dir ) > 3 )
			strcat( the_dir, "\\" );
		else
			the_dir[2] = 0;

		fsel_input( the_dir, the_filename, &exit_button );

		if( exit_button == 0 ) return;

	}
	/*
	 * Now 'CD' to the directory in 'the_dir'
	 */
	if( the_dir[1] == ':' ) {
		/*
		 * Change default drive
		 */
		c = the_dir[0];
		if( isupper( c ) )
			c = tolower( c );
		Dsetdrv( c - 'a' );
		path = &the_dir[2];
	}
	 else
		path = the_dir;

	/*
	 * Now strip the directory of any pattern
	 */
	i = strlen( path ) - 1;
	while( (i>=0) && path[i] != '\\' )
		i--;

	if( i >= 0 )
		path[i] = 0;

	if( Dsetpath( path ) )
		form_alert( 1, LDS( 390, 
		"[2][Could not change directory][ OK ]" ));

}

IsSuspended( count )
int count;
{

	int flag;
	extern long gl_menu;

	flag = count > 0;

	menu_ienable( gl_menu, RMCONTIN, flag );
	menu_ienable( gl_menu, DMWHOCAL, flag );
	menu_ienable( gl_menu, DMPOPSUS, flag );

}
#endif SAI

long
FindBitMap( the_file )
char *the_file;
{
	int i;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( AW_USED(i) && (AW_FILE(i) == the_file) )
			return AW_BITS(i);
	}
	return 0L;
}
