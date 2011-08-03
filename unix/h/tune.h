/*
 * Constants defining various maximum sizes, plus the location of files
 * on machines and certain other system dependent information.
 */

#define INT32 1
#define POINT32 1
#define TURBO 1
#define FULL 1
#define COLPOS 1

#ifdef unix
#define UNIX 1
#endif

#if defined(LARGE) && !defined(PARSER)
#define	ALICE_VERSION	131		/* 1.3 */
#else
#define	ALICE_VERSION	132		/* 1.3.3 */
#endif

#define MAXTREES	10		/* size of forest (# of buffers) */

#ifdef RELEASE
#define	HIST_SIZE	100		/* initial size of history list */
#else
#define HIST_SIZE	20
#endif
#define	HIST_EVENTS	10		/* max HIST_MARK delimited events */
#ifdef DEMO
# define MAX_STACK_SIZE 2500
# define LOC_STACK_SIZE 400
#else
# define MAX_STACK_SIZE 0		/* pick appropriate default */
# define LOC_STACK_SIZE 0
#endif

#ifdef INT32
# define MAXINT	2147483647
#else
# define MAXINT	32767
#endif


#define HASH_SIZE 101			/* hash table size for symbol table */
#define HTABSIZE 4
#define KBUF_SIZE 10			/* keyboard input buffer */

#define TURBO_MAXLINE 80
/* turbo style string length */
#define MAX_STR_LEN 255
/* buffer zone on runtime stack */
#define STACK_BUFFSZ 100
#define MAX_LINES 26			/* max folding */
#define MAX_SCOPE 10			/* max # of nested scopes */
#define LONG_KEYWORD 10			/* max length of language keyword */
#define MAX_NODE_KIDS 8			/* kids of a regular node */
#define MAX_LEN_LINE 255		/* max length of folded line */
#define MAX_NUM_LEN 50			/* max length of a number */
#define MAX_FN_LEN 16			/* file name in menu */
#define MAX_WS_NLEN 20			/* workspace name */
#define MAX_PCODE 127			/* indicating hightest pcode */
#define BITS_IN_BYTE 8
#define SET_BYTES 32
#define MIN_SET 0			/* lowest ordinal in a set */
#define MAX_SET 255			/* biggest value */
#define LIST_CHUNK	4		/* how many entries to grow lists by */
#define NIL NCAST 0
#define LNIL LCAST 0
#define NILSYM ((symptr)0)
#define NULLTYPE (type_code)0
#define NULLWORD (bits16)0
#define NULLBYTE (bits8)0
#define HELP_ESCAPE '|'			/* in help files */
#define MX_FILE_MENU 23
#define MX_FM_WIDTH 30
#define MAX_COLOURS 20			/* maximum types of objects */
#define MAX_BACKPATCH 60		/* maximum hide nodes */
#define MAX_F_OPEN 9			/* max files open */
#define STR_KID sizeof(pointer)/sizeof(nodep)	/* how many node pointers for a
						  char * */

#define MAX_LIST_KIDS	255		/* you can count this many in bits8 */

#define HIGHKEY 410			/* room for special keys thru UNDO */

#define	APIN		"apin"		/* text to AP converter */
#define FILEEXT		"ap"		/* file name extension for save files */
#define LIBEXT		"lap"		/* file name extension for libraries */


#define ALICE_DIR	"/usr/local/lib/alice"

#ifdef msdos
# define TURBOLIB	"?:\\turbolib.ap"
#else
# define TURBOLIB	"turbolib.ap"
#endif

#ifdef LOADABLE_TEMPLATES
# ifdef msdos
# define ALICETPL	"?:\\aptempla.suf"
# else
# define ALICETPL	"aptempla.suf"
# endif
#endif

#define TEXTEXT		"pas"		/*file name extension for text files */

#ifdef DEMO
#define _DEMO_LINES 5
#define _DEMO_BORD 3
extern int DEMO_LINES;
extern int DEMO_BORD;
#else
#define DEMO_LINES 0
#define DEMO_BORD 0
#endif
#define BORD 0

#ifdef TURBO
#define STR_START 1
#else
#define STR_START 0
#endif

#define	LOGFILE		"log"

#define PATHLEN		200		/* maximum file path length */

#define	BUILT_COUNT	273
#define MAX_TRACE 20
