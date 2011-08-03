
/*
 *DESC: More general top level text input processing
 */


/*
 * text2.c - since QNX C compiler couldn't handle all of text.c in
 * one gulp.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "token.h"
#include "scan.h"
#include "ole.h"
#include "class.h"
#include "flags.h"
#include "jump.h"
#include "keys.h"
#ifndef QNX
# include "ctype.h"
#endif
#ifndef A_CHARTEXT
#define A_CHARTEXT 0x7f
#endif

#ifdef DB_TEXT
# define PRINTT
#endif
#include "printt.h"

extern char *oledit();
extern int savestate;
extern buf_el *savecp;
extern buf_el *savep;
extern buf_el *sptext, *sctext, *sftext;
extern int saveindent;
extern buf_el clbuf[];
extern int nptok;
extern struct token ptok[];
extern buf_el BENULL[];
extern inchar tokentext[];

static is_const();
static isValidID();

/* Get a command from the bottom line */
getbotline()
{
	char holdbuf[MAX_LEN_LINE];
	char *p1;
	register char *p2;
	char *strrchr();
	getprline(">", holdbuf, MAX_LEN_LINE);
	/* scan for AL_FNAME code */
	p1 = holdbuf;
	p2 = tokentext;

	while( *p2 = *p1++ ) {
		if( *p2 == AL_FNAME ) {
			char *fnptr;
			char *dotpos;
			fnptr = work_fname( curr_workspace );
			if( !fnptr )
				fnptr = "Null";
			printt2( "expanding fname %s in line %s\n",fnptr,holdbuf);
			strcpy( p2, fnptr );
			if( dotpos = strrchr( p2, '.' ) )
				p2 = dotpos;
			 else
				p2 += strlen( fnptr );
			}
		 else
			p2++;
		}

}
#ifdef IBMPC
#define PROMPTC AC_NORMAL|A_REVERSE
#else
# ifdef ICON
# define PROMPTC _STANDOUT
# else
# define PROMPTC 0
# endif
#endif

static
promptBot(str)
char	*str;
{
	wmove(curWindows[CMDWIN], 0, 0);
	curOn(curWindows[CMDWIN]);
	waddstr(curWindows[CMDWIN], str);
	wclrtoeol(curWindows[CMDWIN]);
	wrefresh(curWindows[CMDWIN]);
}

/*
 * Get a string from the prompt line.  The given prompt is used to
 * prompt the user, and the result will be placed in the caller
 * provided buffer retbuf.
 */
getprline(prompt, retbuf, maxlen)
char *prompt;		/* prompt string on bottom line */
char *retbuf;		/* place to store result */
int maxlen;		/* max length of input */
{
	register char *cp;
	register int c;
	char *max = retbuf + maxlen - 2;
	extern inchar *macp;
	buf_el	beprompt[MX_WIN_WIDTH];
	char inbuf[MAX_LEN_LINE];	/* to avoid stomp */

	cp = inbuf;
	if (macp && *macp == 0)
		macp = NULL;
	if(!macp) {
		promptBot(prompt);

		charToBE(prompt, beprompt,PROMPTC);

		oledit(curWindows[CMDWIN], 0, OLE_NOHL|OLE_CMD, beprompt, BENULL, BENULL, (char *)NULL, inbuf, 0);
		statusLine();
	} else {
		/* we're inside a macro, just scoop up the chars. */
		for (cp=inbuf; ; ) {
			c = readkey();
			if (c == '\n')
				break;
#ifdef Notdef
			if( c == AL_ICOMMAND )
				err_return();
#endif 
			if (cp < max)
				*cp++ = c;
		}
		*cp++ = 0;
	}
	setlachar(0);
	/* now copy safely into user buffer */
	strncpy( retbuf, inbuf, maxlen );
}

getKeyPrompt(prompt)
char	*prompt;
{
	int	c;

	promptBot(prompt);
	c = readkey();
	statusLine();
	return c;
}

/*
 * Return a string in buf.  If the supplied str is non null, copy it into
 * buf, else prompt for a string.  This is commonly used for commands which
 * permit command line args, and must prompt for them if they are missing.
 */
getIfNull(str, prompt, buf, buflen)
char	*str;
char	*prompt;
char	*buf;
int	buflen;
{
	if (str && strlen(str) > 0)
		strncpy(buf, str, buflen);
	else
		getprline(prompt, buf, buflen);
}

