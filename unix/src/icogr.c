/*
 * msdos graphics
 */

#include "alice.h"
#include <curses.h>
#include "interp.h"
#include "extramem.h"
#include "workspace.h"
#include "dbflags.h"


#ifdef DB_GRAPHICS
# define PRINTT
#endif
#include "printt.h"

typedef	long int32;

extern WINDOW	*pg_out;
static WINDOW	*SavedPgOut;
static WINDOW	*RunPgOut;

extern Boolean	restoreScreen();

#ifdef LARGE

#define ex_alloc malloc
#define ex_free(obj,size)	 mfree(obj)

#endif


#ifndef SAI
exptr	ScreenBuf	= 0;
#endif

struct rect {
	int	x;
	int	y;
	int	w;
	int	h;
} Win;

static int cur_X, cur_Y;
static rfloat cur_Xturt, cur_Yturt;
static rfloat cur_DXturt, cur_DYturt;
static int cur_Aturt;
static int cur_COL;
static int DrawTurtle;
static int PenDown = TRUE;

#define	SETWIN(x1,y1,w1,h1)	(Win.x=(x1),Win.y=(y1),Win.w=(w1),Win.h=(h1))

/*
 * Run modes
 */
#define	BW40		0			/* text modes */
#define	C40		0
#define	BW80		0
#define	C80		0
#define	MEDCOLOUR	1			/* graphics modes */
#define	HIRES		2
#define MONO		0			/* others */
#define	USER_DEFD	0
#define	CURSES		2


#define NUM_COLOURS 4

#define COLOUR_MAP_BASE   0xf900

#define ONE_BPP   1
#define TWO_BPP   2

#define SCREEN_PORT     0xf983
#define LO_RES          0xfb
#define HI_RES          0x04


#define	INFO(m)		(&ModesInfo[(m)])

/* 
 define	TEXTMODE(m)	(m == MONO || ((m) >= BW40 && (m) <= C80))
 define	GRMODE(m)	(((m) >= MEDCOLOUR && (m) <= HIRES) || (m) == USER_DEFD)
 */
#define	HIMODE(m)	((m) != MEDCOLOUR)
/* define	MEDMODE(m)	((m) >= MEDCOLOUR && (m) <= MEDBW) */
/* define	KNOWNMODE(m)	((m) >= BW40 && (m) <= MONO) */

/* video "modes" are 0 for hires and 1 for lores */

int	RunMode;				/* runtime video mode */
		/* Alice editor video mode */
#define AliceVMode 0
int	LastTextMode;				/* last text video mode */

#define PXPERLINE 10
#define BYTESPERLINE 80

struct modeInfo	ModesInfo[] = {
/* BW80 */	{ 0,	1,	22,	80,	639,	22*PXPERLINE-1,	1 },
/* MEDCOLOUR */	{ 1,	3,	24,	40,	319,	24*PXPERLINE-1,	0 },
/* CURSES24 */	{ 0,	1,	24,	80,	639,	24*PXPERLINE-1,	0 },
};

struct modeInfo UDDefault = { 0, 1, 22, 80, 639, 219, PXPERLINE };

int	line();
int	slowline();
int	(*LineFunc)()	= line;

struct modeInfo	*RunModeInfo;			/* current runtime mode info */

#define	SIGN(x)		((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))
#define	ABS(x)		((x) >= 0 ? (x) : -(x))

static int	Palette;			/* current palette */
	/* current background */
#define Background	colpal[0]
#define Foreground	colpal[2]
static int	GrStatus	= 0;
/*
 * Bits in GrStatus
 */
#define	GRUSED		1			/* graphics used */
#define	GRONSCREEN	2			/* screen in graphics mode */
#define	GRBUFFERED	4			/* off screen buffer exists */

static int init_colpal[NUM_COLOURS];

/* HARD CODED ADDRESSES WITHIN STRUCT. REALL KLUDGE */
#define st_colour_map @(unsigned int *)108
#define st_monitor_info (@(unsigned int *)110)

initGraphics()
{
	unsigned int i;
	unsigned int *int60_ptr;
	extern int mon_is_colour;
	unsigned int monitor_type, mask;
	extern unsigned ESSeg;

	/* get segment of system table */

	set_extra_segment (0);
	int60_ptr = 0x60 * 4 + 2;
	set_extra_segment (@int60_ptr);

	/* extract entries from system table.  See #defines above */

        mask = st_colour_map;
	monitor_type =  3 - ((st_monitor_info >> 2) & 0x03) + 1;

	/* restore extra segment to alice extra segment */
	set_extra_segment( ESSeg );

	mon_is_colour = monitor_type != 1;
	for( i = 0; i <= NUM_COLOURS; i ++ ) {
		init_colpal[i] = mask & 0xf;
		mask = mask >> 4;
		}
}

