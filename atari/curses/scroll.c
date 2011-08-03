
#include <stdio.h>
#include <obdefs.h>
#include <gemdefs.h>
#include <curses.h>
#include "stwin.h"

static int pxy[8];
static MFDB	src;

extern int vdi_handle;

int
OneBox( win )
register WINDOW *win;
{
	register int slot;
	register int handle;
	GRECT r;
	extern GRECT scr_area;

	/*
	 * Return whether or not the window 'win' is obscured at all
	 */
	slot = WinSlot( win );

	handle = AW_HANDLE(slot);

	wind_get( handle, WF_FIRSTXYWH, &r.g_x, &r.g_y, &r.g_w, &r.g_h );

	/* Intersect it with the whole screen */
	rc_intersect( &scr_area, &r );

	/* If it is the whole working area then go do it ! */
	return rc_equal( &r, &AW_WORK(slot) );

}

ScrollWin( win, offset )
register WINDOW *win;
int offset;
{
	register int handle;
	GRECT *r;
	int slot;

	slot = WinSlot( win );

	r = &AW_WORK(slot);

	/* Set the x-coordinates */
	pxy[0] = pxy[4] = r->g_x;
	pxy[2] = pxy[6] = pxy[0] + r->g_w - 1;

	/*
	 * If the offset is positive then we are scrolling downwards by
	 * 'offset'
	 */
	if( offset > 0 ) {

		/* Scrolling down */
		pxy[1] = r->g_y;	/* Y - origin of source upper left */
					/* Y - origin of lower right */
		pxy[3] = r->g_y + r->g_h - offset - 1;

		pxy[5] = pxy[1] + offset;
		pxy[7] = pxy[3] + offset;

	} else {

		/* Scrolling up (offset is negative) */
		pxy[1] = r->g_y - offset; /* Y - origin of source upper left */
					/* Y - origin of lower right */
		pxy[3] = r->g_y + r->g_h - 1;

		pxy[5] = pxy[1] + offset;
		pxy[7] = pxy[3] + offset;

	}
	src.fdaddr = 0L;

	vro_cpyfm( vdi_handle, 3, pxy, &src, &src );

	/* Now go and clear out the portion scrolled */
	if( offset > 0 ) {
		pxy[1] = r->g_y;
		pxy[3] = r->g_y + offset - 1;
	} else {
		pxy[1] = r->g_y + r->g_h + offset;
		pxy[3] = r->g_y + r->g_h - 1;
	}
	vsf_color( vdi_handle, win->c_background );
	v_bar( vdi_handle, pxy );
}
