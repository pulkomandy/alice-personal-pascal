
#include "alice.h"
#include <ctype.h>
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "dbflags.h"
#include <gemdefs.h>
#include <obdefs.h>
#include "stwin.h"
#include "interp.h"

extern int BufferGraphics;
extern long gl_menu;
#include "alrsc.h"

extern struct pas_file fil_input, fil_output;

#ifdef DB_WIN
#define PRINTT
#endif
#include "printt.h"

#ifndef OLMSG
char no_mem_alert[] = "[2][Error ! - Not enough|memory to run ALICE][ OK ]";
#endif

WINDOW *pg_out;		/* Where to put program output */

int main_slot;

extern int gl_rmsg[8];
extern char *checkalloc();

int windows_made = FALSE;
extern int charHeight, charWidth;
extern int gl_xfull, gl_yfull, gl_hfull, gl_wfull;

int Current_win = -1;	/* The slot of the currently active edit window */

STWINDOW WinInfo[MAXWINDOWS];

#ifdef DEBUG
DumpInfo()
{
	int i;

	if( !trace ) return;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( !AW_USED(i) ) continue;
		fprintf( trace, "Slot %d:\n", i );
		fprintf( trace, "Handle: %d, Win: %lx, Kind: %d\n",
				AW_HANDLE(i), AW_WIN(i),
				AW_KIND(i) );
		fprintf( trace, "NumLines: %d, AliceWin: %lx, Title: %s\n",
			WinInfo[i].num_lines, AW_ALWIN(i),
			AW_TITLE(i) );
		if( AW_ALWIN(i) ) {
			fprintf( trace, "Alwin->wspace: %lx\n",
				 WinInfo[i].alwin->wspace);
			fprintf( trace, "Alwin->start_cursor: %lx\n",
				WinInfo[i].alwin->start_cursor );
		}
		fprintf( trace, "BitImage: %lx\n", WinInfo[i].bit_image );
	
	}
}
#endif
		

/*
 * In the ST version, we are always ready to run
 */
prep_to_run()
{
}

prep_r_window()
{
}

del_r_window()
{
}

refresh_scr()
{
	touchwin( curr_window->w_desc );
	wrefresh( curr_window->w_desc );
}

redrw_screen( recreate )
int recreate;
{
#ifndef SAI

	refresh_scr();
	show_whole_doda();
#endif SAI
}

/*
 * Align to a character position along the x co-ord
 */
align_w( x )
int x;
{
	if( x % charWidth == 0 ) return x;
	return (x & ~(charWidth-1)) + charWidth;
}

/*
 * Given a gem window, get the work area, and then calculate the
 * appropriate size of a curses window, then allocate it
 */
WINDOW *
MkCurWin( handle )
int handle;
{
	int	x, y, w, h;
	WINDOW  *win;

	wind_get( handle, WF_WORKXYWH, &x, &y, &w, &h );

	printt5( "Work area of window %d: %d, %d, %d, %d\n",
		handle, x, y, w, h );

	printt4( "Making a curses window with %d, %d, %d, %d\n",
		x, y, w, h );

	printt4( "In chars: %d, %d, %d, %d\n",
		h / charHeight, w / charWidth,
		y / charHeight, x / charWidth );

	win = newwin( h / charHeight, w / charWidth, 0, 0 );
	return win;
}

#ifndef SAI
initWindows()
{
	int i;

	/*
	 * Prepare two windows for GEM Alice - the initial editing window
	 * and the output window
	 */

	/* Get size of the full screen version of the window */
	wind_get( 0, WF_FULLXYWH, &gl_xfull, &gl_yfull, &gl_wfull, &gl_hfull);

	/*
	 * Make the Main GEM window
	 */

	for( i=0; i<MAXWINDOWS; i++ ) {
		InitSlot(i);
	}

	printt4( "Full size = %d, %d, %d, %d\n",
		gl_xfull, gl_yfull, gl_wfull, gl_hfull );


	/*
 	 * Create the output and edit windows
 	 */
	if( MkOutput() ||
	    MkMain() ) {
		form_alert( 1, LDS( 391, no_mem_alert ));
		done( (char *)0 );
	}

	/*
	 * Say that they were made okay
	 */
	windows_made = TRUE;

	scrollok( AW_WIN(main_slot), TRUE );
	crmode();
	noecho();

	curOn( AW_WIN(main_slot) );

	printt2( "Initial workspace: %lx, initial curr_window: %lx\n",
		curr_workspace, curr_window );

	/*
	 * Set the slider size to 1000
	 */
	wind_set( AW_HANDLE(main_slot), WF_VSLSIZE, 1000 );


#ifdef DEBUG
	printt0( "Dumping initial wininfo struct\n" );
	DumpInfo();
#endif
}
#else SAI