/* colour map - 108 -- monitor_info 110 */

static int colpal[NUM_COLOURS];

/*
 * resetGraphics() is called from run_prog(), just before running a program
 */
resetGraphics()
{
	int heigh;
	int i;
	printt1("resetGraphics %x\n", GrStatus);

#ifndef SAI
	if (GrStatus)				/* "can't happen" */
		finishGraphics();
#endif
	GrStatus = 0;

	/* normal colours and mode */
	normalcols();
	blk_move( colpal, init_colpal, sizeof(colpal) );

	RunPgOut = SavedPgOut = pg_out;
	RunMode = LastTextMode = 0;
	RunModeInfo = INFO(RunMode);
	heigh = RunModeInfo->pixHeight = pg_out->_maxy * PXPERLINE;
	SETWIN(0, pg_out->_begy * PXPERLINE, 639, heigh );
	/* home the turtle */
	Fhome();
	DrawTurtle = FALSE;
	cur_COL = 1;
}

change_pg_out(newpg)
WINDOW *newpg;
{
	SavedPgOut = pg_out = newpg;
}

/*
 * suspendGraphics() is called from do_suspend(), when the interpreter state
 * is suspended, due to a runtime error, break hit, breakpoint, etc.
 */
suspendGraphics()
{
#ifndef SAI
	printt1("suspendGraphics %x\n", GrStatus);

	checkModeChanged();

	if ((GrStatus & (GRUSED | GRONSCREEN)) == (GRUSED | GRONSCREEN)) {
		if (!(GrStatus & GRBUFFERED)) {
			GrStatus |= GRBUFFERED;
			allocScreenBuf();
			}
		saveScreen();
		GrStatus &= ~GRONSCREEN;
		restoreAliceEditing(TRUE);
		redraw_errline();
		}
#endif
}

checkModeChanged()
{
#ifdef Not
	int	vmode	= dosGetVideoMode();

	if (((GrStatus & GRONSCREEN) && vmode != RunModeInfo->vmode) ||
	     (!(GrStatus & GRONSCREEN) && vmode != AliceVMode)) {
		GrStatus |= GRUSED | GRONSCREEN;
		changeRunMode(vmode);
		}
#endif
}

/*
 * Called from slmessage, this returns TRUE if its okay to write
 * to the screen.
 */
int
okayWithGraphics()
{
	return !(GrStatus & GRONSCREEN);
}

/*
 * resumeGraphics() is called from resume(), when the suspended interpreter
 * is about to be resumed, due to single step, super step, or continue.
 * Unfortunately its also called from do_superstep().  Mumble.
 *
 * If graphics were on and we're not in single step mode (e.g. "continue")
 * restore the graphics.
 *
 * If we *are* in single step mode, do nothing -- statements which affect
 * the screen will first call checkScreen() (if a text operation) or
 * checkGraphics() (if a graphics operation).
 */
resumeGraphics(step)
int	step;
{
#ifndef SAI
	extern int	imm_mode;

	printt1("resumeGraphics() %x\n", GrStatus);

	if (((GrStatus & (GRUSED | GRBUFFERED | GRONSCREEN)) ==
			 (GRUSED | GRBUFFERED | 0)) &&
	    !(step & (STEP_SINGLE | DBF_CURSOR)) && !imm_mode) {
		GrStatus |= GRONSCREEN;
		restoreScreen();
		}
	/* should probably reset pxperline */
#endif
}

/*
 * finishGraphics() is called from terminate(), when a program terminates.
 */
finishGraphics()
{
#ifndef SAI
	printt1("finishGraphics() %x\n", GrStatus);

	checkModeChanged();

	if (pg_out && pg_out != SavedPgOut) {
#ifdef msdos
		if (pg_out->_flags & _DOSWIN)
			delwin(pg_out);
#endif
		pg_out = SavedPgOut;
		}

	if (GrStatus & GRUSED) {
		if (GrStatus & GRONSCREEN) {
			GrStatus &= ~GRONSCREEN;
			restoreAliceEditing(FALSE);
			}

		if (GrStatus & GRBUFFERED)
			freeScreenBuf();
		}
	redraw_errline();
	GrStatus = 0;
#endif
}

/* 
 * checkGraphics() is called from every graphics function, just before
 * it dirties the graphics screen.
 */
checkGraphics()
{
	printt1("checkGraphics() %x\n", GrStatus);

	checkModeChanged();

#ifndef SAI
	/* If in graphics mode, but buffer not displayed, display it */
	if ((GrStatus & (GRUSED | GRBUFFERED | GRONSCREEN)) ==
		       (GRUSED | GRBUFFERED | 0))
		restoreScreen();
#endif

	GrStatus |= GRUSED | GRONSCREEN;
}

