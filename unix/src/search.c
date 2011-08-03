/*
 *DESC: Editor search support
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "scan.h"
#include "token.h"
#include "alctype.h"
#include "search.h"

#ifdef DB_SEARCH
# define PRINTT
#endif
#include "printt.h"

int	Searching	= 0;


#ifndef OLD_STUFF

#define	NOT_SEARCHING	0
#define	SRCH_FWD	1
#define	SRCH_BACK	2

static int	SrchDir		= SRCH_FWD;

#define	PATLEN		MX_WIN_WIDTH

char		SrchPattern[PATLEN];

static
search(dir)
int	dir;
{
	markPopBack(cursor);
	Searching = SrchDir = dir;
	disp_lines(curr_window, 0, 99, t_up_to_line(cursor), 0, 0);
	Searching = NOT_SEARCHING;
}

srchFwd(args)
char	*args;
{
	getPattern(args);
	search(SRCH_FWD);
}

srchBack(args)
char	*args;
{
	getPattern(args);
	search(SRCH_BACK);
}

srchAgain()
{
	search(SrchDir);
}

/*
 * Prompt using old pattern.  If new pattern is nil, use old pattern
 */
getPattern(args)
char	*args;
{
	char	promptbuf[PATLEN+15];
	char	patbuf[PATLEN+1];

	strcpy(promptbuf, "Search for: ");
	if (SrchPattern[0])
		sprintf(&promptbuf[12], "[%s] ", SrchPattern);
	getIfNull(args, promptbuf, patbuf, PATLEN);
	if (patbuf[0])
		strcpy(SrchPattern, patbuf);
}


/*
 * Called from out_line to see if the pattern matches on the line.  Returns
 * index of pat into string, or -1 if no match.  Permits '?' wildcard.
 */
int
matchPattern(line, startCol)
char	*line;
int	startCol;
{
	register char	*pat;		/* walks along pattern */
	register char	*l;		/* walks along piece of line */
	register int	i;
	int	patlen		= strlen(SrchPattern);
	int	linelen		= strlen(line);
	int	ret		= -1;

	printt3("matchPattern(line=%s, startCol=%d) pat=%s",
		line, startCol, SrchPattern);

	for (i = startCol; i + patlen <= linelen; i++) {
		for (pat = SrchPattern, l = &line[i];
		     *pat && (*pat == *l || *pat == '?');
		     pat++, l++)
			;
		if (!*pat) {
			ret = i;
			break;
		}
	}

	printt1(" returns %d\n", ret);
	return ret;
}

srchDone()
{
	int	was	= Searching;

	Searching = NOT_SEARCHING;

	if (was != SRCH_FOUND)
		ncerror(ER(114,"No match for \"%s\" found"), SrchPattern);
}

#else

static struct token pt;
static char pttext[MAX_LEN_LINE];	/* have to remember previous search */

extern char *ctext();
static int lastdir = TRUE;

