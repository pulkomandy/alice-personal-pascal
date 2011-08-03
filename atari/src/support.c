
#include "alice.h"
#include <osbind.h>
#include <ctype.h>
#include <gemdefs.h>
#include "interp.h"

#include "alrsc.h"

#ifdef DEBUG
#define PRINTT
#endif
#include "printt.h"

int BrkFreq = 125;

/*
 * Set the break key frequency, and return the old frequency
 */
int
SetFreq( freq )
int freq;
{
	int oldfreq = BrkFreq;

	BrkFreq = freq;
	return oldfreq;

}

int BrkCount = 0;

/*
 * Return whether 'c' is a hex digit or not
 */
int
isxdigit( c )
char c;
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F');
}


#ifdef notdef
/*
 * Check_Undef
 */
CU( ptr, b )
long ptr;
int b;
{
	extern unsigned int max_stack_size;
	extern char *undef_bitmap;
	
	long off;
	int bit;

	ptr = ptr - exec_stack;

	if( ptr > max_stack_size ) return;

	off = ptr >> 3;
	bit = 1 << (ptr & 7);
	if( undef_bitmap[off] & bit )
		return;

	CUerror();
}

#endif

#ifndef GEM
/*
 * Dummy get userid
 */
getuid()
{
	return 1;
}
#endif

/*
 * Break Handling Routines
 */

int BrkBit = FALSE;

clear_break()
{
	BrkBit = FALSE;
}


#define CTRL_C	('C' & 0x1f)
#define CTRL_S	('S' & 0x1f)

int
ST_chr_waits()
{
	extern int kreturn;

	if( is_event( MU_KEYBD ) ) {

		/* Yes, there is a character waiting
		 * push the key into the incom_buf
		 */
		/*
		 * If we got a Ctrl-C, then set the break bit and
		 * return FALSE
		 */
		if( kreturn & 0xff == CTRL_C ) {
			BrkBit = TRUE;
			return FALSE;
		}

		push_key( ConvKey() );
		return TRUE;

	}
	return FALSE;
}

runkeyget()
{
	int state;
	extern int inbuf_count;

	state = MenuCtrl( FALSE );
	for( ;; ) {
		ST_chr_waits();
		if( inbuf_count ) break;
	}
	MenuCtrl( state );
	return keyget();
}

/*
 * See if we got a break character or not.  If we got a key, but not
 * a CTRL-C, then push it onto the incom_buf queue.
 */

int
get_break()
{
	extern int kreturn;
	int did_wait = FALSE;

	if( BrkBit ) {
		/* Break key was already pressed */
		return TRUE;
	}

	/* Reset the Break count */
	BrkCount = BrkFreq;

	/* Did we get a keyboard event ? */
	if( is_event( MU_KEYBD ) ) {
		/*
	 	 * Got a CTRL-C, return TRUE
		 */
		if( (kreturn & 0xff) == CTRL_S ) {
			/* loop here until another key is pressed */
			did_wait = TRUE;
			while( !is_event( MU_KEYBD ) ) ;
			/* Another key was pressed, exit */
		}
		if( (kreturn & 0xff) == CTRL_C ) return TRUE;

		if( !did_wait )
			push_key( ConvKey() );
	}
	return FALSE;
}

/*
 * Return whether or not the mouse is down, and where it is
 */
int
m_down()
{
	int mx, my, mstate, kstate;

	graf_mkstate( &mx, &my, &mstate, &kstate );
	return mstate & 1;

}

/*
 * See whether there is an event waiting of a particular type
 */
int
is_event( kind )
int kind;
{
	int ev_which;
	extern int m_out;
	extern int *ad_rmsg;
	extern int mousex, mousey, bstate, kstate;
	extern int kreturn, bclicks;

	ev_which = evnt_multi( MU_TIMER | kind,
			/* The number of clicks, the mask, and the state */
			0x01, 0x01, m_out,
			/* The M1 flags */
			0, 0, 0, 0, 0,
			/* The M2 flags */
			0, 0, 0, 0, 0,
			/* The address of the message buffer */
			ad_rmsg, 
			/* The Low Count and High Counter for Timer */
			0, 0,
			/* The values passed back: */
			
			/* The X and Y position of the mouse */
			&mousex, &mousey,

			/* The button state */
			&bstate,

			/* The keyboard state */
			&kstate,

			/* The return value */
			&kreturn,

			/* And the number of clicks received */
			&bclicks );

	if( ev_which & kind ) return TRUE;

	return FALSE;
}

int MouseStat = TRUE;	/* Initially, the mouse is ON */

MouseOn( flag )
int flag;
{
	/*
	 * See if we want to turn mouse control on or off
	 */
	if( MouseStat != flag )
		if( flag ) {
			graf_mouse( M_ON, 0L );
			MouseStat = TRUE;
		} else {
			graf_mouse( M_OFF, 0L );
			MouseStat = FALSE;
		}
}

int MenuState = TRUE;

MenuCtrl( flag )
int flag;
{

	int oldstate;

	oldstate = MenuState;

	/* Turn on the mouse or not as the case may be... */
	MouseOn( flag );

	if( MenuState == flag ) return oldstate;

	if( flag ) {
		/*
		 * Of course we want menu control, etc.
		 */
		wind_update( END_MCTRL );
	} else {
		/*
		 * We don't want events from the mouse, turn off
		 * menus, etc
		 */
		wind_update( BEG_MCTRL );
	}
	MenuState = flag;
	return oldstate;
}

static
int BusyState = FALSE;

BusyBee( flag )
int flag;
{

	if( flag )
		graf_mouse( HOURGLASS, 0L );
	else
		graf_mouse( ARROW, 0L );
	BusyState = flag;
}

