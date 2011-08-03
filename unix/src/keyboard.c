/*
 *DESC: Keyboard input routines, called by scanner and editor.  These routines
 *DESC: handle keyboard macros, which do simple character substitution.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "scan.h"
#include "keys.h"

#ifdef DB_KEYBOARD
# define PRINTT
#endif
#include "printt.h"

int lookaheadchar = 0;		/* should be an inchar */
static int domacros = 1;	/* nonzero if macros to be expanded */

char *keymacros[HIGHKEY];	/* key maps to string if nonnull */

inchar *macp = NULL;		/* If in a macro, this is what's left */
inchar *scaninptr;		/* For force feeding input */

char macbuf[256];		/* place where macros are pushed */



/*
 * Return one keystroke from the keyboard.  If a macro has been invoked
 * or we have unread a character, we'll return that first.  Note a difference
 * in macro conventions:
 * (1) lookaheadchar is to unread a single character we just read.
 * (2) scaninptr is used for internal scanning and returns EOFCHAR when done.
 * (3) macp is used for keyboard macros and gives no indication when done.
 * (4) otherwise we just read from the keyboard.
 */
int
readkey()
{
	register int c;		/* the character */
	int x,y;		/* position on screen */
	extern int bugcount;	/* used to detect infinite loops */

	printt5("readkey, lookaheadchar '%c' (%o), domacros %d, macp '%s', scaninptr '%s'\n",
		lookaheadchar, lookaheadchar, domacros,
		macp ? macp : "(null)", scaninptr ? scaninptr : "(null)");

	if (lookaheadchar) {
		c = lookaheadchar;
		lookaheadchar = 0;
		return c;
	}

	if (domacros && macp) {
		c = *macp++;
		if (*macp == 0)
			macp = NULL;
		printt2("macp char %o, macp now %x\n", c, macp);
		c &= 0377;
		if (c)
			return c;
	}

	if (scaninptr) {
		c = *scaninptr++;
		if (c == 0) {
			scaninptr = 0;
			c = EOFCHAR;
		}
		printt2("scaninptr char %o, scaninptr now %x\n", c, scaninptr);
		return c & 0377;
	}

	/*
	 * If we ran off the end of the internal input,
	 * don't try to get it from keyboard
	 */
	if (domacros == 0) {
		printt0("no domacros, return EOFCHAR\n");
		return EOFCHAR;
	}

	clear_em(FALSE);

	c = keyget();
	printt3("readkey reads '%c' (%o), keymacros '%s'\n", c,c, keymacros[c]);
	bugcount = 0;	/* got key, must not be in a loop */

checkmac:
	if (domacros && c < HIGHKEY && keymacros[c]) {
		if (domacros == 2 && isolechar(c)) {
			return c;
			}
		macpush(keymacros[c]);
		c = *macp++ & 0377;
		if (*macp == 0)
			macp = NULL;
	}
	return c;
}

/* checks for and gets lookahead.  null if none */

static unsigned int incom_buf[KBUF_SIZE];
int inbuf_count = 0;

get_lookahead()
{
	if( lookaheadchar )
		return lookaheadchar;
	if( domacros && macp )
		return *macp;
	if( scaninptr )
		return *scaninptr;
	if( inbuf_count )
		return incom_buf[0];
	return 0;
}

/*
 * Push the string "str" onto the front of the buffered input (macp)
 * Note that we have no provision for pushing keys numbered > 255.
 */
macpush(str)
char *str;
{
	register char *d;
	int slen = strlen(str);

	if (macp == NULL || *macp == 0) {
		/* No macro currently active */
		strcpy(macbuf, str);
		macp = macbuf;
		return;
	}

	if( macp - macbuf + slen > sizeof(macbuf)-1 )
		return;	/* can't fit this macro */

	/* Otherwise, macp must point into macbuf */
	blk_move( macbuf+slen, macp, strlen(macp)+1 );
	blk_move( macbuf, str, slen );
	macp = macbuf;
}