initWindows()
{
	int i;

	/* Get size of the full screen version of the window */
	wind_get( 0, WF_FULLXYWH, &gl_xfull, &gl_yfull, &gl_wfull, &gl_hfull);

	for( i=0; i<MAXWINDOWS; i++ ) {
		InitSlot( i );
	}
	MkOutput();
}
#endif SAI

#ifndef SAI
/*
 * Activate the workspace indicated by the slot number
 */
ActWork( ws, saveold )
int ws;		/* The slot number */
int saveold;
{
	workspace *tospace;
	extern workspace *curr_workspace;
	int debug = 0;

	printt0( "Dump of window slots\n" );
#ifdef DEBUG
	DumpInfo();
#endif

	printt1( "Making slot %d active\n", ws );

	/*
	 * If this is already the active workspace, then just top the
	 * window
	 */
	if( ws == Current_win ) {
		wind_set( AW_HANDLE( Current_win ), WF_TOP, 0, 0, 0, 0 );
		return;
	}

	/*
	 * Make an edit window the active one
	 */
	if( AW_ALWIN(ws) == (alice_window *)0 ) return;

	printt1( "Activating edit window in slot %d\n", ws );

	if( saveold ) {
		printt2( "Saving old window: cursor: %lx, start: %lx\n",
			(long)cursor, (long)(curr_window->start_cursor) );
		work_cursor( curr_workspace ) = cursor;
		work_scursor( curr_workspace ) = curr_window->start_cursor;
	}

	Current_win = ws;
	curWindows[MAINWIN] = AW_WIN(ws);
	curr_window = AW_ALWIN(ws);

	/*
	 * Top the window
	 */
	wind_set( AW_HANDLE(ws), WF_TOP, 0, 0, 0, 0 );

	/*
	 * Workspace is in curr_window->wspace
	 */
	tospace = curr_window->wspace;

	printt1( "Workspace = %lx\n", tospace );

	/* set the current workspace */
	curr_workspace = tospace;

	printt1( "Setting cursor to %lx\n", work_cursor( tospace ));

	cursor = work_cursor(tospace); 
	curr_window->start_cursor = work_scursor(tospace);
	curr_window->start_pcode = 0;
	curr_window->start_sline = 0;
	curr_window->big_change = TRUE;

	zap_history();

	SetCurWinTitle();

}

set_wscur( alwin, newtop )
alice_window *alwin;
nodep newtop;
{
	alwin->start_cursor = newtop;
	alwin->start_sline = 0;
	alwin->start_pcode = kcget( -1, newtop );
}

/*
 * Set top of screen to a given line
 */

set_sc_fline( win, line )
alice_window *win;
struct scr_line *line;
{
	win->start_cursor = line_headnode( *line );
	win->start_pcode = line->pcode;
	win->start_sline = line->sub_line;
}


show_whole_doda()
{
	extern int cantredraw;
	extern int error_present;

	if( cantredraw )
		return;
	statusLine();

	display(TRUE);
}

/*
 * setup the lines table for a window
 */

setwlines( xalwin, nlines )
alice_window *xalwin;
int nlines;
{
	register int lin;
	register alice_window *alwin = xalwin;

	alwin->w_lines = (struct scr_line *)checkalloc(nlines *
			    sizeof(struct scr_line) );	
	alwin->big_change = TRUE;
	alwin->w_height = nlines;
	for( lin = 0; lin < nlines; lin++ ) {
		(alwin->w_lines[lin]).sc_size = (bits8) 0;
		(alwin->w_lines[lin]).listptr = NIL;
	}
}
#endif SAI

/*
 * Given a GEM window handle, return the slot number
 */
int
HndlSlot( handle )
int handle;
{
	int i;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( WinInfo[i].used && WinInfo[i].handle == handle ) return i;
	}
	return -1;
}

/*
 * Given a curses window pointer, return the slot number
 */
int
WinSlot( win )
WINDOW *win;
{
	int i;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( WinInfo[i].used && WinInfo[i].win == win ) return i;
	}

	return -1;
}

/*
 * GemTop - Top the window desired
 */

do_gtop()
{
	int slot;

	slot = HndlSlot( gl_rmsg[3] );
	/*
	 * If we can't find it, then something is wrong... go and delete
	 * the spurious window (must be from a confused GEM)
	 */
	if( slot == -1 ) {
		wind_close( gl_rmsg[3] );
		wind_delete( gl_rmsg[3] );
		return;
	}

	printt1( "Making window in slot %d active\n", slot );

	/*
	 * If it is an edit window, change the current workspace
	 */
	if( AW_KIND( slot ) == WIN_EDIT ) {
#ifndef SAI
		ActWork( slot, TRUE );
#endif SAI

		/* Otherwise, just top it */
	} else {
		wind_set( AW_HANDLE( slot ), WF_TOP, 0, 0, 0, 0 );
	}

}

