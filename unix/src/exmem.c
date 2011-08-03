/*
 *DESC: extra segment storage management Hybrid model
 */
#include "alice.h"
#include "extramem.h"

#ifdef DB_EXMEM
# define PRINTT
#endif
#include "printt.h"

/*
 * Storage allocator: maintains a sorted (by address) free list, allocates
 * using first fit strategy, coalesces free blocks when freeing.
 */

struct blk {
	struct blk	*next;			/* next in free list */
	unsigned 	size;			/* size, including blk header */
};
typedef blk	*BlkP;
typedef blk far	*FarBlkP;

BlkP	FreeList;

#define	BLKALIGN(x)	(BlkP)(((unsigned)x + 3) & ~3)

#define	EOLIST		(BlkP)0xfffc

FarBlkP
farBlk(nearBlkP)
BlkP	nearBlkP;
{
	FarBlkP	bp;

	FP_SEG(bp) = ex_segment;
	FP_OFF(bp) = nearBlkP;
	return bp;
}

#ifdef DB_EXMEM
dumpFreeList()
{
	FarBlkP	bp;

	printt0("\n<dump of free list>\n");
	for (bp = farBlk(FreeList); FP_OFF(bp) != EOLIST; FP_OFF(bp) = bp->next)
		printt2("bp=%lx, bp->size=%u\n", bp, bp->size);
	printt0("\n");
}
#else
# define dumpFreeList()
#endif DB_EXMEM

/*
 * Initialize the extra segment storage allocator.  Builds one large
 * free block, starting at 'startOfFree', ending at 'MaxESBytes'.
 */
initESAlloc()
{
	printt1("initESAlloc(%x)\n", startOfFree);

	FreeList = BLKALIGN(1);
	farBlk(FreeList)->next = EOLIST;
	farBlk(FreeList)->size = extra_bytes - (unsigned)FreeList;
	dumpFreeList();
}

/*
 * LastFreeSearched remembers the last free list entry examined
 * during the last esfree().  It is supposed to speed up the freeing of
 * data by avoiding what could be a very long search from the beginning
 * of the free list to the entry being freed.  It must be invalidated
 * by esalloc().
 */

static BlkP	LastFreeSearched = NULL;

/*
 * Allocate 'size' bytes from the extra segment.  First fit strategy.
 */
exptr
ex_alloc(size)
unsigned size;
{
	FarBlkP	bp;			/* current blk in list */
	FarBlkP	prev;			/* previous blk in list */
	exptr	ret;			/* return value */

	printt1("esalloc(%u)", size);
	size = BLKALIGN(size);
	printt1(" ...size now %u\n", size);

	LastFreeSearched = NULL;		/* invalidate it */

	/*
	 * For each free block
	 *     If the block is big enough
	 *         If it's just right,
	 *	       Unlink and return it
	 *         Else
	 *             Split it and return one piece
	 * Otherwise return failure
	 */
	ret = NULL;
	for (bp = farBlk(FreeList), prev = NULL;
	     FP_OFF(bp) != EOLIST;
	     prev = bp, FP_OFF(bp) = bp->next) {
		printt3("bp=%lx, bp->next=%x, bp->size=%u\n", bp, bp->next, bp->size);
		if (bp->size >= size) {
			if (bp->size == size) {
				if (prev)
					prev->next = bp->next;
				else
					FreeList = bp->next;
				ret = (exptr)bp;
				break;
			} else {
				ret = (exptr)bp + (bp->size - size);
				bp->size -= size;
				break;
			}
		}
	}

	if (ret == NULL)
		return ret;

	printt1("returning %lx\n", ret);
	dumpFreeList();
	return ret;
}

/*
 * Free size bytes at p in extra segment.  Maintains sorted free list
 * and coalesces free adjacent free blocks.
 */
esfree(p, size)
FarBlkP	p;
unsigned size;
{
	FarBlkP	bp;		/* blk ptr */
	FarBlkP prev;		/* previous blk in list */

	printt2("esfree(%lx, %u)\n", p, size);

	/*
	 * To decrease the search time, start searching at LastFreeSearched if
	 * it is valid and before p.
	 */
	if (LastFreeSearched && LastFreeSearched < FP_OFF(p)) {
		bp = farBlk(LastFreeSearched);
		printt0("bp = LastFreeSearched\n");
	} else {
		bp = farBlk(FreeList);
		printt0("bp = FreeList\n");
	}

	/*
	 * For each entry in the free list,
	 *     When we are between the correct free blocks,
	 *	   Create a free block from p and link it into the free list.
	 *	   Coalesce adjacent free blocks if possible.
	 *	   Remember last free searched.
	 */
	for (prev = NULL; ; prev = bp, FP_OFF(bp) = bp->next) {
		printt3("bp=%lx, bp->next=%x, bp->size=%u\n", bp, bp->next,
			bp->size);
		if (FP_OFF(p) < FP_OFF(bp)) {
			printt2("inserting between %x and %x\n", prev, bp);
			p->size = BLKALIGN(size);
			p->next = FP_OFF(bp);
			if (prev)
				prev->next = FP_OFF(p);
			else
				FreeList = FP_OFF(p);

			/* coalesce adjacent blocks */
			if (FP_OFF(bp) != EOLIST &&
			    (exptr)p + p->size == (exptr)bp) {
				p->size += bp->size;
				p->next = bp->next;
			}
			if (prev && (exptr)prev + prev->size == (exptr)p) {
				prev->size += p->size;
				prev->next = p->next;
			}
			LastFreeSearched = FP_OFF(prev);
			printt1("LastFreeSearched = %x\n", LastFreeSearched);
			break;
		}
		if (FP_OFF(bp) == EOLIST) {
			printt0("didn't get freed\n");
			break;
		}
	}
	dumpFreeList();
}