#ifdef msdos
# ifndef SAI
/*
 * viewGraphics() displays the graphics buffer, waits for a key press,
 * and restores the text screen.
 */
viewGraphics()
{
	printt1("viewGraphics() %x\n", GrStatus);
 
	if (!(GrStatus & (GRUSED | GRBUFFERED)) || !ScreenBuf)
		error(ER(298, "cantview`There is no graphics buffer to view"));

	if (restoreScreen())
		keyget();
	normalcols();
	refresh_scr();
}
# endif
#endif

#ifndef SAI
restoreAliceEditing(force)
Boolean	force;
{
	printt1("restoreAliceEditing(%d)\n", force);

	normalcols();

	if (force || work_debug(curr_workspace) & DBF_ON) {
		redraw_status = TRUE;
		display(TRUE);
		}
}
#endif

normalcols()
{
	changeVMode(AliceVMode);
	setColours(init_colpal);
}

/*
 * Called before writing any curses output, checkScreen() will flip
 * to the graphics screen if it was in operation.
 */
checkScreen()
{
	printt0("checkScreen()\n");

	if( RunMode == MEDCOLOUR )
		run_error( ER(330,"textgr`Text Output in colour graphics mode"));

#ifdef msdos
	if (GrStatus & GRUSED)
		checkGraphics();
	else
		checkModeChanged();
#endif

	if (GrStatus & GRUSED && pg_out && RunMode != CURSES && RunPgOut)
		pg_out = RunPgOut;
}


#ifndef SAI

/*
 * Routines for manipulating graphics buffer
 */

saveScreen()
{
#ifdef NOT
	printt2("saveScreen() %lx, %u\n", ScreenBuf, RunModeInfo->screenSize);

	if (ScreenBuf)
		dos_blk(ScreenBuf, 0, CGASEG, RunModeInfo->screenSize);
#endif
}

Boolean
restoreScreen()
{
	printt2("restoreScreen() %lx, %u\n", ScreenBuf, RunModeInfo->screenSize);

	changeVMode(RunModeInfo->vmode);
	setColours(colpal);
#ifdef msdos
	if (ScreenBuf) {
		dos_blk(0, CGASEG, ScreenBuf, RunModeInfo->screenSize);
		return TRUE;
		}
	else {
		warning(ER(299, "noscrmem`Sorry, previous graphics screen lost (not enough memory to keep it)"));
		return FALSE;
		}
#endif
}

allocScreenBuf()
{
#ifdef NOT
	extern exptr	ex_alloc();
	int	size = RunModeInfo->screenSize;

	printt1("allocScreenBuf(), %u\n", RunModeInfo->screenSize);

	if (size)
		ScreenBuf = ex_alloc(size);
	GrStatus |= GRBUFFERED;
#endif
}

freeScreenBuf()
{
	printt2("freeScreenBuf() %lx %u\n", ScreenBuf, RunModeInfo->screenSize);
#ifdef Not

	if (ScreenBuf) {
		ex_free(ScreenBuf, RunModeInfo->screenSize);
		ScreenBuf = 0;
		}
#endif
	GrStatus &= ~GRBUFFERED;
}

#endif

extern int	mono_adapter;		/* set by curses */
extern int	multi_monitor;		/* set by user in main.c */
static int CurVMode = 0;		/* current video mode, 1 for 40 column*/

changeVMode(vmode)
int	vmode;
{
	printt1("changeVMode(%d)\n", vmode);

	if( vmode != CurVMode ) {

		if ( vmode )
       			io_out (SCREEN_PORT, (io_in (SCREEN_PORT) & LO_RES));
	 	else
        		io_out (SCREEN_PORT, (io_in (SCREEN_PORT) | HI_RES));
		}

	/* code to change the screen video mode */
	CurVMode = vmode;

}

/*
 * Graphics Routines!
 */

#define	BLACK		0
#define	WHITE		7			/* low intensity white */

do_grmode(argc, argv, sym)
int	argc;
rint	*argv;
symptr	sym;
{
	register int	mode;
	int	saveid = sym_saveid(sym);

	printt0("do_grmode()\n");

#ifdef msdos
	if (mono_adapter /* && !multi_monitor */)
		run_error(ER(310, "needgr`graphics requires graphics adapter"));
#endif

	checkGraphics();

	/* set defaults a la silly Turbo */
	Palette = 0;

	if (saveid == -106) {
		mode = BW80;
		}
	else if (saveid == -107)
		mode = MEDCOLOUR;
	else {
		mode = HIRES;
		}

#ifdef SIEMENS
	LineFunc = slowline;
#endif

	changeRunMode(mode);
	changeVMode(RunModeInfo->vmode);
	setColours(colpal);
	clear_screen();

	if( mode = 0 ) {
		int heigh;
		heigh = RunModeInfo->pixHeight = pg_out->_maxy * PXPERLINE;
		SETWIN(0, pg_out->_begy * PXPERLINE, 639, heigh );
		}
	 else {
		SETWIN(0, RunModeInfo->startline*PXPERLINE,
			RunModeInfo->pixWidth, RunModeInfo->pixHeight);
		}
	Fhome();
}

