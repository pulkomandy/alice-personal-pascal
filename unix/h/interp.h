/* Declarations and typedefs important to the interpreter */

/* A 'set' is an array of up to 256 bits. */

typedef bits8 set[SET_BYTES];

/* There are two stacks used in the running of the program.  One is the
 * E-stack, which is the stack of the state machine, indicating what states
 * are pending, and what state we should go to after this state is finished
 * etc.  Some other minor information is stored there.

 * The other stack is the user stack, where the displays and user variables
 * are kept.

 * There is third stack, namely the C stack (which is the real processor's
 * stack.  We don't mess with it.
 */

typedef union {
	nodep e_loc;
	StateNum e_state;
	int e_int;
	pointer e_pointer;
	} estackloc;


/* The user stack.  We just have a big union so that we can refer to just
 * about anything 'sp' might be pointing at by its proper type
 */

typedef union  {
	rint *st_int;
	pointer *st_pointer;
	pint *st_pint;
	rfloat *st_float;
	bits8 *st_byte;
	stackloc *st_stackloc;
	pointer st_generic;
	set *st_set;
	struct susp_block *st_susp;	/* suspended block on stack */
	} stack_pointer;

/* Number of integers units on the stack to store a pointer 
   This is 1 on nice machines, 2 on machines with 32 bit pointers and 16
   bit ints */

#define P_S (sizeof(pointer)/sizeof(rint))

/*
 * One thing that can sit on the stack is the record of a program
 * interruption, or suspension.  This comes from the break key, errors,
 * breakpoints, etc.   We need to store lots about what was happening
 * at the time so that we may resume.
 */
struct susp_block {
	int susp_type;		/* code or just data */
	int susp_count;		/* how deep are we in suspension */
	struct susp_block *susp_prev;	/* previous suspend block */
	estackloc *susp_estack;		/* previous exec stack */
	stack_pointer *susp_display;	/* where is the display */
	nodep susp_todo;		/* new location about to run */
	nodep susp_resume;		/* resume location */
	int susp_cdepth;		/* how deep in calls */
	};


/* On the 8086, if we want a true pointer, we have to form a 20 bit number
 * from the segment and offset.  On other machines, a machine pointer is
 * a unique pointer.   We keep track of true pointers when we are tracing,
 * so that any reference to the location is caught, no matter how it is
 * formed from segment and offset.
 */

#if defined(LARGE) && defined(msdos)
typedef long truepointer;
#define ptrmap(new,x)	new = (((long)x & 0xffff) + (((long)x & 0xffff0000) >> 12 ) )
#else
typedef pointer truepointer;
#define ptrmap(new,x)	new = x
#endif

/* struct listing information about a trace slot */

/*
 * Trace slots are defined from the do if ... set statement.
 */
struct tr_range {
	truepointer tr_lower;		/* lower bound of trace range */
	truepointer tr_upper;		/* top byte of trace range */
	listp tr_code_list;		/* code to execute on exception */
	stack_pointer *tr_display;	/* display for code */
	int tr_hide;			/* is trace block hidden? */
	};

extern int most_trlocs;			/* trace on variables */
extern int within_trace;

/* Graphics support screen mode infor */

struct modeInfo {
	bits8	vmode;
	bits8	maxColour;
	bits8	rows;
	bits8	columns;
	int	pixWidth;
	int	pixHeight;
#ifdef QNX
	int	startline;
#else
	int	screenSeg;
	int	screenSize;
#endif
};

typedef struct pas_file *fileptr;	/* pointer to pascal file */

extern stack_pointer *st_display;		/* current st_display pointer */
extern struct susp_block *curr_suspend;
extern stack_pointer sp;		/* current top of stack */
extern nodep new_loc;
extern int within_hide;
extern int had_type_error;		/* had a type error */

/*
 * Now some macros to push, pop and reference the top of the stack.
 *
 * How the stack is referenced depends a lot on our code model
 * On the small machines, we have tended to still store ints in 32 bits
 * and waste the upper 16, just so that every stack unit is at least as
 * bit as a pointer.
 */
#ifdef LARGE
#define int_stop 	(*sp.st_int)
#define int_spop()	(*(rint *)(sp.st_pint++))
#define int_spush(val)	(*(rint *)(--sp.st_pint) = (val))
/* push a byte as an integer */
#define ib_spush(val)	(*(rint *)(--sp.st_pint) = (rint)(val))
#define ib_spop()	((bits8)*(rint *)sp.st_pint++)
#else
	/* integer top of stack, read or assign to */
#define int_stop 	(*sp.st_int)
	/* pop and return integer top of stack */
#define int_spop()	(*sp.st_int++)
	/* push integer on stack */
#define int_spush(val)	(*--sp.st_int = (rint)(val))
	/* push a byte as an integer */
#define ib_spush(val)	(*--sp.st_int = (rint)(val))
	/* pop a byte from an integer on the top of stack */
#define ib_spop()	(bits8)(*sp.st_int++)
#endif
/* special offset showing how far into a word the low byte is */
#ifdef BIGENDIAN
#define WB_ADJUST(size)	(size == 1)
#define BYTE_ADD 1
#else
#define BYTE_ADD 0
#define WB_ADJUST(size)	0
#endif

#ifdef WORD_ALLIGNED
# define word_allign(x)	(x)+((x)&1)
#else
# define word_allign(x)	x
#endif

/* Number of extra words in each display, over and above the rest of
 * the display */

