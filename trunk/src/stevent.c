
#include "alice.h"
#include "token.h"
#include <gemdefs.h>
#include <gembind.h>
#include <obdefs.h>
#include <osbind.h>
#include "command.h"
#include "jump.h"
#include "alrsc.h"
#include "gemcmd.h"
#include "keys.h"
#include "workspace.h"
#include "dbflags.h"

/*
 * This is the main Gem Event handler
 */

extern long gl_menu;

static int GemRet;

extern int m_out;
extern int gl_rmsg[];
extern int mousex, mousey;
extern int bstate, bclicks;
extern int kstate, kreturn;

int
GemEvent()
{
	int done;
	int ev_which;

	done = FALSE;

	do {

	ev_which = evnt_multi( MU_BUTTON|MU_MESAG|MU_KEYBD,
			/* The number of clicks, the mask, and the state */
			0x01, 0x01, m_out,
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

	if( ev_which & MU_BUTTON ) {

		/* We had a mouse click. */
		done = hndl_button();

	}

	if( ev_which & MU_M1 ) {

		/* We had a rectangle event, change mouse shape ? */
		done = hndl_mouse();

	}

	if( ev_which & MU_MESAG )  {

		/* We had a message, handle it */
		done = hndl_msg();

	}

	if( ev_which & MU_KEYBD ) {

		done = hndl_keyboard();

	}

	} while ( !done );

	return GemRet;

}

hndl_button()
{
	int handle;
	int topwin, dummy;

	/*
	 * Handle a button press
	 */
	/*
	 * This must be in the currently topped window
	 * but make sure that it is an edit window
	 */
	handle = wind_find( mousex, mousey );

	wind_get( 0, WF_TOP, &topwin, &dummy, &dummy, &dummy );

	if( EditWin( handle ) ) {
		/*
		 * If for some reason, this is not the topped window,
		 * top it
		 */
		if( handle != topwin ) {
			GemRet = GM_MESSAGE + WM_TOPPED;
			gl_rmsg[3] = handle;
			return TRUE;
		}
		GemRet = KEY_COORD;
		return TRUE;
	} else {
		/*
		 * Otherwise 'ding'
		 */
		Cconout( '\007' );
		return FALSE;
	}

}

hndl_mouse()
{
	/*
	 * Mouse crossed the M1 rectangle
	 */
	return FALSE;
}

hndl_keyboard()
{
	/*
	 * Just pass the keyboard character back to the caller
	 */
	GemRet = ConvKey();

	/*
	 * As long as there are characters in the queue, transfer them to
	 * incom_buf queue.
	 */
	while( ST_chr_waits() );

	return TRUE;
}

int
hndl_msg()
{
	
	if( gl_rmsg[0] == MN_SELECTED ) {

		/* we selected an item from the menu, return it as a
		 * code
		 */
		if( gl_rmsg[4] == HMSYMCMP )
			GemRet = AL_ID;
		else
			GemRet = GM_MENUITEM + gl_rmsg[4];
		/*
		 * Unhighlight the entry now, instead of later
		 */
		menu_tnormal( gl_menu, gl_rmsg[3], TRUE );

	} else if( gl_rmsg[0] == WM_ARROWED ) {

		/* Handle up and down arrow events like cursor keys */
		if( gl_rmsg[4] == 2 )
			GemRet = KEY_UP;
		else if( gl_rmsg[4] == 3 )
			GemRet = KEY_DOWN;
		else
			GemRet = GM_ARROWS + gl_rmsg[4];

	} else {

		GemRet = GM_MESSAGE + gl_rmsg[0];

	}
	return TRUE;
}


/*
 * Topcmd calls readkey, which calls GemEvent.  GemEvent returns a 
 * GM_MENUITEM code + item which indicates which menu item was selected
 * topcmd recognizes that a menu selection has been made, and calls
 * this routine which stuffs the appropriate command name into
 * the tokentext buffer. Ick.
 */

struct {
	int	item;
	char	*cmd;
	} MenuHandlers[] = {

	/* Desk Menu */
	ABTINFO,  "gemabout",

	/* File Menu */
	FMLOAD,   "load",
	FMSAVE,   "save",
	FMSAVEAS, "gemsaveas",
	FMMERGE,  "merge",
	FMTEXT,	  "text",
	FMCOMPIL, "compile",
	FMSHELL,  "shell",
	FMQUIT,   "exit",
	FMCDIR,	  "cd",

	/* edit menu */
	EMEDIT,   "edit",
	EMCOPY,   "copy clipboard",
	EMCUT,    "move clipboard",
	EMPASTE,  "get clipboard",
	EMUNDO,   "undo",
	EMREDO,   "redo",
	EMEXTEND, "extend",
	EMINSERT, "insert",
	EMDELETE, "delete it",

	/* structure menu */
	SMHIDE,   "hide",
	SMREVEAL, "reveal",
	SMENCLOS, "enclose",
	SMRAISE,  "raise",
	SMSPECIA, "specialties",
	SMCOMOUT, "comout",

	/* Run Menu */
	RMRUN,    "run",
	RMSINGLE, "step",
	RMSUPER,  "superstep",
	RMTYPECK, "check",
	RMLOG,    "gemlog",
	RMCONTIN, "continue",

	/* Debug Menu */
	DMCURFOL, "gemfollow",
	DMIMMED,  "immediate",
	DMSETBRK, "breakpoint",
	DMCLRBRK, "clearpoint",
	DMDEBUG,  "gemdebug",
	DMWHOCAL, "traceback",
	DMPOPSUS, "pop",
	DMEXEC,   "execute",

	/* Miscellaneous Menu */
	MMMACRO,  "map",
	MMOPTION, "option",
	MMAUTOSV, "autosave",
	MMNEWWS,  "createws",
	MMEXECWS, "execws",
	MMBUFFER, "buffer",
	MMCLEAR,  "new",
	MMRECOVR, "recover",
	MMMLEFT,  "freemem",

	GMMARK,	  "mark",
	GMSEARCH, "gemsearch",
	GMAGAIN,  "again",
	GMGOMARK, "go",
	GMPARENT, "parent",
	GMDECL,	  "decl",
	GMTOPBLK, "block",
	GMLSTPOS, "popback",

	/* Help Menu */
	HMSTART,  "help misc/intro",
	HMALLABT, "help misc/commands",
	HMPASCAL, "phelp",
	HMKEYS,   "gemkeys",
	HMWHAT,   "toklist",
	HMSYMCMP, "",			/* Handled separately */
	HMLASTER, "lasterror",
	HMCMD,	  "chelp",

	/* End of struct */
	-1, ""
	};

struct {
	int	item;
	char	*cmd;
	} MsgHandlers [] = {

	WM_TOPPED, "gemtop",
	WM_REDRAW, "gemredraw",
	WM_SIZED,  "gemsized",
	WM_MOVED,  "gemmoved",
	WM_FULLED, "gemfulled",
	WM_CLOSED, "gemclose",
	WM_VSLID, "gemslide",

	-1, "" };

struct {
	int	item;
	char	*cmd;
	} ArrHandlers[] = {
	0, "pageup",
	1, "pagedown",
/*	2, "prev",	*/
/*	3, "next",	*/
	4, "",
	5, "",
/*	6, "left",	 */
/*	7, "right",	 */
	-1, "" };

GemCmd( cmd )
int cmd;
{
	int i;
	extern char tokentext[];

	if( cmd >= GM_MENUITEM &&
            cmd <= GM_ENDMENU ) {

		for( i=0; MenuHandlers[i].item != -1; i++ ) {
			if( (cmd - GM_MENUITEM) == MenuHandlers[i].item) {
				strcpy( tokentext, MenuHandlers[i].cmd);
				return;
			}
		}

	} else if( cmd >= GM_MESSAGE &&
		   cmd <= GM_MSGEND ) {

		for( i=0; MsgHandlers[i].item != -1; i++ ) {
			if( (cmd - GM_MESSAGE) == MsgHandlers[i].item ) {
				strcpy( tokentext, MsgHandlers[i].cmd );
				return;
			}
		}
	} else if( cmd >= GM_ARROWS &&
		   cmd <= GM_ARREND ) {

		for( i=0; ArrHandlers[i].item != -1; i++ ) {
			if( (cmd - GM_ARROWS) == ArrHandlers[i].item ) {
				strcpy( tokentext, ArrHandlers[i].cmd );
				return;
			}
		}
	}
	form_alert( 1, LDS( 368, "[1][Error: Code not handled][ OK ]" ));
}

do_gabout()
{
	menu_dialog( DESKMEN,  ABOUT );
}

menu_dialog( title, index )
int title;
int index;
{
	OBJECT *tree;
	GRECT	box;
	int 	button;

	/*
	 * Standard routine to pop a dialog from a menu selection
 	 * just wait for any exit object
	 */
	objc_xywh( gl_menu, title, &box );
	rsrc_gaddr( R_TREE, index, &tree );
	button = 0x7f & hndl_dial(tree,0,box.g_x, box.g_y, box.g_w, box.g_h );
	tree[button].ob_state = 0;

}

/*
 * Toggle cursor following
 */
do_gfollow()
{
	int f;

	f = work_debug(curr_workspace);
	if( f & DBF_CURSOR )
		/*
		 * Turn it off
		 */
		do_gfoff();
	else
		do_gfon();
}

do_gfoff()
{
	work_debug(curr_workspace) &= ~DBF_CURSOR;
	menu_icheck( gl_menu, DMCURFOL, 0 );
}

do_gfon()
{
	work_debug(curr_workspace) |= DBF_CURSOR;
	work_debug(curr_workspace) |= DBF_ON;

	menu_icheck( gl_menu, DMCURFOL, 1 );
	menu_icheck( gl_menu, DMDEBUG, 1 );

}

do_gdebug()
{
	if( work_debug( curr_workspace ) & DBF_ON )
		do_gbugoff();
	else
		do_gbugon();
}

do_gbugoff()
{
	work_debug( curr_workspace ) &= ~DBF_ON;
	menu_icheck( gl_menu, DMDEBUG, 0 );
	display( TRUE );
}

do_gbugon()
{
	work_debug( curr_workspace ) |= DBF_ON;
	menu_icheck( gl_menu, DMDEBUG, 1 );
	display( TRUE );
}

do_gkeys()
{
	menu_dialog( HELPMEN, KEYSUMRY );
}

