/*
 * expression parser.
 * Method used: operator precedence.  See Aho & Ullman, "Principles of
 * Compiler Design" ("the dragon book"), pp 158-162, or Gries, "Compiler
 * Construction for Digital Computers", pp 122-132, for details on the method.
 * Operator precedence was used, in spite of its fading popularity, because
 * it's simple, small, and handles expressions well.  This would not
 * generalize to handle things like if statements well.
 *
 * The code in this file turns character strings (or lists of already
 * scanned tokens) and produces a parsed tree.  It only handles expressions,
 * which currently does not include assignment statements (but it could
 * be added easily.)  Routines for editing the text of expressions and
 * inserting things into the tree are in oledit.c and text*.c.
 */
/*
 *DESC: Operator precedence expression parser used when the user is typing
 *DESC: in an expression in infix mode.
 */


#include "alice.h"
#include "scan.h"
#include "token.h"
#include "class.h"

#ifdef DB_EXPPARSE
#define PRINTT
#endif
#include "printt.h"

char Null_Str[] = {0};
#ifndef DB_EXPPARSE
#define stackdump()	/* nothing */
#endif

extern char tokentext[];	/* output from scanner */

#ifdef HAS_EXPPARSE
#include "typecodes.h"
/* #undef string (who defines this?) */

/* Possible values in operator precedence table */
#define GT 0	/* .> relation */
#define RR 1	/* syntax error */
#define EQ 2	/* .= relation */
#define LT 3	/* .< relation */
#define FIN 4	/* finished - end of expression */

#define NT 16	/* # of types of tokens */

/* Possible types of tokens in expressions */
#define E_ADDOP		0	/* +, -, etc (binary) */
#define E_MULOP		1	/* *, /, div */
#define E_NAME		2	/* identifier, constant */
#define E_RELOP		3	/* <, =, <=, etc */
#define E_UNOP		4	/* unary +, -, not */
#define E_LPAREN	5	/* ( */
#define E_RPAREN	6	/* ) */
#define E_COMMA		7	/* , */
#define E_DOT		8	/* . */
#define E_LBRACK	9	/* [ */
#define E_RBRACK	10	/* ] */
#define E_CARET		11	/* ^ */
#define E_COLON		12	/* :, for write statement */
#define E_DOTDOT	13	/* .., WHY WAS THIS DELETED? */
#define E_EOF		14	/* end of expression */
#define E_EXP		15	/* nonterminal */

/*
 * The operator table.  This table controls the precedence of the various
 * operators, and is the heart of the parser.  Since it was constructed
 * by hand, it may contain errors.  The EXP and := cases are untested.
 *
 * The basic idea of the table is that, if the parsing stack contains
 * a b c d, then some relation holds between a . b, b . c, and c . d,
 * where the . represents the relation, and the relation can be determined
 * by looking up optab[a][b], etc.  The handle (piece to be reduced) will
 * be magically bracketted by <. and >. relations, with =. between tokens
 * within the handle.
 *
 * To determine the correct entry for any given table entry optab[a][b],
 * you must envision the situation when the parser comes across
 *	... a b
 * and decide what should be done.  If a is likely to be at the end
 * of something that needs to be reduced, set optab[a][b] = >. which
 * will force a to be reduced and then retried next to b.  If they are
 * syntactic pieces of the same construction, that is, appear side by side
 * in a production, then =. is right.  If b begins a production then
 * .< is probably right.  If the two cannot appear next to each other, it's
 * an error.  Note that optab[a][b] and optab[b][a] are totally unrelated;
 * this operation is not symmetric.  It is also possible that there will
 * be ambiguities, which may mean changing the grammar or parsing method.
 *
 * In more concrete terms, >. means that in this state, the parser should
 * reduce, and the handle will go back to the rightmost <. on the stack.
 * =. and <. mean the parser should shift (e.g. push onto the stack and get
 * another token.)
 *
 * The EXP row and column are meaningless and probably should be deleted.
 * Only operators count - expressions and identifiers get deleted.
 */

/* changes to the table */

/* changed [comma][lparen] to LT from GT */

/* changed [colon][rbrack] from RR to GT */  /* for Memx, Portx[exp:exp] */
/* changed [lbrack][colon] from RR to LT */

/* changed [exp][unop] from RR to LT	 */  /* for <expr, -expr>	 */
/* changed [comma][unop] from GT to LT   */