#define EXTR_DISP_WORDS	6
/* one for scope, one for old display, one for caller, one for block */
/* These byte push/pop are not currently used -- byte as integer macros
   are used instead */
	/* byte on top of the stack, read or assign to */
#define b_stop 		(*sp.st_byte)
	/* pop a byte from the top of stack */
#define b_spop()	(*sp.st_byte++)
	/* push a byte on the top of stack */
#define b_spush(val)	(*--sp.st_byte = (val))

/* set operations work with pointers to sets */
	/* A set, on the top of stack, just a byte address really */
#define set_stop	(sp.st_byte)
	/* pop a set, return the address it was at */
#define set_spop()	(sp.st_set++)
	/* push a 32 byte set on the stack */
#define set_spush(adr)	(--sp.st_set, blk_move(sp.st_generic, adr, SET_BYTES))
	/* push an empty set on the stack */
#define eset_push()	(--sp.st_set, zero( sp.st_generic, SET_BYTES ))

	/* operations on pointers on the stack */

#define p_stop 		(*sp.st_pointer)
#define p_spop() 	(*sp.st_pointer++)
#define p_spush(val)	(*--sp.st_pointer = (val))
/* skip a type on the stack, pop but do not read */
	/* In built in functions that have types along with them, sometimes
	 * we don't care about the type, so we have this macro to pop it
	 * and throw it away.
	 */
#define skip_type()	sp.st_pointer++

/* str_spop same as p_spop for now */
#ifdef TURBO
extern unsigned char *str_spop();
#else
# define str_spop()	(*sp.st_pointer++)
#endif

	/* floating point numbers on the stack */
#define f_stop 		(*sp.st_float)
#define f_spop() 	(*sp.st_float++)
#define f_spush(val)	(*--sp.st_float = (val))

	/* adjust the stack by a given amount, pushing that many bytes onto
	   it */
#define bump_stack(val)	(sp.st_stackloc -= val)
	/* pop a given number of bytes off the stack */

#define unload_stack(val) (sp.st_stackloc += val)
/* special value for unspecified field widths etc. */
#define OEXP_DEFAULT -32767

/* Special macro for debug output about runtime operations. */

#ifdef XDEBUG
#define EDEBUG(level,string,ar1,ar2) if(level<=ed_level)fprintf(etrace,string,ar1,ar2)

#else
#define EDEBUG(a,b,c,d)  /* */
#endif

#ifndef UDEBUG
#define do_undef(a,b,c) /* */
#define copy_mask(a,b,c) /* */
#define sr_check(a,b) /* */
#define sr_ccheck(a,b) /* */
#endif

	/* macros that work on the E-stack */

#define espush(val,typ)	(*--ex_stack).typ = (val)
#define state_push(val) (*--ex_stack).e_state = (val)
#define espop(typ)	((*ex_stack++).typ)
#define estop(typ)	((*ex_stack).typ)
#define esoff(off, typ)	(ex_stack[off].typ)
#define stackof(type)	(sizeof(type)/sizeof(stackloc))

#define stloc(offset,typ) (*(typ *)(sp.st_stackloc + offset))
extern estackloc *loc_stack;
extern bits8 *undef_bitmap;
extern unsigned int max_stack_size;
extern unsigned int lc_stack_size;
extern stack_pointer *up_frame();
extern nodep *get_fpointers();
	/* suspension block */

/* manifests for undef function */
#define U_CHECK 0
#define U_SET 1
#define U_CLEAR 2
extern estackloc *ex_stack;

/* pascal file */
typedef FILE *PFILE;

/* The special states for the interpreter state machine */

#define  S_NOP		-1
#define  S_CALL1	-2
#define  S_CALL2	-3
#define  S_CALL3	-4
#define  S_FOR2		-5
#define  S_FOR1		-6
#define  S_1DOWNTO	-7
#define  S_2DOWNTO	-8
#define  S_OEXP21	-9
#define  S_OEXP31	-10
#define  S_LIST1	-11
#define  S_GRABK2	-12
#define  S_WHILE1	-13
#define  S_RPT1		-14
#define  S_RPT2		-15
#define  S_RETURN	-16
#define	 S_UNHIDE	-17
#define	 S_SET1		-18
#define	 S_PROGEND	-19
#define  S_WITHDONE	-20
#define  S_FORTB	-21
#define	 S_UNTRACE	-22
#define  Main(anode)	anode

/* file flags */

#define FIL_EOF 1
#define FIL_EOL 2
#define FIL_OPEN 4
#define FIL_LAZY 8
/* data has not yet been read in */
#define FIL_NREAD 0x10
#define FIL_TEXT 0x20
#define FIL_RANDOM 0x40
#define FIL_KEYBOARD 0x80
#define FIL_SCREEN 0x100
#define FIL_WRITE 0x200
#define FIL_READ 0x400
#define FIL_DEVICE 0x800
#define FIL_RAWKEY 0x1000

#ifdef TURBO
extern int io_res_on;		/* is i/o result checking on */
extern int last_ioerr;		/* code from last operation */
#define tp_error(ec,ret)	if( last_ioerr = ec,  io_res_on ) return ret
#else
#define tp_error(ec,ret)
#endif


/* type checks */

#define is_integer(typ)	(typ == Basetype(BT_integer))
#define is_real(typ)	(typ == Basetype(BT_real))
#define is_boolean(typ)	(typ == Basetype(BT_boolean))
#define is_char(typ)	(typ == Basetype(BT_char))

#define BRK_DISABLE (pointer)1