WINDOW *
CurWin( h )
int h;
{
	int slot;

	slot = HndlSlot( h );
	if( slot == -1 )
		return (WINDOW *)0;
	else
		return AW_WIN( slot );
}

/*
 * Return the GEM handle associated with a curses window
 */

int
GemWin( win )
WINDOW *win;
{
	int slot;

	slot = WinSlot( win );
	if( slot == -1 )
		return -1;
	else
		return AW_HANDLE( slot );
}

/*
 * Redraw the window and area that was specified in the message
 * buffer
 */

do_gredraw()
{
	WINDOW *win;
	extern int CurClip;
	extern GRECT CurCBox;

	/*
	 * Redraw the window that was specified in gl_rmsg[3]
	 */
	win = CurWin( gl_rmsg[3] );
	if( !win ) return;

	/*
	 * Indicate to CURSES that we are clipping to a particular
	 * rectangle
	 * (As opposed to wrefresh when ALICE changes something in a
	 * window).
	 * If we are updating the output window, update all of it, not
	 * just the portion requested.  This is so that if he has graphics
	 * but hasn't buffered it, then ALL the graphics will be erased, rather
	 * than just the area updated.  I don't like this, but brad wanted
	 * it that way.
	 */
	touchwin( win );
	wrefresh( win );
}

/*
 * Handle the user moving around one of the windows
 */
do_gmoved()
{
	int i;
	int x, y, w, h;

	printt1( "Moving window %d\n", gl_rmsg[3] );

	gl_rmsg[4] = align_w( gl_rmsg[4] );

	wind_set( gl_rmsg[3], WF_CURRXYWH, gl_rmsg[4] - 1,
		gl_rmsg[5], gl_rmsg[6], gl_rmsg[7] );

	i = HndlSlot( gl_rmsg[3] );
	if( i != -1 )
		SetWork( i );

}

/*
 * Retrieve the work area rectangle for a given curses window
 */
GetWork( win, r )
WINDOW *win;
GRECT *r;
{
	int i;

	i = WinSlot( win );
	if( i != -1 )
		rc_copy( &AW_WORK(i), r );
}

/*
 * Handle the user re-sizing a window
 */
do_gsized()
{
	int handle;
	int i;

	/* Resize the window specified in gl_rmsg[3] */

	handle = gl_rmsg[3];

	i = HndlSlot( handle );
	if( i != -1 )
		SizeWin( i, &gl_rmsg[4] );
}

/*
 * Resize the window to the values held in the GRECT 'r'
 */

SizeWin( slot, r )
int slot;
GRECT *r;
{
	WINDOW *win;
	MFDB *bits, *old_bits;
	int o_wh, o_ww;
	int n_wh, n_ww;
	int dummy;
	int x, y, w, h;
	int old_x, old_y, old_w, old_h;
	int retval;

	/*
	 * Resize the curses window
	 */
	x = r->g_x;
	y = r->g_y;
	w = r->g_w;
	h = r->g_h;

	w = max( charWidth * 12, w );
	h = max( charHeight * 5, h );

#ifndef SAI
	if( AW_KIND(slot) == WIN_EDIT ) {
		w = max( charWidth * 22, w );
		h = max( charHeight * 6, h );
	}
#endif SAI

	/*
	 * Get the old values
	 */
	wind_get( AW_HANDLE(slot), WF_WORKXYWH, &dummy, &dummy,
			&o_ww, &o_wh );

	wind_get( AW_HANDLE(slot), WF_CURRXYWH, &old_x, &old_y,
			&old_w, &old_h );

	win = AW_WIN(slot);

	/*
	 * Now resize the GEM window to the appropriate size
	 */
	win_align( AW_FEATURES(slot), &x, &y, &w, &h );

	wind_set( AW_HANDLE(slot), WF_CURRXYWH, x, y, w, h );

	wind_get( AW_HANDLE(slot), WF_WORKXYWH, &dummy, &dummy,
			&n_ww, &n_wh );

	SetWork( slot );

	/*
	 * Try and resize the window
	 */
	retval = wresize( win, n_ww / charWidth, n_wh / charHeight );

	/*
	 * But wait ! We couldn't allocate the new window
	 * Go back to the old size
	 */

	if( retval ) {
		/*
		 * What should we do ? We can't resize his window !
		 */
		wind_set( AW_HANDLE(slot),WF_CURRXYWH,old_x,old_y,old_w,old_h);
		SetWork( slot );
		form_alert( 1, LDS( 392,
	"[2][Not enough memory|to resize window][ OK ]"));
		return;
	}

	/*
	 * We have resized the curses window, now do the rest
	 */

	switch( AW_KIND(slot) ) {

#ifndef SAI
		case WIN_EDIT:
			/*
			 * Free up the old line struct
			 */
			mfree( curr_window->w_lines );
			curr_window->w_width = win->_maxx;
			setwlines( curr_window, win->_maxy );
			/*
		  	 * Go and redraw the screen
			 */
			display( TRUE );

			break;
#endif SAI

		case WIN_OUTPUT:
			/*
			 * If the window is graphics buffered
			 * allocate the new one
			 */
			if( !BufferGraphics ) break;

			old_bits = AW_BITS(slot);
			if( old_bits == (MFDB *)0 ) break;

			/*
			 * Try to allocate the new buffer
			 */
			bits = AW_BITS(slot) = MkImage( n_ww, n_wh );

			/*
			 * Couldn't allocate the new graphics buffer
			 */
			if( bits == 0L ) {
				form_alert( 1, LDS( 393,
   "[2][Caution: Not enough memory|to buffer new graphics|window][ OK ]" ));
			} else {
				copy_gwin( old_bits, bits, o_ww, o_wh, n_ww,
							n_wh );
			}
			FreeImage( old_bits );
			break;

	}
	/*
	 * Tell people to redraw the area affected
	 */
	form_dial( 3, 0, 0, 0, 0, r->g_x, r->g_y, r->g_w, r->g_h );
}