char optab[NT][NT] = {
/*LHS rel RHS: +   *  id   <   -   (   )   ,   .   [   ]   ^   :  ..  $ EXP*/
/*ADDOP  + */ GT, LT, LT, GT, LT, LT, GT, GT, LT, LT, GT, LT, GT, GT,GT, LT,
/*MULOP  * */ GT, GT, LT, GT, LT, LT, GT, GT, LT, LT, GT, LT, GT, GT,GT, LT,
/*NAME  id */ GT, GT, RR, GT, RR, EQ, GT, GT, GT, GT, GT, GT, GT, GT,GT, RR,
/*RELOP  < */ LT, LT, LT, RR, LT, LT, GT, GT, LT, LT, GT, LT, GT, GT,GT, LT,
/*UNOP   - */ GT, GT, LT, GT, LT, LT, GT, GT, LT, LT, GT, LT, GT, GT,GT, LT,
/*LPAREN ( */ LT, LT, LT, LT, LT, LT, EQ, LT, LT, LT, RR, LT, GT, GT,GT, LT,
/*RPAREN ) */ GT, GT, RR, GT, RR, GT, GT, GT, GT, GT, GT, GT, GT, GT,GT, RR,
/*COMMA  , */ LT, LT, LT, GT, LT, LT, GT, GT, LT, LT, GT, LT, LT, LT,GT, LT,
/*DOT    . */ GT, GT, EQ, GT, RR, RR, GT, GT, GT, GT, GT, GT, GT, GT,GT, EQ,
/*LBRACK [ */ LT, LT, LT, LT, LT, LT, RR, LT, LT, LT, EQ, LT, LT, LT,GT, LT,
/*RBRACK ] */ GT, GT, RR, GT, RR, RR, GT, GT, GT, GT, GT, GT, GT, GT,GT, RR,
/*CARET  ^ */ GT, GT, RR, GT, GT, RR, GT, GT, GT, GT, GT, GT, GT, GT,GT, RR,
/*COLON  : */ LT, LT, LT, LT, LT, LT, GT, GT, LT, LT, GT, LT, EQ, RR,GT, LT,
/*DOTDOT.. */ LT, LT, LT, LT, LT, LT, GT, GT, LT, LT, GT, LT, RR, GT,GT, LT,
/*EOF    $ */ LT, LT, LT, LT, LT, LT, RR, LT, LT, LT, RR, LT, LT, LT,RR, LT,
/*EXP    E */ GT, GT, RR, GT, LT, LT, GT, GT, GT, EQ, GT, GT, GT, GT,GT, RR,
};

#define PSTACK_MAX 50

struct parstk {
	int toknum;	/* token number, from the scanner */
	int toktype;	/* token number, from the E_ list above */
	char *toktext;	/* text of token, useful for ids and constants */
	nodep subtree;	/* tree structure being built */
} parsestack[PSTACK_MAX];	/* parsing stack */

int stacktop;		/* top of stack */
int domalloc;		/* if true, actually build expression tree */

char ttexts[MAX_LEN_LINE];	/* string space for token texts */
char *ttptr;			/* next available loc in ttexts */

extern char *scaninptr;	/* input source for scanner */

extern char *opname();

/*
 * Parse the character string into an expression.
 * Return 1 if the expression is syntactically legal, 0 otherwise.
 * If legal, the parsed tree is placed in rtree.
 * rtree can be null, in this case just parse and don't build the tree.
 */
int
s_expparse(string, rtree)
char *string;	/* text char string to be parsed */
nodep *rtree;	/* place to put resulting tree, or NULL for just syntax check */
{
	struct token ta[80];		/* token array from scanner */
	char tbuf[MAX_LEN_LINE];	/* space for token texts */
	register int nt;		/* number of tokens */

	/* This routine just calls the scanner to break the string into
	 * tokens, and then it calls the parser to parse the tokens
	 */
	printt1( "s_expparse( '%s' )\n", string);
	/* if the string is null, it won't parse properly */
	if (string[0] == 0)
		return FALSE;
	/* break the string up into tokens */
	expscan(string, ta, tbuf);
	/* count the number of tokens (just for debuging) */
	for (nt=0; ta[nt].token_no != TOK_EOF; nt++);
	printt3( "s_expparse gets %d tokens, first %d '%s'\n", nt, ta[0].token_no, ta[0].ttext);

	/* If there are no tokens, return */
	if( nt == 0 )
		return TRUE;

	/* and now actually parse it based on the list of tokens */
	return t_expparse(ta, rtree);
}

