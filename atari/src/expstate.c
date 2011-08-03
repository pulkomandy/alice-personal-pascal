
#define INTERPRETER 1
/*
 * The C version of the Alice interpreter
 * My guess is that this is not very efficient and the whole thing
 * has to be coded in assembler.  At least the inner core
 *  Mostly a big switch statement, soon to become a jump table
 *  if necessary at a theatre near you.
 */

#include "alice.h"
#include "typecodes.h"

#ifdef UDEBUG
/*#define XDEBUG 1*/
#endif

#include "interp.h"
#include "flags.h"


#ifdef STUPID_COMPILER
#define ISTAT extern
#else
#define ISTAT
#endif
extern stack_pointer sp;
extern stack_pointer *st_display;       /* all the fps and aps we need */
extern estackloc *loc_stack;
extern bits8 *undef_bitmap;
extern estackloc *ex_stack;
/* here are the externals we will be using */


ISTAT listp arg_list;               /* general aguments */
ISTAT int built_index;                  /* index into arguments */
ISTAT StateNum cur_state;               /* state we are in now */
extern nodep new_loc;                           /* new location */
ISTAT nf_type el_flags;                 /* flags of location type */
ISTAT rint *intstar;                    /* quick integer pointer */
ISTAT rint temp2;                       /* two quick temporaries */
                                        /* temp1 a register */
ISTAT unsigned untemp;                  /* unsigned temporary */
ISTAT rfloat fltemp;                    /* floating temp */
ISTAT rfloat *flstar;                   /* floating temp ptr */
ISTAT bits8 btemp1;                     /* a quick byte temporary */
ISTAT int thescope;                     /* scope of new symbol */
ISTAT int nkid;                         /* kid in a list */
ISTAT bits16 nod_flags;                 /* flags of our node friend */
ISTAT pointer tempptr;                  /* temporary pointer */
ISTAT pointer resptr;                   /* pointer to a result on stack */
ISTAT nodep type_ptr;                   /* for type checking */
ISTAT nodep tempnode;                   /* quick node to have */
ISTAT stack_pointer *dummy;             /* for ge_routine */
extern symptr get_routine();
extern int step_flag;                   /* are we stepping */
extern nodep ex_loc;                            /* storage for program counter */

expstate()
{
        register nodep eloc = ex_loc;
        register symptr thesym;                 /* symbol table entry */
        register rint temp1;

        switch( cur_state ) {
#include "mlowstat.c"
                }
}

