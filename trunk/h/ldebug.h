
/*
 * Control of debugging defines.
 * DEBUG must be defined or all debugging is turned off.
 * Others control individual debugging outputs.
 */

#ifndef msdos
#define DEBUG 1
#define XDEBUG 1
#endif

#define SAI_SCREEN 1
#define SAI_NORMAL 2

#ifdef HERE
# define here(x)			gotHere(__FILE__, __LINE__)
#else
# define here(x)
#endif

#ifdef DMFREE
# define dmfree(p)		mfree(p, __FILE__, __LINE__)
#else
# define dmfree(p)		mfree(p)
#endif DMFREE

#ifdef DEBUG

# define tracing	(trace != NULL)

/*#define	DB_MALLOC 1*/
#define DB_SEGALLOC 1 
/*#define DB_ACTION 1 */
/*#define DB_ACTROUT 1 */
/*#define DB_ALSYM 1 */
/*#define DB_CMDS2 1*/
/*#define DB_COMMAND 1*/
/*#define DB_COMMON 1*/
/*#define DB_DISPLAY 1 */
#define DB_DUMP 1
/*#define DB_EXPPARSE 1*/
/*#define DB_HELP 1*/
/*#define DB_HISTORY 1*/
/*#define DB_KEYBOARD 1*/
/*#define DB_LOAD 1  */
/*#define DB_MAIN 1*/
/*#define DB_MARK	1 */
/*#define DB_MENU 1 */
/*#define DB_RUNTIME 1*/
/*#define DB_SAVE 0 */
/*#define DB_SCAN 1 */
/*#define DB_SUBR 1  */
/*#define DB_TREEPRINT 1*/

/*#define DB_TEXT 1*/
/*#define DB_TREESUBR 1*/
/*#define DB_TYPECHECK 1*/
/*#define DB_TYPES 1*/
/*#define DB_WS 1*/
/*#define DB_SEARCH 1 */

/* # define Stomp_Check 1 */	/* enable edit tree child number checking */

#else

# define tracing 0

#endif

/* Defines to turn off features to make alice fit in 64K */


/* See also DB_DUMP above */

#ifdef QNX
/* # define MULTI_PROC	1 */
# ifdef INTERPRETER
#  define HAS_INTERP	1
#  define HAS_EXP	1
/* #  define PROC_INTERP 1 */
#  define HAS_TYPECHECK 1
#  define HAS_IOFUNCS 1
# else
#  define PROC_EDITOR 	1
	/* also turn on HAS_EDITOR in this case */
# endif

#else
/*#define HAS_INTERP	0*/
#endif

#ifdef SAI 

#  define HAS_EDITOR	1
#  define HAS_EXPPARSE	1
#  define HAS_HELP	1
#  define HAS_INTERP	1
#  define HAS_EXP	1
#  define HAS_IOFUNCS	1
#  define HAS_LOAD	1
#  define HAS_SAVE	1
#  define HAS_TYPECHECK	1
#  define HAS_UNDO	1
#  define HAS_COM	1
#  define HAS_SEARCH	1
#  define HAS_WORKSPACE 1
#  define HAS_MARK	1
#  define HAS_MENU	1
#  define HAS_ACT	1

#else SAI 
# ifdef INTERP
#  define HAS_INTERP	1
#  define HAS_LOAD	1
#  define HAS_ACT	1
#  define HAS_EDITOR	1
#  define HAS_EXPPARSE	1
#  define HAS_HELP	1
#  define HAS_LOAD	1
#  define HAS_SAVE	1
#  define HAS_TYPECHECK	1 
#  define HAS_UNDO	1
#  define HAS_MENU	1
#  define HAS_COM	1
#  define HAS_MARK	1
# endif
#endif