/*
 * Parse an expression, represented as a list of tokens.
 * This assumes the scanner has already been called 
 */
int
t_expparse(ta, rtree)
struct token *ta;	/* array of tokens to be parsed */
nodep *rtree;		/* place to put resulting tree */
{
	register int tn;	/* token number */
	reg int tt, pt;		/* token type, previous token */
	int parseok = 0;	/* return value */
	reg int i;
	int tnn = 0;		/* current token number in array */
	extern char *macp;
	extern int lookaheadchar;

	printt2( "t_expparse( ta %x, rtree %x )\n", ta, rtree);
	domalloc = (rtree != NULL);
	stacktop = 0;
	ttptr = ttexts;
	parsestack[0].toknum = E_EOF;

	tt = TOK_ID;
	while (tt != TOK_EOF) {
		pt = parsestack[stacktop].toknum;
		stackdump();
		tt = ta[tnn].token_no;
		tn = expmap(tt, pt);
		printt3( "expmap(%d, %d) returns %d\n", tt, pt, tn);
		/*
		 * If the token isn't an expression token, or is one that
		 * can't begin an expression, like ), return cantbegexp.
		 */
		if (stacktop == 0 && (tn==-1 || (0xc234 & (1<<tn)) == 0)) {
		printt0( "non-exp, returning cantbegexp\n");
			/* We have found a token that can not start an
			 * expression, thus this is not an expression
		 	 * indicate to the caller this by setting cantbegexp
			 * and store the offending token number in canttokno
			 */
			parseok = FALSE;
			goto err;
		}
		/* abort if the parse stack overflows! */
		if( stacktop >= PSTACK_MAX-1 ) {
			if( domalloc ) {
				warning( ER(8,"complex`Expression too complex") );
				}
			parseok = FALSE;
			goto err;
			}
		parsestack[++stacktop].toknum = tn;
		parsestack[stacktop].toktype = tt;
		printt3( "push toknum %d toktype %s (%d)\n", tn, tokname(tt), tt);
		if (tn == E_NAME)
			parsestack[stacktop].toktext = ta[tnn].ttext;
		tnn++;

		/* Look it up and decide if we've found the RHS of the handle */
		switch(optab[pt][tn]) {
		case EQ:
		case LT:
			/* No.  Shift and keep going. */
			continue;
		case FIN:
			/* End of expression */
			parseok = TRUE;
			continue;
		case RR:
			/* Syntax error */
			parseok = FALSE;
			goto err;
		case GT:
			/* Yes, reduce it */
			do {	/* possible chain reductions */
				if (reduce()) {
					/*
					 * No matching production on RHS, must
					 * be a syntax error.
					 */
					parseok = FALSE;
					goto err;
				}
				tn = parsestack[stacktop].toknum;
				for (i=stacktop-1; parsestack[i].toknum==E_EXP; i--)
					;
				pt = parsestack[i].toknum;
			} while (optab[pt][tn] == GT);
			continue;
		}
	}
	/* We drop out if FIN or hit EOF in input - check that it's OK */
	if (stacktop == 2 && parsestack[1].toknum == E_EXP &&
			     parsestack[2].toknum == E_EOF) parseok = TRUE;

err:
	printt2( "end of e_expparse parseok %d, subtree %x\n",
		parseok, parsestack[1].subtree);
	if (domalloc)
		if (parseok) {
			*rtree = parsestack[1].subtree;
		} else {
			*rtree = NIL;
		}
	return parseok;
}

/* Table for mapping token numbers into expression piece numbers */
bits8
tok_e_table[] =
{
	TOK_PLUS, E_ADDOP,
	TOK_OR, E_ADDOP,
#ifdef TURBO
	TOK_XOR, E_ADDOP,

	TOK_SHL, E_MULOP,
	TOK_SHR, E_MULOP,
#endif
	TOK_STAR, E_MULOP,
	TOK_SLASH, E_MULOP,
	TOK_AND, E_MULOP,
	TOK_DIV, E_MULOP,
	TOK_MOD, E_MULOP,

	TOK_ID, E_NAME,
	TOK_CHAR, E_NAME,
	TOK_STRING, E_NAME,
	TOK_INT, E_NAME,
	TOK_NUMB, E_NAME,
	TOK_NIL, E_NAME,

	TOK_EQ, E_RELOP,
	TOK_NE, E_RELOP,
	TOK_LE, E_RELOP,
	TOK_LT, E_RELOP,
	TOK_GE, E_RELOP,
	TOK_GT, E_RELOP,
	TOK_IN, E_RELOP,

	TOK_NOT, E_UNOP,
	TOK_LPAREN, E_LPAREN,
	TOK_RPAREN, E_RPAREN,
	TOK_COMMA, E_COMMA,
	TOK_DOT, E_DOT,
	TOK_LBRACKET, E_LBRACK,
	TOK_RBRACKET, E_RBRACK,
	TOK_UPARROW, E_CARET,
	TOK_COLON, E_COLON,
	TOK_DOTDOT, E_DOTDOT,
	TOK_EOF, E_EOF,
	0, 0
};

