
#include <stdio.h>
#include <curses.h>

/*
 * DrawLine draws a line onto the screen with left/right clipping
 */

extern long s_nxt_lin;
extern long f_nxt_lin;
extern int  next_char;
extern int  charHeight;
extern char *font_base;
extern int  num_planes;
extern char *font_data;
extern unsigned char lmask, rmask;
extern int CurColMap[];

static	long	scr_incr;

draw_line( buffer, curptr, length, first_scan, num_scan, bkgd )
register char *buffer;
register char *curptr;
int length;
int first_scan;
int num_scan;
register unsigned char bkgd;
{
	register char *fontptr;
	register unsigned char c;
	register unsigned char style;
	register int ColBit;
	register int j, k;
	register int col;
	register char *saveptr;
	char cbuf[20];
	
	int i;

	/*
	 * Calculate where we should start copying to in the screen
	 */
	buffer += (s_nxt_lin * first_scan);
	scr_incr = (s_nxt_lin) - (next_char + 1);

	/*
	 * Do the leftmost character
	 */
	/* Save where we are on the screen */

	saveptr = buffer;
	style = *curptr++;
	col = CurColMap[ style & 0xf ];
	c = *curptr++;
	draw_char( cbuf, c, style, 1L );
	fontptr = &cbuf[first_scan];

	for( j = num_scan; j > 0; j-- ) {
		c = *fontptr;
		ColBit = FirstPlane;
		for( k=num_planes; k > 0; k-- ) {
			if( ColBit & bkgd )
				*buffer = (lmask & ~c) | (*buffer & ~lmask);
			else
				*buffer = *buffer & ~lmask;

			if( col & ColBit )
				*buffer |= c & lmask;

			buffer += 2;
			ColBit <<= 1;
		}
		buffer += scr_incr;
		fontptr++;
	}

	if( ((long)buffer) & 1 )
		buffer = saveptr + next_char; /* 1, 3, or 7 */
	else 
		buffer = saveptr + 1;

	if( length == 1 ) return;

	length--;
	/*
	 * Do the middle characters
	 */

	for( i = length; i>1; i--) {

		/* Save where we are on the screen */
		saveptr = buffer;

		style = *curptr++;
		col = CurColMap[ style & 0xf ];
		c = *curptr++;
		draw_char( cbuf, c, style, 1L );
		fontptr = &cbuf[first_scan];

		for( j = num_scan; j > 0; j-- ) {
			c = *fontptr;
			ColBit = FirstPlane;
			for( k=num_planes; k > 0; k-- ) {
				if( ColBit & bkgd )
					*buffer = ~c;
				else
					*buffer = 0;

				if( col & ColBit )
					*buffer |= c;
				
				buffer += 2;
				ColBit <<= 1;
			}
			buffer += scr_incr;
			fontptr++;
		}
		if( ((long)buffer) & 1 )
			buffer = saveptr + next_char; /* 1, 3, or 7 */
		else 
			buffer = saveptr + 1;

	}

	/*
	 * Do the last character
	 */

	/* Save where we are on the screen */

	style = *curptr++;
	col = CurColMap[ style & 0xf ];
	c = *curptr++;
	draw_char( cbuf, c, style, 1L );
	fontptr = &cbuf[first_scan];

	for( j = num_scan; j > 0; j-- ) {
		c = *fontptr;
		ColBit = FirstPlane;
		for( k=num_planes; k > 0; k-- ) {
			if( ColBit & bkgd )
				*buffer = (rmask & ~c) | (*buffer & ~rmask);
			else
				*buffer = *buffer & ~rmask;

			if( col & ColBit )
				*buffer |= c & rmask;
				
			buffer += 2;
			ColBit <<= 1;
		}
		buffer += scr_incr;
		fontptr++;
	}
}
