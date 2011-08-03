/* Definitions of types defined and used by Alice */

/*
 * ireg? are integer registers, preg? are pointer registers.
 *
 * Apparently some compilers generate not-so-good code if you ask for
 * too many registers, hence these defines.
 */
#ifdef QNX
# define	ireg1	register
# define	ireg2
# define	ireg3
# define	ireg4
# define	ireg5
# define	preg1	register
# define	preg2	register
# define	preg3
# define	preg4
# define	preg5
#else
# define	ireg1	register
# define	ireg2	register
# define	ireg3
# define	ireg4
# define	ireg5
# define	preg1	register
# define	preg2	register
# define	preg3
# define	preg4
# define	preg5
#endif

#ifdef QNX
# define	reg
#else
# define	reg	register
#endif

#ifndef HYBRID
#define far
#ifndef SAI
#define near
#endif
#endif


typedef unsigned char bits8;
#ifdef QNX
typedef unsigned bits16;
#else
typedef unsigned short bits16;
#endif QNX
typedef unsigned char	Boolean;
typedef bits8 NodeNum;
typedef bits8 ClassNum;
typedef bits8 ActionNum;
typedef bits8 stackloc;	/* base pointer type on stack */
typedef unsigned nf_type;	/* node flag type */
typedef short StateNum;

typedef char	*treep;

#ifdef FLOATING
	typedef double rfloat;		/* runtime floating point */
# define FLOATNUM(x)	x
#else
	typedef int rfloat;
# define FLOATNUM(x)	0
#endif
typedef int rint;		/* runtime integer */
#if defined(LARGE) || defined(HYBRID) || defined(LONGPINT)
typedef long pint;
#else
typedef int pint;	/* integer that might be asked to hold a pointer */
#endif
#ifdef LARGE
#define LARGEPTR 1
#endif

#ifdef MC68000
typedef pint stacksize;
typedef pint framesize;
#else
typedef unsigned stacksize;
typedef unsigned framesize;
#endif
typedef unsigned asize;

#ifdef LARGEPTR
typedef unsigned char far *rpointer;
#else
typedef unsigned char *rpointer;
#endif

typedef char *pointer;

typedef unsigned char uns_char;

#ifdef ECURSES
typedef bits16 buf_el;
#else ECURSES
typedef char buf_el;
#endif ECURSES

/*
 * struct definitions for the various tree nodes
 * plus macros for referencing within them
 */

typedef bits8 holdlist;			/* how many in a list */

struct standard_node {
	bits8 node_type;	/* classification of node */
	struct standard_node *n_parent;	/* pointer to parent */
	holdlist flags1;	/* doubles for list count */
	holdlist flags2;	/* doubles for allocation count */

	struct standard_node *n_kids[1];	/* children of various kinds */
	};

typedef struct standard_node node;

#ifdef ES_TREE
typedef unsigned nodep;
typedef unsigned listp;
#else
typedef node *nodep;
typedef node *listp;
#endif

typedef node far *realnodep; 



	
	/*
		struct for a list node
		The list node has the same format as the standard node,
		but there is a child count instead of the flags, and
		a variable number of children, all of the same type.
		This has been deleted.  Use the list node, using flags1
		and flags2 for counts
	 */


	/* struct for a "builtin routine" node, roughly like a node */
	/* all that really matters here is that the parameter list be in
	   the second kid position */
typedef pointer bargs;			/* built in argument entry */

typedef struct _rout_node {
#ifdef M_I86MM
	bits8 node_type;	/* unimportant */
# ifdef NOBFUNC
	long b_func;		/* just a table index in multi-process */
# else
	int (*b_func)();	/* routine to call to do it, replace parent */
# endif
	/*
	 * The following 2 fields overlap n_flags+kid1 in standard_node,
	 * which are of type bits16 and node *, respectively.  For simplicity
	 * here I assume sizeof (int) == sizeof (node *).
	 */
	bits8 min_params;	/* minimum number of params, replace flags */
	bits8 max_params;	/* maximum number of parameters */
	bargs *r_decls;		/* declaration list, kid2 */
#else
	bits8 node_type;	/* unimportant */
# ifdef NOBFUNC
	pint b_func;		/* just a table index in multi-process */
# else
	int (*b_func)();	/* routine to call to do it, replace parent */
# endif
	/*
	 * The following 2 fields overlap n_flags+kid1 in standard_node,
	 * which are of type bits16 and node *, respectively.  For simplicity
	 * here I assume sizeof (int) == sizeof (node *).
	 */
	bits16 min_params;	/* minimum number of params, replace flags */
	pint max_params;	/* maximum number of parameters */
	bargs *r_decls;		/* declaration list, kid2 */
#endif M_I86MM
	} rout_node;