changeRunMode(mode)
int	mode;
{
	extern WINDOW *doswin();
#ifndef SAI
	Boolean	buffered = GrStatus & GRBUFFERED;
#endif

	printt1("changeRunMode(%d)\n", mode);

#ifndef SIEMENS
	if (mode != USER_DEFD)
		LineFunc = line;
#endif

	if (RunMode != mode || RunMode == USER_DEFD) {
#ifndef SAI
		if (buffered)
			freeScreenBuf();
#endif

		RunMode = mode;
		RunModeInfo = INFO(RunMode);

#ifndef SAI
		if (buffered)
			allocScreenBuf();
#endif
		}

	if (RunMode == CURSES) {
		RunPgOut = pg_out = stdscr;
		wclear(stdscr);
		}
	else
		newPg_out(FALSE, RunModeInfo->rows, RunModeInfo->columns,
			RunModeInfo->startline, 0);
}

newPg_out(curses, h, w, y, x)
bool curses;		/* do we want a curses window */
int h, w, y, x;		/* height, width, start y, start x */
{
#ifdef msdos
	if (pg_out->_flags & _DOSWIN)
		delwin(pg_out);
#endif

	pg_out = subwin(stdscr, h, w, y, x);
	if (!pg_out)
		run_error(ER(316, "nowinmem`Not enough memory to create output window"));

	RunPgOut = pg_out;

	scrollok(pg_out, TRUE);
}

do_textmode(argc, argv)
int	argc;
rint	*argv;
{
	int	mode;

	checkGraphics();

	printt0("do_textmode()\n");

#ifdef Not
	if (argc == 1) {
		mode = int_spop();

		if (mode == -1) 
			mode = CURSES;
		else if (!TEXTMODE(mode))
			run_error(ER(295, "badtextmode`Invalid text mode %d"),
				  mode);

		LastTextMode = mode;
		}
	else
		mode = LastTextMode;


	changeRunMode(mode);
	changeVMode(RunModeInfo->vmode);
#endif
}

/* Some builtin symbol saveids */
#define	PALETTE		-110
#define	GRAPHBACKGND	-111
#define	HIRESCOLOUR	-112
#define	TEXTCOLOUR	-113
#define	TEXTBACKGND	-114

static int the_pals[4][3] = {
{ 2, 1, 3 },
{ 6, 5, 7 },
{ 10, 9, 11 },
{ 14, 13, 15 }
};

do_colour(argc, argv, sym)
int	argc;
rint	*argv;
symptr	sym;
{
	int	c	= int_spop();
	int	id	= sym_saveid(sym);
	static int	ranges[] = {
		/* PALETTE */		4,
		/* GRAPHBACKGND */	16,
		/* HIRESCOLOUR */	16,
		/* TEXTCOLOUR */	32,
		/* TEXTBACKGND */	8
		};
	int	range;

	printt0("do_colour()\n");

	range = ranges[PALETTE - id];		/* tricky */
	if (c < 0 || c > range)
		run_error(ER(297, "badcolour`Invalid colour %d (should be between 0 and %d)"), c, range);

	switch (sym_saveid(sym)) {
	case PALETTE:				/* Palette */
		{
		Palette = c;
		Foreground = the_pals[Palette][2];
		colpal[1] = the_pals[Palette][0];
		colpal[3] = the_pals[Palette][1];
		monocheck();
		checkGraphics();
		setColours(colpal);
		}
		break;
	case GRAPHBACKGND:			/* GraphBackground */
		Foreground = c;
		monocheck();
		checkGraphics();
		setColours(colpal);
		break;
	case HIRESCOLOUR:			/* HiResColor */
		Background = c;
		monocheck();
		checkGraphics();
		setColours(colpal);
		break;
	case TEXTCOLOUR:			/* TextColor */

		if (c & 8) 
			wattron( pg_out, A_BOLD );
		 else
			wattroff(pg_out, A_BOLD);
		if( c == 1 )
			wattron( pg_out, A_UNDERLINE );
		 else
			wattroff( pg_out, A_UNDERLINE );
		break;
	case TEXTBACKGND:			/* TextBackground */
		/* not defined */
		break;
	}
}

