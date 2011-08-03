#include <stdio.h>
#include <curses.h>
#include <gemdefs.h>
#include <obdefs.h>
#include "stwin.h"

#include "printt.h"

extern int CurClip;
extern GRECT CurCBox;
extern GRECT ClipBox;
extern int num_planes;

wrefresh( win )
register WINDOW *win;
{

	GRECT box, area;
	register int i, wh;
	register int cx, cy;
	char *oldcur;
	unsigned char oldattr;
	extern GRECT scr_area;
	register int slot;

	slot = WinSlot( win );
	if( slot < 0 ) return;

	wind_update( BEG_UPDATE );
	graf_mouse( M_OFF, 0x0L );

	if( (win->_flags & _WRITTENTO) == 0 ) {
		/* Window has not been written to, refresh */
		if( AW_BITS( slot ) )
			Grefresh( slot, BITS_REPLACE );
		else
			Grefresh( slot, 0 );
		goto ref_finish;
	}

	/*
	 * Where is the cursor ?
	 */
	cy = win->_cury;
	cx = win->_curx;

#ifdef DEBUG
	if( tracing ) {
	fprintf( trace, "wrefresh(%lx), following lines are dirty:\n",
			(long)(win) );
	for( i=0; i<win->_maxy; i++ ) {
		if( isdirty( win, i ) ) {
			fprintf( trace, "%d, ", i );
		}
	}
	fprintf( trace, "\n" );
	}
#endif

	/* line where old cursor was is touched */
	if( win->_hascur ) {	 /* has a cursor */
		if( win->c_oldy != cy ) {
			win->_firstch[win->c_oldy] = TRUE;
			win->_firstch[cy] = TRUE;
		}
	 	else 
		   if( win->c_oldx != cx )
			win->_firstch[cy] = TRUE;
	
		oldcur = &win->_y[cy][cx];

		oldattr = *oldcur;
		*oldcur = ((oldattr & 0x40) ^ 0x40) | A_CURSOR;
	}

	printt2( "\nRefreshing window %lx, clipflag: %d\n",
			 (long)win, CurClip );

	/*
	 * If this is a 'redraw event', then copy the clipping box into
	 * 'area'
	 */
	if( CurClip ) {
		rc_copy( &CurCBox, &area );
		/*
		 * Make all the lines dirty, so that we refresh it all
		 */
		if( CurClip ) touchwin( win );

	} else {
		/* Otherwise copy 'scr_area' (the whole screen) into area */
		rc_copy( &scr_area, &area );
	}

	/*
	 * If we don't know anything about this window, return
	 */
	wh = AW_HANDLE(slot);

	printt1( "corresponds to slot %d\n", slot );


	/* We are updating the screen, get the list of rectangles to
	 * redraw.
	 */
	wind_get( wh, WF_FIRSTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h);

	/* While the rectangle is not null */
	while( box.g_w && box.g_h ) {

		/*
		 * If the area to update intersects with this rectangle in
		 * the window's rectangle list, go and redraw that region
	 	 */

		/* Intersect the area of interest with the clipping box */
		if( rc_intersect( &area, &box ) ) {

			printt4( "Redrawing clip: %d, %d, %d, %d\n",
				box.g_x, box.g_y, box.g_w, box.g_h );

			if( rc_equal( &box, &AW_WORK(slot) ) ) {
				fast_refresh( slot, win );
			} else {
				rc_copy( &box, &ClipBox );
				clip_refresh( slot, win );
			}
		}
		wind_get( wh, WF_NEXTXYWH, &box.g_x, &box.g_y, &box.g_w,
				&box.g_h );
	}
	/*
	 * Say that all the lines are clean now
	 */
	for( i=0; i<win->_maxy; i++ ) {
		clean_line( win, i );
	}

/*
 * If the window has a cursor, then un-reverse it
 */
	if( win->_hascur ) {
		*oldcur = oldattr;
		win->c_oldy = cy;
		win->c_oldx = cx;
	}

/*
 * If there is a bitmap associated with this window, OR it on
 */
	Grefresh( slot, BITS_OR );

	/*
	 * Turn the mouse back on
	 */
ref_finish:
	graf_mouse( M_ON, 0x0L );
	wind_update( END_UPDATE );
}

fast_refresh( slot, win )
register int slot;
register WINDOW *win;
{
	if( num_planes == 1 )
		fast_mono( slot, win );
	else
		fast_multi( slot, win );
}
