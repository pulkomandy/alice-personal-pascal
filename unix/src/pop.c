#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "menu.h"
#include "keys.h"
#include "alctype.h"

/*
 *DESC: Pop up Menu system
 */


#ifdef DB_POP
# define PRINTT
#endif
#include "printt.h"

#ifdef MACRO_KEYS

#include "command.h"

static struct {
	int	cmd;
	int	keystroke;
	}
	menu_table[] = {
		CM_CURDOWN,	KEY_DOWN,
		CM_CURUP,	KEY_UP,
		CM_HELP,	AL_HELP,
		CM_PGUP,	AL_PGUP,
		CM_PGDOWN,	AL_PGDOWN,
		CM_CURLEFT,	KEY_LEFT,
		CM_CURRIGHT,	KEY_RIGHT,
		-1, -1
	};

static int
ConvKey( cmd )
int cmd;
{
	int i;
	while( menu_table[i].cmd != -1 ) {
		if( menu_table[i].cmd == cmd ) return menu_table[i].keystroke;
		i++;
	}
	return -1;
}

#endif
/*
 * Offer user a pop-up menu, and return the item selected.
 * Return values range from 0 to n-1.  -1 means no selection was made
 * it is common to use item[0] = "No action", but this is not required.
 * If the help key is hit, the help function will be called
 * with arguments of the selected item, the menu structure and whatever
 * externals are prepared
 */

#define SCRL_LINES	8

#ifdef msdos
#define AL_MENUHOME	KEY_UP-1
#define AL_MENUEND	KEY_DOWN-1
#endif

int  mdisp_width;

#ifndef OLMSG
static char nomenumem[] = "There is not enough memory for a menu";
#endif

static int menu_getkey();
static draw_menu( WINDOW *w, struct menu_data *menu, int sel, int base,
		int nrow);
static show_item();
static draw_more();
static erase_more();