typedef rout_node *rout_np;
typedef rout_node far *realrout_np;


typedef char	*nameptr;	/* for now */

typedef bits8 type_code;

struct symbol_node {
	bits8 node_type;		/* always N_DECL_ID */
	nodep n_parent;			/* the parent, where it is declared */
	holdlist flags1;		/* symbol flags, different meanings */
	holdlist flags2;
	char *s_name;			/* name of symbol */
	type_code s_decl_type;		/* what kind of symbol this is */
	bits8 s_namhash;		/* hash of name of symbol */
	struct symbol_node *s_next;	/* in chain of symbols */
	nodep s_type;			/* pointer to relevant type tree */
	pint s_value;			/* offset in frame, record, size of 
					    frame, that sort of thing */
	asize s_size;			/* size of the value if relevant */
	bits8 s_scope;			/* scope level of this symbol */
	bits8 s_mflags;			/* more symbol flags */
	bits16 s_saveid;			/* special id for save files */
	};

typedef struct symbol_node *symptr;
typedef struct symbol_node far *realsymptr;





/*
 * A curspos is now just a node pointer, this makes us spend a lot of memory,
 * but is worth it in code simplicity
 */
typedef nodep curspos;

/* struct describing a screen line */
struct scr_line {
	bits16 pcode;			/* which print code we are on */
	bits8 sub_line;			/* which fold line we are on */
	nodep listptr;			/* parent of the node that generates 
					 * line */
	holdlist listindex;		/* which child of that parent */
	bits8 sc_size;			/* how many real lines start here */
	bits8 ind_level;		/* indent level */
	bits8 linelen;			/* position of right margin */
	bits8 sc_change;		/* line changed flag */
	};

	/* Structure for a pascal open file */
	/* stick a byte on the front so it can pretend to be a node */
struct pas_file {
	bits8 node_type;		/*zero to indicate this isn't anything*/
	union {
		FILE *f_stream;		/* stream for this file */
		char *f_window;
		} desc;
	int f_flags;			/* flags for this file */
	bits16 f_size;			/* size of buffer */
	char *f_name;			/* name of file */
	char f_buf;			/* start of buffer */
	};

/* structure defining the data on a node */

#ifdef LOADABLE_TEMPLATES
struct node_info {
	char far * far *printcodes;	/* codes for printing lines */
	bits8 far *kidcodes;		/* what printcode for what kid */
	bits8 ni_descend;		/* help for infix printing */
	bits8 fullkids;			/* how many kids we need memory for */
	};
#else

	/* If we are the stand-alone interpreter, don't define the
	 * node_info struct
	 */
#ifndef SAI

struct node_info {
	char * *printcodes;	/* codes for printing lines */
	bits8 *kidcodes;		/* what printcode for what kid */
	nf_type ni_flags;		/* what kind of node it is */
	bits8 ni_descend;		/* help for infix printing */
	bits8 kidcount;			/* how many children for this node */
	bits8 fullkids;			/* how many kids we need memory for */
	bits8 foobar;			/* dummy extra field until I get it right */
	};
#endif

#endif LOADABLE_TEMPLATES
/*
 * inchar is the type for keystrokes we manipulate and store.
 * There is much to be said for type "inchar" being an int.
 * Input keys might be bigger than 255.  However, for now we
 * use char because our internal strings and such are arrays of char.
 * The disadvantage is we can't buffer special keystrokes.
 */
typedef char inchar;

typedef int (*funcptr)();		/* function pointer */

struct anyvar {
	union {
		struct {
			pointer point;
			int comp_len;
		} varp;
		int integer;
		rfloat real;
	} obj;
	int av_len;
	int av_tcode;
	};


#ifdef OLMSG
typedef int	ErrorMsg;
#else
typedef char *ErrorMsg;
#endif