/*
 * Map a token type into a table index into optab.
 */
int
expmap(tt, pt)
register int tt; /* the token returned from the scanner. */
register int pt; /* the previous token, used to detect unary operators. */
{
	register bits8 *p;
	extern bits8 *table_lookup();

	/*
	 * A minus is a unary operator unless it's followed by one
	 * of these four tokens, in which case it's binary.
	 */
	if (tt == TOK_MINUS) {
		if ((1 << pt) & ((1<<E_NAME) | (1<<E_RPAREN) | (1<<E_RBRACK) |
				 (1<<E_CARET)))
			return E_ADDOP;
		return E_UNOP;
	}
	p = table_lookup(tok_e_table, tt, 2);
	if (p == NULL)
		return -1;
	return *p;

}

/*
 * Macros used to get at the stack.  LHS is the left hand side, used to
 * indicate where the result of the reduction (the LHS of the production)
 * goes.  RHS(i) is the ith thing previously on the stack, from the right hand
 * side of the production.  Note that LHS and RHS(1) occupy the same physical
 * memory on the stack.
 */
#define rhssize() (stacktop-bot-1)	/* number of elements in handle */

/* Productions have fixed size RHS, so this macro checks it */
#ifdef DB_EXPPARSE
#define checkrhs(num) if (rhssize() != num) { if (tracing) fprintf(dtrace, "checkrhs %d fails, found %d\n", num, rhssize()); return TRUE; }
#else
#define checkrhs(num) if (rsiz != num) return TRUE;
#endif

/* Push and pop things onto and off of the stack */
#define pop(num) { np = num-1; stacktop -= (num-1); }
#define push(t) { stacktop += 1; parsestack[stacktop].subtree = t; }
#define RHS(n) (parsestack[bot+n])
#define LHS    parsestack[bot+1]

#ifdef notdef
/* Nodes which can be the root of a LHS */
static char ValidLHSNodes[] = {
	N_ID,
	N_VAR_ARRAY,
	N_VAR_POINTER,
	N_VAR_FIELD,
	0
};
#endif

/*
 * reduce: find a production to reduce and do the reduction.
 * The RHS can be found in RHS(n), put the result in LHS.
 * Caution: LHS is the same as RHS(1), so storing in LHS will
 * clobber RHS(1).  Also, you must keep in mind that the handle
 * will be surrounded by RHS(0) and RHS(n+1) (at bot and stacktop,
 * respectively) which are not part of the handle.
 *
 * I changed almost every reference from LHS and RHS() to pointers
 * LHSp and RHS*p, changed checkrhs and rhssize, and moved the
 * pop on reduction outside of the switch.  All this saved 1300 bytes
 * (out of 2700 the function originally used) on the ICON...Jan
 */