/*
 * Copy the appropriate portion of the old bitmap to the new
 */
copy_gwin( old, new, old_w, old_h, new_w, new_h )
MFDB *old, *new;
int old_w, old_h;
int new_w, new_h;
{
	/*
	 * Copy the old bitmap to the new
	 */
	extern int vdi_handle;
	int pxy[8];
	int w, h;

	w = min( old_w, new_w );
	h = min( old_h, new_h );

	pxy[0] = 0;
	pxy[1] = 0;
	pxy[2] = w - 1;
	pxy[3] = h - 1;
	pxy[4] = 0;
	pxy[5] = 0;
	pxy[6] = w - 1;
	pxy[7] = h - 1;

	vro_cpyfm( vdi_handle, 3, pxy, old, new );

}

GWinFree()
{
	int i;
	for( i=0; i<MAXWINDOWS; i++ ) {
		FreeSlot( i );
	}
}

#ifndef SAI

int	
gptr_xp( win, flag )
WINDOW *win;
int flag;
{
	int handle;
	GRECT r;
	int mx, my;
	int dummy;

	handle = GemWin( win );
	if( handle < 0 ) return 0;

	wind_get( handle, WF_WORKXYWH, &r.g_x, &r.g_y, &r.g_w, &r.g_h );

	graf_mkstate( &mx, &my, &dummy, &dummy );
	if( mx < r.g_x ) return -1;
	if( mx > (r.g_x + r.g_w - 1) ) return win->_maxx;

	if( inside( mx, my, &r ) ) {
		return min( (mx - r.g_x) / charWidth, win->_maxx - 1 );
	}
	return -1;
}

int	
gptr_yp( win, flag )
WINDOW *win;
int flag;
{
	int handle;
	GRECT r;
	int mx, my;
	int dummy;

	graf_mkstate( &mx, &my, &dummy, &dummy );

	handle = GemWin( win );
	if( handle < 0 ) return 0;

	wind_get( handle, WF_WORKXYWH, &r.g_x, &r.g_y, &r.g_w, &r.g_h );
	if( my < r.g_y ) return -1;
	if( my > (r.g_y + r.g_h - 1) ) return win->_maxy;

	if( inside( mx, my, &r ) ) {
		return min( (my - r.g_y) / charHeight, win->_maxy - 1 );
	}
	return -1;
}
#endif SAI

	
/*
 * Flip between the FULLED window and the smaller one
 */

do_gfull()
{
	GRECT prev;
	GRECT curr;
	GRECT full;
	int   wh;
	int   i;

	wh = gl_rmsg[3];

	/* Turn off the mouse */
	graf_mouse( M_OFF, 0x0L );

	wind_get( wh, WF_CURRXYWH, &curr.g_x, &curr.g_y, &curr.g_w, &curr.g_h );
	wind_get( wh, WF_PREVXYWH, &prev.g_x, &prev.g_y, &prev.g_w, &prev.g_h );
	wind_get( wh, WF_FULLXYWH, &full.g_x, &full.g_y, &full.g_w, &full.g_h );

	if( rc_equal( &curr, &full ) ) {

		graf_shrinkbox( prev.g_x, prev.g_y, prev.g_w, prev.g_h,
			full.g_x, full.g_y, full.g_w, full.g_h );
		rc_copy( &prev, &curr );
	}
	else
	{
		graf_growbox( curr.g_x, curr.g_y, curr.g_w, curr.g_h,
			      full.g_x, full.g_y, full.g_w, full.g_h );
		rc_copy( &full, &curr );
	}

	i = HndlSlot( wh );
	if( i != -1 )
		SizeWin( i, &curr );

	graf_mouse( M_ON, 0x0L );
}

