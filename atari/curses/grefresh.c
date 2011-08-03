#include <stdio.h>
#include <curses.h>
#include <gemdefs.h>
#include <obdefs.h>
#include "stwin.h"

extern int vdi_handle;

Grefresh( slot, mode )
register int slot;
register int mode;
{
	int pxy[10];
	MFDB scrMFDB;
	MFDB *ptr;
	GRECT box;
	int handle; 
	extern GRECT scr_area;
	int x, y;

	/*
	 * Ptr points to a MFDB
	 * x and y are the screen co-ordinates at which we are to
	 * display the window, and 'box' is a pointer to a box
	 * which indicates where on the screen we are to update
	 */

	handle = AW_HANDLE(slot);
	ptr = AW_BITS(slot);

	x = AW_WORK(slot).g_x;
	y = AW_WORK(slot).g_y;

	if( ptr == (MFDB *)0 ) {
		if( mode != 0 ) return;
		ptr = &scrMFDB;
	}

	wind_get( handle, WF_FIRSTXYWH, &box.g_x, &box.g_y, &box.g_w, &box.g_h );

	while( box.g_w || box.g_h ) {

		/*
		 * Refresh the 'box'
		 */

		if( rc_intersect( &scr_area, &box ) ) {

			pxy[0] = box.g_x - x;
			pxy[1] = box.g_y - y;
			pxy[2] = pxy[0] + box.g_w - 1;
			pxy[3] = pxy[1] + box.g_h - 1;	

			pxy[4] = box.g_x;	
			pxy[5] = box.g_y;
			pxy[6] = box.g_x + box.g_w - 1;
			pxy[7] = box.g_y + box.g_h - 1;

			/*
			 * OR on the graphics image
			 */

			scrMFDB.fdaddr = 0L;

			vro_cpyfm( vdi_handle, mode, pxy, ptr, &scrMFDB );

		}
		wind_get( handle, WF_NEXTXYWH, &box.g_x, &box.g_y, &box.g_w,
				&box.g_h );
	}
}