ConvKey()
{
	int lowbyte;
	int highbyte;

	lowbyte = kreturn & 0xff;
	highbyte = (kreturn >> 8) & 0xff;

	if( lowbyte != 0 )
		return( lowbyte );
	else
		return( highbyte + 128 );
}

#ifndef SAI
/*
 * Prompt the user for the key the macro is to be assigned to
 */
static int mp_x, mp_y, mp_w, mp_h;

MacroPrompt()
{
	long tree;

	/*
	 * Draw the box prompting for a macro key
	 */
	rsrc_gaddr( R_TREE, MACPROMP, &tree );

	form_bottom( tree, &mp_x, &mp_y, &mp_w, &mp_h );
	scr_save( mp_x, mp_y, mp_w, mp_h );
	DrawObj( tree, 0 );
}

MacroClear()
{
	scr_restore( mp_x, mp_y, mp_w, mp_h );
}

MarkPrompt()
{
	long tree;

	/*
	 * Draw the box prompting for a mark key
	 */
	rsrc_gaddr( R_TREE, MARKPROM, &tree );

	form_bottom( tree, &mp_x, &mp_y, &mp_w, &mp_h );
	scr_save( mp_x, mp_y, mp_w, mp_h );
	DrawObj( tree, 0 );
}

MarkClear()
{
	scr_restore( mp_x, mp_y, mp_w, mp_h );
}

#endif SAI

/*
 * The Famous qfopen() routine
 */
#define MAXTRY 3
static int drorder[] = { 1, 0, 2 };
int qf_drive = 0;

FILE *
qfopen( fname, opargs )
register char *fname;
char *opargs;
{
	char fnbuf[100];
	FILE *thedrive;
	int curdrive;

	if( fname[0] == '?' && fname[1] == ':' ) {
		int numdrives;
		int trydrive;
		extern char *apdir;
		char *lstslash, *strrchr();

		/* try default drive */

		qf_drive = Dgetdrv();
		if( thedrive = fopen( fname + 2, opargs ) )
			return thedrive;
		if( (lstslash = strrchr( fname, '\\' )) ||
				(lstslash = strrchr( fname, '/' ) ) ) {
			if( thedrive = fopen( lstslash+1, opargs ) )
				return thedrive;
			}
		 else
			lstslash = fname + 1;	/* the colon */

		if( apdir ) {
			if( apdir[1] == ':' )
				qf_drive = tolower( apdir[0] ) - 'a';
			sprintf( fnbuf, "%s\\%s", apdir, lstslash+1 );
			if( thedrive = fopen( fnbuf, opargs ) )
				return thedrive;
			}
		strcpy( fnbuf, fname );

		curdrive = getDisk();
		numdrives = selDisk(curdrive) - 1;

		if( numdrives > MAXTRY-1 )
			numdrives = MAXTRY - 1;
		for( trydrive = numdrives; trydrive >= 0; trydrive-- ) {
			int physdrive;
			physdrive = drorder[trydrive];
			qf_drive = physdrive;
			if( physdrive != curdrive ) {
				if( physdrive == 1 && !hasbdrive() )
					continue;
				fnbuf[0] = 'A' + physdrive;
				if( thedrive = fopen( fnbuf, opargs ) )
					return thedrive;
				}
			}
		return (FILE *)0;	/* open failed */
		}
	 else {
		if( fname[1] == ':' )
			qf_drive = tolower( fname[0] ) - 'a';
		else
			qf_drive = Dgetdrv();

		if( fname[strlen(fname)-1] == ':' ) {
			char **p;
			static char *devmaparray[] = {
				"con:",  "con:",
				"trm:",  "con:",
				"kbd:",  "con:",	
				"lst:",  "prn:",
				"lpt:",  "prn:",
				"prt:",  "prn:",
				"lpt2:", "prn:",
				"lpt1:", "prn:",

				0 };
			for( p = devmaparray; *p; p++ )
				if( case_equal( *p++, fname ) ) {
					fname = *p;
					break;
					}
			}

		return fopen( fname, opargs );
		}
}


/*
 * Say whether or not there is a 'b' drive
 */
hasbdrive()
{
	long save_ssp;
	int *nflops;
	int hasb;

	nflops = (int *)(0x4a6);

	save_ssp = Super( 0L );

	hasb = (*nflops == 2);

	Super( save_ssp );

	return hasb;
}

int dir_current = FALSE;

typedef struct _dta_ {
	unsigned char  dta_rsv[21];
	unsigned char  dta_type;
	unsigned short dta_time;
	unsigned short dta_date;
	unsigned long  dta_size;
	unsigned char  dta_name[14];
	} DTA_Buff;

DTA_Buff our_dta;

int
findFirst(pat, attr)
char *pat;
int attr; 
{
	long old_dta;
	int ret;

	old_dta = Fgetdta();
	Fsetdta( &our_dta );

	ret = Fsfirst( pat, attr );

	Fsetdta( old_dta );
	return ret;
}

dofirst( pat, attr )
char *pat;
int attr;
{
	dir_current = !findFirst( pat, attr );
}

char *
curfilename( att )
int *att;
{
	*att = our_dta.dta_type;
	return our_dta.dta_name;
}

int
findNext()
{
	int ret;
	long old_dta;

	old_dta = Fgetdta();
	Fsetdta( &our_dta );

	ret = Fsnext();

	Fsetdta( old_dta );
	return ret;
}

int
getDisk()
{
	return Dgetdrv();
}

int
getCDir(s, drive)
char	*s;
int	drive;
{
	Dgetpath( s, drive );
}

int
selDisk(drive)
int	drive;
{
	return Dsetdrv( drive );
}
