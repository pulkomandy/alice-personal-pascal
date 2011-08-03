/*
 * Action routines, called when a simple expansion isn't enough.
 */

/*
 *DESC: More action routines, usually for complex actions that are more
 *DESC: than the expansion of a node at the placeholder
 */

#include "alice.h"
#include "action.h"
#include "typecodes.h"
#include "flags.h"

#ifdef DB_ACTROUT
#define PRINTT
#endif
#include "printt.h"

int	got_colon = FALSE;

extern char tokentext[];
/*
 * This is the big case statement for special actions.  The action has
 * been looked up based on the input token and context and passed here.
 */
actrout(arn, xtn, xfoundnode)
NodeNum arn;	/* action routine number */
int xtn;	/* token number user typed, text in extern tokentext */
nodep xfoundnode;/* where the action was found */
{
	preg1 nodep foundnode;
	ireg1 NodeNum	tn;
	preg2 nodep newcurs;
	foundnode = xfoundnode;
	tn = xtn;

#if defined(DB_ACTROUT)
	if (tracing) fprintf(dtrace, "action routine #%d, token %d\n", arn, tn);
#endif
	switch(arn) {
	/* These are defined in "action.h" */
	case ACT_SKIP:		skip_to_next_stub();		break;
	/* no code for ACT_IGNORE */
	case ACT_EXPLIST:	exp_list(1);			break;

	/*case ACT_LDINT:	do_decl(N_LABEL_DECL,T_LABEL);	break;*/
	case ACT_CDID:		do_decl(N_CONST_DECL, T_CONST);	break;
	case ACT_TDID:		do_decl(N_TYPE_DECL, T_TYPE);	break;
	case ACT_VDID:		do_decl(N_VAR_DECL, T_VAR);	break;
	case ACT_FDID:		exp_prod(N_FIELD);
				do_exp_prod(N_ID_FIELD, T_FIELD);
				skip_down();			break;
	case ACT_PDID:		do_decl(N_FORM_VALUE, T_FORM_VALUE);
								break;
	case ACT_CONTOK:	contok(tn);			break;
	case ACT_TYPID:		typid();			break;
	case ACT_COMMENT:	exp_prod(N_ST_COMMENT);
	case ACT_IDCOMMENT:	contok(N_T_COMMENT);		break;
	case ACT_TYPSR:		typsr(tn);			break;
	case ACT_WRITELN:
				exp_prod(N_ST_CALL);
				strcpy(tokentext, "writeln" );
				exp_prod(N_ID);			break;
	case ACT_STID:		stid();				break;
	case ACT_EXPID:		expid();			break;
	case ACT_DPROC:		dec_block(N_DECL_PROC);		break;
	case ACT_DFUNC:		dec_block(N_DECL_FUNC);		break;
	case ACT_DPGM:		dec_block(N_PROGRAM);		break;
	case ACT_DPPROC:	dec_block(N_FORM_PROCEDURE);	break;
	case ACT_DPFUNC:	dec_block(N_FORM_FUNCTION);	break;
	case ACT_DECLARE:	decl_it(N_DECL_ID);		break;
	case ACT_HDECLARE:	decl_it(N_HIDECL_ID);		break;
	/* Kludge to handle colons */
	case ACT_ASGCOLON:	
			cursor = realcp( node_kid( foundnode, 1));
			got_colon = TRUE;
			break;
	case ACT_CHGOTO:
	case ACT_CHGOTO+1:
	case ACT_CHGOTO+2:
	case ACT_CHGOTO+3:
	case ACT_CHGOTO+4:
	case ACT_CHGOTO+5:	cursor = realcp(
					node_kid( foundnode, arn-ACT_CHGOTO ));
									break;
	case ACT_C_EXPAND:	exp_list(1);				break;
	case ACT_AREXP:		cursor = kid2(foundnode);
				if( is_a_stub( cursor ) )
					exp_prod( N_TYP_ARRAY );
									break;
	case ACT_CMMA_EXPAND:	comma_expand(foundnode);		break;
	case ACT_CRIGHT:	newcurs = c_rightpos(foundnode);
				if( !c_at_root(newcurs) )
					cursor = newcurs;
									break;
	case ACT_H_EXPAND:	cursor = foundnode;
				exp_list(1);				break;
	case ACT_GDECL:		if( lookaheadchar == '\n' || lookaheadchar == '\r' )
					setlachar(0);
				/* this fixes the VAR<CR> problem */
				go_declare(tn,foundnode);		break;
	case ACT_BLCR:
	case ACT_BLCR+1:
	case ACT_BLCR+2:	block_cr(foundnode,arn-ACT_BLCR);	break;
	case ACT_CR:		do_cr(foundnode);			break;
	case ACT_MDOWNTO:	change_ntype(foundnode, N_ST_DOWNTO);
				cursor = kid3(foundnode);		break;
	case ACT_MTO:		change_ntype(foundnode, N_ST_FOR);	break;
	case ACT_TVAR:		change_ntype(foundnode, N_FORM_REF); 
									break;
#ifdef notdef
	case ACT_CEXPR:		expr_typein(NULL, tokentext);		break;
#endif
	case ACT_RECORD:	dec_block(N_TYP_RECORD);		break;
	case ACT_CCONST:	exp_prod( ntype(find_true_parent(foundnode))
					== N_VARIANT ? N_ST_VCASE: N_CASE );
				contok(tn);				break;
	case ACT_INDEX:		op_insert(foundnode, N_VAR_ARRAY,tn);	break;
	case ACT_INDIRECT:	op_insert(foundnode, N_VAR_POINTER,tn);	break;
			/* dots in and ID don't apply within . select */
	case ACT_FIELD:		if(ntype(tparent(foundnode)) == N_VAR_FIELD)
					foundnode = tparent(foundnode);
				op_insert(foundnode, N_VAR_FIELD,tn);	break;	
	case ACT_CDOTDOT:	if( ntype(cursor) != N_TYP_SUBRANGE )
					cursor = kid2( ins_node(cursor,N_TYP_SUBRANGE,0));
									break;
	case ACT_FDUSE:		do_fdid();				break;
	case ACT_CALLMAKE:	do_mcall(foundnode);			break;
				/* this code can't be undone as yet since
				   the children are lists.  we need a way
				   to do that sort of thing */
	case ACT_DELSE:	
				do_elsemap( foundnode );		break;
/*	case ACT_STUBCR:	do_stubcr(cursor);			break;*/
	case ACT_LBSTAT:	lblst();				break;
	case ACT_SET_SUBRANGE:	if(ntype(find_true_parent(cursor)) != N_EXP_SET)
					error( ER(3,"badsub`Subranges are only allowed in sets") );
				exp_prod(N_SET_SUBRANGE);		break;
	case ACT_REEXPAND:	reexpand(foundnode);			break;
	case ACT_BLCHG:		transmog( tparent(cursor), tn );
				skip_down();				break;
	case ACT_COMCHG:	transmog( tparent(cursor),N_ST_COUT);	break;
	case ACT_PUTME:		cursor = foundnode;			break;
	case ACT_RECID:		do_recid();				break;
			/* transmog const into init const */
	case ACT_TMINIT:	change_ntype(foundnode,N_CONST_INIT);
			/* stub not undone, who cares, full kids both 4 */
				fresh_stub( foundnode, 3 );
				prune(kid3(foundnode));
				prune(kid2(foundnode));
				cursor = kid2(foundnode);
									break;

#ifdef N_VAR_ABSOLUTE
	case ACT_ABSCON:	abscon(tn);				break;
	case ACT_ABSMOG:	cursor = kid3( transmog( foundnode,
					N_VAR_ABSOLUTE ) );		break;
#endif
				
				
	}
}
/* the user has declared a block with program, function or procedure
 * we must expand the block and allocate a symbol table for it.
 */

