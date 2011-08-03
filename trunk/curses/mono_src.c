#include <stdio.h>
#include <curses.h>
#include <osbind.h>
#include <obdefs.h>
#include "stwin.h"

extern long s_nxt_lin;
extern int charHeight;
extern int num_planes;

/*
 * Quickly refresh a monochrome screen, from window 'win' origining the window
 * at (x,y)
 */

fast_mono( slot, win )
int slot;
register WINDOW *win;
{
	register int i,j;
	register char *scrptr;
	register char *str;
	register long saveptr;
	register unsigned char style;
	register unsigned char c;
	register long incr;
	int x, y;

	x = AW_WORK(slot).g_x;
	y = AW_WORK(slot).g_y;

	incr = s_nxt_lin * charHeight;
	scrptr = (long) Physbase();
	scrptr += (long)(y * s_nxt_lin);

	scrptr += (long) ((x / 16) * num_planes * 2);
	scrptr += (long) ((x / 8) & 1);

	/* For each line... */
	for( i=0; i<win->_maxy; i++ ) {
		saveptr = scrptr;
		/* If it is dirty, output it */
		if( isdirty( win, i ) ) {
			str = win->_y[i];
			for( j = win->_maxx; j>0; j-- ) {
				style = *str++;
				c = *str++;
				draw_char( scrptr, c, style, s_nxt_lin );
				scrptr++;
			}
		}
		scrptr = saveptr + incr;
	}
}