/*
 * Put a message in the Information line of a particular window
 */

STMessage( win, msg )
WINDOW *win;
char *msg;
{
	int handle;
	int i;

	i = WinSlot( win );
	if( i == -1 ) return;
	handle = AW_HANDLE(i);

	if( AW_INFO(i) ) {
		strncpy( AW_INFO(i), msg, AW_TLEN(i) );
		wind_set( handle, WF_INFO, AW_INFO(i) );
	}

}
	
clr_screen()
{
	extern int vdi_handle;
	v_clrwk( vdi_handle );
}

/*
 * Redraw the whole screen
 */

draw_all()
{
	int i;
#ifndef SAI
	extern long gl_menu;
#endif
	extern GRECT scr_area;

	for( i=0; i<MAXWINDOWS; i++ ) {

		if( AW_USED(i) ) {
			touchwin( AW_WIN(i) );
			wrefresh( AW_WIN(i) );
		}
	}

#ifndef SAI
	menu_bar( gl_menu, 1 );
#endif SAI

	form_dial( 3, 0, 0, 0, 0, 0, 0, scr_area.g_w, scr_area.g_h );
}

/*
 * We cache the working area of a window, so that curses won't have to
 * do wind_get's all the time.  This routine save's it in the slot's
 * work rectangle
 */

SetWork( slot )
int slot;
{
	wind_get( AW_HANDLE(slot), WF_WORKXYWH, 
		&AW_WORK(slot).g_x,
		&AW_WORK(slot).g_y,
		&AW_WORK(slot).g_w,
		&AW_WORK(slot).g_h );
}

/*
 * Set the title of the output window to the specified string
 */

PgTitle( str )
char *str;
{
	int i;
	extern WINDOW *pg_out;

	if( !pg_out ) return;

	i = WinSlot( pg_out );
	if( i < 0 ) return;

	if( strcmp( AW_TITLE(i), str ) == 0 ) return;

	strncpy( AW_TITLE(i), str, AW_TLEN(i) );
	wind_set( AW_HANDLE(i), WF_NAME, AW_TITLE(i) );
}

/*
 * Create a new workspace
 */

int NwsNum = 0;

#ifndef SAI
do_gnewws()
{
	int x, y, w, h;
	int i;
	int handle;
	WINDOW *win;
	char line[5];
	alice_window *alwin;
	char *wsname;
	extern workp get_workspace();
	workp tospace;
	int our2Flag;
	int features;
	int slot;

	/*
	 * Try and find a spare slot
	 */

	slot = FindSlot();
	if( slot < 0 ) return;

	wsname = line;
	line[0] = 'a' + NwsNum;
	line[1] = 0;
	NwsNum++;

	/*
	 * Save away attributes of old workspace
	 */
	work_cursor( curr_workspace ) = cursor;
	work_scursor( curr_workspace ) = curr_window->start_cursor;

	/*
	 * Make an edit window
	 */

	if( MkEditWin( slot, wsname ) ) return;

	/*
	 * Well, this makes a new workspace...
	 */
	tospace = get_workspace( wsname, TRUE );

	our2Flag = node_2flag(curr_workspace);

	init_pwork( tospace );		/* dirties our ws */
	s_node_flag(tospace, 0 );
	s_2flag(tospace, 0);

	s_2flag(curr_workspace, our2Flag);

	AW_ALWIN(slot)->wspace = tospace;

	/*
	 * Now that we have made it, go and activate it
	 */
	ActWork( slot, FALSE );

	SetCurWinTitle();

	count_lines();
}
#endif SAI

/*
 * Close an edit window
 */
do_gclose()
{
#ifndef SAI

	int handle;
	int i, slot;
	int edit;

	handle = gl_rmsg[3];

	slot = HndlSlot( handle );
	if( slot == -1 ) return;

	/* Ignore close requests on anything but an edit window */
	if( AW_KIND(slot) != WIN_EDIT )
		return;

	if( node_2flag( curr_workspace ) & WS_DIRTY ) {

		if( form_alert( 1, LDS( 394,
	"[2][You have not saved this|workspace. Close it|anyway?][ Yes | No ]")
			) == 2
		 ) return;
	}

	/*
	 * Count the number of edit windows
 	 */
	edit = 0;
	for( i=0; i < MAXWINDOWS; i++ ) {
		if( AW_USED(i) && AW_KIND(i) == WIN_EDIT ) edit++;
	}

	/*
 	 * If this is the only one, then the user is saying he wants to
	 * quit
	 */
	if( edit <= 1 )
		done( (char *)0 );

	/*
	 * Since the user will not have access to this workspace anymore
	 * go and say it isn't dirty
	 */
	node_2flag( curr_workspace ) &= ~WS_DIRTY;

	/*
	 * Okay, go and free everything
	 */
	FreeSlot( slot );

	/*
	 * Now go and look for the first edit workspace
	 */
	for( i=0; i < MAXWINDOWS; i++ ) {
		if( AW_USED(i) && AW_KIND(i) == WIN_EDIT ) break;
	}

	/*
	 * Activate the new edit window, but don't try to save the
	 * old workspace's status (since it ain't there no more)
	 */
	ActWork( i, FALSE );
#endif SAI
}