#ifdef HAS_SEARCH
search(fwd, tok, again)
int fwd;	/* TRUE if search is in forward direction */
int tok;	/* TRUE if it must match the whole token */
int again;	/* TRUE to repeat previous search */
{
	char tbuf[MAX_LEN_LINE];
	nodep p;
	extern char tokentext[];

	markPopBack(cursor);
	if (!again) {
		lastdir = fwd;
		getprline("Search for: ", tbuf, MAX_LEN_LINE);
		if (!isprint(tbuf[0]))
			return;	/* user aborted */
		strcpy(pttext, tbuf);
		strcat(tbuf, " ");
		setmac(0);
		scaninptr = tbuf;
		setlachar(0);
		pt.token_no = bscan();
		setmac(1);

		pt.ttext = pttext;
		if (pt.token_no == TOK_EOF || isleaftok(pt.token_no))
			pt.token_no = TOK_EOF;
	}
	else {
		if (pt.token_no == 0)
			error(ER(115,"No previous thing to search for."));
		fwd = lastdir;
		}
	
	printt5("search, fwd %d, tok %d, again %d, tnum %d, ttext '%s'\n",
		fwd, tok, again, pt.token_no, pt.ttext);

	/*
	 * Basic idea:
	 *  normally we just look for leaf tokens with text and
	 *  see if pt.ttext is part of (all of) the leaf text.
	 *  However, if we hit a nonterminal with the same type
	 *  as the token we typed in (using the node<=>token
	 *  correspondence, which we can since the numbers match)
	 *  we can consider that a match, thus searching for "for"
	 *  will stop on a "for" nonterminal or a string containing
	 *  "for".  This isn't perfect, it won't find semicolons,
	 *  for example, because they aren't represented in the tree.
	 *  It also won't find strings that have more than one token.
	 */
	
	p=cursor;
	do {
		/* move p to next position - note we move before 1st check */
		if (fwd)
			p = uc_nextpos(p);
		else
			p = uc_prevpos(p);
		printt1( "trying %x\n", p);
		
		if (pt.token_no != TOK_EOF) {
			printt2( "tokno compare: %d and %d\n", pt.token_no, ntype(p));
			if (pt.token_no == ntype(p)) {
				printt3("search found type %d, move from %x to %x\n",
					pt.token_no, cursor, p);
				cursor = p;
				return;
			}
		} else {	/* pt.token_no == TOK_EOF; */
			if (tok) {
				printt2("whole token compare: %s and %s\n",
					pt.ttext, ctext(p));
				if (strcmp(pt.ttext, ctext(p)) == 0) {
					printt3(
				 "search found string %s, move from %x to %x\n",
						pt.ttext, cursor, p);
					cursor = p;
					return;
				}
			} else {
				printt2("subtoken compare: %s and %s\n", pt.ttext, ctext(p));
				if (stindex(ctext(p), pt.ttext)) {
					printt3(
				 "search found substr %s, move from %x to %x\n",
						pt.ttext, cursor, p);
					cursor = p;
					return;
				}
			}
		}
	} while (!c_at_root(p));
	printt1("search failed at %x\n", p);
	ncerror(ER(116,"Can't find '%s'"), pt.ttext);
}

/*
 * Return the character text of leaf token p.  p must be a leaf token with
 * variable text, e.g. an ID, string, number, or comment.
 */
char *
ctext(p)
nodep p;
{
	register char *s;
	static char rbuf[4];

	switch (ntype(p)) {
	case N_ID:
		/* Symbol table reference */
		s = sym_name((symptr) kid1(p));
		break;
	case N_DECL_ID:
	case N_HIDECL_ID:
		/* Declaration symbol table reference */
		s = sym_name((symptr) p);
		break;
	case N_T_COMMENT:
	case N_CON_INT:
	case N_CON_REAL:
	case N_CON_CHAR:
		/* Tokens that are just stored as strings in the tree */
		s = str_kid(0,p);
		break;
	case N_CON_STRING:
		/* Tokens that are just stored as strings in the tree */
#ifdef PROC_EDITOR
		s = "";
#else
		s = str_kid(0,p);
#endif
	
		break;
	default:
		/* Not a leaf.  Get the generic name of token. */
		s = tokname(ntype(p));
		if (*s == '\'') {
			/*
			 * If it's punctuation, remove the quotes.
			 * Have to save it in a static buffer since
			 * we're modifying the string to return.
			 */
			strcpy(rbuf, s+1);
			s = strchr(rbuf, '\'');
			if (s)
				*s = '\0';
			s = rbuf;
		}
	}
	printt2( "ctext(%x) returns '%s'\n", (int)p, s);
	return s;
}

bits8
leaftoks[] = {
TOK_ID, 
TOK_CMNT,
TOK_INT,
TOK_NUMB,
TOK_CHAR,
TOK_STRING,
0
};

/*
 * Return TRUE only if the token number tn is one of a leaf (e.g.
 * one that has text stored with it.)
 */
isleaftok(tn)
int tn;		/* token number */
{
	bits8 t;

	t = !!(int)strchr( leaftoks, tn );

	printt2("isleaf(%s) returns %d\n", tokname(tn), t != NULL);
	return t;
}

#else

search(){}
#endif HAS_SEARCH

#endif SEARCH
