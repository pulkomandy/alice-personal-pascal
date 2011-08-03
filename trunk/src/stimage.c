
#include "alice.h"
#include <curses.h>

#include "workspace.h"
#include "window.h"
#include <obdefs.h>
#include <gemdefs.h>

#include "stwin.h"
#include "alrsc.h"

#include <linea.h>

#include "printt.h"

long BufGraphFailed;

/*
 * VDI can not draw to screens that are not the width of the current screen
 */
#define BROKEN_VDI	1

int BufferGraphics = FALSE;

extern GRECT scr_area;

#ifndef SAI
extern long gl_menu;
#endif

extern char *mmalloc();

ClearBits( win )
WINDOW *win;
{
	MFDB *bits;
	int size;
	char *ptr;

	bits = GemBits( win );
	if( bits ) {

		ptr = bits->fdaddr;
		size = bits->fdwdwidth * 2 * VPLANES * bits->fdh;
		while( size > 0 ) {
			*ptr++ = 0;
			size--;
		}
	}
}

MFDB *
GemBits( win )
WINDOW *win;
{
	int slot;

	if( !win ) return 0L;

	slot = WinSlot( win );
	if( slot >= 0 )
		return AW_BITS(slot);
	return 0L;
}

MFDB *
MkImage( width, height )
int width;
int height;
{
	register char *s;
	MFDB *ptr;
	int size;
	char *image;
	int i;
	int bit_size;
	extern long MemUsed;

	if( !BufferGraphics ) return 0L;

	printt2( "MkImage( %d, %d )\n", width, height );

#ifdef BROKEN_VDI
	width = scr_area.g_w;
	printt1( "Setting width to %d\n", width );
#endif

	size = ((width + 15) / 16) * 2 * height * VPLANES;
	printt1( "MFDB + Bitmap is %d bytes long\n", size );

	bit_size = size;

	size += sizeof( MFDB );

	/* If we failed to allocate a graphics buffer before, then see
	 * if we are going to try and get more or the same amount of
	 * memory as when we failed
	 */
	if( BufGraphFailed ) {
		/* If we are trying for the same, or more, fail without
		 * trying to malloc
		 */
		if( (MemUsed + (long)size) >= BufGraphFailed )
			return 0L;
	}
	/* Otherwise, try to do the malloc */
	ptr = (MFDB *) mmalloc( size );

	/* If we failed, store the value which indicates how much memory
	 * we should have for it to succeed
	 */
	if( ptr == 0L )	{
		BufGraphFailed = MemUsed + (long)size;
		return 0L;
	}
	BufGraphFailed = 0L;

	image = ((long)ptr + (long)sizeof( MFDB ));

	ptr->fdaddr = (long)image;
	ptr->fdh = height;
	ptr->fdwdwidth = ((width+15) / 16);
	ptr->fdw = ptr->fdwdwidth * 16;
	ptr->fdnplanes = VPLANES;
	ptr->fdstand = 0;

	return ptr;

}

FreeImage( ptr )
MFDB *ptr;
{
	if( ptr == (MFDB *)0 ) return;
	mfree( ptr );
}

/*
 * Used by the button event handler to determine whether the window clicked
 * on should return the event (for selection of the tree)
 */

int
EditWin( handle )
int handle;
{
	int i;

	i = HndlSlot( handle );
	if( i != -1 )
		return AW_KIND(i) == WIN_EDIT;

	return FALSE;
}

/*
 * Toggle the Buffer Graphics status.
 *
 * If we are buffering, then uncheck the menu item, and free all
 * the memory associated with it.
 */

do_gbuffer()
{
#ifndef SAI

	extern WINDOW *pg_out;
	GRECT winrect;
	MFDB *bits;
	int x, y, w, h;
	int handle;
	int i;

	/*
	 * Buffering is already on, turn it off
	 */
	if( BufferGraphics ) {
		menu_icheck( gl_menu, MMBUFFER, 0 );
		for( i=0; i<MAXWINDOWS; i++ ) {
			if( !AW_USED(i) ) continue;
			if( AW_KIND(i) != WIN_OUTPUT ) continue;
			FreeImage( AW_BITS(i) );
			AW_BITS(i) = (MFDB *)0;
		}
		BufferGraphics = FALSE;
	} else {
		/*
		 * Turn on graphics buffering
		 */
		menu_icheck( gl_menu, MMBUFFER, 1 );
		BufferGraphics = TRUE;
	}
#endif SAI
}
