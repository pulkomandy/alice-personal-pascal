

#include "alice.h"
#include "interp.h"

#include <curses.h>
#include <obdefs.h>
#include <vdibind.h>
#include <gemdefs.h>
#include <osbind.h>
#include <linea.h>

#include "workspace.h"
#include "window.h"
#include "stwin.h"
#include "stmenu.h"
#include "dbflags.h"

#define MED_RES	1

int AspectRatio[] = { 1, 2, 1 };
int CurAspect;	/* The current aspect ratio */

#include "printt.h"
extern struct pas_file fil_input, fil_output;

extern GRECT scr_area;

#ifdef SAI
extern int sai_out;
#endif

/* The number of colours for each resolution - can i get it from vdi ? */
int GemCols[] = { 16, 4, 2 };

/* The RGB Values for the default palette, as returned by the vq_color
 * inquire color command in the GEM VDI
 */

int ThePalettes[5][3] = {
	GREEN, RED, YELLOW,
	CYAN, MAGENTA, LWHITE,
	LGREEN, LRED, YELLOW,
	LCYAN, LMAGENTA, WHITE,
	BLACK, RED, GREEN,		/* Back to normal */
	};

int RGBVals[16][3] = {
	/* 0  */	1000, 1000, 1000,
	/* 1  */	0,    0,    0,
	/* 2  */	1000, 0,    0,
	/* 3  */	0,    1000, 0,
	/* 4  */	0,    0,    1000,
	/* 5  */	0,    1000, 1000,
	/* 6  */	1000, 1000, 0,
	/* 7  */	1000, 0,    1000,
	/* 8  */	714,  714,  714,
	/* 9  */	428,  428,  428,
	/* 10 */	1000, 428,  428,
	/* 11 */	428,  1000, 428,
	/* 12 */	428,  428,  1000,
	/* 13 */	428,  1000, 1000,
	/* 14 */	1000, 1000, 428,
	/* 15 */ 	1000, 428,  1000
	};

extern WINDOW *pg_out;
extern int vdi_handle;
extern int BufferGraphics;

int gr_pxy[8];

extern STMenu *UsrMenu;
#ifndef SAI
extern long gl_menu;
#endif

WINDOW *	_currGraph;
int 		_currSystem;
int 		_currColor;

int		EditHidden;

extern int pts_buf[];

typedef int (*grfunc)();

/*
 * This routine takes a function pointer and calls it with varying origins
 * to draw the desired shapes on both the off-screen bitmap, and for each
 * rectangle in the rectangle list
 */

extern int MouseStat;

DrawWin( the_func )
grfunc the_func;
{
	GRECT winrect;
	int handle;
	MFDB *Bits;
	long SaveBase;
	int x, y, w, h;
	int slot;
	int flag;
	extern MFDB *MkImage();

	if( !_currGraph ) return;

	slot = WinSlot( _currGraph );
	if( slot < 0 ) return;

	rc_copy( &AW_WORK(slot), &winrect );

	handle = AW_HANDLE( slot );
	Bits = AW_BITS( slot );

	if( BufferGraphics && (Bits == (MFDB *)0) ) {
		/*
		 * We are trying to buffer graphics, but haven't been
		 * able to get the memory so far, so try again.
		 * (Also allocates the image only once graphics calls are
		 * made).
		 */
		Bits = MkImage( winrect.g_w, winrect.g_h );
		AW_BITS( slot ) = Bits;
	}

	printt1( "do_draw: bits = %lx\n", (long) Bits );
	printt1( "Function to draw: %lx\n", (long) the_func );

	if( Bits ) {

		/*
		 * Draw the line into the off-screen buffer
	 	 */
		SaveBase = (long) Logbase();

		Setscreen( Bits->fdaddr, -1L, -1 );

		/*
		 * Set the clip region to the size of the window
		 */
		gr_pxy[0] = 0;
		gr_pxy[1] = 0;
		gr_pxy[2] = winrect.g_w - 1;
		gr_pxy[3] = winrect.g_h - 1;

		vs_clip( vdi_handle, 1, gr_pxy );

		/*
		 * Now draw the appropriate stuff ( 0-origin )
		 */
		(*the_func)( 0, 0 );

		/*
		 * Set the pointer back to screen memory
		 */
		Setscreen( SaveBase, -1L, -1 );

	}

	wind_get( handle, WF_FIRSTXYWH, &x, &y, &w, &h );

	flag = MouseStat;
	MouseOn( FALSE );

	while( w && h ) {

		gr_pxy[0] = x;
		gr_pxy[1] = y;
		gr_pxy[2] = x + w - 1;
		gr_pxy[3] = y + h - 1;

		vs_clip( vdi_handle, 1, gr_pxy );

		(*the_func)( winrect.g_x, winrect.g_y );

		wind_get( handle, WF_NEXTXYWH, &x, &y, &w, &h );
	}

	MouseOn( flag );

	/*
	 * Then turn off clipping
	 */
	vs_clip( vdi_handle, 0, gr_pxy );

}