#define EDITQUERY 127
#define USEEXP	127

static
bits8
canedit[] =
{
	N_ID, TOK_ID,
	N_DECL_ID, TOK_ID,
	N_CON_INT, TOK_INT,
	N_CON_REAL, TOK_NUMB,
	N_CON_STRING, TOK_STRING,
	N_T_COMMENT, TOK_CMNT,
	N_CON_PLUS, USEEXP,
	N_CON_MINUS, USEEXP,
	N_CON_INT, USEEXP,
	N_CON_REAL, USEEXP,
	N_CON_CHAR, USEEXP,
	N_ST_ASSIGN, USEEXP,
	N_VAR_ARRAY, USEEXP,
	N_VAR_POINTER, USEEXP,
	N_VAR_FIELD, USEEXP,
	N_OEXP_2, USEEXP,
	N_OEXP_3, USEEXP,
	N_EXP_NIL, USEEXP,
	N_EXP_PAREN, USEEXP,
	/* N_EXP_ESET, USEEXP,*/
	N_EXP_SET, USEEXP,
	N_EXP_NOT, USEEXP,
	N_EXP_FUNC, USEEXP,
	N_EXP_UPLUS, USEEXP,
	N_EXP_UMINUS, USEEXP,
	N_EXP_PLUS, USEEXP,
	N_EXP_MINUS, USEEXP,
	N_EXP_OR, USEEXP,
	N_EXP_TIMES, USEEXP,
#ifdef TURBO
	N_EXP_XOR, USEEXP,
	N_EXP_SHL, USEEXP,
	N_EXP_SHR, USEEXP,
#endif
	N_EXP_SLASH, USEEXP,
	N_EXP_DIV, USEEXP,
	N_EXP_MOD, USEEXP,
	N_EXP_AND, USEEXP,
	N_EXP_EQ, USEEXP,
	N_EXP_NE, USEEXP,
	N_EXP_LT, USEEXP,
	N_EXP_LE, USEEXP,
	N_EXP_GT, USEEXP,
	N_EXP_GE, USEEXP,
	N_EXP_IN, USEEXP,
	N_EXP_ERROR, USEEXP,
	0, 0,
};

/*
 * Returns true if the type of the node "nod" is the equivalent of the
 * type of the token "tok".  That is, if a subtree of type "nod" can
 * be replaced by a token of type "tok".  As a special case, expression
 * nodes are considered replaced only by the tree in oletree.
 *
 * Passing tok=EDITQUERY causes a check for any type, that is, tells if that
 * type of subtree can be edited at all.
 */
static int
typematch(nod, tok)
int nod, tok;
{
	register bits8 *p;
	register int rv;
	extern nodep oletree;

	p = table_lookup(canedit, nod, 2);
	printt3("typematch(nod=%d, tok=%d), table_lookup %x\n", nod, tok, p);
	if (p == NULL)
		rv = FALSE;
	else if ( nod == N_DECL_ID )
		rv = tok == *p || tok == TOK_INT;
	else if (tok == EDITQUERY)
		rv = TRUE;
	else if (*p == USEEXP )
		rv = oletree != NIL;
	else
		rv = tok == *p;
	printt3("p %x, *p %d, typematch returns %d\n", p, p ? *p : 0, rv);
	return rv;
}

static
bits8
edclasses[] =
{
	C_COMMENT,
	C_LABEL,
	C_PROC_ID,
	C_FUN_ID,
	C_CONSTANT,
	C_TYPEID,
	C_FIELD,
	C_FORMAL,
	C_VAR,
	C_EXP,
	C_DECL_ID,
	C_LABEL_DECL,
	C_HIDECL_ID,
	C_FLD_NAME,
	0,
};

/*
 * Return TRUE if the node given can be textually edited.
 */
/*int
canbeedited(xn)
nodep xn;
{
	register nodep n = xn;
	ClassNum cl;
	register bits8 *p;

	if (!is_a_stub(n))
		return typematch(ntype(n), EDITQUERY);
	cl = int_kid(0,n);
	p = table_lookup(edclasses, cl, 1);
	if (p)
		return TRUE;
	return FALSE;
}*/