reduce()
{
    register int bot;			/* bottom of handle */
    register struct parstk *LHSp;	/* ptr to LHS on stack */
#define RHS1p LHSp
    register struct parstk *RHS2p;	/* 2nd element on stack */
    struct parstk *RHS3p;		/* 3rd element on stack */
    int tn, btn;			/* current & bottom token number */
    int key;				/* token number to key on */
    int ost;				/* old stack top */
    int np;				/* number popped, set in pop() above */
    nodep tree();
    int rsiz;				/* size of RHS */

    printt1("reduce, stack top %d\n", stacktop);

    ost = stacktop;
    stackdump();

    /* Look for the handle, whose left end is marked with LT */
    tn = parsestack[stacktop].toknum;
    for (bot=stacktop-1; bot>=0; bot--) {
	btn = parsestack[bot].toknum;
printt2( "btn = %d, tn = %d\n", btn, tn );
	if (btn < E_EXP) {
	    key = btn;
printt1( "optab = %d\n", optab[key][tn] );
	    if (optab[key][tn] == LT)
		break;
	    tn = key;
	}
    }

    printt2( "reduce found bot %d, tn %d\n", bot, tn);

    if (tn == E_LBRACK && parsestack[bot+1].toknum == E_EXP) {
	    /* handle starts with EXP, must be array [ Elist ] */
	    printt1("handle starts with EXP, change tn from %d to E_NAME\n", tn);
	    tn = E_NAME;
    }

    LHSp = &LHS;
    RHS2p = &RHS(2);
    RHS3p = &RHS(3);

    rsiz = rhssize();	/* to shrink checkrhs() */

    switch (tn) {
    case E_EOF:       /* START ::= $ E $ */
    case E_LPAREN:    /* E ::= ( E ) */
	checkrhs(3);
	if (domalloc)
	    LHSp->subtree = tree( N_EXP_PAREN, RHS2p->subtree );
	break;
    case E_NAME:    /* E ::= nil */
		    /* E ::= func ( Elist ) */
	    	    /* E ::= array [ Elist ] */
	    	    /* E ::= id */

	if( RHS1p->toktype == TOK_NIL ) {	/* nil */
		checkrhs(1);
		if( domalloc )
			LHSp->subtree = tree( N_EXP_NIL );
		break;
		}
	
	if (RHS2p->toknum == E_LPAREN) {    /* func (Elist) */
	    checkrhs(4);
	    if (domalloc) {
		LHSp->subtree = tree(N_EXP_FUNC,
		    symref(namestring(RHS1p->toktext)), newlist(RHS3p->subtree));

	    }
	/*
	 * Reduce "array".  This is a hack to fix the problems with "a.b[c]".
	 * At this point we can have "NAME [ Elist ]" or "EXP [ Elist ]",
	 * since an earlier hack might have changed "tn" to NAME from EXP.
	 *
	 * The test "rsiz > 1" ensures we don't look past the "handle".
	 */
	} else if (rsiz > 1 && RHS2p->toknum == E_LBRACK) { /* arr [ Elist ] */
	    checkrhs(4);
	    if (domalloc) {
		if (RHS1p->toknum == E_NAME)	/* it really *is* an E_NAME */
		    LHSp->subtree = symref(namestring(RHS1p->toktext));
		/* else
			LHSp->subtree = RHS1p->subtree 	(this is a no-op) */

		if (!is_a_list(RHS3p->subtree)) {
		    LHSp->subtree = tree(N_VAR_ARRAY, LHSp->subtree,
					 RHS3p->subtree, NIL);
		} else {
		    /* multi dim. array - subscripts are kids of list */
		    listp l = LCAST RHS3p->subtree;
		    int		i;
		    int		nk = listcount(l);
		    for (i = 0; i < nk; i++)
			LHSp->subtree = tree(N_VAR_ARRAY, LHSp->subtree,
					     node_kid(l, i), NIL);
		}
	    }
	} else {    /* id */
	    int ty;
	    char *charp;
	    nodep id_node;

	    checkrhs(1);

	    charp = RHS1p->toktext;
	    switch (RHS1p->toktype) {
	    case TOK_ID:
		if (domalloc) {
		    extern listp paramStubs();
		    id_node = symref(namestring(charp));
		    switch (sym_dtype(kid_id(id_node))) {
		    case T_FUNCTION:
		    case T_FORM_FUNCTION:
		    case T_BFUNC:
		    case T_BTFUNC:
			LHSp->subtree = tree(N_EXP_FUNC, id_node,
				paramStubs(kid_id(id_node)));
			break;
		    default:
			LHSp->subtree = id_node;
			break;
		    }
		}
		break;

	    case TOK_STRING:
	    case TOK_INT:
	    case TOK_NUMB:
	    case TOK_CHAR:
		ty = tok_to_node(RHS1p->toktype,0);
		if (domalloc) {
		    LHSp->subtree = tree(ty, NIL);
		    s_conval(LHSp->subtree, ty, charp );
		}
		break;
	    }
	}
	break;
    case E_ADDOP:    /* E ::= E + E */
    case E_MULOP:    /* E ::= E * E */
    case E_RELOP:    /* E ::= E < E */
	checkrhs(3);
	if (domalloc)
	    LHSp->subtree=tree( tok_to_node(RHS2p->toktype, 0),
				RHS1p->subtree, RHS3p->subtree);
	break;
    case E_UNOP:    /* E ::= - E */
	checkrhs(2);
	if (domalloc) 
	    LHSp->subtree=tree( tok_to_node(RHS1p->toktype, 1), RHS2p->subtree);
	break;
    case E_DOT:    /* E ::= E . id */
	checkrhs(3);
	if (domalloc) {
	    symptr fldsym;
	    nodep res;
	    extern char fld_msg[];
	    nodep rectype;

		/* THIS CODE AND fdid SHOULD BE PUT TOGETHER */
		/* signal NO errors, with -1 code */
	    rectype = c_typecheck( RHS1p->subtree, 0, TC_QUICKIE );
		/* big hairy if statment */
	    if( ntype(rectype) == N_TYP_RECORD && 
	    	sym_dtype((fldsym = slookup(namestring(RHS3p->toktext), 
			NOCREATE, sym_kid(rectype), NOMOAN, NIL))) == T_FIELD ) {

		res = tree(N_ID, fldsym );
		}
	     else {
		warning( ER(189,fld_msg), RHS3p->toktext );
		res = make_stub( C_FLD_NAME );
		}
	    LHSp->subtree=tree( N_VAR_FIELD, RHS1p->subtree, res );
	    }
	break;
    case E_CARET:    /* E ::= E ^ */
	checkrhs(2);
	if (domalloc)
	    LHSp->subtree=tree( N_VAR_POINTER, RHS1p->subtree);
	break;
    case E_COLON:    /* E ::= E : E */
	if (domalloc) {
	    if (rsiz == 5)
		LHSp->subtree=tree( N_OEXP_3, RHS1p->subtree, RHS3p->subtree, RHS(5).subtree);
	    else if (rsiz == 3)
		LHSp->subtree=tree( N_OEXP_2, RHS1p->subtree, RHS3p->subtree);
	    else
		checkrhs(3);	/* will fail */
	}
	break;
    case E_COMMA:    /* E ::= Elist , E */
	checkrhs(3);
	if (domalloc) {
	    register listp l;
	    register listp r;
	    listp __exp_list();

	    l = (listp ) RHS1p->subtree;
	    r = (listp ) RHS3p->subtree;
	    l = newlist(l);
	    printt2( "comma, l %x, r %x\n", l, r);
	    /* ll_exp_list(l, NIL, listcount(l), r); */
	    l = __exp_list(l, listcount(l), r );
	    LHSp->subtree = (nodep ) l;
	    printt2( "after comma, comma, l %x, r %x\n", l, r);
	}
	break;
#ifdef Has_Assign
    case E_ASSIGN:    /* E ::= E := E */
	checkrhs(3);
	if (domalloc) {
#ifdef notdef
	    if (strchr(ValidLHSNodes, ntype(RHS1p->subtree)))
#endif
		LHSp->subtree=tree( N_ST_ASSIGN, RHS1p->subtree, RHS3p->subtree);
#ifdef notdef
	    else
		return TRUE;	/* failure */
#endif
	}

	break;
#endif
    case E_DOTDOT:	/* E ::=  E .. E */
	checkrhs(3);
	if( domalloc )
		LHSp->subtree = tree( N_SET_SUBRANGE, RHS1p->subtree,
						RHS3p->subtree );
	break;
    case E_LBRACK:
	/*
	 * There are two possibilities here.
	 * (1) [ setelements  ]	E ::= [ Elist ]
	 * (2) [ ]		E ::= [ ]
	 */
	printt3( "E_LBRACK, bot %d, stacktop %d, toknum %d\n",
	    bot, stacktop, parsestack[bot].toknum);
	if (rsiz == 2 && RHS2p->toknum == E_RBRACK ) {
	    /* [ ] */
	    if (domalloc)
		LHSp->subtree = tree(N_EXP_SET, newlist(NIL));
	} else {
	    /* [ setelements ] */
	    checkrhs(3);
	    if (domalloc)
		LHSp->subtree = tree(N_EXP_SET, newlist(RHS2p->subtree));
	}
	break;
    default:
	printt1( "t_expparse reduce default, key %d, syntax error\n", key);
	return TRUE;
    }

    pop(rsiz);

    /* Fill in the rest of the blanks */
    LHSp->toktype = 0;
    LHSp->toknum = E_EXP;
    LHSp->toktext = Null_Str;

    /* Copy extra node that was outside the handle. */
    stru_assign( parsestack[ost-np], parsestack[ost] );

    stackdump();
    return FALSE;
}