/* do a simple macro definition */
macdef(args)
char *args;
{
	register char *p;
	char buf[256];
	register int key;
	int c;
	char *defn;
	char *termkey;
	char *savekm;
	char *cp;			/* position of last command char */
	char *crp;			/* postiion of last '\n' */
	extern char *strrchr();

	printt1("macdef args '%s'\n", args ? args : "");
	if (args && (p = strchr(args, ' '))) {
		*p++ = 0;
		key = atoi(args);
		defn = p;
	} else {
		char *macsave;
		setmac(0);
		message(ER(214,"Press the key you want to place the macro on:"));
		key = keyget();

		if (key >= HIGHKEY)
			bug(ER(258,"Can't put a macro on key # %d!"), key);
		message(ER(215,"Type the macro text and press ENTER. See the manual for details on \\ codes.") );
		macsave = keymacros[key];
		keymacros[key] = (char *)0;	/* make sure this macro is dead*/
		getprline( "Expansion: ", buf, sizeof(buf) );

		setmac(1);
		keymacros[key] = macsave;
		if( strlen( buf ) == 0 ) {
			message(ER(216,"macro definition aborted"));
			return;
			}
		defn = buf;
	}
	printt2("macdef assigns %o string '%s'\n", key, defn);
	macroize( defn );

	/*
	 * Make sure we don't create a "command" macro without
	 * a trailing carriage return, or confusion will result.
	 */
	cp = strrchr(defn, AL_ICOMMAND);
	/* don't give error if single \c on a line */
	if (cp && defn[1] ) {
		crp = strrchr(defn, '\n');
		if (!crp || cp > crp)
			error(ER(64,"comac`Command macros must be completed with '\\n'"));
	}
		
	keymacros[key] = allocstring( defn );
}


/*
 * Throw away all queued input, e.g. if an error occurred.
 */
flushla()
{
	lookaheadchar = 0;
	scaninptr = NULL;
	macp = NULL;
	domacros = 1;
}

/*
 * set the lookahead character to ch.
 */
setlachar(ch)
int ch;	/* should be an inchar */
{
	printt1( "setlachar to '%c'\n", ch);
	lookaheadchar = ch;
}

/*
 * Set the macro expansion flag to the value given.  Possible values are:
 * 0	don't expand any macros - used for internally generated input.
 * 1	expand all macros - used for input from the keyboard.
 * 2	expand all macros except AL_TERM - used for keyboard input from oledit.
 *
 * setmac also saves and restores the buffered input - it saves it when
 * macro expansion is turned off (0) and restores it when turned on (1 or 2).
 */
static int savelac;	/* saved lookaheadchar */
static char *savemacp;	/* saved macro ptr */
static char *savesc;	/* saved scaninptr */

setmac(xnewval)
int xnewval;
{
	register int newval;
	newval = xnewval;
	printt3( "set macros from %d to %d, lookaheadchar %o\n", domacros, newval, lookaheadchar);
	if (newval != 0 && domacros == 0) {
		printt8("setmac 1, restore lookaheadchar %o (was %o),\nmacp %o (%s) (was %o), scaninptr %o (%s) (was %o)\n",
			savelac, lookaheadchar, savemacp, savemacp, macp,
			savesc, savesc, scaninptr);

		lookaheadchar = savelac;

		macp = savemacp;
		scaninptr = savesc;
	} else if (newval == 0 && domacros != 0) {
		printt5("setmac 0, save lookaheadchar %o, macp %o (%s), scaninptr %o (%s)\n",
			lookaheadchar, macp, macp, scaninptr, scaninptr);
		savelac = lookaheadchar;
		savemacp = macp;
		savesc = scaninptr;
	}
	domacros = newval;
}

#ifdef DEMO

static int string_mode = FALSE;
FILE *demo_file;
extern int demo_mode;
extern WINDOW *demo_win;

