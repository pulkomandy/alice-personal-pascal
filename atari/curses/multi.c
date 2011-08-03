
#include <stdio.h>
#include <curses.h>

extern long s_nxt_lin;
extern int next_char;
extern long f_nxt_lin;
extern char *font_data;
extern char *font_base;
extern int num_planes;
extern int charHeight;
extern int charWidth;

multi_refresh( win, x, y )
WINDOW *win;
int x;
int y;
{
	/*
	 * Multiplane fast update
	 */
	register int m, k;
	register char *ptr;
	register unsigned char style;
	register unsigned char ColBit;
	register int next = next_char;

	char buffer[20];
	long saveptr;
	char *scrptr;
	char *str;
	unsigned char c;
	char *lineptr;
	int i, j;

	scrptr = (long) Physbase();
	scrptr += (long)(y * s_nxt_lin);

	scrptr += (long) ((x / 16) * num_planes * 2);
	scrptr += (long) ((x / 8) & 1);


	for( i=0; i<win->_maxy; i++ ) {
		lineptr = scrptr;
		if( isdirty( win, i ) ) {
			str = win->_y[i];
			for( j = win->_maxx; j>0; j-- ) {
				saveptr = scrptr;
				style = *str++;
				c = *str++;
				draw_char( buffer, c, style, 1L );
				ptr = buffer;
				for( k=charHeight; k>0; k-- ) {
					ColBit = FirstPlane;
					for( m=num_planes; m>0; m-- ) {
						if( style & ColBit )
							*scrptr = *ptr;
						else
							*scrptr = 0;
						ColBit <<= 1;
						scrptr += 2;
					}
					scrptr += s_nxt_lin - (next + 1);
					ptr++;
				}
				if( ((long)scrptr) & 1 )
					scrptr = saveptr + next;
				else
					scrptr = saveptr + 1;

			}
		}
		scrptr = lineptr + (charHeight * s_nxt_lin);
	}
}