#ifdef notdef
/*
 * Kludge to avoid editing the opening and closing quotes of a string.
 * We move the quotes outside the ^A's and adjust the pointers.
 */
fixquote(xcp)
buf_el *xcp;
{
	register buf_el *cp;
	register buf_el *p;
	cp = xcp;

	printt3("fixquote(->%s<-), cp[-1] %d, cp[0] %d\n", cp, cp[-1], cp[0]);
	if (cp[-1] != 1)
		return;
	if (!strchr("\"'", cp[0]))
		return;
	cp[-1] = cp[0];
	cp[0] = 1;
	sctext++;

	for (p=sctext; *p && *p != 1; p++)
		;
	
	if (*p != 1) {
		message(ER(235,"fixquote can't find closing ^A"));
		return;
	}
	p[0] = p[-1];
	p[-1] = 1;
	sftext--;
}
#endif

/*
 * Textually change the tree underneath node xcp (which has type t) to
 * look like the character string newstr.
 *
 * There are fairly severe restrictions on what kind of subtree we
 * can have here.  Basically, the subtree must be something we know
 * how to parse, specifically, a comment, an identifier, or an
 * expression.  We figure out what type is expected, create a
 * new subtree from the string with the appropriate type, and prune
 * out the old subtree and graft in the new one.
 */
reuse(xcp, t, xnewstr)
curspos xcp;
NodeNum t;
char *xnewstr;
{
	register curspos cp;
	register char *newstr;
	nodep newsubtree;

	register int cl;

	extern nodep oletree;

	cp = xcp;
	newstr = xnewstr;

	printt4("reuse subtree %x, type %s, newstr >%s<, oletree %x\n",
		(int)cp, NodeName(t), newstr, oletree);
	/* ASSUME: cp is cursor */
	if (cp != cursor) bug(ER(268,"cp not cursor in reuse"));

	mark_line(cp, SMALL_CHANGE );
	cl = ex_class(cp);
	printt1("Class of cursor = %d\n", cl );


	if (cl == C_VAR || cl == C_EXP || cl == C_CONSTANT)
		t = FIRST_EXPRESSION;

	if( cl == C_CONSTANT ) {
		if( !is_const( oletree ) ) {
			error( ER( 278, "'%s' is not a valid constant"),
				newstr );
		}
	}

	if( cl == C_VAR ) {
		if( not_variable( oletree ) && !isValidID(newstr) ) {
			error( ER( 279, "'%s' is not a valid variable name"),
				newstr );
		}
	}

	/* if class is an expression, anything goes */

	printt1( "Reuse - switching on type %d\n", t );
	
	switch(t) {
	case N_T_COMMENT:
		newsubtree = tree( t, NIL );
		s_conval( newsubtree, t, newstr );
		break;

	case N_ID:
		if( isValidID(newstr) ) {
			newsubtree = symref(newstr);
		} else {
			error( ER( 280, "'%s' is not a valid identifier"),
				newstr );
		}
		break;

	case N_DECL_ID:
	case N_HIDECL_ID:
		/* If newstr is an identifier, install it */
		if (isValidID(newstr)) {
			chg_decid(cp, newstr);
			newsubtree = cp;
		} else {
			error( ER( 280, "'%s' is not a valid identifier" ),
				newstr );
		}
		break;

	case FIRST_EXPRESSION:
	case N_ST_ASSIGN:
		printt1("First Expression oletree = %d\n", (int)oletree);
		if (oletree == NIL ||
		    ((ntype_info(t) ^ ntype_info(ntype(oletree))) & F_LINE))
			return FALSE;
		newsubtree = oletree;
		break;

	default:
		error(ER(122,"cantedit`Can't change a %s textually"), NodeName(t));
	}

	prune(cp);
	/* another list insertion can we modularize this? */
	if( is_a_list(newsubtree) ) {
		int index;
		graft( kid1(newsubtree), cursor );
		for( index = 1; index < listcount(newsubtree); index++ ) {
			exp_list(1);
			graft( node_kid(newsubtree,index), cursor );
			}
		}
	 else
		graft(newsubtree, cursor);

	printt1("before expmove, cursor at %x\n", cursor);
	/* Now move cursor to rightmost position so tab goes on */
	/* KLUDGE - don't go down if a comma is pending */
	if (t == FIRST_EXPRESSION && !(nptok > 0 && ptok[nptok-1].token_no
					== TOK_COMMA ) ) {
		int nk;		/* number of kids */
		nodep nc;	/* new cursor */

		while ((nk=kid_count(ntype(cursor))) > 0) {
			nc = node_kid(cursor, nk-1);
			if (nc != NIL)
				cursor = nc;
			else
				break;	/* don't follow NIL */
		}
	}
	/*
	 * Here's a quick hack: Don't leave the cursor on an empty
	 * list, move it to the parent instead.
	 */
	if (ntype(cursor) == N_LIST && !listcount(FLCAST cursor))
		cursor = tparent(cursor);
	printt1("after expmove, cursor at %x\n", cursor);
	return TRUE;
}