do_vid( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
	if( sym_saveid( sym ) == -133 )	/* lowvideo */
		wattroff( pg_out, A_BOLD );
	 else
		wattron( pg_out, A_BOLD );
}

/*
 * setColours() -- update palette and background colours
 *
 * Stupid BIOS.  To select the high intensity palette, you must instead
 * select a high intensity background.
 */
#define	INTENSE		16
#define	PAL		1
#define	BG		0

setColours(pal)
int *pal;
{
	int i;
	printt0("setColours\n");

	/* CHANGE THIS (OR CALLS TO IT) for colours */

#ifdef msdos
	dosColour(PAL, Palette & 1);
	dosColour(BG, (Palette > 1 ? INTENSE : 0) | Background);
#endif

	for( i = 0; i < NUM_COLOURS; i++ )
		io_out( COLOUR_MAP_BASE + 2*i, pal[i] & 0x0f );
}

/*
 * monocheck
 * in Monochrome mode, colours 0 and 1 must match, as must 2 and 3
 */

monocheck()
{
	if( RunModeInfo->vmode == 0 && !mon_is_colour ) {
		colpal[1] = colpal[0];
		colpal[3] = colpal[2];
		}
}

#define	GRAPHWINDOW	-105
#define	TEXTWINDOW	-115

do_setpal( )
{
	unsigned ourcol = int_spop();
	unsigned whatcol = int_stop;
	if( ourcol > 15 )
		run_error(ER(297, ""), ourcol, 15);
	if( whatcol > 3 )
		run_error(ER(297, ""), whatcol, 3);
	colpal[ whatcol ] = ourcol;
	monocheck();
	setColours(colpal);
}

do_window(argc, argv, sym)
int	argc;
rint	*argv;
symptr	sym;
{
	int	y2	= int_spop();
	int	x2	= int_spop();
	int	y1	= int_spop();
	int	x1	= int_spop();
	int	id	= sym_saveid(sym);

	printt0("do_window()\n");

	if (x1 > x2 || y1 > y2 || x1 < 0 || y1 < 0 ||
	    (id == TEXTWINDOW && (x2 > RunModeInfo->columns ||
	     (y2 > RunModeInfo->rows))) ||
            (id == GRAPHWINDOW && (x2 > RunModeInfo->pixWidth ||
				   y2 > RunModeInfo->pixHeight)))
		run_error(ER(294, "badwin`Invalid window coordinates"));

	if (id == TEXTWINDOW)
		newPg_out(TRUE, y2-y1+1, x2-x1+1, y1 - 1, x1 - 1);
	else
		SETWIN(x1, y1, x2-x1, y2-y1);
}

/*
 * Line drawing with clipping using Cohen-Sutherland clipping algorithm
 *
 * See "Fundamentals of Interactive Computer Graphics" by Foley and Van Dam,
 * p. 146 for more information.
 */
#define	TOP		0
#define	BOT		1
#define	LEFT		2
#define	RIGHT		3
#define	OUTCODE(x,y)	((((y) < 0) << TOP) | (((y) > Win.h) << BOT) |\
			(((x) < 0) << LEFT) | (((x) > Win.w) << RIGHT))
#define	REJECT(o1,o2)	(((o1) & (o2)) != 0)
#define	ACCEPT(o1,o2)	(((o1) | (o2)) == 0)

do_draw(argc, argv)
int	argc;
rint	*argv;
{
	int	colour	= int_spop();
	int	y2	= int_spop();
	int	x2	= int_spop();
	register int	y1	= int_spop();
	register int	x1	= int_spop();


	printt5("do_draw(%d,%d, %d,%d, %d)\n", x1, y1, x2, y2, colour);

	/* Negative colour denotes xor line drawing */
	checkModeAndColour(ABS(colour));
	clipdraw( x1, y1, x2, y2, colour );
}

