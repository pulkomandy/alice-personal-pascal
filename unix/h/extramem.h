/* Definitions for our memory allocator, which works in multiple segments */

#ifndef FP_OFF
#define FP_SEG(fp) (*((unsigned *)&(fp) + 1))
#define FP_OFF(fp) (*((unsigned *)&(fp)))
#endif

typedef unsigned int offset;
#ifdef HYBRID
typedef char far *exptr;


/*
 * Segments
 */
#define SEG_MALLOC	0
#define	SEG_NEW		1
#define	SEG_FIXED	2
#define	SEG_STRING	3
#define	SEG_TREE	4

#define	FIRST_SEG	SEG_MALLOC
#define	LAST_SEG	SEG_TREE

extern unsigned int ex_segment, st_segment, tr_segment;

#define	VALID_SEG(seg)	((seg) >= FIRST_SEG && (seg) <= LAST_SEG)

struct seginfo {
	unsigned size;				/* current size (paragraphs) */
	unsigned grow;				/* amt. to grow by (paras) */
	unsigned free;				/* start of free list */
	int	NeedsReserve;			/* needs a reserve area */
	char	*ReserveMem;			/* reserved area */
	pint	Amount_Used;			/* how much memory used */
};
extern struct seginfo Segs[];

extern exptr ex_alloc();
extern exptr ex_checkalloc();
extern exptr choosealloc();
/*
 * Free block
 */

#else
#define ex_checkalloc checkalloc
typedef char *exptr;


struct seginfo {
	unsigned size;				/* current size (paragraphs) */
	unsigned grow;				/* amt. to grow by (paras) */
	pointer free;				/* start of free list */
	int	NeedsReserve;			/* needs a reserve area */
	char	*ReserveMem;			/* reserved area */
	pint	Amount_Used;			/* how much memory used */
};
#endif
struct blk {
	offset	next;			/* next in free list */
	unsigned size;			/* size, including blk header */
};
typedef offset	BlkP;
typedef struct blk far	*FarBlkP;