dec_block(xblocknode)
NodeNum xblocknode;
{
	preg1 nodep blk_parent;	/* block parent */
	ireg1 NodeNum blocknode;
	blocknode = xblocknode;

	exp_prod(blocknode);
	/* graft on the symbol table off the end */
	/* cursor will now be on first child of block - the name.
	   thus the parent of the cursor is our block node itself */

	blk_parent = find_true_parent( cursor );

#if defined(DB_ACTROUT)
	if( tracing )
		fprintf( dtrace, "Sticking symbol table on %x at child %d\n",
		blk_parent, kid_count(blocknode)  );
#endif
	linkup(blk_parent, (int)kid_count(blocknode),
		tree(N_SYMBOL_TABLE, NIL));
}

/* routine to do a declaration when we know damn little about what is
   being declared */
/* 
 *
 */

do_decl( nodetype, decltype )
bits8 nodetype;			/* what node to expand */
type_code decltype;		/* for use in s_declare */
{
	exp_prod( nodetype );
	do_exp_prod( N_DECL_ID, decltype );
	skip_down();
}

	/* Table of relationships between parents of declarations and
	   what kind of symbol is to be declared */

static
bits8 symtypes[] = {
N_FORM_VALUE, T_FORM_VALUE,
N_FORM_REF, T_FORM_REF,
N_VAR_DECL, T_VAR,
#ifdef N_VAR_ABSOLUTE
N_VAR_ABSOLUTE, T_ABSVAR,
#endif
N_CONST_DECL, T_CONST,
#ifdef TURBO
N_CONST_INIT, T_INIT,
#endif
N_FIELD, T_FIELD,
N_TYPE_DECL, T_TYPE,
N_DECL_PROC, T_PROCEDURE,
N_DECL_FUNC, T_FUNCTION,
N_TYP_ENUM, T_ENUM_ELEMENT,
N_PROGRAM, T_PROGRAM,
N_DECL_LABEL, T_LABEL,
N_FORM_FUNCTION, T_FORM_FUNCTION,
N_FORM_PROCEDURE, T_FORM_PROCEDURE,
N_VARIANT, T_FIELD,
0 };
/* 
 *  This routine makes a declaration on a C_DECL_ID stub.  It looks at
 *  the parent to see what the sym_dtype of the symbol should be, although
 *  often the type compile routines set this themselves so you don't have
 *  to worry to much about it.
 */
	/* routine to do a declaration from a C_DECL_ID stub.
	   looks up parent to see what is going on, and does
	   what need be done */

