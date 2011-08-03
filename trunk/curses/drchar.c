#include <curses.h>

extern char *font_base;
extern long f_nxt_lin;
extern int charHeight;

draw_char( buffer, c, style, next_line )
register char *buffer;
register unsigned char c;
register unsigned char style;
register long next_line;
{
	register char *fontptr;
	register unsigned char und_mask, xor_mask;
	
	fontptr = &font_base[c];
		
	if( style & B_REVERSE ) {
		xor_mask = 0xff;
	} else {
		xor_mask = 0;
	}

	/*
	 * For all but the last scan line...
	 */
	for( und_mask = charHeight-1; und_mask != 0; und_mask-- ) {
		c = *fontptr;
		if( style & B_BOLD ) {
			c |= (c << 1);
		}
		*buffer = (c ^ xor_mask);
		buffer += next_line;
		fontptr += f_nxt_lin;
	}

	c = *fontptr;
	if( style & B_BOLD ) {
		c |= (c >>1);
	}
	if( style & B_UNDERLINE ) {
		c |= 0xff;
	} else if( style & B_DOTUNDERLINE ) {
		c |= 0xaa;
	}
	*buffer = c ^ xor_mask;
}