int
pop_menu(xwindow, xmenu, helpfunc)
WINDOW *xwindow;                /* put pop-up menu in this window */
struct menu_data *xmenu;        /* info to control style of menu  */
int (*helpfunc)();      /* help function for this menu */
{
#ifdef HAS_MENU

        int nrow, ncol;         /* size of menu text area */
        int firstcol;           /* upper left corner of pop up window */
        int selected;           /* currently selected window item */
        register int i;
        int c;                  /* char returned from user */
        int basemax;            /* maximum base */
        register WINDOW *w;             /* icon window */
        register struct menu_data *menu = xmenu;
        WINDOW *wbase;          /* icon windows */
        int window_base;
        int hasmore;
        int numlin,firstlin;

#ifdef SAI
	extern int sai_out;

	if( sai_out == SAI_NORMAL ) {
		printf( "You cannot use menus without being in screen mode\n" );
		return 0;
	}
#endif SAI

	if( menu->num_items == 0 )
		return 0;

	
start_again:
	mdisp_width = menu->max_width;
	if( mdisp_width > (COLS-10) )
		mdisp_width = COLS - 10;

        /* Calculate number of rows in the physical menu window */
        nrow = min(menu->num_items, LINES - DEMO_LINES - 7);

        /* How big can the base get ? */
        basemax = menu->num_items - nrow;
        /* Base starts at zero */
        window_base = 0;

        /* See whether there are mores lines than are displayable */
        hasmore = basemax > 0 ? 1 : 0;
        ncol = mdisp_width+2;
        firstcol = COLS - ncol - 6;

        numlin = nrow+hasmore+3;
        firstlin = LINES - numlin - 1;

	/* Grab the outer window */
        wbase = newwin( numlin, ncol+2, firstlin, firstcol );
	if( wbase == (WINDOW *)ERR ) {
		printt0( "Could not allocate outer window\n" );
		warning( ER(228,nomenumem) );
		return 0; /* pointless after an error */
		}

	printt1( "Wbase = %x\n", (int) wbase );

	/* Grab the inner one */
#ifndef QNX
	if( hasmore )
		w = newwin( nrow, ncol, firstlin+2, firstcol+1 );
	 else
#endif
		w = subwin( wbase, nrow, ncol, firstlin+2, firstcol+1 );

	if( w == (WINDOW *)ERR ) { 
		printt0( "Could not allocate inner window\n" );
		delwin(wbase);
		warning( ER(228,nomenumem) );
		return 0;
		}

	printt1( "w = %x\n", (int) w );

	curOff( wbase );
	curOff( w );
        box( wbase, ACS_VLINE, ACS_HLINE );

        /* use show_item to do the title */
        show_item( wbase, menu, -1, 0, TITLE_HIGHLIGHT );

        /* Initially the first item is selected */
        selected = 0;

        /* Display each of the visible menu lines */
        draw_menu( w, menu, selected, window_base, nrow );

        /* If more, draw MORE on the bottom line of the outer window */
        if( hasmore )
                draw_more( wbase, nrow+2 );

        /* now scan keyboard */
	/* Until we get a non cursor key */
#ifdef notdef 
	setmac(0);
#endif 
        for (;;) {
                wrefresh( wbase );
		wrefresh(w);

		c = menu_getkey();

		/* Unhighlight current selection */
                show_item( w, menu, selected, window_base, NO_HIGHLIGHT );
                switch( c ) {

		case 0:
			break;
#ifdef ICON
		case KEY_LEFT:
		case KEY_RIGHT:
			break;
#endif
                 case KEY_DOWN:
                        if( selected < menu->num_items - 1 )
                                selected++;
                         else
                                break;

                        if( selected == (window_base + nrow) ) {
                                /* Scroll window to show selection */
                                window_base++;
                                wmove( w, 0, 0 );
                                wdeleteln( w );
                        }
                        if( ((window_base + nrow) == menu->num_items) &&
                                hasmore ) {
                                erase_more( wbase, nrow+2 );
                        }
                        break;
                 case KEY_UP:
                        if( selected > 0 )
                                selected--;
                         else
                                break;

                        if( selected < window_base ) {
                                /* Scroll window back to show selection */
                                window_base--;
                                wmove( w, 0, 1 );
                                winsertln( w );
                        }
                        if( (window_base + nrow) == (menu->num_items-1) ) {
                                draw_more( wbase, nrow+2 );
                        }
                        break;
                 case AL_HELP:
                        if( helpfunc ) {
                                (*helpfunc)(selected, menu );
				delwin(w);
				delwin(wbase);
#ifdef SAI
				wrefresh(stdscr);
#else
				refresh_scr();
#endif	

                                goto start_again;
                                }
                        break;
#ifdef msdos
		 case AL_MENUHOME:
			selected = 0;
			window_base = 0;
			draw_menu( w, menu, selected, window_base, nrow );
			if( hasmore ) draw_more( wbase, nrow+2 );
			break;
		 case AL_MENUEND:
			selected = menu->num_items - 1;
			window_base = selected - nrow + 1;
			draw_menu( w, menu, selected, window_base, nrow);
			if( hasmore ) erase_more( wbase, nrow+2 );
 			break;
		 case AL_PGUP:
			for( i=0; i<SCRL_LINES; i++ ) {
				if( selected > 0 )
					selected--;
				 else
					break;
				if( selected < window_base ) {
					window_base--;
					wmove( w, 0, 1 );
					winsertln( w );
				}
				if((window_base+nrow) == (menu->num_items-1) ) {
					draw_more( wbase, nrow+2 );
				}
				show_item( w, menu, selected, window_base,
						NO_HIGHLIGHT );
			}
			break;
		 case AL_PGDOWN:
                        for( i=0; i<SCRL_LINES; i++ ) {
				if( selected < menu->num_items - 1 )
					selected++;
				 else
					break;

				if( selected == (window_base + nrow) ) {
					window_base++;
					wmove( w, 0, 0 );
					wdeleteln( w );
				}
				if( ((window_base + nrow) == menu->num_items) &&
					hasmore ) {
					erase_more( wbase, nrow+2 );
				}
				show_item( w, menu, selected, window_base, 
						NO_HIGHLIGHT );
			}
			break;
#endif msdos
                 default:
                        if( islower(c) )
                                c -= 'a' - 'A';
                        /* We are making a selection */
                        if( (c >= 'A') && (c <= ('A' + menu->num_items - 1)) )
                                selected = c - 'A';
                        /* Close the windows, and return the selection */
			delwin( w );
			delwin( wbase );
#ifdef SAI
			wrefresh(stdscr);
#else
			refresh_scr();
#endif

#ifdef notdef
			setmac(1);
#endif 
                        return selected;
                 }
                show_item( w, menu, selected, window_base, SELECT_ITEM );
        }
}