clipdraw( x1, y1, x2, y2, colour )
int x1, y1, x2, y2, colour;
{		
	register int	out1;		/* outcodes of endpoints */
	int out2;

	out1 = OUTCODE(x1, y1);
	out2 = OUTCODE(x2, y2);

	for (;;) {
		if (REJECT(out1, out2))		/* trivially reject */
			return;
		if (ACCEPT(out1, out2))		/* trivially accept */
			break;
		/*
		 * Subdivide the line since at most one endpoint is inside.
		 *
		 * First, if P1 is inside window, exchange points 1 and 2
		 * and their outcodes to guarantee that P1 is outside window.
		 */
		if (out1 == 0) {
			int	t;

			t = out1;	out1 = out2;	out2 = t;
			t = x1;		x1 = x2;	x2 = t;
			t = y1;		y1 = y2;	y2 = t;
		}
		/*
		 * Now perform a subdivision, move P1 to the intersection
		 * point; use the formulas
		 * y = y1 + slope * (x-x1),
		 * x = x1 + (1/slope) * (y-y1).
		 *
		 * Note we must cast the intermediate result to 32 bits
		 * to avoid overflow problems.
		 */
		if (out1 & (1 << TOP)) {
			x1 += (int32)(x2 - x1) * (0 - y1) / (y2 - y1);
			y1 = 0;
			}
		else if (out1 & (1 << BOT)) {
			x1 += (int32)(x2 - x1) * (int32)(Win.h - y1) /
						(int32)(y2 - y1);
			y1 = Win.h;
			}
		else if (out1 & (1 << LEFT)) {
			y1 += (int32)(y2 - y1) * (0 - x1) / (x2 - x1);
			x1 = 0;
			}
		else if (out1 & (1 << RIGHT)) {
			y1 += (int32)(y2 - y1) * (Win.w - x1) / (x2 - x1);
			x1 = Win.w;
			}
		else {
			printt6("do_draw: (%d,%d)[%x] (%d,%d)[%x]!\n",
				x1, y1, out1, x2, y2, out2);
			}

		/* Now recalculate the outcode for the new x1 */
		out1 = OUTCODE(x1, y1);
		}

	checkGraphics();

	/* Draw the resulting line */
	line(Win.x + x1, Win.y + y1, Win.x + x2, Win.y + y2, colour);
}


#define PLOT(x,y,c)	line( x, y, x, y, c )

#ifdef msdos
/*
 * line -- draw line from (x1,y1) to (x2,y2), using Bresenham's algorithm
 *
 * Slow C version
 *
 * For more information see "Smalltalk-80: The Language and Its
 * Implementation", p. 352
 */
slowline(x1, y1, x2, y2, colour)
int	x1;
int	y1;
int	x2;
int	y2;
int	colour;
{
	register int	x;
	register int	y;
	register int	p;
	register int	i;
	int	t;
	int	dx;
	int	dy;
	int	px;
	int	py;

	printt5("line(%d,%d, %d,%d, %d)\n", x1, y1, x2, y2, colour);

	x = x1;
	y = y1;

	t  = x2 - x1;
	dx = SIGN(t);
	py = ABS(t);

	t = y2 - y1;
	dy = SIGN(t);
	px = ABS(t);

	PLOT(x, y, colour);			/* first point */

	if (py > px) {				/* more horizontal */
		p = py / 2;
		for (i = 0; i < py; i++) {
			x += dx;
			if ((p -= px) < 0) {
				y += dy;
				p += py;
				}
			PLOT(x, y,  colour);
			}
		}
	else {					/* more vertical */
		p = px / 2;
		for (i = 0; i < px; i++) {
			y += dy;
			if ((p -= py) < 0) {
				x += dx;
				p += px;
				}
			PLOT(x, y, colour);
			}
		}
}

#endif msdos
/*
 * Pixel drawing macros, medium and high resolution.  Writes directly to
 * screen video memory.
 *
 * The screen is organized as two banks of memory, one bank for the even scan
 * lines, the other for odd scan lines.  The first bank starts at $B80000,
 * the second at $B80000 + 8K.  Each bank contains 100 lines of 80 bytes
 * each, representing 640 pixels in high resolution monochrome mode, and
 * 320 pixels in medium resolution colour or grey scale mode.
 *
 * In high resolution mode, a byte represents 8 pixels on the screen,
 * with the MSB being the leftmost pixel, and the LSB being the rightmost
 * pixel; in medium res. mode, the two most significant bits encode
 * one of four colours for the leftmost pixel, etc.
 *
 * This super fast assembler version first computes constants and then
 * dispatches to the various assembler line drawing routines.
 */

/* Constants for assembler routines */
unsigned char	LStartMask;	/* starting mask */
unsigned char	LHiPixelMask;	/* mask for high pixel in byte */
unsigned char	LColour;	/* colour in every pixel pos in byte */

unsigned char	LXor;		/* not used */

unsigned char	LBitsPerPixel;	/* 1 for hires, 2 for med. */
int	LStartOffset;		/* offset of first byte touched */
int	LStartYDelta;		/* first Y increment (80 or 8192-80) */
int	LPX;			/* x DDA increment */
int	LPY;			/* y DDA increment */
int	LCount;			/* points to draw */

/*
 * line -- draw line from (x1,y1) to (x2,y2), using Bresenham's algorithm
 *
 * Compute constants and dispatch to assembler routines.
 */
