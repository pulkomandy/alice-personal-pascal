/************************************************************************/
/*                                                                      */
/*      (c) Copyright 1986                                              */
/*      David Beckemeyer                                                */
/*      All Rights Reserved                                             */
/*                                                                      */
/*      memory.c - memory allocation module                             */
/*                                                                      */
/*      This is an implementation of the Unix standard C runtime        */
/*      library routines malloc(), free(), and realloc().               */
/*                                                                      */
/*      The routines manage "heaps" allocated from the system.          */
/*      Each heap is carved up into some number of user memory          */
/*      blocks, as requested by malloc() and realloc() calls.           */
/*                                                                      */
/*      As blocks are returned with free() calls, they are merged       */
/*      with any neighboring blocks that are free. Un-mergable          */
/*      blocks are stored on a doubly linked list.                      */
/*                                                                      */
/*      As heaps become full, new ones are created. The list of         */
/*      heaps is a singly linked list.  Heaps are returned to the       */
/*      system during garbage collection, which occurs whenever         */
/*      the current set of heaps cannot fill a memory request.          */
/*                                                                      */
/*      This scheme avoids GEMDOS memory management problems            */
/*      and minimizes fragmentation.                                    */
/*                                                                      */
/*      MINSEG below defines the minimum segment size allocated.        */
/*      Whenever the remaining portion of a block is smaller than       */
/*      this value, the entire block is returned to the caller.         */
/*                                                                      */
/*      HEAPSIZE is the smallest system allocation we will ever         */
/*      make.  This value can be adjusted to your application.          */
/*      If it is small, more GEMDOS Malloc calls will have to           */
/*      be performed.  If it is large compared to the amount of         */
/*      memory actually aquired at runtime, there will be wasted        */
/*      memory.  Since too many GEMDOS Malloc calls may produce         */
/*      a crash, it is wise to make HEAPSIZE at least 8K bytes.         */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <osbind.h>


/* memory manager definitions */
#define MAGIC 0x55aa            /* magic number used for validation */
#define HEAPSIZE 16384L         /* minimum size of each heap */
#define MINSEG 256L             /* minimum size of allocated memory chunk */

/* the structure controlling the object known as heap */
#define HEAP struct _heap

/* Memory Control Block */
#define MCB struct _mcb
struct _mcb {
        MCB *fore;              /* forward link */
        MCB *aft;               /* backward link */
        MCB *neighbor;          /* nearest lower address neighbor */
        long size;              /* size of this chunk including MCB */
        HEAP *heap;             /* 0L if free, else owner of this chunk */      
        int magic;              /* magic number for validation */
};

/* and here is the heap control block */
struct _heap {
        HEAP *link;             /* pointer to next heap (0 if end) */
        MCB *freelist;          /* pointer to first free block or 0 */
        MCB *limit;             /* address of the end of this heap */
};

/* List of allocated heaps aquired from GEMDOS.
 * Start off with no heaps allocated (NULL terminated linked list).
 */
static HEAP heaplist = { (HEAP *)0 };

        
/* get another heap from GEMDOS */

static HEAP *alloc_heap(x)
long x;
{
        MCB *m;
        HEAP *heap;

        /* locate end of the heaplist (a tail pointer might help here) */
        for (heap = &heaplist; heap->link; heap = heap->link)
                ;

        /* adjust the request for the minmum required overhead */
        x = (x + sizeof(HEAP) + sizeof(MCB) + 1) & ~1L;

        /* grab a chunk from GEMDOS */
        if ((heap->link = (HEAP *)Malloc(x)) == 0)
                return((HEAP *)0);

        /* add the heap to the heaplist */
        heap = heap->link;
        heap->link = (HEAP *)0;

        /* first chunk is just after header */
        m = (MCB *)(heap + 1);

        /* set up size and mark it as a free chunk */
        m->size = x - sizeof(HEAP);
        m->heap = 0L;

        /* this is the last (only) chunk on the linked list */
        m->fore = (MCB *)0;
        m->aft = (MCB *)(&heap->freelist);

        /* there is no lower addressed neighbor to this chunk */
        m->neighbor = (MCB *)0;

        /* mark the heap limit and place chunk on freelist */
        heap->limit = (MCB *)((char *)heap + x); 
        heap->freelist = m;
        return(heap);
}


/* split a segment into two chunks */

static split_seg(mcb, x)
MCB *mcb;
long x;
{
        MCB *m;
        HEAP *heap;

        /* check for ownership here */
        if (mcb == 0 || (heap = mcb->heap) == 0 || mcb->magic != MAGIC) {
                return(-40);
        }

        /* make a new chunk inside this one */
        m = (MCB *)((char *)mcb + x);
        m->size = mcb->size - x;
        m->neighbor = mcb;
        m->heap = mcb->heap;
        m->magic = MAGIC;
        /* shrink the old chunk */
        mcb->size = x;
        /* establish the forward neighbor's relationship to us */
        mcb = m;
        if ((m = (MCB *)((char *)mcb + mcb->size)) < heap->limit)
                m->neighbor = mcb;
        free(++mcb);
        return(0);
}