/*
 * Convert a token which is an expression element to a node type.
 * t is the token, u is nonzero if the context is unary.
 */
tok_to_node(t, u)
int t, u;
{
	/* operator nodes */
	if( t == TOK_PLUS)		return u ? N_EXP_UPLUS : N_EXP_PLUS;
	if( t ==  TOK_MINUS )		return u ? N_EXP_UMINUS : N_EXP_MINUS;
	 return t;
}

#ifdef DB_EXPPARSE
/* Dump stack to trace file for debugging */
stackdump()
{
	register int bot;	/* bottom of stack (really current location) */
	register nodep btt;	/* bottom subtree */
	reg int btn;	/* bottom token number */
	reg int ptn;	/* previous token number */

	if (dtrace == NULL)
		return;
	fprintf(dtrace, "stack(%d): ", stacktop);
	for (bot=0; bot<=stacktop; bot++) {
		btn = parsestack[bot].toknum;
		btt = parsestack[bot].subtree;
		if (btn < E_EXP) {
			if (bot > 0) {
				switch (optab[ptn][btn]) {
				case EQ: fprintf(dtrace, " ="); break;
				case LT: fprintf(dtrace, " <"); break;
				case GT: fprintf(dtrace, " >"); break;
				case RR: fprintf(dtrace, " ?"); break;
				case FIN: fprintf(dtrace, " ."); break;
				}
			}
			ptn = btn;
		}
		fprintf(dtrace, " %s(%x)", opname(btn), btt);
	}
	fprintf(dtrace, "\n");
}