line(x1, y1, x2, y2, colour)
register int	x1;
int	y1;
int	x2;
int	y2;
int	colour;
{
	register int	t;
	int	dx;
	int	dy;

	/* Make p1 the leftmost point */
	if (x1 > x2) {
		t = x1; x1 = x2; x2 = t;
		t = y1; y1 = y2; y2 = t;
		}
	/* If vertical, make p1 the smaller point */
	else if (x1 == x2 && y1 > y2) {
		t = y1; y1 = y2; y2 =t;
		}

	t  = x2 - x1;
	dx = SIGN(t);
	LPY = ABS(t);

	t = y2 - y1;
	dy = SIGN(t);
	LPX = ABS(t);

	if (colour < 0) {		/* xor line drawing? */
		LXor = 0xff;		/* (although its not yet implemented) */
		colour = -colour;
		}
	else
		LXor = 0;

	LStartYDelta = BYTESPERLINE;
	LStartOffset = BYTESPERLINE*y1;
	if (HIMODE(RunMode)) {		/* high res 2 colour */
		LStartOffset += x1 >> 3;
		LBitsPerPixel = 1;
		LHiPixelMask = 0x01;
		LStartMask = 1 << (x1 & 7);
		if (colour > 0)
			colour = 0xff;
		}
	else {				/* medium res 4 colour */
		LStartOffset += x1 >> 2;
		LBitsPerPixel = 2;
		LHiPixelMask = 0x03;
		LStartMask = 3 << (2 * (x1 & 3));
		colour |= (colour << 2);
		colour |= (colour << 4);
		}
	LColour = colour;

	if (dx > 0) {
		if (dy > 0) {
			if (LPY <= LPX)		/* 45 <= theta < 90 */
				l_nne();
			else			/* 0 < theta < 45 */
				l_ene();
			}
		else if (dy < 0) {
			if (LPY <= LPX)		/* -45 >= theta > -90 */
				l_sse();
			else	 		/* 0 > theta > -45 */
				l_ese();
			}
		else /* dy == 0 */		/* horizontal line */
			l_hor();
		}
	else /* dx == 0 */			/* vertical line */
		l_vert();
}

checkModeAndColour(colour)
int	colour;
{
#ifdef not
	if (!GRMODE(RunMode))
		run_error(ER(296, "badgr`Not in graphics mode"));
#endif
	if (colour < 0 || colour > RunModeInfo->maxColour)
		run_error(ER(297, "badcolour`Invalid colour %d (should be between 0 and %d)"), colour, RunModeInfo->maxColour);
}

do_plot(argc, argv)
int	argc;
rint	*argv;
{
	int	colour		= int_spop();
	register int	y	= int_spop();
	register int	x	= int_spop();

	printt3("do_plot(%d,%d, %d)\n", x,  y, colour);

	checkModeAndColour(colour);

	checkGraphics();

	if (OUTCODE(x,y) == 0) {
		PLOT(Win.x + x, Win.y + y, colour);
		}
}
float sin_table[] = {
  0.0,
  0.017452406437283514,
  0.034899496702500969,
  0.052335956242943835,
  0.069756473744125298,
  0.087155742747658156,
  0.104528463267653460,
  0.121869343405147450,
  0.139173100960065430,
  0.156434465040230860,
  0.173648177666930330,
  0.190808995376544810,
  0.207911690817759300,
  0.224951054343864950,
  0.241921895599667720,
  0.258819045102520730,
  0.275637355816999150,
  0.292371704722736760,
  0.309016994374947450,
  0.325568154457156740,
  0.342020143325668830,
  0.358367949545300400,
  0.374606593415912180,
  0.390731128489273870,
  0.406736643075800330,
  0.422618261740699630,
  0.438371146789077670,
  0.453990499739547010,
  0.469471562785891010,
  0.484809620246337220,
  0.5,
  0.515038074910054530,
  0.529919264233205210,
  0.544639035015027410,
  0.559192903470747100,
  0.573576436351046400,
  0.587785252292473440,
  0.601815023152048570,
  0.615661475325658620,
  0.629320391049837810,
  0.642787609686539690,
  0.656059028990507720,
  0.669130606358858680,
  0.681998360062498940,
  0.694658370458997690,
  0.707106781186547990,
  0.719339800338651610,
  0.731353701619170940,
  0.743144825477394730,
  0.754709580222772480,
  0.766044443118978610,
  0.777145961456971340,
  0.788010753606722500,
  0.798635510047293360,
  0.809016994374947980,
  0.819152044288992260,
  0.829037572555042240,
  0.838670567945424490,
  0.848048096156426460,
  0.857167300702112730,
  0.866025403784439080,
  0.874619707139396310,
  0.882947592858927520,
  0.891006524188368410,
  0.898794046299167520,
  0.906307787036650400,
  0.913545457642601370,
  0.920504853452440840,
  0.927183854566787950,
  0.933580426497202250,
  0.939692620785908690,
  0.945518575599317220,
  0.951056516295153990,
  0.956304755963035900,
  0.961261695938319240,
  0.965925826289068640,
  0.970295726275996800,
  0.974370064785235530,
  0.978147600733805860,
  0.981627183447664100,
  0.984807753012208300,
  0.987688340595138030,
  0.990268068741570490,
  0.992546151641322180,
  0.994521895368273510,
  0.996194698091745670,
  0.997564050259824240,
  0.998629534754573980,
  0.999390827019095690,
  0.999847695156391220,
  1.0
  };