demo_pause( time )
int time;
{
	int i,j;
	int tc;
	wrefresh( demo_win );
	if( demo_mode == 1 )
		pauseget( time > 5 );
	 else {
		j = 0;
		for( tc = 0; tc <= time * 100; tc += 25 ) {
			/* dos hundredths of a second delay */ 
			if( tst_charwait() ) {
				pauseget( time > 5 );
				return;
				}
			tdelay( 25 );
			}
		}
}

pauseget( mustret )
int mustret;		/* key must be return */
{
	int ch;

	do {
		ch = wgetch(stdscr);
		if( ch == 033 ) {
			done("Y");
			}
	} while( mustret && (ch != '\n' && ch != '\r' ) );
}

static int keyscount = 0;

#define DEMO_ESC '/'
demoget()
{
	int ch;

	if( string_mode ) {
		ch = dgetc();
		if( ch != DEMO_ESC ) {
			if( string_mode != -2 && string_mode != 2 )
				demo_pause(0);
			if( string_mode > 0 )
				chr_output( ch );
			return ch;
			}
		 else
			string_mode = FALSE;
		}

	for( ;; ) {
		ch = dgetc();
		if( ch == EOF)
			done("Y");
		switch( ch ) {
		 case DEMO_ESC :
			ch = dgetc();
			switch( ch ) {
				case DEMO_ESC:

				case '{': /* } */
					/* output an open brace  or slash */
					waddch( demo_win, ch );
					continue;
				case 'k': /* prepare for keyboard input */
					wattron( demo_win, A_BOLD );
					waddstr( demo_win, "Keys: " );
					if( keyscount++ < 3 && demo_mode == 1 )
						waddstr(demo_win,
							"[Press SPACEs now] ");
					wattroff( demo_win, A_BOLD );
					continue;
				case 'c': /* clear demo window */
					werase( demo_win );
					wmove( demo_win, 0, 0 );
					continue;
				case 'C': /* clear main screen */
					{
					int i;
					for( i = 5; i <= 13; i++ ) {
						wmove( stdscr, i, 0 );
						wclrtoeol(stdscr);
						}

					wrefresh(stdscr);
					continue;
					}

				case 'p':
					/* pause for a new screen */
					if( demo_mode == 1 ) {
						static char contkey[] = "Press ENTER to continue";
					    wmove(demo_win,demo_win->_maxy-1,
							demo_win->_maxx -
							sizeof( contkey) );
						wattron( demo_win, A_BOLD );
						waddstr( demo_win,contkey );
						wattroff( demo_win, A_BOLD );
						}
					wrefresh( demo_win );
					demo_pause(7);
					continue;
				case '"':
					/* simulate input of a string, echo
					   and pause after each char */
					string_mode = -1;
					demo_pause(1);
					return dgetc();
				case 's':
					/* string input, no echo, no pause */
					string_mode = -2;
					return( dgetc() );
				case '\047':  /* single quote */
					string_mode = 2; /* echo, no pause */
					return( chr_output( dgetc() ) );

				/* simulate the pressing of certain keys */

				case 'u':
					ch = KEY_UP;
					break;
				case 'd':
					ch = KEY_DOWN;
					break;
				case 'r':
					ch = KEY_RIGHT;
					break;
				case 'h':
					ch = AL_HELP;
					break;
				case 'f':
					ch = KEY_F(dgetc() - '0');
					break;
				case 'i':
					ch = AL_ID;
					break;
				case 't':
					ch = '\t';
					break;
				case 'Q':
					/* quit into alice with a dot, or
					   out with anything else */
					if(wgetch(stdscr) == '.' ) {
						demo_mode = 0;
						fclose(demo_file);
						waddstr( demo_win, "Enjoy yourself - You're now using ALICE interactively" );
						wrefresh(demo_win);
						return keyget();
						}
					continue;
				case 'T':
					/* conditional terminate */
					if( demo_mode != 1 ) 
						done("Y");
					continue;

				case '\n':
					continue;
				
				}
			/* broke out, must want returned */
			 demo_pause(1);
			 chr_output( ch );
			 return ch;
			 break;
		 case '{':	/* single quote, typed char } */
			/* string input, echo and pause */
			string_mode = 1;
			ch = dgetc();
			demo_pause(1);
			chr_output( ch );
			return ch;

		 case '\n':
			wrefresh( demo_win );
		 default:
			waddch( demo_win, ch );
		 }
		}
}