/* allocate a chunk out of a heap */

static MCB *alloc_seg(x, heap)
long x;
HEAP *heap;
{
        MCB *mcb;

        /* use first fit algorithm to find chunk to use */
        for (mcb = heap->freelist; mcb; mcb = mcb->fore)
                if (mcb->size >= x + sizeof(MCB))
                        break;
        if (mcb) {
                /* remove it from the freelist */
                unfree(mcb);
                /* set up owner */
                mcb->heap = heap;
                mcb->magic = MAGIC;
                /* if it's bigger than we need and splitable, split it */
                if (mcb->size - x > MINSEG)
                        split_seg(mcb, x + sizeof(MCB));
                /* return start of data area to caller */
                mcb++;
        }
        return(mcb);
}



/* remove (unlink) a chunk from the freelist */

static unfree(mcb)
MCB *mcb;
{
        if ((mcb->aft->fore = mcb->fore) != 0)
                mcb->fore->aft = mcb->aft;
}



/* GEMDOS garbage collection, return TRUE if anything changes */

static collect()
{
        HEAP *heap, *h;
        MCB *mcb;
        int flag;
        
        for (flag = 0, heap = &heaplist; (h = heap->link) != 0; ) {
                if ((mcb = h->freelist) != 0 &&
                 !mcb->neighbor && ((char *)mcb + mcb->size) == h->limit) {
                        heap->link = h->link;
                        Mfree(h);
                        flag++;
                }
                else
                        heap = h;
        }
        return(flag);
}

/****************************************************
 *
 *      Unix standard C runtime library routines
 *      malloc(), free(), and realloc() follow.
 *
 *      The three calls work as described in K & R.
 *
 *      This implementation uses a first fit algorithm
 *      and does occasional garbage collection to
 *      minimize system memory fragmentation.
 *
 *****************************************************

/****************/
/*              */
/*    malloc    */
/*              */
/****************/

char *malloc(n)
int n;
{
        register HEAP *heap;
        register long x;
        char *p;

        /* malloc is supposed to accept all 16 bits so fix it up here */
        if (n < 0)
                x = 65537L + (long)n & ~1L;
        else
                x = (long)(n + 1) & ~1L;

        /* first check all current heaps */
        for (heap = heaplist.link; heap; heap = heap->link)
                if ((p = alloc_seg(x, heap)) != 0)
                        return(p);

        /* not enough room on heaps, try garbage collection */
        collect();

        /* now allocate a new heap */
        if ((heap = alloc_heap(max(x, HEAPSIZE))) != 0)
                if ((p = alloc_seg(x, heap)) != 0)
                        return(p);

        /* couldn't get a chunk big enough */
        return((char *)0);
}


/****************/
/*              */
/*     free     */
/*              */
/****************/

free(mcb)
MCB *mcb;
{
        MCB *m;
        HEAP *heap;

        /* address header */
        mcb--;

        /* check for ownership here */
        if (mcb == 0 || (heap = mcb->heap) == 0 || mcb->magic != MAGIC) {
                return(-40);
        }
                
        /* connect to chunks behind this one */
        while (mcb->neighbor) {
                if (mcb->neighbor->heap)
                        break;
                mcb->neighbor->size += mcb->size;
                mcb = mcb->neighbor;
                unfree(mcb);
        }

        /* now connect to chunks after this one */
        while ((m = (MCB *)((char *)mcb + mcb->size)) < heap->limit) {
                m->neighbor = mcb;
                if (m->heap)
                        break;
                mcb->size += m->size;
                unfree(m);
        }
        /* place the resultant chunk on the free list */
        for (m = (MCB *)(&heap->freelist); m->fore; m = m->fore)
                ;
        m->fore = mcb;
        mcb->fore = (MCB *)0;
        mcb->aft = m;
        mcb->heap = 0L;
        return(0);
}



/****************/
/*              */
/*   realloc    */
/*              */
/****************/

char *realloc(mcb, n)
MCB *mcb;
int n;
{
        long x;
        char *t, *s, *p;

        /* address header */
        --mcb;

        /* check for ownership here */
        if (mcb == 0 || mcb->magic != MAGIC) {
                return((char *)0);
        }

        /* malloc is supposed to accept all 16 bits so fix it up here */
        if (n < 0)
                x = (65537L + (long)n + sizeof(MCB)) & ~1L;
        else
                x = (long)(n + 1 + sizeof(MCB)) & ~1L;

        /* if less than current size, just shrink it */
        if (mcb->size > x) {
                split_seg(mcb, x);
                return((char *)(++mcb));
        }

        /* it's bigger - allocate new block, copy data, and free old one */
        if ((p = malloc(n)) != 0) {
                x = mcb->size - sizeof(MCB);
                s = ++mcb;
                t = p;
                while (x--)
                        *t++ = *s++;
                free(mcb);
                return(p);
        }
        return((char *)0);
}