#ifndef SAI
count_lines()
{
	struct scr_line ourline;
	int pos;
	char line[50];

	register int num_lines = 0;
	register int not_finished = TRUE;
	curspos startnode;
	int size;
	int shown;
	int cur_line;
	register curspos cur_list;
	register int cur_kid;
	nodep c;
	WINDOW *win;

	win = AW_WIN(Current_win);

	cur_line = 0;
	c = t_up_to_line( curr_window->start_cursor );
	cur_list = tparent( c );
	cur_kid = get_kidnum( c );

	startnode = cur_work_top;
	ourline.ind_level = 0;
	ourline.sub_line = 0;
	ourline.listptr = tparent( startnode );
	ourline.listindex = get_kidnum( startnode );
	ourline.pcode = 0;

	while( not_finished ) {

		if( ourline.pcode == 0 && 
		    ourline.listptr == cur_list &&
		    ourline.listindex == cur_kid )
			cur_line = num_lines;

		not_finished = go_to_next( &ourline, 1, 1 );
		num_lines++;

	}

	/*
	 * The number of lines shown:
	 */
	shown = win->_maxy;

	AW_NUMLINES(Current_win) = num_lines;
	AW_CURLINE(Current_win) = cur_line;

	/*
	 * So... there are num_lines lines, and we are displaying
	 * win->_maxy of them, so figure out how big the slider
	 * should be and set it accordingly
	 */

	if( shown >= num_lines ) 
		size = 1000;
	else
		size = (long)shown * 1000L / (long)num_lines;

	wind_set( AW_HANDLE(Current_win), WF_VSLSIZE, size );

	printt3( "There are %d lines, %d shown, size = %d\n",
		num_lines, shown, size );

	SetLine( 0 );
}

SetLine( rel )
int rel;
{
	int cur_line;
	int num_lines;
	int pos;
	int shown;
	int size;
	WINDOW *win;

	win = AW_WIN(Current_win);

	/*
	 * CurLine is the line number of the top of the screen
	 */

	shown = win->_maxy;

	cur_line = AW_CURLINE(Current_win);
	num_lines = AW_NUMLINES(Current_win);

	cur_line += rel;
	if( cur_line < 0 )
		cur_line = 0;

	if( cur_line > num_lines )
		cur_line = num_lines;

	AW_CURLINE(Current_win) = cur_line;

	/*
	 * Now calculate the position of the slider.
	 * Curline theoretically should not be greater than
	 * num_lines - shown, but it sometimes is, so we must take
	 * that into account
	 */

	if( cur_line > (num_lines - shown) )
		cur_line = num_lines - shown;

	num_lines -= shown;

	pos = ((long)cur_line) * 1000L / (long)num_lines;

	wind_set( AW_HANDLE(Current_win), WF_VSLIDE, pos );

}

/*
 * Set the title of the current window
 */

SetCurWinTitle()
{ 
	char line[80];

	sprintf( line, " %s%c %s ", (curr_workspace->ws_name) ? 
		curr_workspace->ws_name : "",
		(node_2flag( curr_workspace ) & WS_DIRTY) ? '?' : ':',
		(curr_workspace->ws_fname) ? curr_workspace->ws_fname :
			LDS( 396, "Untitled") );

	/*
	 * If the title hasn't changed, don't tell GEM
	 * Since it will flicker
	 */
	if( strcmp( AW_TITLE(Current_win), line ) == 0 )
		return;

	strncpy( AW_TITLE(Current_win), line, AW_TLEN(Current_win));
	wind_set( AW_HANDLE(Current_win), WF_NAME, AW_TITLE(Current_win) );

}


/*
 * Handle the slider event
 */