do_gsprite( argc, argv )
int argc;
rint *argv;
{

	pointer saveblock;
	pointer sprite_def;
	int x, y;
	int action;
	nodep save_type;
	nodep def_type;
	int spritesize, savesize;

	saveblock = p_spop();		/* the address */
	save_type = (nodep) p_spop();	/* the type of the save block */

	savesize = (10 + 64 * VPLANES);
	spritesize = 37 * sizeof(int);

	if( calc_size( save_type, TRUE ) < savesize )
		run_error( ER(351, "sbsmall`Saveblock too small for sprite" ));

	sprite_def = p_spop();
	def_type = (nodep)p_spop();

	if( calc_size( def_type, TRUE ) < spritesize )
		run_error( ER(352, "sdsmall`Sprite definition too small"));

	y = int_spop();
	skip_type();

	x = int_spop();
	skip_type();

	WinMap( &x, &y );

	if( x < 0 )
		x = 0;
	if( x >= scr_area.g_w )
		x = scr_area.g_w - 1;
	if( y < 0 )
		y = 0;
	if( y >= scr_area.g_h )
		y = scr_area.g_h - 1;

	action = int_spop();

	if( action <= 0 ) {
		/* Make sure the saveblock is defined */
		do_undef( saveblock, U_CHECK, savesize );
	}
	if( action >= 0 ) {
		do_undef( sprite_def, U_CHECK, spritesize );
		do_undef( saveblock, U_SET, savesize );
	}

	/*
	 * Undraw the sprite
	 */
	if( action <= 0 ) {
		lineac( saveblock );
	}

	/*
	 * Draw the sprite
	 */
	if( action >= 0 ) {
		linead( (long)x, (long)y, sprite_def, saveblock );
	}
}

/*
 * convert a (x,y) pixel point into the window relative coordinates
 * according to the current system
 */

