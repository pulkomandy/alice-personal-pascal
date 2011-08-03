
nodep
raw_transmog( oldnode, wharray, newtype )
nodep oldnode;		/* node being altered */
int *wharray;		/* array of mappings */
NodeNum newtype;	/* the new nodetype */
{
	int whindex;
	int newcount;
	register nodep newguy;
	register nodep oldnode;
	nodep keepers[MAX_NODE_KIDS];

	for( whindex = 0; whindex < MAX_NODE_KIDS; whindex++ ) {
		if( wharray[whindex] ) {
			keepers[whindex] = node_kid(oldnode,whindex);
			prune(keepers[whindex]);
			}
		 else
			keepers[whindex] = NIL;
		}

	prune(oldnode);

	newguy = tree(newtype, NIL, NIL, NIL, NIL, NIL, NIL, NIL, NIL);
	newcount = reg_kids(newguy);
	graft(newguy, cursor);
	for (i = 0; i < newcount; i++)
		fresh_stub(newguy, i);

	for( whindex = 0; whindex < MAX_NODE_KIDS; whindex++ ) 
		if( wharray[whindex] ) 
			graft( keepers[whindex], node_kid(newguy,
					wharray[whindex]-1 ) );

	return newguy;
}
