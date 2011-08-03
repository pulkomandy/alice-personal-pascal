
#include "alice.h"
#include <gemdefs.h>
#include "alrsc.h"
#include <osbind.h>
#include <obdefs.h>
#include <gembind.h>
#include <curses.h>

extern int gl_xfull, gl_yfull, gl_hfull, gl_wfull;
extern int BufferGraphics;

/*
 * This file contains the routines to initialize and terminate the
 * GEM Interface for ST Alice
 *
 * July 25th, 1986 - David
 *
 * The Curses Style windows are set up in window.c
 */

int LoRes = FALSE;
long avail_mem;
int LowMemCheck = FALSE;

/*
 * Global Data Structures
 */
int	contrl[12];
int	intin[80];
int	ptsin[256];
int	intout[45];
int	ptsout[12];


/*
 * LOCAL Definitions
 */

int gl_wchar;		/* character height */
int gl_hchar;		/* character width */
int gl_hbox;		/* box (cell) width */
int gl_wbox;		/* box (cell) height */

int gl_hspace;		/* Height of space between lines */
long gl_menu = 0L;

int gem_handle;		/* GEM VDI Handle */
int vdi_handle;		/* application vdi handle */

/* The x,y,h,w of the full size window */
int gl_xfull;
int gl_yfull;
int gl_hfull;
int gl_wfull;

GRECT scr_area;		/* The Full Sized Screen */
GRECT work_area;	/* Drawing area for current window */

int gl_apid;		/* The application ID */

int scr_planes;		/* Number of planes in video memory */

GemInit()
{
	int	work_in[11];
	int	i;
	int 	dev_id;
	int work_out[57];


	appl_init();

	if( gl_apid == -1 ) 
		return(4);

	/* Make Alice compatible with the fine Atari GDOS */
        dev_id = Getrez() + 2;

	gem_handle = graf_handle( &gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox );
	vdi_handle = gem_handle;

	/*
	 * Open the Virtual Workstation
	 */
	for( i=1; i<10; i++ ) work_in[i] = 1;
	work_in[0] = dev_id;
	work_in[10] = 2;		/* Raster coords */

	v_opnvwk( work_in, &vdi_handle, work_out );

	scr_area.g_w = work_out[0] + 1;
	scr_area.g_h = work_out[1] + 1;
	scr_area.g_x = 0;
	scr_area.g_y = 0;

	if( (scr_area.g_w / gl_wchar) < 70 )
		LoRes = TRUE;

#ifdef EMBEDDED_RCS
	rsc_fix();
#else

	if( !rsrc_load( "ALICE.RSC" ) ) {
		form_alert( 1, LDS( 375, 
		"[3][Fatal Error !|ALICE.RSC must be|in current directory][ Abort ]"));
		v_clsvwk( vdi_handle );
		appl_exit();
		exit( 1 );
	}

#endif EMBEDDED_RCS

#ifndef SAI
	/*
	 * Set up the menu bar
	 */

	rsrc_gaddr( R_TREE, MENU, &gl_menu );

	if( LoRes ) {
		ShortenMenu( gl_menu );
		FixBar( gl_menu );
	}

	menu_bar( gl_menu, TRUE );

#endif SAI

	/*
	 * Set the mouse to an arrow, and end the update
	 */
	graf_mouse( ARROW, 0x0L );

	resetVDI();

}

long
malloc_left()
{
	extern long MemUsed;
	return avail_mem - MemUsed;
}

CalcMemLeft()
{
	extern long MemUsed;

	MemUsed = 0L;

	if( (avail_mem = Malloc( -1L )) > 100000L ) {
		BufferGraphics = TRUE;
#ifndef SAI
		menu_icheck( gl_menu, MMBUFFER, 1 );
#endif

	}
	LowMemCheck = TRUE;

}

GemTerm()
{

#ifndef SAI
	/*
	 * Remove the menu bar
	 */
	menu_bar( gl_menu, FALSE );
#endif SAI

	/*
 	 * Free up the memory used by the resource
	 */
	rsrc_free();

	/*
	 * Close the workstation
	 */
	v_clsvwk( vdi_handle );

	/*
	 * And terminate the application
	 */
	appl_exit();

}

do_gfreemem()
{
	char msg[150];
	extern long MemUsed;
	long left;
	long sys_left;

	left = avail_mem - MemUsed;
	if( left < 0L )
		left = 0;

	sys_left = Malloc( -1L );

	sprintf( msg, LDS( 376, "[1][%ld bytes free][ OK ]"), left );
	form_alert( 1, msg );
}
