
#include <stdio.h>
#include <curses.h>
#include <osbind.h>
#include <obdefs.h>
#include <gemdefs.h>
#include "stwin.h"
#include <linea.h>

extern int charWidth;
extern int charHeight;
extern long s_nxt_lin;
extern int next_char;
extern int num_planes;
extern int CurColMap[];

static long next_line;

fast_multi( slot, win )
int slot;
WINDOW *win;
{
	/*
	 * Multiplane fast update
	 */
	register char *scrptr;
	register char *ptr;
	register unsigned char ColBit;
	register int bkgd;
	register int col;
	register int m, k;
	register unsigned char style;
	register int next = next_char;
	int x, y;

	char buffer[20];
	long saveptr;
	char *str;
	unsigned char c;
	char *lineptr;
	int i, j;

	x = AW_WORK(slot).g_x;
	y = AW_WORK(slot).g_y;

	next_line = s_nxt_lin - (next + 1);

	/* Get the background color */
	bkgd = CurColMap[win->c_background];

	/* Calculate the screen address */
	scrptr = (long) Physbase();
	scrptr += (long)(y * s_nxt_lin);

	scrptr += (long) ((x / 16) * VPLANES * 2);
	scrptr += (long) ((x / 8) & 1);

	/* For each curses line */
	for( i=0; i<win->_maxy; i++ ) {
		lineptr = scrptr;
		/* If it is dirty... */
		if( isdirty( win, i ) ) {
			/* Get a pointer to the text */
			str = win->_y[i];
			for( j = win->_maxx; j>0; j-- ) {
				saveptr = scrptr;
				style = *str++;
				col = CurColMap[ style & 0xf ];
				c = *str++;
				/* Draw the character into a buffer */
				draw_char( buffer, c, style, 1L );
				ptr = buffer;
				/* For each scan line... */
				for( k=charHeight; k>0; k-- ) {
					ColBit = FirstPlane;
					/* For each plane */
					for( m=num_planes; m>0; m-- ) {
						if( ColBit & bkgd )
							*scrptr = ~*ptr;
						else
							*scrptr = 0;

						if( col & ColBit )
							*scrptr |= *ptr;

						ColBit <<= 1;
						scrptr += 2;
					}
					scrptr += next_line;
					ptr++;
				}
				if( ((long)scrptr) & 1 )
					scrptr = saveptr + next;
				else
					scrptr = saveptr + 1;

			}
		}
		/* Go onto the next scan line */
		scrptr = lineptr + (charHeight * s_nxt_lin);
	}
}
