/*
 * Atari 520ST curses package definitions
 */

#define SCREENIMAGE 1
#define bool    char
#define chtype  short
typedef chtype *cbufptr;

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define BITS_OR	7
#define BITS_REPLACE 3

#ifndef ERR
#define ERR (0)
#define OK  (1)
#endif

#define ESC     0x1b        /* Escape */

#define _SUBWIN     1
#define _ENDLINE    2
#define _FULLWIN    4
#define _SCROLLWIN  0x8
#define _FLUSH      0x10
#define _KEYPAD     0x20
#define _WRITTENTO  0x40

#define AC_BLACK	0x0100
#define BC_BLACK	0x01

#define	A_UNDERLINE	0x1000
#define A_DOTUNDERLINE  0x2000
#define A_REVERSE	0x4000
#define A_BOLD		0x8000

#define A_CURSOR	0x01		/* Cursors are always black */

#define FirstPlane	0x01

#define A_ATTRIBUTES    0xff00
#define A_CHARTEXT  	0xff
#define A_STANDOUT	A_REVERSE

#define REV_BIT	    A_REVERSE

#define _STANDOUT   A_STANDOUT

/* For now, say that we aren't a colour device */
#ifdef notdef
#define COLPOS 1
#endif

#define _NOCHANGE   -1

typedef short colnum;		/* holds a row or column */

struct _win_st {
    short	_cury, _curx;		/* cursor position */
    short	_maxy, _maxx;		/* dimensions of window */
    short	_begy, _begx;		/* location of upper left */
    short	_flags;			/* screen attrs, etc */
    bool	_clear;
    bool	_leave;			/* actually cursor on/off */
    bool	_scroll;		/* window can scroll */
    cbufptr	*_y;
    colnum	*_firstch;		/* line is dirty */
    colnum	*_lastch;		/* currently unused */
    colnum	c_oldx, c_oldy;		/* old cursor */
    short	c_background;		/* background colour */
    colnum	*_firstnw;		/* first non white */
    colnum	*_lastnw;		/* last non white */
};

#ifdef MIN_OUT
# define FIRARS 4
# define isdirty(win,y)	( win->_lastch[y] > 0 )
#else
# define FIRARS 1
# define isdirty(win,y)	win->_firstch[y]
# define clean_line(win,y) win->_firstch[y] = FALSE
#endif

#define _hascur _leave

typedef struct _win_st WINDOW;

#define MX_WIN_WIDTH 80			/* how wide a window can be */	
/* extended curses */
#define ECURSES 1

extern int  LINES, COLS;

extern WINDOW   *stdscr, *curscr;

/*
 *  Define VOID to stop lint from generating "null effect"
 * comments.
 */
#define VOID    /* some compilers can't handle this */

/*
 * psuedo functions for standard screen
 */
#define addch(ch)   VOID(waddch(stdscr, ch))
#define getch()     VOID(wgetch(stdscr))
#define addstr(str) VOID(waddstr(stdscr, str))
#define getstr(str) VOID(wgetstr(stdscr, str))
#define move(y, x)  VOID(wmove(stdscr, y, x))
#define clear()     VOID(wclear(stdscr))
#define erase()     VOID(werase(stdscr))
#define clrtobot()  VOID(wclrtobot(stdscr))
#define clrtoeol()  VOID(wclrtoeol(stdscr))
#define insertln()  VOID(winsertln(stdscr))
#define deleteln()  VOID(wdeleteln(stdscr))
#define refresh()   VOID(wrefresh(stdscr))
#define insch(c)    VOID(winsch(stdscr,c))
#define delch()     VOID(wdelch(stdscr))
#define standout()  VOID(wstandout(stdscr))
#define standend()  VOID(wstandend(stdscr))
#define attrstr(st) VOID(wattrstr(stdscr,st))
#define clrfrombeg() VOID(wclrfrombeg(stdscr))

/*
 * mv functions
 */
#define mvwaddch(win,y,x,ch)    VOID(wmove(win,y,x)==ERR?ERR:waddch(win,ch))
#define mvwgetch(win,y,x)   VOID(wmove(win,y,x)==ERR?ERR:wgetch(win))
#define mvwaddstr(win,y,x,str)  VOID(wmove(win,y,x)==ERR?ERR:waddstr(win,str))
#define mvwgetstr(win,y,x)  VOID(wmove(win,y,x)==ERR?ERR:wgetstr(win))
#define mvwinch(win,y,x)    VOID(wmove(win,y,x) == ERR ? ERR : winch(win))
#define mvwdelch(win,y,x)   VOID(wmove(win,y,x) == ERR ? ERR : wdelch(win))
#define mvwinsch(win,y,x,c) VOID(wmove(win,y,x) == ERR ? ERR:winsch(win,c))
#define mvwattrstr(win,y,x,ar) VOID(wmove(win,y,x) ==ERR? ERR:wattrstr(win,ar))
#define mvwclrfrombeg(win,y,x) VOID(wmove(win,y,x) == ERR? ERR:wclrfrombeg(win))
#define mvaddch(y,x,ch)     mvwaddch(stdscr,y,x,ch)
#define mvgetch(y,x)        mvwgetch(stdscr,y,x)
#define mvaddstr(y,x,str)   mvwaddstr(stdscr,y,x,str)
#define mvgetstr(y,x)       mvwgetstr(stdscr,y,x)
#define mvinch(y,x)     mvwinch(stdscr,y,x)
#define mvdelch(y,x)        mvwdelch(stdscr,y,x)
#define mvinsch(y,x,c)      mvwinsch(stdscr,y,x,c)

/*
 * psuedo functions
 */
#define clearok(win,bf)  
#define	scrollok(win,canscr) (win->_scroll = canscr)
#define getyx(win,y,x)   (y = win->_cury, x = win->_curx)

#define leaveok(win,bf)  
#define flushok(win,bf)  
#define inch()      VOID(winch(stdscr))

extern WINDOW   *initscr(), *newwin(), *doswin(), *subwin();

#define SPACE ' '