static
is_const( t )
register nodep t;
{
	register int i;

	if( t == (nodep)0 ) return FALSE;

	i = ntype(t);
	if( i == N_CON_INT || i == N_CON_REAL || i == N_CON_CHAR ||
	    i == N_CON_STRING ) return TRUE;

	if( i == N_EXP_UMINUS ) {
		s_ntype( t, N_CON_MINUS );
		i = N_CON_MINUS;
	}

	if( i == N_EXP_UPLUS ) {
		s_ntype( t, N_CON_PLUS );
		i = N_CON_PLUS;
	}

	if( i == N_CON_PLUS || i == N_CON_MINUS ) 
		return is_const( kid1(t) );

	return FALSE;
}

static
isValidID(str)
register char	*str;
{
	if (isalpha(*str++)) {
		for ( ; isalnum(*str) || *str == '_'; str++)
			;
		if (*str == 0)
			return TRUE;
	}
	return FALSE;
}

/*
 * The following routines are called from the output/Treeprint
 * routines.  They save the text of the current line away in
 * the buffers named at the beginning of text.c, so we'll have
 * a record of the current line for oledit.
 */

/* Save the number of columns to indent */
save_indent(indent)
int indent;
{
	saveindent = indent;
}

#ifdef OLDEDITOR

/* Save preceding part of line */
save_ptext(bp, xcp)
reg buf_el *bp;	/* beginning of part we want to save */
buf_el *xcp;	/* ending of part we want to save */
{
	register buf_el *p;
	register buf_el *cp;
	cp = xcp;

	if (savestate != 0)
		return;
	sptext = clbuf;

/* Do not put preceding whitespace into clbuf buffer.  Out_Line will
 * handle the indents for the editor window, and there is no indent
 * for the command window 
 */

#ifdef notdef
	for (p=clbuf; saveindent--; )
		*p++ = ' ';
#else
        p=clbuf;
#endif
	while (bp < cp)
		*p++ = *bp++;
	*p++ = 'X';	/* flag char */
	*p = 0;
	savecp = cp;
	savep = p;
	savestate = 1;
	printt3("save_ptext, saveindent %d, clbuf >%s<, len %d\n",
		saveindent, clbuf, savep - clbuf);
}

/* Save the cursor part of line */
save_ctext(xcp)
buf_el *xcp;	/* ptr to end of area we want, beg was in cp to save_ptext */
{
	register buf_el *cp;
	cp = xcp;

	if (savestate != 1)
		return;
	sctext = savep;
	while (savecp < cp)
		*savep++ = *savecp++ & A_CHARTEXT;
	*savep++ = 'X';
	*savep = 0;
	savestate = 2;
	printt3("save_ctext, saveindent %d, clbuf >%s<, len %d\n",
		saveindent, clbuf, savep - clbuf);
}

/* Save ending part of the line */
save_ftext(xcp)
buf_el *xcp;	/* ptr to end of area we want, beg was in cp to save_ctext */
{
	register buf_el *cp;
	cp = xcp;

	if (savestate != 2)
		return;
	sftext = savep;
	while (savecp < cp)
		*savep++ = *savecp++;
	*savep++ = 0;
	savestate = 0;
#ifdef DB_TEXT
	if (tracing) {
		int i;

		fprintf(dtrace, "save_ftext, saveindent %d, len %d, clbuf -",
			saveindent, savep - clbuf);
		for (i=0; clbuf[i]; i++)
			fprintf(dtrace, "%x ", clbuf[i]);
		fprintf(dtrace, "-\n");
	}
#endif
}
#endif OLDEDITOR