Fforward( )
{
	rfloat delx, dely;
	int dist;
	int oldx, oldy;

	dist = int_spop();
	delx = cur_DXturt * dist;
	dely = cur_DYturt * dist;
	oldx = cur_X;
	oldy = cur_Y;
	drawaturt( ); /* erase old turtle */
	cur_X = (int)(cur_Xturt += delx);
	/* negative as top of screen is 0 */
	cur_Y = (int)(cur_Yturt -= dely);
	EDEBUG( 7, "New xturt %g yturt %g\n", cur_Xturt, cur_Yturt );
	if( PenDown ) {
		clipdraw( oldx, oldy, cur_X, cur_Y, cur_COL );
		}
	drawaturt( ); /* draw new turtle */
}

#define TURTSIZ 25

drawaturt( newturt )
int newturt;		/* true for a new one, false to erase */
{
#ifdef XORMASK
	int delx, dely;

	if( DrawTurtle ) {
		delx = cur_DXturt * TURTSIZ;
		dely = cur_DYturt * TURTSIZ;
		clipdraw( cur_X - delx/2, cur_Y + dely/2, cur_X + delx,
						cur_Y + dely, -1 );
		clipdraw( cur_X + delx/2, cur_Y - dely/2, cur_X + delx,
						cur_Y + dely, -1 );
		clipdraw( cur_X - delx/2, cur_Y + dely/2, cur_X + delx/2,
						cur_Y - dely/2, -1 );
		
		}
#endif
}

Fbackward( )
{
	int_stop = -int_stop;
	Fforward;
}

Fright( )
{
	setangle( cur_Aturt - int_spop());
}

static double Aspect = 0.485;

setangle( ang )
int ang;
{
	int minorang, sinang, cosang;
	int cosneg;			/* cos must be negative */


	ang = ang % 360;
	if( ang < 0 )
		ang += 360;

	cosneg = FALSE;
	switch( ang / 90 ) {
		case 0:
			sinang = ang;
			cosang = 90 - ang;
			break;
		case 1:
			sinang = 180 - ang;
			cosang = ang - 90;
			cosneg = TRUE;
			break;
		case 2:
			sinang = ang - 180;
			cosang = 270 - ang;
			cosneg = TRUE;
			break;
		case 3:
			sinang = 360 - ang;
			cosang = ang - 270;
			break;
		}
	drawaturt();
	cur_DYturt = Aspect*(ang>180 ? -sin_table[sinang] : sin_table[sinang]);
	cur_DXturt = cosneg ? -sin_table[cosang] : sin_table[cosang];
	cur_Aturt = ang;
	drawaturt();
	EDEBUG( 7,"Turtle DX %g, DY %g", cur_DXturt, cur_DYturt );
	EDEBUG( 7, " new angle %d\n", cur_Aturt, 0 );
}


Fleft( )
{
	setangle( cur_Aturt + int_spop());
}

Fheading( )
{
	setangle( int_spop() );
}

Fhome()
{
	drawaturt();
	cur_Xturt = (rfloat)( cur_X = Win.x + Win.w / 2 );
	cur_Yturt = (rfloat)( cur_Y = Win.y + Win.h / 2 );

	/* default to the right for turtle */
	cur_Aturt = 0;
	cur_DXturt = 1.0;
	cur_DYturt = 0.0;
	PenDown = TRUE;
	drawaturt();
}

Fpen( argc, argv, sym )
int argc;
rint *argv;
symptr sym;
{
	PenDown = sym_saveid(sym) == -264;
}
Fhasturt( argc, argv, sym )
int argc;
rint *argv;
symptr sym;
{
	int newturt;
	newturt = sym_saveid(sym) == -272;
	if( newturt != DrawTurtle ) {
		DrawTurtle = newturt;
		drawaturt();
		}
}

Fsetpencolor()
{
	int colour = int_spop();

	checkModeAndColour( ABS(colour) );
	cur_COL = colour;
}

Fsetposition()
{
	drawaturt();
	cur_Y = int_spop();
	cur_X = int_spop();
	cur_Xturt = (rfloat)cur_X;
	cur_Yturt = (rfloat)cur_Y;
	drawaturt();
}

Fgetheading( argc, argv )
int argc;
rint *argv;
{
	*argv = cur_Aturt;
}
