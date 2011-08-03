/*
 * The master types file for Alice, with most typedefs and many of
 * the important structures.
 */


/* Mixed models use the 'far' and 'near' keywords to define pointer size.
   If you aren't on one of these difficult machines, you just define these
   words to be null and your compiler will be fine */

#ifndef HYBRID
# define far
# ifndef SAI
#  define near
# endif
#endif


/* Our general 8 bit type, for flags and other bytes */

typedef unsigned char bits8;

/* A general 16 bit type */
#ifdef QNX
typedef unsigned bits16;
#else
typedef unsigned short bits16;
#endif QNX

typedef unsigned char	Boolean;
typedef bits8 NodeNum;		/* node types, first byte of any node */
typedef bits8 ClassNum;		/* Child 'class' numbers */
typedef bits8 ActionNum;	/* action codes for token on a node or stub */
typedef bits8 stackloc;		/* base pointer type on stack */
typedef unsigned short nf_type;	/* node flag type */
typedef short StateNum;

typedef char	*treep;		/* generic pointer type in trees */

/* We can disable floating point support by not defining FLOATING */

#ifdef FLOATING
	typedef double rfloat;		/* runtime floating point */
# define FLOATNUM(x)	x
#else
	typedef int rfloat;
# define FLOATNUM(x)	0
#endif

/* A runtime integer is the kind of integer that user programs get to
   use.   It has always been an int, actually, but it's nice to have
   a typedef */

typedef int rint;		/* runtime integer */

/* Here we define an integer type that we know we can stuff a pointer in.
   This is mainly important in the segmented modes that have 16 bit ints
   and 32 bit pointers */

#if defined(LARGE) || defined(HYBRID) || defined(LONGPINT)
typedef long pint;
#else
typedef int pint;	/* integer that might be asked to hold a pointer */
#endif

#ifdef LARGE
# define LARGEPTR 1	/* Pointers are indeed large model pointers */
# define LOTSMEM 1	/* we have lots of memory */
#endif

#ifdef POINT32		/* Pointers are in fact 32 bit words, and not funny
				segments, if this is defined */
# define LOTSMEM 1
#endif


/* The pointers that user programs get to use */

#ifdef LARGEPTR
typedef unsigned char far *rpointer;
#else
typedef unsigned char *rpointer;
#endif

typedef char *pointer;		/* A general pointer type */

typedef unsigned char uns_char;

/* If we are using an extended curses library, such as our own library,
   or the System V curses library, which supports multiple character
   attributes (such as underline, bold, blink, reverse etc.) then we need
   16 bits to hold each character instead of 8.  A buf_el is a buffer
   element in a string that will be displayed on the screen.  The low
   7 bits gets the ascii char, and the upper bits (1 or 9 of them) get
   attributes */

#ifdef ECURSES
typedef unsigned long buf_el;
#else ECURSES
typedef char buf_el;
#endif ECURSES

/*
 * struct definitions for the various tree nodes
 * plus macros for referencing within them
 */

 /* This is a type that can hold the number of elements in a list.
    Lists can thus be up to 255 elements in size, since we use a
    byte */

typedef bits8 holdlist;			/* how many in a list */

/* 
 * Here it is, the master type of a tree node.   The array at the end
 * is actually sort of a flex array.  We allocate enough room for a given
 * tree node to hold the right number of kids, from 0 up to around 7,
 * whatever the maximum is.
 * C lets us do this by being loose on the array checking.
 */

struct standard_node {
	bits8 node_type;	/* classification of node */
	struct standard_node *n_parent;	/* pointer to parent */
	holdlist flags1;	/* doubles for list count */
	holdlist flags2;	/* doubles for allocation count */

	struct standard_node *n_kids[1];	/* children of various kinds */
	};

typedef struct standard_node node;

/* The most commonly used types in alice are pointers to nodes and lists.
   Here are typedefs for them */

#if defined(ES_TREE) && !defined(QNX)
typedef unsigned nodep;
typedef unsigned listp;
#else
typedef node *nodep;
typedef node *listp;
#endif

/* In segmented mode we keep node pointers in 16 bits, but as such they
 * aren't really pointers and don't work unless inside one of our tree
 * structure reference macros.   Sometimes we want to make a real pointer
 * to a node, and this is it.
 */

typedef node far *realnodep; 

/* What we call a 'list' node is really a standard node.
 * The ntype will always be N_LIST, and the number of kids is flexible.
 * There can be no flags on a list node.  Instead, we use the flags1 byte
 * as a count of the items in the list, and the flags2 byte as a
 * count of the number allocated.  (These numbers are different so that
 * we can allocate in chunks to avoid excessive reallocs.)
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

/* Pointers to routine nodes */

typedef rout_node *rout_np;
typedef rout_node far *realrout_np;


typedef char	*nameptr;	/* for now */

typedef bits8 type_code;	/* 'type of symbol' variables */

/*
 * The master node for a symbol.  Symbol nodes start out like standard
 * nodes, with a type, parent and flags.  Because of this, they actually
 * exist in the tree, at their unique point of declarations.
 * (All references to the symbol are an N_ID with a pointer to the symbol
 * node)
 *
 * These nodes are also found in the chains that are the symbol tables
 * for any given scope.
 */

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
	bits16 s_size;			/* size of the value if relevant */
	bits8 s_scope;			/* scope level of this symbol */
	bits8 s_mflags;			/* more symbol flags */
	short s_saveid;			/* special id for save files */
	};

typedef struct symbol_node *symptr;
typedef struct symbol_node far *realsymptr;





/*
 * A curspos is now just a node pointer, this makes us spend a lot of memory,
 * but is worth it in code simplicity.  (We used to allow NIL pointers in the
 * tree to represent stubs, and the cursor thus had to consist of a parent and
 * child number, but this was abandoned.  The typedef is still around.
 */
typedef nodep curspos;


/* Screen lines.   Alice screen display is one of its most complex
 * elements.  We have to deal with folded lines and going up and down
 * smoothly.  It's quite a lot of work.  We thus have to keep a lot of
 * data for every line on the screen, so that we may easily find the
 * information to print the next or previous line at any time.
 */

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

/* This structure is what a file variable is.  It's here because of
 * some file initialization that has to be done outside the interpreter.
 */

	/* stick a byte on the front so it can pretend to be a node */
struct pas_file {
	bits8 node_type;		/*zero to indicate this isn't anything*/
	union {
		FILE *f_stream;		/* stream for this file */
#ifdef getyx
		WINDOW *f_window;
#endif
		} desc;
	int f_flags;			/* flags for this file */
	bits16 f_size;			/* size of buffer */
	char *f_name;			/* name of file */
	char f_buf;			/* start of buffer */
	};

/* We now load the template information into the editor from a file
 * generated by our template program.   We load in all sorts of info
 * about the nodes.  Much of it goes into an array of pointers to
 * these structures, although other arrays, with one element per node,
 * are also generated.
 */

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

#endif /* LOADABLE_TEMPLATES */
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

/* We can either keep the error messages right in the program text, or
 * load them from a file.  If we load them from a file, error messages
 * are really index numbers into the error file.  Otherwise they are
 * pointers to the message strings themselves.
 */

#ifdef OLMSG
typedef int	ErrorMsg;
#else
typedef char *ErrorMsg;
#endif

/*
 * ireg? are integer registers, preg? are pointer registers.
 *
 * Apparently some compilers generate not-so-good code if you ask for
 * too many registers, hence these defines.  With these you can define
 * priority register variables and lower priority register variables, and
 * be assured that the priority variables get registers first.  Otherwise
 * you would just get the order the compiler picks.
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