decl_it(dtype)
NodeNum dtype;		/* what kind of declaration */
{
	preg1 nodep lookup;		/* what parent declaration has */
	preg2 NodeNum *lp;		/* lookup pointer */
	ireg1 NodeNum looktype;		/* what parent's type is */

	lookup = cursparent;
	if( is_a_list(cursparent) )
		lookup = tparent(cursparent);
	looktype = ntype(lookup);
	lp = table_lookup( symtypes, looktype, 2 );

#ifdef DB_ACTROUT
	if (tracing) fprintf(dtrace, "decl_it dtype %d, lookup %x, looktype %d, lp %x\n", dtype, lookup, looktype, lp);
#endif

	if( lp ) {
		/* special check for a field */
		if( *lp == T_FIELD )
			dtype = N_ID_FIELD;
		do_exp_prod( dtype, *lp );
		skip_down();
		return;
		}
	/* oh no!  We did not find our type */
	bug( ER(239,"Declaration under node type %d not possible"), looktype );
}

/*
 * This takes a node that has a kid which is a list of zero elements.
 * (procedure call, set, parameters etc.)  It expands the list again
 * to have one element, a placeholder
 */

reexpand(_np)
nodep _np;
{
	preg1 nodep np = _np;
	preg2 listp lp;

	printt1("reexpand(%x)\n", np);

	if (ntype(np) == N_EXP_SET)
		lp = kid1(np);
	else
		lp = kid2(np);

	if (!is_a_list(lp))
		bug(ER(240,"reexpand: %x is not a list!\n"), lp);

	if (listcount(lp))
		cursor = kid1(lp);
	else
		open_list(lp);
}

/* Routine used when an ID is typed on an initializer.  It checks to see
 * if it is a record initializer, and makes a field init if so
 */

do_recid()
{
	preg1 nodep rectype = kid2(find_true_parent(cursor));

	if( rectype && ntype( rectype ) == N_TYP_RECORD ) 
		exp_prod( N_FLD_INIT );
	exp_prod( N_ID );
	
}
