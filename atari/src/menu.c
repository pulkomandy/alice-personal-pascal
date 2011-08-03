#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "menu.h"
#include "keys.h"
#include "alctype.h"
#include "dbflags.h"

#ifdef XENIX
#define FUDGE 2	/* compensate for Xenix ANSI emulator bug */
#else
#define FUDGE 1
#endif
#ifdef ICON_WIN_MGR
#define NOSCREENIMAGE 1
#endif

/* externals for help menu functions */

#ifndef SAI
char **com_m_array;	/* command menu array */
char *com_helpdir;	/* directory for command help */
#endif SAI

struct menu_data *curr_menu;


add_menu_item( str )
char *str;	/* string to add */
{
#ifdef HAS_MENU
	register struct menu_data *menu;
	register int stlen;	/* length of new string */
	menu = curr_menu;

	if( menu->num_items >= MAX_ITEMS )
		return ERROR;

	menu->item[menu->num_items++] = str;

	stlen = strlen( str );
	if( stlen > menu->max_width )
		menu->max_width = stlen;
	return 0;
#endif HAS_MENU
}

#ifndef SAI
/*
 * Get the help file name for a certain selection.  The menu table
 * consists of pairs of char *'s, one which points to the menu text
 * and one which points to the help file name.  This string can
 * be NIL, meaning extract the 0th word from the first string, or
 * it can be a pointer to a char between 1 and 10, meaning extract
 * the nth word from the first string...
 */

char *
getHelp(sel)
int	sel;
{
	register unsigned	count;
	register char	*s;
	static char	word[MAX_FN_LEN + 1];
	char		*wp;

	s = com_m_array[2 * sel + 1];
	if (s == (char *)0)
		count = 0;
	else if ((unsigned)s[0] <= 10)
		count = s[0];
	else
		return s;		/* string was stored in table */

	/*
	 * extract count-th word of menu string, lowercase it, and store it
	 * in "word"
 	 */
	s = com_m_array[2 * sel];

	for ( ; count > 0; count--)
		while (isalpha(*s++))
			;
	for (wp = word; isalpha(*s) && wp < &word[sizeof(word)-1]; s++, wp++)
		*wp = (isupper(*s)) ? tolower(*s) : *s;
	*wp = NULL;

	return word;
}

int
comhelp( selection, menu )
int selection;
struct menu_data *menu;
{
#ifdef HAS_MENU
	helpfile( com_helpdir, getHelp(selection));
#endif HAS_MENU
}


#ifndef GEM
	
deb_menu()
{
#ifdef HAS_MENU
	struct menu_data m;
	int ourdb = work_debug( curr_workspace );
	extern struct al_window main_win;
	extern char com_dir[];
	extern int count_suspend;
	extern char menu_nop[], nop_command[];
	extern FILE	*LogFile;
	char *h_array[20*2];	/* help array, must have enough for
					   largest version of the menu */

	
	new_menu( &m, "Runtime" );
	com_helpdir = com_dir;
	com_m_array = h_array;
	add_hmenu_item( menu_nop, nop_command );
	if( anchor /*&& ntype(sel_node) >= FIRST_RUN && ntype(sel_node) <= LAST_RUN */)
		add_hmenu_item( "Execute this statement", "execute" );
	add_hmenu_item( "Typecheck Program", "check" );
	add_hmenu_item( count_suspend ? "Continue" : "Run Program", "runit" );
	add_hmenu_item( "Immediate Mode Block", "immediate" );
	if( ourdb & DBF_ON )
		add_hmenu_item( "Debug Off", "bugoff" );
	 else
		add_hmenu_item( "Debug On", "bugon" );
	add_hmenu_item( "Set Breakpoint", "breakpoint" );
	add_hmenu_item( "Clear Breakpoints", "clearpoint" );

#ifdef msdosGraphics
	if (ScreenBuf)
		add_hmenu_item( "View Graphics", "view" );
#endif
	if( count_suspend ) {
		add_hmenu_item( "Who Called Me?", "traceback" );
		add_hmenu_item( "Pop Suspended State", "pop" );
		add_hmenu_item( "Pop Suspend & Run", "run" );
		add_hmenu_item( "Big Step", "superstep" );
		}
	add_hmenu_item( "Single Step", "step" );

	if (!LogFile)
		add_hmenu_item( "Log output to file", "log" );
	else
		add_hmenu_item( "Don't log output to file", "nolog" );

	if( ourdb & DBF_CURSOR )
		add_hmenu_item( "Cursor Following Off", "clearfollow" );
	 else
		add_hmenu_item( "Cursor Following On", "setfollow" );

	d_com_menu( &m, FALSE );
#endif HAS_MENU
}
#endif GEM

static
add_hmenu_item( name, hname )
char *name;		/* menu item string */
char *hname;		/* help name or zero */
{
#ifdef HAS_MENU
	register int msize = curr_menu->num_items * 2;

	com_m_array[msize] = name;
	com_m_array[msize+1] = hname;
	add_menu_item(name);
	
#endif HAS_MENU
}

com_menu( mname, retstat )
char *mname[];
int retstat;		/* do we return or do a command */
{
#ifdef HAS_MENU
	struct menu_data m;
	register char **mscan;
	extern struct al_window main_win;

	new_menu( &m, mname[0] );
	com_helpdir = mname[1];
	com_m_array = mname + 2;

	for( mscan = mname + 2; *mscan; mscan += 2 )
		if( add_menu_item( *mscan ) == ERROR ) {
			bug( ER(262,"Builtin menu too long") );
			break;
			}
	return d_com_menu( &m, retstat );
}

d_com_menu( m, retstat )
struct menu_data *m;
int retstat;
{
	int rnum;
#ifdef GEM
	int i;
	char *str;
	char *pop_menu();

	pop_menu( m, comhelp, FALSE, &str, &rnum );
#else
	rnum = pop_menu( curr_window->w_desc, m, comhelp );
#endif

	if( retstat )
		return rnum;
	if( rnum >= 0 ) {
		/* do the command */
		menuact( com_helpdir, getHelp(rnum) );
		}
	return 0;
#endif HAS_MENU
}

#endif /* not SAI */

new_menu( xmenu, mname )
struct menu_data *xmenu;
char *mname;		/* name of new menu */
{
#ifdef HAS_MENU
	register struct menu_data *menu;
	menu = curr_menu = xmenu;

	menu->num_items = menu->m_options = 0;
	menu->m_title = mname;
	menu->max_width = strlen(mname);
#endif HAS_MENU
}