dgetc()
{
	int ch;

	ch = fgetc( /* */demo_file );
	if( ch < 0 ) {
		done("Y");
		}
	return ch;
}
#endif DEMO

#ifdef KEYFILE
FILE *keyfile = 0;
int keyfwrite = 0;
#endif

keyget()
{
	int gotch;
	int tomove;
#ifdef KEYFILE
	if( keyfile  && !keyfwrite ) {
		gotch = fgetc( keyfile );
		if( gotch >= 0 )
			return gotch;
		fclose( keyfile );
		keyfile = 0;
		}
#endif
#ifdef DEMO
	if( demo_mode )
		return demoget();
#endif
#if defined(unix) || defined(ICON) || defined( ATARI520ST )
#ifdef ATARI520ST
	return GemEvent();
#else 
	return wgetch(stdscr);
#endif
#else
	while( inbuf_count < KBUF_SIZE && (inbuf_count == 0 || tst_charwait()) )
		incom_buf[inbuf_count++] = wgetch(stdscr);

	gotch = incom_buf[0];
	/* hope size zero works */
	if( tomove = sizeof(unsigned int) * --inbuf_count )
		blk_move( incom_buf, incom_buf+1, tomove );
		
#ifdef KEYFILE
	if( keyfile ) {
		fputc( gotch, keyfile );
		fflush( keyfile );
		}
#endif
	return gotch;
#endif
}

#ifdef DEMO
/* in non demo, the above are #defines to zero */
int DEMO_LINES, DEMO_BORD;

init_demo()
{
	if( demo_mode ) {
		demo_file = fopen( "ap1.dem", "r" );
		if( !demo_file ) {
			printf( "No Demo File found" );
			done("Y");
			}
		DEMO_BORD = _DEMO_BORD;
		DEMO_LINES = _DEMO_LINES;
		}
	 else
		DEMO_BORD = DEMO_LINES = 0;
}
struct keytab {
	int key_ch;
	char *key_knam;
	} ourtab[] = {
KEY_UP, "Up",
KEY_DOWN, "Down",
KEY_LEFT, "Left",
KEY_RIGHT, "Right",
#ifdef ICON
AL_ID, "Complete",
#else
AL_ID, "End",
#endif
AL_DELETE, "Del",
AL_INSERT, "Ins",
033, "Esc",
#ifdef ICON
AL_HELP, "HELP",
#else
AL_HELP, "HELP(F9)",
#endif
'\t', "Tab",
8, "Backspace",
'\n', "Enter",
'\r', "Return",
0, 0};


chr_output( ch )
int ch;
{
	register int i;
	int y,x;
	char keybuf[20];

	keybuf[1] = 0;
	keybuf[0] = ch;

	for( i = 0; ourtab[i].key_ch; i++ )
		if( ourtab[i].key_ch == ch ) {
			strcpy( keybuf, ourtab[i].key_knam );
			goto foundit;
			}
	if( ch < ' ' )
		sprintf( keybuf, "Ctrl-%c", ch + '@' );
	 else if( ch >= KEY_F(0) && ch <= KEY_F(10) )
		sprintf( keybuf, "F%d", ch - (KEY_F(0))  );
 foundit:
	getyx( demo_win, y, x );
	if( x + strlen(keybuf) + 2 >= demo_win->_maxx )
		waddch( demo_win, '\n' );
	wstandout( demo_win );
	waddstr( demo_win, keybuf );
	wstandend( demo_win );
	waddch( demo_win, ' ' );
	wrefresh( demo_win );
	return ch;
}

#endif
