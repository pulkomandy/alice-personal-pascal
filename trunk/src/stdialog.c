
#include "alice.h"
#include "interp.h"

#include <curses.h>
#include <obdefs.h>
#include <vdibind.h>
#include <gemdefs.h>
#include <osbind.h>

#include "workspace.h"
#include "window.h"
#include "stwin.h"
#include "gemevnt.h"

#define PRINTT
#include "printt.h"

#define WOClicks	0x1000
#define WOExtend	0x2000
#define WOTop		0x4000

int HadKeyEvent = FALSE;

int ev_which;
int m_out = 1;

unsigned int mousex, mousey;
unsigned int bstate, bclicks;
unsigned int kstate, kreturn;

int gl_rmsg[8];	/* Message buffer */
int *ad_rmsg;		/* A pointer to the message buffer */


/*
 * Routines for the ST Alice interface to the Dialog Manager
 * As similar to OSS Personal Pascal as possible
 */

do_alert( argc, argv )
int argc;
rint *argv;
{
	char *str = str_spop() + STR_START;
	int  def = int_spop();
	extern int MenuState;
	int mstate;
	int err = FALSE;
	char *ptr;
	int max_len;

	int value;

	if( str[0] != '[' ||
	    str[2] != ']' ||
	    str[3] != '[' ) err = TRUE;

	if( str[1] > '3' || str[1] < '0' ) err = TRUE;

	ptr = &str[4];
	max_len = 0;
	while( *ptr ) {
		if( *ptr == '|' )
			max_len = 0;
		if( *ptr == ']' ) break;
		if( *ptr == '[' ) err = TRUE;
		max_len++;
		if( max_len >= 30 )
			err = TRUE;
		ptr++;
	}
	if( *ptr != ']' ) 
		err = TRUE;
	else {
		ptr++;
		if( *ptr != '[' ) err = TRUE;
		max_len = 0;
		while( *ptr ) {
			if( *ptr == ']' && ptr[1] ) err = TRUE;
			if( *ptr == '|' ) max_len = 0;
			max_len++;
			if( max_len >= 20 )
				err = TRUE;
			ptr++;
		}
		if( ptr[-1] != ']' ) err = TRUE;
	}

	if( err )
		run_error( ER(335, "illalert`Illegal Alert format" ));

	/*
	 * Now onto the buttons...
	 */

		
	mstate = MenuState;
	MenuCtrl( TRUE );

	value = form_alert( def, str );
	MenuCtrl( mstate );

	*argv = value;

}

/*
 * A Queue to hold events
 */
#define MAX_EVENT	50
int EvntQueue[MAX_EVENT];
int EvntCount = 0;

/*
 * Get an event for our beloved user
 */

static int ev_nparms = 0;
static int ev_parm[10];

do_event( argc, argv )
int argc;
rint *argv;
{
	int flag;
	int event;

	flag = int_spop();

	NeedsGem();		/* Make sure ApplInit is running */

	/*
	 * Get the event
	 */
	event = do_gevent( flag );

	/*
	 * If this event has parameters, pop them into the parm array
	 */
	switch( event ) {
		case EVNT_BUTTON:
			ev_parm[1] = PopEvnt(); /* X - rel to window  */
			ev_parm[2] = PopEvnt(); /* Y - rel to window  */
			ev_parm[3] = PopEvnt(); /* Abs X              */
			ev_parm[4] = PopEvnt(); /* Abs Y              */
			ev_parm[0] = PopEvnt();	/* Number of clicks   */
			if( m_down() && (ev_parm[0] == 1) )
				ev_parm[0] = 0;

			ev_nparms = 5;
			break;
		case EVNT_HSLIDE:
		case EVNT_VSLIDE:
			ev_parm[0] = PopEvnt();
			ev_nparms = 1;
			break;
		case EVNT_REDRAW:
			ev_parm[0] = PopEvnt();	/* GEM Handle */
			ev_parm[1] = PopEvnt(); /* X coord */
			ev_parm[2] = PopEvnt(); /* Y coord */
			ev_parm[3] = PopEvnt(); /* W idth  */
			ev_parm[4] = PopEvnt(); /* H eight */
			ev_nparms = 5;
			break;
		case EVNT_MENU:
			ev_parm[1] = PopEvnt();  /* Title */
			ev_parm[2] = PopEvnt();  /* Item  */
			ev_parm[0] = PopEvnt();  /* Mnem  */
			ev_nparms = 3;
			break;
		case EVNT_MOVED:
		case EVNT_SIZED:
			ev_parm[0] = PopEvnt();  /* X or W */
			ev_parm[1] = PopEvnt();  /* Y or H */
			ev_nparms = 2;
			break;
		default:
			ev_nparms = 0;
	}

	if( event == EVNT_KEY )
		HadKeyEvent = TRUE;

	*argv = event;
}