CoMap( x, y )
int *x;
int *y;
{
	long size;
	long width, height;
	long dwidth, dheight;

	GRECT workrect;

	if( !_currGraph ) return;

	GetWork( _currGraph, &workrect );

	switch( _currSystem ) {

		case CO_PIXEL:
			return;
/*
 * Make sure that med rez squares are actually square. Since the aspect
 * ratio is very different in med rez, we must kludge the ratio.
 */
		case CO_NORMAL:
			width = min( workrect.g_w, workrect.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_REVNORMAL:
			width = max( workrect.g_w, workrect.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;

		case CO_STRETCH:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = dheight = 32767L;
			break;
		case CO_HIRES:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = 640L;
			dheight = 200L;
			break;
		case CO_MEDRES:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = 320L;
			dheight = 200L;
	}
	*x = (long)*x * width / dwidth;
	*y = (long)*y * height / dheight;

}

/*
 * Map from pixel coordinates to window relative coordinates
 */

MapCo( slot, x, y )
int slot;
int *x, *y;
{
	long size;
	GRECT workrect;
	long width, height;
	long dwidth, dheight;

	rc_copy( &AW_WORK(slot), &workrect );

	switch( _currSystem ) {

		case CO_PIXEL:
			return;
		case CO_NORMAL:
			width = min( workrect.g_w, workrect.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_REVNORMAL:
			width = max( workrect.g_w, workrect.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_STRETCH:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = dheight = 32767L;
			break;
		case CO_HIRES:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = 640L;
			dheight = 200L;
			break;
		case CO_MEDRES:
			width = workrect.g_w;
			height = workrect.g_h;
			dwidth = 320L;
			dheight = 200L;

	}
	*x = (long)*x * dwidth / width;
	*y = (long)*y * dheight / height;

}

WinMap( x, y )
int *x;
int *y;
{
	long size;
	long dwidth, dheight;
	long width, height;

	switch( _currSystem ) {

		case CO_PIXEL:
			return;
		case CO_NORMAL:
			width = min( scr_area.g_w, scr_area.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_REVNORMAL:
			width = max( scr_area.g_w, scr_area.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_STRETCH:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = dheight = 32767L;
			break;
		case CO_HIRES:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = 640L;
			dheight = 200L;
			break;
		case CO_MEDRES:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = 320L;
			dheight = 200L;
	}
	*x = (long)*x * width / dwidth;
	*y = (long)*y * height / dheight;
}

/*
 *  Map from pixel coordinates to full screen coordinates in the current system
 */
MapWin( x, y )
int *x;
int *y;
{
	long size;
	long width, height;
	long dwidth, dheight;

	switch( _currSystem ) {

		case CO_PIXEL:
			return;
		case CO_NORMAL:
			width = min( scr_area.g_w, scr_area.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_REVNORMAL:
			width = max( scr_area.g_w, scr_area.g_h * CurAspect );
			height = width / CurAspect;
			dwidth = dheight = 32767L;
			break;
		case CO_STRETCH:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = dheight = 32767L;
			break;
		case CO_HIRES:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = 640L;
			dheight = 200L;
			break;
		case CO_MEDRES:
			width = scr_area.g_w;
			height = scr_area.g_h;
			dwidth = 320L;
			dheight = 200L;
	}
	*x = (long)*x * dwidth / width;
	*y = (long)*y * dheight / height;
}

int pt_x1, pt_x2;
int pt_y1, pt_y2;
int pt_color;
char *pt_str;

do_draw( argc, argv )
int argc;
rint *argv;
{
	extern int _do_draw();

	if( argc == 5 ) {
		pt_color = int_spop();
		SetColor( pt_color );
	}

	pt_y2 = int_spop();
	pt_x2 = int_spop();
	pt_y1 = int_spop();
	pt_x1 = int_spop();

	CoMap( &pt_x1, &pt_y1 );
	CoMap( &pt_x2, &pt_y2 );

	DrawWin( _do_draw );

}

SetColor( color )
int color;
{
	vsl_color( vdi_handle, color );
	vsf_color( vdi_handle, color );
	vst_color( vdi_handle, color );
	_currColor = color;
}

int
_do_draw( x, y )
int x, y;
{
	gr_pxy[0] = pt_x1 + x;
	gr_pxy[1] = pt_y1 + y;
	gr_pxy[2] = pt_x2 + x;
	gr_pxy[3] = pt_y2 + y;

	v_pline( vdi_handle, 2, gr_pxy );
}

/*
 * Draw an Ellipse
 */

do_gellipse( argc, argv )
int argc;
rint *argv;
{
	extern int _do_gellipse();

	pt_y2 = int_spop();	/* Y - Radius */
	pt_x2 = int_spop();	/* X - Radius */
	pt_y1 = int_spop();	/* Y - origin */
	pt_x1 = int_spop();	/* X - origin */

	/*
	 * Map the origin and the radii
	 */
	CoMap( &pt_x1, &pt_y1 );
	CoMap( &pt_x2, &pt_y2 );

	/* if y-radius is zero, make it a circle */

	if( pt_y2 == 0 )
		if( Getrez() == MED_RES )
			pt_y2 = pt_x2 / 2;
		else
			pt_y2 = pt_x2;

	DrawWin( _do_gellipse );
}

_do_gellipse( x, y )
int x, y;
{
	v_ellarc( vdi_handle, pt_x1 + x, pt_y1 + y, pt_x2, pt_y2, 0, 3600 );
}

do_gsetcolor( argc, argv )
int argc;
rint *argv;
{
	int color = int_spop();

	SetColor( color );
}

/*
 * Set the co-ordinate system
 */
do_SetCoord( argc, argv )
int argc;
rint *argv;
{
	int system = int_spop();

	if( system > CO_MEDRES || system < 0 )
		run_error( ER(336, "illcoor`Illegal coordinate system" ));

	_currSystem = system;
}

do_plot( argc, argv )
int argc;
rint *argv;
{
	extern int _do_plot();

	if( argc == 3 ) {
		pt_color = int_spop();
		SetColor( pt_color );
	}

	pt_y2 = int_spop();
	pt_x2 = int_spop();

	CoMap( &pt_x2, &pt_y2 );

	DrawWin( _do_plot );
}

_do_plot( x, y )
int x, y;
{
	gr_pxy[0] = pt_x2 + x;
	gr_pxy[1] = pt_y2 + y;
	gr_pxy[2] = gr_pxy[0];
	gr_pxy[3] = gr_pxy[1];

	v_pline( vdi_handle, 2, gr_pxy );
}

do_gtext()
{
	extern int _do_gtext();

	int style = int_spop();
	pt_str = str_spop() + STR_START;
	pt_y1 = int_spop();
	pt_x1 = int_spop();
	vst_effects( vdi_handle, style );

	CoMap( &pt_x1, &pt_y1 );

	DrawWin( _do_gtext );
}

_do_gtext( x, y )
int x, y;
{
	v_gtext( vdi_handle, pt_x1 + x, pt_y1 + y, pt_str );
}

do_drmode()
{
	int mode = int_spop();

	vswr_mode( vdi_handle, mode );
}

int pt_scaled;

do_fillpoly( argc, argv )
int argc;
pointer *argv;
{
	extern int _do_poly();

	/*
	 * Pop the address of the array off the stack
	 */
	int i;
	int *array_ptr;
	nodep arr_type;
	int arrsize;
	int num_vert;

	array_ptr = (int *) p_spop();
	arr_type = p_spop();

	num_vert = int_spop();

	if( num_vert > 128 )
		run_error( ER( 361, "maximum of 128 points allowed" ));

	arrsize = 2 * sizeof(int) * num_vert;

	if( calc_size( arr_type, TRUE ) < arrsize )
		run_error( ER(338, "illvert`Not enough vertices in array" ));

	do_undef( array_ptr, U_CHECK, arrsize );

	pt_x1 = num_vert;

	for( i=0; i<num_vert * 2; i++ ) {
		pts_buf[i] = array_ptr[i];
	}

	/*
	 * Convert the points to the proper co-ordinates
	 */
	for( i=0; i<num_vert * 2; i += 2 ) {
		CoMap( &array_ptr[i], &array_ptr[i+1] );
	}

	pt_str = (char *) array_ptr;
	pt_scaled = FALSE;

	DrawWin( _do_poly );

	for( i=0; i<num_vert * 2; i++ ) {
		array_ptr[i] = pts_buf[i];
	}
}

_do_poly( x, y )
int x, y;
{
	int *array;
	int i;

	array = (int *)pt_str;

	if( !pt_scaled && (x || y) ) {
		for( i=0; i<pt_x1 * 2; i += 2 ) {
			array[i] += x;
			array[i+1] += y;
		}
		pt_scaled = TRUE;
	}
	v_fillarea( vdi_handle, pt_x1, array );
}

do_gseedfill( argc, argv )
int argc;
rint *argv;
{
	extern int _do_seed();

	pt_y1 = int_spop();
	pt_x1 = int_spop();

	CoMap( &pt_x1, &pt_y1 );

	DrawWin( _do_seed );

}

_do_seed( x, y )
int x, y;
{
	v_contourfill( vdi_handle, pt_x1 + x, pt_y1 + y, -1 );
}


/*
 * The Turbo Pascal - GraphWindow routine
 */
do_window( argc, argv )
int argc;
rint *argv;
{
	int x1, y2;
	int x2, y1;
	int slot;
	extern WINDOW *pg_out;
	GRECT r;
	extern int charWidth, charHeight;

	y2 = int_spop();
	x2 = int_spop();
	y1 = int_spop();
	x1 = int_spop();

	if( !pg_out ) return;

	slot = WinSlot( pg_out );
	if( slot < 0 ) return;

	/*
	 * Move the default output window to (x1,y1,x2,y2);
	 */
	x1 *= charWidth;
	y1 *= charHeight;
	x2 *= charWidth;
	y2 *= charHeight;

	r.g_w = x2 - x1 + 1;
	r.g_h = y2 - y1 + 1;
	r.g_x = x1;
	r.g_y = y1;

	SizeWin( slot, &r );
}

/*
 * Generic 'do graphics mode' routine - the Turbo Pascal routines.
 * GraphMode -  sym 106        - set to 320 x 200
 * GraphColorMode - sym 107    - set to 320 x 200
 * HiRes - sym 108             - set to 640 x 200
 */
do_grmode(argc, argv, sym)
int	argc;
rint	*argv;
symptr	sym;
{
	register int	mode;
	int	saveid = sym_saveid(sym);

	printt0("do_grmode()\n");

	/* GraphMode */
	if (saveid == -106)
		_currSystem = CO_MEDRES;
	/* GraphColorMode */
	else if (saveid == -107)
		_currSystem = CO_MEDRES;
	/* HiRes */
	else
		_currSystem = CO_HIRES;
}


/*
 * The TextMode routine - just clears the output window
 */
do_textmode()
{
	if( pg_out ) {
		wclear( pg_out );
		wrefresh( pg_out );
	}
}

/*
 * The TurboPascal Palette command, among others...
 */


/* Some builtin symbol saveids */
#define	PALETTE		-110
#define	GRAPHBACKGND	-111
#define	HIRESCOLOUR	-112
#define	TEXTCOLOUR	-113
#define	TEXTBACKGND	-114

do_colour(argc, argv, sym)
int	argc;
rint	*argv;
symptr	sym;
{
	extern WINDOW *pg_out;
	WINDOW *win = pg_out;
	fileptr whfile;
	int	c;
	int	id; 
	int i;

	static int	ranges[] = {
		/* PALETTE */		5,
		/* GRAPHBACKGND */	16,
		/* HIRESCOLOUR */	16,
		/* TEXTCOLOUR */	16,
		/* TEXTBACKGND */	16
		};
	int	range;

	if( argc > 1 ) {
		whfile = p_spop();
		scr_check( whfile );
		win = whfile->desc.f_window;
	}

	c = int_spop();
	id = sym_saveid(sym);
	printt0("do_colour()\n");

	range = ranges[PALETTE - id];		/* tricky */
	if (c < 0 || c >= range)
		run_error(ER(297, "badcolor`Invalid colour %d (should be between 0 and %d)"), c, range);

	switch (sym_saveid(sym)) {
	case PALETTE:				/* Palette */
		/* Set up a palette */
		for( i=0; i<3; i++ ) {
			vs_color( vdi_handle, i+1, 
				&RGBVals[ThePalettes[c][i]][0] );
		}
		break;
	case GRAPHBACKGND:			/* GraphBackground */
		/* Set the background colour */
		vs_color( vdi_handle, 0, &RGBVals[c][0] );
		break;
	case HIRESCOLOUR:			/* HiResColor */
		/* Set the foreground colour */
		vs_color( vdi_handle, 1, &RGBVals[c][0] );
		break;
	case TEXTCOLOUR:			/* TextColor */
		if( !win ) return;
		reswflags( win, c << 8 );
		break;
	case TEXTBACKGND:			/* TextBackground */
		if( !win ) return;
		win->c_background = c;
		break;
	}
}

do_vid( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
	fileptr whfile;
	WINDOW *win = pg_out;

	if( argc ) {
		whfile = p_spop();
		scr_check( whfile );
		win = whfile->desc.f_window;
	}

	if( !win ) return;

	if( sym_saveid( sym ) == -133 )	/* lowvideo */
		wattron( win, A_BOLD );
	 else
		wattroff( win, A_BOLD );
}

do_gsetpallette()
{
	int rgb[3];
	int c_index;

	rgb[2] = int_spop();
	rgb[1] = int_spop();
	rgb[0] = int_spop();
	c_index = int_spop();

	vs_color( vdi_handle, c_index, rgb );

}

do_gfillpat()
{
	int pat = int_spop();

	int style;
	int interior;

	if( pat < 2 ) {
		style = 0;
		interior = pat;
	} else if( pat < 26 ) {
		style = pat - 2;
		interior = 2;
	} else if( pat < 38 ) {
		style = pat - 26;
		interior = 3;
	} else {
		run_error( ER(337, "illpatt`Illegal pattern number"));
	}
	vsf_style( vdi_handle, style );
	vsf_interior( vdi_handle, interior );
}
		
do_gmouseon()
{
	int flag = int_spop();
	MouseOn( flag );
}

initGraphics()
{
	/*
	 * Initialize the graphics system
	 */
	resetVDI();
}

/*
 * This routine is called upon initially running the program
 */

int IsRunning = FALSE;

resetGraphics()
{
	extern int InGemApp;
	extern long BufGraphFailed;
	MenuCtrl( FALSE );

	CurAspect = AspectRatio[ Getrez() ];

	BufGraphFailed = 0L;

#ifdef SAI
	if( sai_out == SAI_SCREEN )
#endif

	if( pg_out == (WINDOW *)0 ) {
		MkOutput();
		fil_output.desc.f_window = pg_out;
		fil_input.desc.f_window = pg_out;
		fil_output.f_flags |= FIL_OPEN;
		fil_input.f_flags |= FIL_OPEN;
		if( pg_out )
			wrefresh( pg_out );
	}

	_currGraph = pg_out;
	SetColor( BLACK );

	_currSystem = CO_PIXEL;

#ifndef SAI
	PgTitle( " Running " );
#endif

	/*
	 * Don't allow the user control of the mouse
	 */

	InGemApp = FALSE;

	ClrEvnt();

	UsrMenu = 0L;

	/*
	 * If we are not in debug mode, then top the output window
	 */
#ifndef SAI
	if( !(work_debug( curr_workspace ) & DBF_ON) ) {
		TopPgOut();
	}
#endif SAI

	/* There are no hidden windows yet */
	EditHidden = FALSE;

	/*
	 * Turn the cursor of the output window OFF
	 */

	if( pg_out )
		curOff( pg_out );

	resetVDI();

	IsRunning = TRUE;
}

finishGraphics()
{
	extern int InGemApp;

	if( InGemApp )
		do_gterm();

	/*
	 * The program is finished, completely
	 */
	PgTitle( LDS( 370, " Output " ) );
	/*
	 * Turn the cursor back on
	 */
	if( pg_out ) curOn( pg_out );

	/*
	 * And restore use of the mouse back to the user
	 */

	MenuCtrl( TRUE );

	ShowHidden();
	/*
	 * If we are not in debug mode, then top the edit window
	 */
#ifndef SAI
	if( !(work_debug( curr_workspace ) & DBF_ON) ) {
		TopCurEdit();
	}

	count_lines();
#endif

	resetVDI();
	IsRunning = FALSE;
}

/*
 * If a call to RemoveEditWindows has been made, then reveal the
 * hidden windows.  Also called by run_error()
 */
ShowHidden()
{
#ifndef SAI
	int i, x, y, w, h ;
	GRECT r;

	if( EditHidden ) {
		for( i=0; i<MAXWINDOWS; i++ ) {
			if( AW_KIND(i) == WIN_EDIT ) {
				rc_copy( &AW_WORK(i), &r );
				wind_open( AW_HANDLE(i), r.g_x, r.g_y, r.g_w,
						r.g_h );
				SetWork(i);
			}
		}
	}
	EditHidden = FALSE;
#endif

}

do_hidewin()
{
#ifndef SAI

	int i;
	GRECT r;

	/* If debugging is on, do nothing */
	if( work_debug( curr_workspace ) & DBF_ON ) return;

	/* If Edit windows are already hidden, return */
	if( EditHidden ) return;

	for( i=0; i<MAXWINDOWS; i++ ) {
		if( AW_KIND(i) == WIN_EDIT ) {
			wind_get( AW_HANDLE(i), WF_CURRXYWH, &r.g_x, &r.g_y,
					&r.g_w, &r.g_h );
			rc_copy( &r, &AW_WORK(i) );
			wind_close( AW_HANDLE(i) );
		}
	}
	flush_events();
	EditHidden = TRUE;
#endif SAI

}

suspendGraphics()
{
	extern int InGemApp;

	/*
	 * The program has been suspended
	 */
	MenuCtrl( TRUE );

	/*
	 * If there is a user-menu, restore ALICE's
	 */
	if( UsrMenu )
		menu_bar( UsrMenu->tree, 0 );

#ifndef SAI
	if( InGemApp )
		menu_bar( gl_menu, 1 );
#endif

	IsRunning = FALSE;

}

#ifndef SAI
resumeGraphics()
{
	extern int InGemApp;

	/*
	 * The program has been resumed
	 */

	PgTitle( LDS( 371, " Running " ) );

	/*
	 * If there is a User Menu, then restore it
	 */
	if( InGemApp ) {
		/*
		 * Remove ours...
		 */
		menu_bar( gl_menu, 0 );

		/*
		 * And put up his...
		 */
		if( UsrMenu )
			menu_bar( UsrMenu->tree, 1 );
	}

	/*
	 * If we are not running a GEM Application, don't give access
	 * to the mouse
	 */
	if( !InGemApp )
		MenuCtrl( FALSE );

	IsRunning = TRUE;
}
#endif SAI

do_scrfunc( argc, argv )
int argc;
rint *argv;
{
	extern WINDOW *pg_out;
	WINDOW *win = pg_out;
	int rv;
	extern int mon_is_colour, mono_adapter;
	int y,x;
	GRECT	winrect;
	fileptr whfile;
	int slot;
	extern int charWidth, charHeight;

	if( argc > 1 ) {
		whfile = p_spop();
		scr_check( whfile );
		win = whfile->desc.f_window;
	}

	if( win ) {
		slot = WinSlot( win );
		getyx( win, y, x );
	} else
		slot = -1;

	/* Switch on the type of scrfunc */
	switch( int_stop ) {
		case 1:
			rv = win ? win->_maxy : 0;
			break;
		case 2:
			rv = win ? win->_maxx : 0;
			break;
		case 3:
			if( slot < 0 )
				rv = 0;
			else {
				rv = AW_WORK(slot).g_h;
				MapWin( &x, &rv );
			}
			break;
		case 4:
			if( slot < 0 )
				rv = 0;
			else {
				rv = AW_WORK(slot).g_w;
				MapWin( &rv, &y );
			}
			break;
		case 5:
			rv = GemCols[ Getrez() ];
			break;
		case 6:
			rv = Getrez();
			break;
		case 7:
			rv = TRUE;
			break;
		case 8:
			rv = (GemCols[ Getrez() ]) > 2;
			break;
		case 9:
			if( slot < 0 )
				rv = 0;
			else {
				rv = AW_WORK(slot).g_x;
				MapWin( &rv, &y );
			}
			break;
		case 10:
			if( slot < 0 )
				rv = 0;
			else {
				rv = AW_WORK(slot).g_y;
				MapWin( &x, &rv );
			}
			break;
		case 11:
			rv = vdi_handle;
			break;
		case 12:
			rv = _currSystem;
			break;
		case 13:
			rv = _currColor;
			break;
		case 14:
			if( slot < 0 )
				rv = -1;
			else 
				rv = AW_HANDLE(slot);
			break;
		case 15:
			rv = scr_area.g_w;
			MapWin( &rv, &y );
			break;
		case 16:
			rv = scr_area.g_h;
			MapWin( &x, &rv );
			break;
		case 17:
			rv = charHeight;
			MapWin( &x, &rv );
			break;
		case 18:
			rv = charWidth;
			MapWin( &rv, &y );
			break;
		case 19:
			if( slot >= 0 )
				rv = AW_BITS(slot) != (MFDB *)0;
			else
				rv = 0;
			break;
		case -1:
			if( !win )
				rv = 0;
			else
				rv = y;
			break;
		case -2:
			if( !win )
				rv = 0;
			else
				rv = x;
			break;
		default:
			scr_err(int_stop);
	}
	*argv = rv;
}

#ifndef SAI
viewGraphics() {}
#endif SAI

checkScreen() {}
char RunModeInfo;

char ModesInfo;


resetVDI()
{
	SetColor( BLACK );
	vswr_mode( vdi_handle, 1 );	/* REPLACE mode */
	vst_effects( vdi_handle, 0 );	/* No italics, etc */
	vsf_style( vdi_handle, 1 );	/* No fill pattern */
	vsf_interior( vdi_handle, 0 );

}
