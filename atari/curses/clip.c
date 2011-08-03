
/*
 * clip - Refresh a clipped region of a curses window
 */

#include <stdio.h>
#include <curses.h>
#include <osbind.h>
#include <obdefs.h>
#include <gemdefs.h>
#include "stwin.h"
#include <linea.h>

extern int charHeight;
extern long s_nxt_lin;

extern FILE *trace;
#define tracing	(trace != (FILE *)0)

#ifdef DEBUG
#define PRINTT
#endif

#include <printt.h>

#define CLIPPING	1

GRECT	CurCBox;	/* The Clip region set by redraw events */
GRECT 	ClipBox;	/* The piece of the window being redrawn */

int	CurClip = FALSE;
extern int CurColMap[];

unsigned char	lmask, rmask;

unsigned char LMasks[] = {
	0x0ff, 0x07f, 0x03f, 0x01f, 0x0f, 0x07,	0x03, 0x01 };

unsigned char RMasks[] = {
	0x80, 0x0c0, 0x0e0, 0x0f0, 0x0f8, 0x0fc, 0x0fe, 0x0ff };


static int start_x, start_y;
static int end_x, end_y;
static int length, num_lines;
static char *str;

/*
 * Force output to the screen.
 * We output whatever's been changed.
 */
clip_refresh( slot, win )
register int slot;
register WINDOW  *win;
{
	register long scrptr;
	register int  y;
	register char *oldscrptr;
	register int	begx, begy;
	register int bkgd;

#ifdef DEBUG
	int i;
	if( tracing ) {
	fprintf( trace, "Winfresh (%lx) The following lines are dirty:\n",
		(long) win  );
	for( i=0; i<win->_maxy; i++ ) {
		if( isdirty( win, i ) ) {
			fprintf( trace, "%d, ", i );
		}
	}
	fprintf( trace, "\n" );
	}
#endif

	begx = AW_WORK(slot).g_x;
	begy = AW_WORK(slot).g_y;
	bkgd = CurColMap[win->c_background];

	printt0( "Using Clipped Refresh\n" );

	/*
	 * Now for each line that falls within the clipping region,
	 * redisplay
	 */

	printt4( "Clip: %d, %d, %d, %d\n",
		ClipBox.g_x, ClipBox.g_y, ClipBox.g_w, ClipBox.g_h);

	/*
	 * Update the rectangle 'ClipBox'
	 */

	/* Calculate the character rectangle to update */
	start_x = (ClipBox.g_x - begx) / 8;
	end_x = (ClipBox.g_x + ClipBox.g_w - 1 - begx) / 8;

	length = end_x - start_x + 1;

	start_y = (ClipBox.g_y - begy) / charHeight;
	end_y = ( ClipBox.g_y + ClipBox.g_h - 1 - begy) / charHeight;
	num_lines = end_y - start_y + 1;

	printt2( "BegX: %d, BegY: %d\n", begx, begy );

	printt4( "StartX: %d, len: %d, StartY: %d, numlin: %d\n",
		start_x, length, start_y, num_lines );

	lmask = LMasks[ (ClipBox.g_x - begx) % 8 ];
	rmask = RMasks[ (ClipBox.g_x + ClipBox.g_w - 1 - begx) % 8 ];

	if( length == 1 ) {
		lmask &= rmask;
	}

	printt2( "LMask: %x   RMask: %x\n", lmask, rmask );

	scrptr = (long) Physbase();
	scrptr += (long)(start_y * charHeight * s_nxt_lin);
	scrptr += (long)(begy * s_nxt_lin);

	scrptr += (long) (start_x + (begx / 8)) / 2 * (VPLANES * 2);
	scrptr += (long) ((start_x + (begx / 8)) & 1);

	/*
	 * First line
	 */
	y = (ClipBox.g_y - begy) % charHeight;
	
	oldscrptr = scrptr;
	if( isdirty( win, start_y ) ) {
		str = (char *)(win->_y[start_y] + start_x);
		if( num_lines == 1 ) {
			int t;
			t = (ClipBox.g_y + ClipBox.g_h - 1 - begy) % charHeight;
			draw_line( scrptr, str, length, y, (t+1)-y, bkgd );
							
		} else {
			printt2( "Starting on scan line %d for %d lines\n",
				y, charHeight-y );
			draw_line( scrptr, str, length,	y, charHeight-y, bkgd );
		}
	}
	start_y++;
	/* Move onto the next char line */
	scrptr = oldscrptr + (s_nxt_lin * charHeight);

	/*
	 * Middle lines
	 */
	for( y = 0; y < num_lines-2; y++ ) {
		oldscrptr = scrptr;
		if( isdirty( win, start_y ) ) {
			str = (char *)(win->_y[start_y] + start_x);
			draw_line( scrptr, str, length,	0, charHeight, bkgd );
		}
		start_y++;
		/* Move onto the next char line */
		scrptr = oldscrptr + (s_nxt_lin * charHeight);
	}
	/*
	 * Last line
	 */
	y = (ClipBox.g_y + ClipBox.g_h - 1 - begy) % charHeight;
	
	oldscrptr = scrptr;
	if( num_lines > 1 && isdirty( win, start_y ) ) {
		str = (char *)(win->_y[start_y] + start_x);
		printt1( "Drawing last %d scan lines\n", y+1 );
		draw_line( scrptr, str, length,	0, y+1, bkgd );
	}
}
