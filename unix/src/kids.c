#include "alice.h"
#include "flags.h"
/*
 *DESC: Some general tree functions
 */


#if defined(SAI) || defined(MAP) || defined(PARSER)
# define NOT_EDITOR
#endif

int
reg_kids( np )
reg nodep np;
{
	register NodeNum nptype = ntype(np);

	return (nptype == N_LIST) ? listcount(np) : (nptype == N_HIDE ?
		2 : kid_count(nptype) );
}

/*
 * Return the number of children of a node.
 */
int
n_num_children(np)
reg nodep np;
{

	return is_a_list(np) ? listcount(np) : kid_count( ntype(np) );
}
#ifdef Stomp_Check
s_node_kid(xthenode, num, val )
nodep xthenode;
int num;
reg nodep val;
{
	register NodeNum nodet;
	register nodep thenode;
	thenode = xthenode;

#ifdef DB_SUBR
	if (tracing) fprintf(dtrace, "s_node_kid thenode %x, num %d, val %x\n", thenode, num, val);
#endif

#ifndef NOT_EDITOR
	nodet = ntype(thenode);
	if( nodet == N_LIST ) {
		if( num >= listcount( thenode ) )
			bug( ER(259,"Stomp_Check fails, Tried to assign kid %d of list %x with %x when count is %d"),
				num, thenode, val, listcount( thenode ) );
	}
	 else if(num >= full_kids( nodet ) )  {
		warning( ER(217,"Stomp_Check fails, thenode %x, ntype is %d, full kids %d"),
			thenode, nodet, full_kids(nodet) );
		bug( ER(260,"Tried to set kid %d of node %x with val %x"),
			num, thenode, val );
	}
#endif /* not NOT_EDITOR */

#ifdef PROC_INTERP
	thenode-}n_kids[num] = val;
#else
	tfarptr(thenode)->n_kids[num] = val;
#endif
}

#endif

int
everyKid(np)
register nodep np;
{
	return is_a_list(np) ? listcount(np) : full_kids(ntype(np));
}

int
nodeSize(np)
register nodep np;
{
	return is_a_symbol(np) ? sizeof(struct symbol_node) :
			(sizeof(node) + sizeof(nodep) * (everyKid(np) - 1));
}

#ifndef MAP
int
get_kidnum(np )
nodep np;
{
	register int kcount;
	int maxkids;
	nodep npar;
	register nodep far *nptr;

	/* search a list of nodes for me */
	npar = tparent(np);
	maxkids = kcount = everyKid(npar);

	/* this is a TREE segment & call */
	nptr = kid1adr(npar);
	while( kcount-- )
		if( STAR(nptr)++ == np )
			return maxkids - kcount - 1;
	/* oh no! */
	bug( ER(261,"get_kidnum: Could not find %x in parent %x"), np, npar );
	return 0;
}
#endif !MAP

nodep 
sym_kid( nodp )
nodep nodp;
{
	int	k	= kid_count(ntype(nodp));
	return node_kid( nodp, k);
}

listp 
decl_kid( nodp )
nodep nodp;
{
	register NodeNum ontype = ntype(nodp);
	register int kco = kid_count(ontype);

	return node_kid( nodp, kco-2 );
}
