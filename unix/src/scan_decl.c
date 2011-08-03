/*
 *DESC: General declaration traversing routine.  Performs a passed routine
 *DESC: on a tree of declarations.
 */

#include "alice.h"
#include "jump.h"
#include "flags.h"
#include "typecodes.h"
#include "class.h"
#ifdef PROC_INTERP
#include "process.h"
#endif

#ifdef DB_SCAN_DECL
# define PRINTT
#endif
#include "printt.h"

/* routine to scan a list of declarations and do things with it
 * this routine makes use of the fact that all declaration nodes
 * have the important information off of the main node and the
 * identifiers or list of identifiers off of kid1
 *
 *
 * The function will be passed
 *	1) The symbol node that is being declared
 *	2) The running total being calculated
 *	3) The parent declaration node, whose kid2 is probably the type.
 *	4) Whether to ignore stub declarations
 *	5) What to reset the error bit to 
 *	6) The genarg to do what it likes with
 *
 *  Total will have to be long if objects can be >64K, same for return value
 */
extern char rdmsg[];
int seterrbit = NF_ERROR;

int
scan_decl( dlist, procfun, total, stub_ignore, ferrset, genarg )
listp dlist;		/* list to scan */
int (*procfun)();		/* thing to do with it */
int total;			/* running tally */
int stub_ignore;		/* should we ignore stubs */
int ferrset;			/* if true, set err bit to this */
pint genarg;			/* general 4th argument for function */
				/* MUST BE BIG ENOUGH TO HOLD POINTER */
{

	/* the proc fun is passed the decl, the running total and the
	   parent node of the appropriate declaration */

	register int count1, count2;	/* indexes into lists */
	register nodep leftchild;	/* internal list of names */
	register nodep sub_el;		/* element in list */
	reg symptr ourdec;			/* our declaration */

#ifndef SAI
	stackcheck();
#endif

	printt1("scan_decl, dlist=%lx\n", dlist);

	/* check for special builtin parameter list */
	if( ntype(dlist) == 0 ) { 
		for( count1 = 0; barglist(dlist,count1,0); count1++ )
			total = (*procfun)(barglist(dlist,count1,1),
				total,barglist(dlist,count1,0),genarg);
		return total;
		}

	for( count1 = 0; count1 < listcount( dlist ); count1++ ) {
		sub_el = node_kid( dlist, count1 );
		if( is_a_comment(sub_el) )
			continue;
		if( is_a_stub(sub_el) ) {
			if( stub_ignore )
				continue;
			 else
				leftchild = sub_el;
			}
		 else if( is_a_hide(sub_el) ) {
			/* recur */
			total = scan_decl( FLCAST kid2(sub_el), procfun, total,
				stub_ignore, ferrset, genarg );
			continue;	/* next decl */
			}
		 else
			leftchild = kid1(sub_el);
		if( ferrset )
			seterrbit = ferrset;

		if( is_a_list( leftchild ) ) {
			for( count2=0; count2 < listcount(FLCAST leftchild);
						count2++ ) {
				ourdec = (symptr)node_kid( leftchild, count2 );
				/* ignore stubs */
				if(  is_a_stub( ourdec ) ) {
					if( stub_ignore )
						continue;
					}
				 else {
#ifndef PARSER
					if( ferrset & node_flag(ourdec) )
						type_error( ourdec, ER(113,rdmsg), sym_name(ourdec) );
#endif
					}
				total = (*procfun)(ourdec,total,sub_el,genarg);
				}
			}
		 else {
			/* not a list, just go */
			if( stub_ignore && is_a_stub(leftchild) &&
					ntype(leftchild) != N_VARIANT )
				continue;
			if( ntype(leftchild) == N_DECL_ID && ferrset & node_flag(leftchild) ) {
#ifndef PARSER
					type_error( leftchild, ER(113,rdmsg),
						sym_name((symptr)leftchild) );
#endif
				}
			total = (*procfun)(leftchild, total, sub_el, genarg );
			}
		}
	return total;
}

/*
 * Build up an appropriately sized list of parameters for this call.
 * Called from expCall() and also inside expparse.
 */

static bits8 get_stubs[] = {
	T_PROCEDURE, T_FUNCTION, T_FORM_PROCEDURE, T_FORM_FUNCTION, 0 };

listp 
paramStubs(sp)
symptr	sp;
{
	register int	i;
	register listp lp;		/* list of params */
	int		nParams;	/* number of params */
	rout_np rp;		/* ptr to builtin's info */
	int		countParams();
	listp sized_list();

	if ( strchr( get_stubs, sym_dtype(sp) ) != 0  )
		nParams = scan_decl(FLCAST kid2(tparent(sp)), countParams, 0,
			FALSE, 0, (pint)0);
	else if ((rp = (rout_np )tparent(sp)) != (rout_np)0)
		nParams = PTS(rp, min_params, rout_node far * );
	else
		nParams = 1;

	lp = sized_list(nParams);
	for (i = 0; i < nParams; i++)
		linkup(FNCAST lp, i, make_stub(C_EXP));

	return lp;
}

countParams(foo1, runningTotal, foo2, foo3)
nodep foo1;
int	runningTotal;
nodep foo2;
int	foo3;
{
	return ++runningTotal;
}

#ifndef PARSER

/* routine to call the argument print function */

argp_call( plist, thenode )
listp plist;		/* list of declarations */
pint thenode;
{
#ifndef SAI
	extern int argp_func();
	stackcheck();
	printt1("argp_call, plist=%lx\n", plist);
	return scan_decl( plist, argp_func, 0, FALSE, 0, thenode );
#endif SAI
}
#endif