static int
menu_getkey()
{
	int c;

#ifdef SAI
	c = wgetch(stdscr);
	return c;
#else

#ifdef MACRO_KEYS

	/* If we are allowing macros to work on menus, see if the first
         * character is the command char.
	 * If so, gather up keystrokes until a newline, then
	 * lookup the command, and convert it to the appropriate
	 * CM_foo 
	 */
	extern char tokentext[];
	char *ptr;

	c = readkey();
	if( c != AL_ICOMMAND ) return c;

	ptr = tokentext;

	/* We did get the command key... gather until newline */
	while( (c = readkey()) != '\n' ) {
		*ptr++ = c;

		/* Put in a check to see if overflowing tokentext buffer */
	}
	/* Move to last char... */
	/* And remove off trailing spaces, tabs */
	if( ptr != tokentext ) {
		ptr--;
		while( ptr != tokentext && ( *ptr == ' ' || *ptr == '\t' ) )
			ptr--;
		/* Move to the real end of the string */
		ptr++;
	}
	*ptr = 0;

	/* Skip leading spaces, tabs */
	ptr = tokentext;
	while( *ptr == ' ' || *ptr == '\t' ) ptr++;

	printt1( "Looking up string '%s'\n", ptr );

	/* We have the string in tokentext... look it up */
	c = look_cmd( ptr );

	/* Now convert the token into a keystroke */

	c = ConvKey( c );

	printt1( "Convkey returns %d\n", c );

	if( c == -1 ) return 0;

	return c;
#else
        c = keyget();
	return c;
#endif
#endif
}

static
draw_menu( w, menu, sel, base, nrow)
WINDOW *w;
struct menu_data *menu;
int sel, base, nrow;
{
	int i;
	for( i=base; i<(base+nrow); i++ )
		/* takes advantage of 0 and 1 codes.  kludgy */
		show_item( w, menu, i, base, (i==sel) ? SELECT_ITEM : NO_HIGHLIGHT );
}

#ifdef COLPOS
static unsigned show_colours[] = { AC_NORMAL, A_STANDOUT, AC_CYAN | A_REVERSE};
#endif
 
static
show_item(xw, menu, item, base, highlight )
WINDOW *xw;
struct menu_data *menu;
reg int item;   /* which one*/
int highlight;  /* show it off */
int base;
{
        /* THIS DEPENDS THAT menu->item[-1] is the menu name */
        register WINDOW *w;
        char format[10];
#ifdef COLPOS
	extern int mon_is_colour;
#endif
        w = xw;

        /* Position the windows cursor on the appropriate line */
        wmove(w, item < 0 ? 1 : item-base, item < 0);
#ifdef COLPOS
	if( mon_is_colour ) {
		reswflags( w, show_colours[highlight] );
		}
	 else 
#endif
        if( highlight )
                wstandout(w);
         else
                wstandend(w);

        wprintw( w, "%c %-*.*s", (item < 0 || item > 25) ? ' ' :
		'A' + item, mdisp_width, mdisp_width, menu->item[item] );

	printt2( "Outputing, format = %s, %s\n", format, menu->item[item] );

#ifdef notdef
        wprintw(w,
        "%c %-*s", (item < 0 || item > 25) ? ' ' : 'A' + item,
                mdisp_width, menu->item[item]);
#endif

#ifdef COLPOS
	if( mon_is_colour )
		reswflags( w, AC_NORMAL );
	 else
#endif
        if( highlight )
                wstandend(w);
}
static
draw_more( w, line )
WINDOW *w;
int line;
{
        char linebuf[60];
	char format[20];
        int i;
	int oldflags;
#ifdef COLPOS
	extern int mon_is_colour;
#endif

        for( i=0; i<(mdisp_width-4)/2; i++ ) linebuf[i] = ' ';
        linebuf[i] = 0;

        strcat( linebuf, "More" );
        wmove( w, line, 1 );
	oldflags = w->_flags;
#ifdef COLPOS
	if( mon_is_colour )
		reswflags( w, AC_GREEN|A_BOLD );
	 else
#endif
        	wstandout( w );
	sprintf( format, " %%-%ds ", mdisp_width );
	printt2( "Drawing 'more': format %s, string %s\n", format, linebuf);
        wprintw( w, " %-*s ", mdisp_width, linebuf );
	w->_flags = oldflags; /* should be a portable way to do this */
}

static
erase_more( w, line )
WINDOW *w;
int line;
{
        int i;

        wmove( w, line, 1 );
        for( i=0; i<mdisp_width+2; i++ ) waddch( w, ' ' );

#endif HAS_MENU
}

