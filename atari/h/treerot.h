extern char	*mmalloc();
extern		mfree();
extern nodep root_node;			/* root of the program tree */
extern listp super_root;		/* special parent of the root */

#ifdef HYBRID
extern char far *ex_checkalloc();
#endif

extern int	NodeCount;		/* number of nodes (0..NodeCount-1) */

#ifdef LOADABLE_TEMPLATES
extern struct node_info far * far *node_table;
extern ClassNum far * far *K_Classes;
extern ActionNum far * far *N_Actions;
extern bits8 far *expr_prec;
extern unsigned int *Node_Names;
extern char *Node_Strings;

/* the new broken out node flags and kid count */
extern nf_type  *lt_node_flags;
extern bits8    *lt_kid_count;
#else

#ifdef SMALL_TPL

extern	nf_type	lt_node_flags[];
extern	bits8	lt_kid_count[];
extern	bits8	lt_full_kids[];
extern	int	lt_class_val[];
extern	int	lt_K_Class[];

#else

extern struct node_info *node_table[];	/* master table of node information */
extern ClassNum *K_Classes[];		/* array of classes for the kids */
extern ActionNum *N_Actions[];		/* actions on nodes */
extern bits8 expr_prec[];		/* expression binding */
extern char *Node_Names[];		/* -> char string name for the node.*/

#endif

#endif 

extern char *Class_Names[];		/* default strings for the classes */
extern type_code *Class_Types[];	/* type codes for each class */
extern ActionNum *Class_Actions[]; /* -> list of ways to fill in stub.	*/
extern bits8 *Valid_Nodes[];		/* valid nodes at a class */
extern int cur_line;			/* line in window being output */
extern int srch_row, srch_scol, srch_width; /* physical locations for search */
extern curspos srch_cursor;
extern int srch_real;			/* the actual text of the nodep */
extern curspos start_cursor;		/* what line we display from */
extern bits8 out_flag;			/* type of output */	
extern int lookaheadchar;		/* what scanner will get next */
extern int sought_column;		/* column we look for in treeprint */
extern int form_column;		/* most recent column of node in treeprint */
extern curspos former_node;		/* most recent node in treeprint */
extern curspos pointed_node;		/* node found in treeprint */
extern int redraw_status;		/* flag to indicate redraw */
/* Major static tables */

/* Major global variables */

extern nodep forest[MAXTREES];	/* set of "buffers" (e.g. trees in memory) */
extern unsigned sc1_size;	/* size of the global stack frame */
extern nodep ex_loc;		/* execution location (former) */
extern nodep suspended;		/* suspended location */
#define root forest[0]		/* main buffer */

extern curspos cursor;		/* main node the cursor points at       */
extern curspos oldcur;		/* old cursor in select */
extern curspos anchor;		/* anchor of a selected range */
extern curspos cursend;		/* nonnull if sweeping out an area -    */
				/* running from cursor through cursend  */

#ifndef trace
extern FILE *trace;		/* for debugging - use if non-null	*/
#define dtrace trace
#endif
extern int md_level;		/* level of main tracing */
extern int novice;		/* is the user a novice */
extern FILE *etrace;
extern int ed_level;		/* level of execution debug */
extern int point_offset;		/* how far along node we were */
extern nodep sel_node;		/* selected nodep */
extern int sel_first;		/* first in a selected list */
extern int sel_last;		/* last in a selected list */
extern stackloc *exec_stack;	/* runtime stack */
#ifdef LARGE
extern unsigned SSeg;		/* segment for runtime stack*/
#endif
#ifndef ERR
# define	ERR	(0)
# define	OK	(1)
#endif

#define ABORT -1

#define COL_ERROR		2
#define COL_STUB		1
#define COL_DEFAULT		0
#define COL_KEYWORD		3
#define COL_OLEDIT		16
/* defines for token modifiers */
#define UT_ANY		0
#define UT_EXACT	0x10
#define UT_UPKID	0x20
#define UT_PCODE	0x40
#define LOOK_CURSOR 2			/* output mode */
#define LOOK_COLUMN 3			/* output mode column -> node map */
#define OLEOUT	    4			/* One line editor output mode */
#define LOOK_LOC 3			/* output mode for loc mapping */
#define MAX_LSEARCH 7			/* how far down to look for cursor */
#define YES 1
#define NO 0
#define ERROR -1

/* selection flags */
#define R_FORCE 1
#define R_NOLIST 2
#define R_LEAF 4
#define R_FLIST 8

#define SAVE_VERSION	4

#define NCAST (nodep)
#define LCAST (listp)
/* fake casts to preserve where they were */
#define FNCAST /* */
#define FLCAST /* */

/* general constants */

#define BELOW_EXPAND 0
#define ABOVE_EXPAND 1

#define SMALL_CHANGE 1
#define BIG_CHANGE 2
#define HUGE_CHANGE 4
#define LINE_INVERT 8
#define LINE_OFF 0x10
extern int phys_row, phys_column;	/* testing pointing */
#ifndef ES_TREE
extern struct symbol_node UnknownSymbol;
extern int built_count;		/* number of builtins */
extern node master_table; 		/* scope level 0 symbol table */
extern struct symbol_node ndlast;	/* symbol table 0 */
extern struct symbol_node BT_integer, BT_char, BT_real, BT_pointer,
		SP_file, SP_ordinal, SP_number, SP_string, SP_proc, SP_variable;
extern struct symbol_node null_undef;
extern char SP_passtype;
extern node BT_boolean, BT_set, BT_text;
#ifndef NOFUNCS
extern node BT_anytype;
#endif
#endif ES_TREE

/* parameters to slookup() */
#define CREATE		TRUE
#define NOCREATE	FALSE
#define REDECLS		TRUE
#define NOREDECLS	FALSE
#define MOAN		TRUE
#define NOMOAN		FALSE

/* interactive checking */

#define TC_DESCEND	0x80		/* descend line level kids */
#define TC_FULL		1		/* full check */
#define TC_QUICKIE	2		/* quickie, no errors at all */
#define TC_NOSTUBS	4		/* if a stub, no errors */
#define TC_MOVECURSOR	8		/* move cursor to first error */
#define TC_ONERROR	0x10		/* mark only the first error found */
#define TC_CHECKDIRTY	0x20		/* check dirty areas */
#define TC_INIT		0x40		/* fill in initializers */


extern char	FileExt[];
extern char	LibExt[];
extern char	TextExt[];
extern char	*NoExt;

#define	LOAD		0
#define	MERGE		1
#define LIBRARY		2

extern char far *	ScreenBuf;