/* convert an expression token type to a string, for debugging */
char *
opname(n)
int n;
{
	static char errbuf[10];

	switch (n) {
	case E_ADDOP: return "ADDOP";
	case E_MULOP: return "MULOP";
	case E_NAME: return "NAME";
	case E_RELOP: return "RELOP";
	case E_UNOP: return "UNOP";
	case E_LPAREN: return "LPAREN";
	case E_RPAREN: return "RPAREN";
	case E_COMMA: return "COMMA";
	case E_DOT: return "DOT";
	case E_LBRACK: return "LBRACK";
	case E_RBRACK: return "RBRACK";
	case E_CARET: return "CARET";
	case E_COLON: return "COLON";
/*	case E_ASSIGN: return "ASSIGN"; */
	case E_EOF: return "EOF";
	case E_EXP: return "EXP";
	default: sprintf(errbuf, "?%d?", n); return errbuf;
	}
}
#endif DB_EXPPARSE

#else
s_expparse(){}
t_expparse(){}
#endif HAS_EXPPARSE

char *ScanPtr;

/*
 * Scan all the tokens in string "str", putting the result in the caller
 * provided array ta.  The caller must also provide a place to store the
 * strings, in the buffer tbuf.
 */
expscan(str, ta, tbuf)
char *str;
struct token *ta;
char *tbuf;
{
	int n = 0;	/* number of tokens */
	int l;		/* length of string */
	register int tn;		/* token number */
	register char *p;	/* current free part of tbuf */
	extern int lookaheadchar;

	printt3( "expscan called, str '%s', ta %x, tbuf %x\n", Safep(str),
					ta, tbuf);
	p = tbuf;
	ScanPtr = str;
	while( n <= 80  ) {
		ta[n].charoff = ScanPtr - str;
		tn = bscan();
		printt2( "Token number: %d, text: '%s'\n", tn, tokentext );
		if( tn == TOK_CMNT ) {
			error( ER(224,"Comments are not allowed within expressions") );
		}
		printt2("token %d, bscan (%s)\n", tn, ScanPtr );
		ta[n].token_no = tn;

		/* If we have scanned to the end of the string, return */
		if (tn == TOK_EOF) {
			*p = 0;
			ta[n].ttext = p;
			break;
		}

		/* Get the text of the token (as put in tokentext by bscan)
		 */
		l = strlen(tokentext);
		if( (p+l+1) < (tbuf+MAX_LEN_LINE) ) {
			/* Copy the text */
			strcpy(p, tokentext);
			ta[n++].ttext = p;
			/* bump pointer */
			p += l + 1;
		} else {
			*p = 0;
			ta[n++].ttext = p;
		}
	}
	/* ignore everything after the 80th token */
	if( n >= 80 ) ta[80].token_no = TOK_EOF;
	return n;
}