/*
 * Return the parm specified for the current event
 */
do_getparm( argc, argv )
int argc;
rint *argv;
{
	int pnum;

	pnum = int_spop();

	/*
	 * Param number is 1 origined
	 */
	if( pnum < 0 || pnum >= ev_nparms ) {
		if( ev_nparms == 0 )
			run_error( 
			  ER(346,"notparm`No parameters for that event" ));
		else
			run_error( 
			  ER(347, "notparm`Not that many parameters available"));
	}
	*argv = ev_parm[ pnum ];
}

/*
 * Return an event from the Queue
 */

int
do_gevent( WaitForEvent )
int WaitForEvent;
{
	int EvMask;
	int ev_which;
	extern int inbuf_count;

	/*
	 * There are still events in the queue, return them QUICK !
	 */

	if( EvntCount ) {
		return PopEvnt();
	}

	/*
	 * Handle events for the user...
	 */

	EvMask = MU_MESAG | MU_BUTTON | MU_KEYBD;

	if( inbuf_count && !HadKeyEvent ) {
		HadKeyEvent = TRUE;
		return EVNT_KEY;
	}

	/*
	 * If we want to wait for an event...
	 */
	if( !WaitForEvent )
		EvMask |= MU_TIMER;

	/*
	 * Otherwise we must go and get an event from
	 * the queue
	 */
GetAgain:
	ev_which = next_event( EvMask );

	/* We timed out, push a time out event */
	if( (ev_which & MU_TIMER) == MU_TIMER )
		PushEvnt( EVNT_TIMEOUT );

	/*
	 * Flush all the events in the queue
	 */
	flush_events();

	/*
	 * yay, we have an event we can return to the user, so
	 * do it !
	 */
	if( EvntCount )
		return PopEvnt();
	else
		goto GetAgain;

}

/*
 * Handle one event from the gem queue
 */

