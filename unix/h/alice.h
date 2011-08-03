/*
 * alice.h: common point to include every alice related header file.
 */

/* The whoami.h file defines parameters about the system we're running on */

#include <whoami.h>
#define CAST(ctp)  (ctp)
#define CNTL(x)	(x & 037)
#ifndef TRUE
# define TRUE (1)
# define FALSE (0)
#endif

#ifndef ECURSES
#define A_ATTRIBUTES 0x80
#define A_CHARTEXT 0x7f
#endif

/*
 * If we are in the stand-alone interpreter, dont load templates from
 * disk, and disable the trace features
 */

#ifdef SAI
#define SMALL_TPL	1
#endif

#ifndef SMALL_TPL
#define LOADABLE_TEMPLATES 1
#define Trace 1
#endif

#define	NOKEYS		1

#ifndef BUFSIZ
#include <stdio.h>
#endif

#ifdef QNX
#ifdef INCLUDEIO
#include <io.h>		/* Stupid QNX redeclares type of FILE */
#endif
#endif


#ifdef qnx
#define void int
#endif

/* Decide what type of debug control file to load. */

#ifdef MAP
# include "mdebug.h"
# define NOBFUNC
#else
# ifdef INTERP
#  include "idebug.h"
#  define NOBFUNC
# else INTERP
#  ifdef SAI 
#   include "ldebug.h"
#   ifndef FUNCTIONS
#     ifdef ES_TREE
#       define NOBFUNC
#     endif
#   endif
#  else SAI 
#   include "debug.h"
#   ifndef FUNCTIONS
#    ifdef ES_TREE
#     define NOBFUNC
#    endif /* ES */
#   endif /* FUNCTIONS */
#  endif /*SAI */
# endif /* INTERP */
#endif /* MAP */

/* Define alice types */

#include "altypes.h"

/* Define alice tunable parameters */

#include "tune.h"

/* external declarations for many functions used in editing */
#ifndef NOFUNCS
# include "functions.h"
#endif

#if defined(msdos)
# define msdosGraphics
#endif

/* include the master macro file */

#ifdef ES
# include "esmacros.h"
# include "esbuilt.h"
#else
# ifdef ES_TREE
#  include "esbuilt.h"
#  include "hmacros.h"
# else
#  include "macros.h"
# endif
#endif

/* general type definitions and function definitions */
#include "node.h"
#include "treerot.h"

#define AC_BLACK     COLOR_PAIR(7)
#define AC_RED       COLOR_PAIR(1)
#define AC_GREEN     COLOR_PAIR(2)
#define AC_YELLOW    COLOR_PAIR(3)
#define AC_BLUE      COLOR_PAIR(4)
#define AC_MAGENTA   COLOR_PAIR(5)
#define AC_CYAN      COLOR_PAIR(6)
#define AC_WHITE    COLOR_PAIR(8)
#define AC_NORMAL	0

