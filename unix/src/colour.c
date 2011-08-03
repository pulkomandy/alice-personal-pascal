
/*
 *DESC: Routines to output a segment of the screen to a window
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "flags.h"
#include "break.h"
#include "search.h"

buf_el Colour_Map[] = {
AC_NORMAL,	       /* default */
A_UNDERLINE|AC_BLUE,	 /* stub */
A_BOLD|AC_RED,	  /* error */
A_BOLD|AC_NORMAL,	/* keywords */
AC_NORMAL,	       /* variables */
AC_NORMAL,		/* named constants */
A_BOLD,      /* comments */
AC_NORMAL,	       /* char constant */
AC_NORMAL,	       /* integer constant */
AC_NORMAL,	       /* real constant */
AC_NORMAL,	       /* string */
AC_BLUE|A_DIM,	/* routine names */
AC_BLUE,		/* built in routines */
AC_NORMAL,	       /* field names */
A_REVERSE|AC_MAGENTA,  /* undefined symbols */
AC_NORMAL,	       /* labels */
AC_NORMAL		/* one line edit */
};

#ifdef IBMPC
#define DEFC AC_NORMAL
#else
#define DEFC 0
#endif

#ifndef A_BOLD
#define A_BOLD 0
#endif
/* map for colour card, b&w monitor  */
buf_el Attr_Map[MAX_COLOURS] = {
DEFC,		      /* default */
A_BOLD|DEFC,		 /* stub */
DEFC|A_STANDOUT,	     /* error */
DEFC,		      /* keywords */
DEFC,		      /* variables */
DEFC,		      /* named constants */
DEFC,		      /* comments */
DEFC,		      /* char constant */
DEFC,		      /* integer constant */
DEFC,		      /* real constant */
DEFC,		      /* string */
DEFC,		      /* routine names */
DEFC,		      /* built in routines */
DEFC,		      /* field names */
DEFC|A_STANDOUT,	     /* undefined symbols */
DEFC,		      /* labels */
A_BOLD|DEFC		  /* one line edit */
};

#ifndef A_UNDERLINE
#define A_UNDERLINE 0
#endif
/* map for monochrome display */
buf_el Mono_Map[MAX_COLOURS] = {
DEFC,		      /* default 0 a */
A_UNDERLINE,	     /* stub 1 b */
DEFC|A_STANDOUT,	     /* error 2 c */
DEFC,		      /* keywords 3 d */
DEFC,		      /* variables 4 e */
DEFC,			/* named constants 5 f */
A_BOLD|DEFC,		/* comments 6 g */
DEFC,		      /* char constant 7 h */
DEFC,		      /* integer constant 8 i */
DEFC,		      /* real constant 9 j */
DEFC,		      /* string 10 k*/
DEFC,		      /* routine names 11 l */
DEFC,		      /* built in routines 12 m */
DEFC,		      /* field names 13 n */
DEFC|A_STANDOUT,		/* undefined symbols 14 o */
DEFC,		      /* labels 15 p */
A_UNDERLINE	     /* one line edit 16 MOVE THIS */
};

/* map from symbol types to our colour types */
bits8 Symc_map[] = {
0,
15, 5, 5, 4, 4, 
4, 4, 14,
13, 4, 4, 4, 11, 11,
11, 11, 12, 12, 12, 12, 12, 12,
5, 5, 4, 4,
/* BENUM, MEM, PORT */
5, 4, 4
};

#ifdef unix
int montype = 1;
#else
int montype = 0;		/* monitor type. 1=underline 2=colour */
#endif

/* set display type and colour maps */


set_mtype( val )
int val;
{
#ifdef msdos
	if( val )
		montype = val;
	if( !montype ) {
		extern int can_underline;
		extern int mon_is_colour;

		/* here 1 means monochrome and 2 means colour montitor */
		if( can_underline )
			montype = 1;
		if( mon_is_colour )
			montype = 2;
		}
#else
	start_color();
	montype = has_colors() ? 2 : 1;
	init_color( COLOR_GREEN, 0, 300, 0 );
	init_color( COLOR_YELLOW, 200, 200, 0 );
	init_color( COLOR_CYAN, 0, 200, 200 );
	init_pair(  PAIR_NUMBER(AC_RED),      COLOR_RED, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_GREEN),	COLOR_GREEN, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_YELLOW),	COLOR_YELLOW, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_BLUE),	COLOR_BLUE, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_MAGENTA), COLOR_MAGENTA, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_CYAN), 	COLOR_CYAN, COLOR_WHITE );
	init_pair(  PAIR_NUMBER(AC_WHITE), 	COLOR_BLACK, COLOR_WHITE );
#endif
	if( montype == 1 )
		blk_move( Attr_Map, Mono_Map, sizeof(Attr_Map) );
	else if( montype == 2 )
		blk_move( Attr_Map, Colour_Map, sizeof(Attr_Map) );
}

setscol( colour )
unsigned colour;
{
	extern int mon_is_colour;

	if( montype > 1 )
		attrset( colour );
}