do_gslide()
{
	int i, n;
	struct scr_line ourline;
	curspos startnode;

	/*
	 * Set the slider to the desired value
	 */
	count_lines();

	n = (long)(AW_NUMLINES(Current_win)-1) * (long)gl_rmsg[4] / 1000L;

	/*
	 * KLUDGE: this is so that we don't wind up on the first line of
	 * the program, when we really wanted the last
	 */
	if( n == AW_NUMLINES(Current_win) - 1 ) 
		n--;

	AW_CURLINE(Current_win) = n;

	/* Start at the top line and go down */
	startnode = cur_work_top;
	ourline.ind_level = 0;
	ourline.sub_line = 0;
	ourline.listptr = tparent( startnode );
	ourline.listindex = get_kidnum( startnode );
	ourline.pcode = 0;

	for( i=0; i<n; i++ ) {
		go_to_next( &ourline, 1, 1 );
	}

	set_sc_fline( curr_window, &ourline );
	cursor = curr_window->start_cursor;

	/*
	 * Position the cursor, somehow, here
	 */
	wind_set( gl_rmsg[3], WF_VSLIDE, gl_rmsg[4] );

}
#endif SAI

/*
 * Align a window onto a character boundary.  Features is the mask passed
 * to wind_create that specifies what features a window has (info line, 
 * sliders, etc).
 */

win_align( features, x, y, w, h )
int features;
int *x, *y, *w, *h;
{
	int wx, wy, ww, wh;

	wind_calc( 1, features, *x, *y, *w, *h, &wx, &wy, &ww, &wh );

	/*
	 * We have work co-ords, align them
	 */
	wx -= (wx % charWidth);
	ww -= (ww % charWidth);
	wh -= (wh % charHeight);

	wind_calc( 0, features, wx, wy, ww, wh, x, y, w, h );
}

/*
 * Make the Main Edit Window
 */
#ifndef SAI
MkMain()
{
	main_slot = FindSlot();
	if( MkEditWin( main_slot, LDS( 397, " main " )) ) return TRUE;

	/*
	 * Now go and activate this window
	 */
	ActWork( main_slot, FALSE );

	return FALSE;
}

int
MkEditWin( slot, win_name )
int slot;
char *win_name;
{
	int x, y, w, h;
	int features;
	alice_window *alwin;
	WINDOW *edit_win;

	/*
	 * This is the standard size and position for new edit windows
 	 */
	x = gl_xfull + charWidth;
	y = gl_yfull + 4;
	w = gl_wfull * 2 / 3;
	h = gl_hfull - (2 * charHeight);

	features = NAME | CLOSER | FULLER | MOVER | SIZER | INFO |
		UPARROW | DNARROW | VSLIDE;

	if( MkWindow( slot, win_name, features, x, y, w, h, FALSE ) ) {
		return TRUE;
	}

	AW_KIND(slot) = WIN_EDIT;
	
	AW_INFO(slot) = checkalloc( (gl_wfull / charWidth) + 1 );
	AW_INFO(slot)[0] = 0;

	edit_win = AW_WIN(slot);

	AW_ALWIN(slot) = (alice_window *)checkalloc(sizeof(alice_window));
	/*
	 * Couldn't allocate the alice window
	 */
	if( AW_ALWIN(slot) == (alice_window *)0 ) {
		FreeSlot( slot );
		return TRUE;
	}

	alwin = AW_ALWIN(slot);

	/*
	 * Set up the alice_window info
	 */

	alwin->wspace = curr_workspace;	/* For now point it to the curr wksp */
	setwlines( alwin, edit_win->_maxy );
	alwin->w_width = edit_win->_maxx;
	alwin->w_desc = edit_win;

	return FALSE;
}
#endif SAI

/*
 * Create the initial output window
 */

MkOutput()
{
	int x, y, w, h;
	int features;
	int slot;
	/*
	 * Make the output window
	 */
#ifdef SAI
	char *title_name;
	extern char *sai_title;

	title_name = sai_title;
	w = gl_wfull / 8 * 7;
	x = align_w( (gl_wfull - w) / 2 );
#else
	char *title_name;

	title_name = LDS( 370, " Output " );

	w = gl_wfull * 2 / 3;
	x = align_w(gl_xfull + w + (2 * charWidth));
	w = (gl_wfull - x - charWidth);
#endif

	y = gl_yfull + 4;
	h = gl_hfull - 2 * charHeight;


	features = NAME | FULLER | SIZER | MOVER;

	slot = FindSlot();

	if( MkWindow( slot, title_name, features, x, y, w, h, FALSE )){
		return TRUE;
	}

	AW_FILE( slot ) = &fil_output;
	pg_out = AW_WIN( slot );

	scrollok( pg_out, TRUE );
	pg_out->_flags |= AC_BLACK;

	werase( pg_out );
	curOn( pg_out );

	fil_output.desc.f_window = pg_out;
	fil_input.desc.f_window = pg_out;
	fil_output.f_flags |= FIL_OPEN | FIL_SCREEN;
	fil_input.f_flags |= FIL_OPEN | FIL_SCREEN;

	return FALSE;
}

