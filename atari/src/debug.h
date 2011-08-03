/*
 * Control of debugging defines.
 * DEBUG must be defined or all debugging is turned off.
 * Others control individual debugging outputs.
 */
#ifndef RELEASE

#define DEBUG 1

#define XDEBUG 1

#endif

#define UDEBUG 1
#ifdef LARGE
#define BIG 1
#define REALLY_BIG 1
#endif

#ifdef HERE
# define here(n)                 (tracing? fprintf(trace, "At line %d in %s\n", __LINE__, __FILE__) : 0)
#else
# define here(n)
#endif

#define checkfar(n)

#ifdef DMFREE
# define dmfree(p)              mfree(p, __FILE__, __LINE__)
#else
# define dmfree(p)              mfree(p)
#endif DMFREE

#ifdef DEBUG

# define tracing        (trace != NULL)

#define	DB_GRAPHICS  1
#define	DB_EXMEM 1
#define DB_SEGALLOC 1
#define DB_FORM 1

/* #define DB_MEM 1 */
/* #define DB_LOADNODES	1 */

/*#define DB_MALLOC 1*/
/*#define DB_NEWEDIT 1*/

/* #define DB_ACTION 1  */
/* #define DB_ACTROUT 1  */
/* #define DB_ALSYM 1  */
/*#define DB_CMDS2 1 */
#define DB_COMMAND 1
/*#define DB_COMMON 1*/
/*#define DB_DISPLAY 1 */
#define DB_DUMP 1
/* #define DB_EXPPARSE 1 */
/*#define DB_HELP 1*/
/*#define DB_HISTORY 1*/
/*#define DB_KEYBOARD 1*/
/*#define DB_LOAD 1 */
#define DB_MAIN 1
#define DB_MAINLOOP 1
/*#define DB_MARK 1*/
/*#define DB_MENU 1*/
/* #define DB_OLEDIT 1 */
/*#define DB_OUTFUNCS 1*/
/* #define DB_OUTPUT 1 */
#define DB_POP 1 
/*#define DB_RUNTIME 1 */
/*#define DB_SAVE 0 */
/*#define DB_SCAN 1 */
/*#define DB_SUBR 1*/
/* #define DB_TREEPRINT 1 */
/*#define DB_TEXT 1*/
/* #define DB_TREESUBR 1 */
#define DB_WIN 1
/* #define DB_TYPECHECK 1 */
/*#define DB_TYPES 1*/
/*#define DB_WS 1*/
/*#define DB_SEARCH 1 */
/*#define DB_SCAN_DECL*/
/*#define DB_FILENAME 1*/

# define Stomp_Check 1  /* enable edit tree child number checking */

#else

# define tracing 0
/*#define Stomp_Check 0*/

#endif

/* Defines to turn off features to make alice fit in 64K */


/* See also DB_DUMP above */

#define HAS_POINTING	1

#  define HAS_ACT       1
#  define HAS_EDITOR    1
#  define HAS_EXPPARSE  1
#  define HAS_HELP      1
#  define HAS_INTERP    1
#  define HAS_EXP       1
#  define HAS_IOFUNCS   1
#  define HAS_LOAD      1
#  define HAS_SAVE      1
#  define HAS_TYPECHECK 1
#  define HAS_UNDO      1
#  define HAS_COM       1
#  define HAS_SEARCH    1
#  define HAS_WORKSPACE 1
#  define HAS_MARK      1
#  define HAS_MENU      1
#  define HAS_ACT       1
