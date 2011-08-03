#include <stdio.h>
#include <curses.h>
#include <osbind.h>
#include <obdefs.h>
#include <gemdefs.h>
#include <linea.h>

int charHeight;
int charWidth;

struct la_font **font_table;
struct la_font *font_hdr;

#define ALICE_FONT	2

long	s_nxt_lin;
long	f_nxt_lin;
char	*font_data;
char 	*font_base;
int	next_char;	/* Number of bytes from last byte of a word to next
			 * byte in the same plane
			 */
int	num_planes;

/*
 * This routine is called once, upon initialization
 * from init_scr()
 */

int AL_Font[] = {
	1, 1, 2 };

init_draw()
{

	/* Get the address of the array of pointers to the 3 system
	 * font headers
	 */
	linea0();

	font_table = la_init.li_a1;

	/* Now set up the pointer to the font we want to use: */
	font_hdr = font_table[ AL_Font[ Getrez() ] ];

	/* Set up the character height */
	charHeight = font_hdr->font_height;
	charWidth = font_hdr->font_fat_cell;

	if( charWidth != 8 ) {
		form_alert( 1, "[2][System Font must be 8 bits wide][OK]" );
		exit(1);
	}

	font_base = font_hdr->font_data;
	f_nxt_lin = font_hdr->font_width;
	s_nxt_lin = VWRAP;

	next_char = (VPLANES * 2) - 1;
	num_planes = VPLANES;

	init_colmap();
}
