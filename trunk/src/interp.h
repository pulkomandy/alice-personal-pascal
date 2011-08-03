typedef bits8 set[SET_BYTES];

typedef union {
	nodep e_loc;
	StateNum e_state;
	int e_int;
	pointer e_pointer;
	} estackloc;

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

#define P_S (sizeof(pointer)/sizeof(rint))

/* suspended state on stack */
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


#if defined(LARGE) && defined(msdos)
typedef long truepointer;
#define ptrmap(new,x)	new = (((long)x & 0xffff) + (((long)x & 0xffff0000) >> 12 ) )
#else
typedef pointer truepointer;
#define ptrmap(new,x)
#endif

/* struct listing information about a trace slot */
struct tr_range {
	truepointer tr_lower;		/* lower bound of trace range */
	truepointer tr_upper;		/* top byte of trace range */
	listp tr_code_list;		/* code to execute on exception */
	stack_pointer *tr_display;	/* display for code */
	int tr_hide;			/* is trace block hidden? */
	};

extern int most_trlocs;			/* trace on variables */
extern int within_trace;

struct modeInfo {
	bits8	vmode;
	bits8	maxColour;
	bits8	rows;
	bits8	columns;
	int	pixWidth;
	int	pixHeight;
	int	screenSeg;
	int	screenSize;
};

typedef struct pas_file *fileptr;	/* pointer to pascal file */

extern stack_pointer *st_display;		/* current st_display pointer */
extern struct susp_block *curr_suspend;
extern stack_pointer sp;		/* current top of stack */
extern nodep new_loc;
extern int within_hide;
extern int had_type_error;		/* had a type error */
#ifdef LARGE
#define int_stop 	(*sp.st_int)
#define int_spop()	(*(rint *)(sp.st_pint++))
#define int_spush(val)	(*(rint *)(--sp.st_pint) = (val))
/* push a byte as an integer */
#define ib_spush(val)	(*(rint *)(--sp.st_pint) = (rint)(val))
#define ib_spop()	((bits8)*(rint *)sp.st_pint++)
#else
#define int_stop 	(*sp.st_int)
#define int_spop()	(*sp.st_int++)
#define int_spush(val)	(*--sp.st_int = (rint)(val))
/* push a byte as an integer */
#define ib_spush(val)	(*--sp.st_int = (rint)(val))
#define ib_spop()	(bits8)(*sp.st_int++)
#endif
/* special offset showing how far into a word the low byte is */
#if defined(Z8000) || defined(MC68000)
#define WB_ADJUST(size)	(size == 1)
#else
#define WB_ADJUST(size)	0
#endif
#define EXTR_DISP_WORDS	6
/* one for scope, one for old display, one for caller, one for block */
#define b_stop 		(*sp.st_byte)
#define b_spop()	(*sp.st_byte++)
#define b_spush(val)	(*--sp.st_byte = (val))
/* set operations work with pointers to sets */
#define set_stop	(sp.st_byte)
#define set_spop()	(sp.st_set++)
#define set_spush(adr)	(--sp.st_set, blk_move(sp.st_generic, adr, SET_BYTES))
#define eset_push()	(--sp.st_set, zero( sp.st_generic, SET_BYTES ))
#define p_stop 		(*sp.st_pointer)
#define p_spop() 	(*sp.st_pointer++)
#define p_spush(val)	(*--sp.st_pointer = (val))
/* skip a type on the stack, pop but do not read */
#define skip_type()	sp.st_pointer++
/* str_spop same as p_spop for now */
#ifdef TURBO
extern unsigned char *str_spop();
#else
# define str_spop()	(*sp.st_pointer++)
#endif
#define f_stop 		(*sp.st_float)
#define f_spop() 	(*sp.st_float++)
#define f_spush(val)	(*--sp.st_float = (val))
#define bump_stack(val)	(sp.st_stackloc -= val)
#define unload_stack(val) (sp.st_stackloc += val)
/* special value for unspecified field widths etc. */
#define OEXP_DEFAULT -32767

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

#ifdef WORD_ALLIGNED
#define word_allign(X) (X)+((X)&1)
#else
#define word_allign(X) (X)
#endif