int
next_event( EvMask )
int EvMask;
{
	int ev_which;

	ad_rmsg = &gl_rmsg[0];

	ev_which = evnt_multi( EvMask,
			/* The number of clicks, the mask, and the state */
			0x02, 0x01, m_out,
			/* The M1 flags */
			0, 0, 0, 0, 0,
			/* The M2 flags */
			0, 0, 0, 0, 0,
			/* The address of the message buffer */
			gl_rmsg, 
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

	/*
	 * Okay, so we got an event, now what !
	 */
	if( ev_which & MU_KEYBD ) {
		extern int BrkBit;

		push_key( ConvKey() );
		if( (kreturn & 0xff) == 0x3 ) {
			breakkey();
			BrkBit = TRUE;
		}
		PushEvnt( EVNT_KEY );
	}

	/* If we got a button event, and we are supposed to return these
	 * in this window, then do it
	 */
	if( ev_which & MU_BUTTON ) {
		int mx, my;
		int slot;
		GRECT rect;
		int handle;

		handle = wind_find( mousex, mousey );
		/* Should only return clicks on desktop when there
		 * is a window that wants clicks
		 */
		if( handle == 0 || ret_click( handle ) ) {
			/* Click on the desktop, or in a window that wants
			 * clicks
			 */
			PushEvnt( EVNT_BUTTON );

			/*
			 * Get the relative mouse position
			 */
			slot = HndlSlot( handle );
			if( slot == -1 || AW_KIND(slot) == WIN_EDIT ) {
				PushEvnt( -1 );
				PushEvnt( -1 );
			} else {
				rc_copy( &AW_WORK(slot), &rect );
				if( inside( mousex, mousey, &rect ) ) {
					mx = mousex - rect.g_x;
					my = mousey - rect.g_y;
					MapCo( slot, &mx, &my );
					PushEvnt( mx );
					PushEvnt( my );
				} else {
					PushEvnt( -1 );
					PushEvnt( -1 );
				}
			}
			
			/* Map to current co-ordinate system (screen rel) */
			MapWin( &mousex, &mousey );
			PushEvnt( mousex );
			PushEvnt( mousey );

			PushEvnt( bclicks );

		}
	}

	if( ev_which & MU_MESAG ) {
		DoMsg();
	}

	return ev_which;
}

DoMsg()
{
	int title, item, mnem;
	int slot;
	int x, y;

	/*
	 * Handle our friend the message event
	 */
	switch( gl_rmsg[0] ) {

		case MN_SELECTED:
			PushEvnt( EVNT_MENU );
			/*
			 * Convert title and item index into square array
			 */
			GetMNumber( gl_rmsg[4], &title, &item, &mnem );
			PushEvnt( title );
			PushEvnt( item );
			PushEvnt( mnem );
			clr_menu( gl_rmsg[3] );
			break;
		case WM_REDRAW:
			/*
			 * Go and redraw the window
			 */
			do_gredraw();
			/*
			 * If it is one of the user's windows, tell him
			 */
			if( ext_event( gl_rmsg[3] ) ) {
				PushEvnt( EVNT_REDRAW );
				PushEvnt( gl_rmsg[3] );
				/* Now push the x y w h */
				PushEvnt( gl_rmsg[4] );
				PushEvnt( gl_rmsg[5] );
				PushEvnt( gl_rmsg[6] );
				PushEvnt( gl_rmsg[7] );
			}
			break;
		case WM_TOPPED:
			wind_set( gl_rmsg[3], WF_TOP, 0, 0, 0, 0 );
			/* If returning the 'topped' event */
			if( the_features( gl_rmsg[3] ) & WOTop )
				PushEvnt( EVNT_TOPPED );
			break;
		case WM_CLOSED:
			PushEvnt( EVNT_CLOSE );
			break;
		case WM_FULLED:
			do_gfull();
			PushEvnt( EVNT_FULLED );
			break;
		case WM_ARROWED:
			PushEvnt( EVNT_ARROW + gl_rmsg[4] );
			break;
		case WM_HSLID:
			if( is_userwin( gl_rmsg[3] ) ) {
				PushEvnt( EVNT_HSLIDE );
				PushEvnt( gl_rmsg[4] );
				wind_set( gl_rmsg[3], WF_HSLIDE, gl_rmsg[4] );
			}
			break;
		case WM_VSLID:
			if( is_userwin( gl_rmsg[3] ) ) {
				PushEvnt( EVNT_VSLIDE );
				PushEvnt( gl_rmsg[4] );
				wind_set( gl_rmsg[3], WF_VSLIDE, gl_rmsg[4] );
			}
			break;
		case WM_SIZED:
			do_gsized();
			if( ext_event( gl_rmsg[3] ) ) {
				PushEvnt( EVNT_SIZED );
				/* Push new width and height */
				slot = HndlSlot( gl_rmsg[3] );
				x = AW_WORK(slot).g_w;
				y = AW_WORK(slot).g_h;
				MapWin( &x, &y );
				PushEvnt( x );
				PushEvnt( y );
			}
			break;
		case WM_MOVED:
			do_gmoved();
			if( ext_event( gl_rmsg[3] ) ) {
				PushEvnt( EVNT_MOVED );
				slot = HndlSlot( gl_rmsg[3] );
				x = AW_WORK(slot).g_x;
				y = AW_WORK(slot).g_y;
				MapWin( &x, &y );
				PushEvnt( x );
				PushEvnt( y );
			
			}
			break;
		case WM_NEWTOP:
			break;
	}
}

ClrEvnt()
{
	EvntCount = 0;
}

PushEvnt( event )
int event;
{
	if( EvntCount >= MAX_EVENT ) return;

	EvntQueue[ EvntCount++ ] = event;
}

PopEvnt()
{
	int event;

	if( EvntCount ) {
		event = EvntQueue[0];
		blk_move( &EvntQueue[0], &EvntQueue[1], sizeof( int ) *
				--EvntCount );
		return event;
	} else
		return EVNT_TIMEOUT;
}

is_userwin( handle )
int handle;
{
	int i;

	i = HndlSlot( handle );
	if( i < 0 ) return FALSE;

	return AW_KIND(i) == WIN_OUTPUT;
}

flush_events()
{
	int EvMask;
	int ev_which;

	EvMask = MU_MESAG | MU_KEYBD | MU_TIMER;

	/*
	 * Flush the following events into our fine user-buffer
	 */
	for( ;; ) {
		ev_which = next_event( EvMask );
		/* If the only event we got was a timer event then break */
		if( ev_which == MU_TIMER ) break;
	}

}

ret_click( handle )
int handle;
{
	int slot;

	slot = HndlSlot( handle );
	if( slot < 0 ) return FALSE;

	/* If we clicked on an output window, and we are supposed to get
	 * those events, then return TRUE
	 */

	if( AW_KIND(slot) == WIN_OUTPUT &&
	   (AW_FEATURES(slot) & WOClicks) )
		return TRUE;

	return FALSE;
}

the_features( handle )
int handle;
{
	int slot;
	slot = HndlSlot( handle );
	if( slot < 0 ) return 0;

	if( AW_KIND(slot) != WIN_OUTPUT ) return 0;

	return AW_FEATURES(slot);
}

int
ext_event( handle )
int handle;
{
	int slot;

	return the_features(handle) & WOExtend;
}