/*
 * Make a General GEM Window, with a curses window inside
 * return TRUE if an error occurs
 */

int
MkWindow( slot, win_name, features, x, y, w, h, bit_save )
int slot;
char *win_name;
int x, y, w, h;
int bit_save;
{
	int handle;
	int dummy;

	int fx, fy, fw, fh;

	AW_USED(slot) = TRUE;
	AW_KIND(slot) = WIN_OUTPUT;

	fx = gl_xfull, fy = gl_yfull, fw = gl_wfull, fh = gl_hfull;
	win_align( features, &fx, &fy, &fw, &fh );

	handle = wind_create( features & 0x0fff, fx, fy, fw, fh );

	/*
	 * If we can't create the window, return the error
	 */
	if( handle < 0 ) {
		return TRUE;
	}

	if( features & NAME )
		wind_set( handle, WF_NAME, win_name );

	if( features & INFO )
		wind_set( handle, WF_INFO, "" );

	AW_FEATURES(slot) = features;

	win_align( features, &x, &y, &w, &h );

	graf_growbox( gl_xfull / 2, gl_yfull / 2, 5, 5, x, y, w, h );

	/*
	 * Now actually open the window
	 */
	wind_open( handle, x, y, w, h );

	AW_HANDLE(slot) = handle;
	AW_WIN(slot) = MkCurWin( handle );

	/*
	 * If we couldn't allocate the curses window, return an error
	 */
	if( AW_WIN(slot) == (WINDOW *)0 ) {
		FreeSlot( slot );
		return TRUE;
	}

	scrollok( AW_WIN(slot), TRUE );

	AW_TITLE(slot) = checkalloc( (gl_wfull / charWidth) + 1);
	AW_TITLE(slot)[0] = 0;
	AW_TLEN(slot) = (gl_wfull / charWidth);

	SetWork( slot );

	wind_get( AW_HANDLE(slot), WF_WORKXYWH, &dummy, &dummy, &w, &h );

	AW_BITS(slot) = 0L;

	curOff( AW_WIN(slot) );

	return FALSE;

}

/*
 * Bring the Output window to the front, and redraw it
 */
#ifndef SAI
TopPgOut()
{
	int handle;
	int top_win, dummy;

	/*
	 * Get the handle of the Gem window for pg_out
	 */
	if( !pg_out ) return;

	handle = GemWin( pg_out );

	if( handle < 0 ) return;

	/*
	 * Get the handle of the current top window
	 */
	wind_get( 0, WF_TOP, &top_win, &dummy, &dummy, &dummy );

	/*
	 * If pg_out is already topped, just return
	 */
	if( handle == top_win ) return;

	/*
	 * Otherwise, top it and refresh the curses window
	 */
	wind_set( handle, WF_TOP, 0, 0, 0, 0 );

	/*
	 * Handle the resultant events
	 */
	flush_events();
}
#endif SAI

/*
 * Free the slot in the array of open windows
 */

FreeSlot( slot )
int slot;
{
	if( !AW_USED(slot) ) return;

	if( AW_TITLE(slot) ) {
		mfree( AW_TITLE(slot) );
	}
	
	if( AW_INFO(slot) )
		mfree( AW_INFO(slot) );

	if( AW_WIN(slot) ) 
		delwin( AW_WIN(slot) );

	if( AW_HANDLE(slot) >= 0 ) {
		int x, y, w, h;
		wind_get( AW_HANDLE(slot), WF_CURRXYWH, &x, &y, &w, &h );
		wind_close( AW_HANDLE(slot) );
		wind_delete( AW_HANDLE(slot) );
		graf_shrinkbox( gl_xfull / 2, gl_yfull / 2, 5, 5,
			x, y, w, h );
	}

	if( (AW_KIND(slot) == WIN_EDIT) && AW_ALWIN(slot) ) {
		mfree( AW_ALWIN(slot)->w_lines );
		mfree( AW_ALWIN(slot) );
	}

	if( AW_BITS(slot) ) {
		FreeImage( AW_BITS( slot ) );
	}

	InitSlot( slot );
}

InitSlot( slot )
int slot;
{
	register char *s;
	register int i;

	s = &WinInfo[slot];	
	for( i=sizeof(STWINDOW); i>0; i-- ) {
		*s++ = 0;
	}
	AW_HANDLE(slot) = -1;
}

int
FindSlot()
{
	int i;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( !AW_USED(i) ) return i;
	}
	return -1;
}

/*
 * Top the current editing window
 */
TopCurEdit()
{
#ifndef SAI
	int slot;

	slot = WinSlot( curr_window->w_desc );
	if( slot < 0 ) return;

	ActWork( slot, TRUE );
#endif SAI
}

