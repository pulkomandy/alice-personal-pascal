

/*
 * The one line editor.
 *
 * The editor is used to edit single tokens, such as identifiers, quoted
 * strings, and comments.  It can also be used for special purposes
 * such as editing expressions textually.  Also, the one line editor
 * is used when typing in such tokens and expressions initially.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "scan.h"
#include "token.h"
#include "ole.h"
#include "flags.h"
#include "keys.h"
#include "alctype.h"

#ifdef DB_OLEDIT
#define PRINTT
#endif
#include "printt.h"

/* Global copies of locals/parms for availability to routines here */
char *edtext;
buf_el *prectext, *foltext;
char *termchars;
int sline;
int somode = 1;
WINDOW *edwin;
static int flags;
int ret_cmd;	/* token no. read by scanner, exported to topcmd */
int folchar;	/* printing char following the expression */
nodep oletree;	/* result of parsed expression, exported to topcmd */
int gcmd;	/* if 1, clear cmd after call to OLE_Accept */
int ptlen;	/* length of pred text */
int ftlen;	/* length of following text */
int edtxtlen;	/* length of edtext */
char *olcursor;
char tbuf[MAX_LEN_LINE];

extern int nptok;	/* number of tokens pushed */
extern char *ScanPtr;
extern int domalloc;
extern int NonExprToken;
extern int line_borders[];

/*
 * Edit a single line of text.  The single line may be part of a larger
 * line - there is a left part and a right part.  The combined line may
 * be longer than 80 characters, so it may take up more than one line
 * on the screen.  The final text is returned as the function value, 
 * which is a pointer to a static buffer.
 */
char *
oledit(aedwin, asline, aflags, aprectext, oedtext, afoltext, 
	atermchars, retbuf, choffset)
WINDOW *aedwin;	/* screen window object being edited is on */
int asline;	/* starting line number in window */
int aflags;	/* flags to cause special handling */
buf_el *aprectext;/* text on current line preceding area to be edited */
buf_el *oedtext;/* (original) text on current line in area to be edited */
buf_el *afoltext;/* text on current line following area to be edited */
char *atermchars;/* Characters other than NL to terminate, or NULL for none */
char *retbuf;	/* place to put result (also returned as fn value) */
int choffset;	/* (relative to whole line) */
{
	char aedtext[MAX_LEN_LINE];	/* text being edited */
	register int cmd;		/* char of cmd being done */
	int col;			/* column on screen */
	register char *p;
	register char *q;
	int c;				/* scratch char */
	int oldsomode = somode;		/* save/restore somode */
	int savex;			/* initial cursor position */
	int savey;			/* initial cursor position */
	int newx;			/* curs pos at beginning of edtext */
	int newy;			/* curs pos at beginning of edtext */
	int needs_update = FALSE;	/* screen update needed on exit */
	struct scr_line *ole_ln;
	int n_cols, n_lines;

	buf_el *pr_ptr;			/* pointer into prec text */

	extern buf_el line_buffer[];

	stackcheck();

	printt0( "Start of OLEdit()\n" );
	/* copy to globals so available to routines */
	prectext = aprectext; edtext = aedtext; foltext = afoltext;
	sline = asline; edwin = aedwin; termchars = atermchars;

	/* get lengths of preceding and following text */
	for (ptlen = 0; prectext[ptlen]; ptlen++)
		;
	for (ftlen = 0; foltext[ftlen]; ftlen++)
		;

	printt2( "length of prec: %d, fol: %d\n", ptlen, ftlen );
	flags = aflags;
	oletree = NIL;

	if (flags&OLE_NOHL) somode = 0;

	/* Copy the text passed to the routine to the working buffer */
	becpy(edtext, oedtext);

	/* Set the cursor to the beginning */
	olcursor = edtext;

	/*
	 * Position cursor within existing string properly by calling displ
	 * and comparing physical cursor positions.
	 */
	getyx(edwin, savey, savex);
	printt4("oledit: savey %d, srch_row %d, strlen %d, choffset %d\n", 
		savey, srch_row, strlen(edtext), choffset);


	edtxtlen = strlen(edtext);

	if( !(flags & OLE_CMD) ) {

		ole_ln = curr_window->w_lines;
		olcursor = edtext;
		/* Start line is first line of line */
		sline = sline - ole_ln[sline].sub_line;

	/* If we are not on the same line as the beginning of the cursor
	 * then start at the beginning
	 */
		ole_update(); /*Call Update with cursor at beginning to get line
			 * starts calculated properly
			 */
		printt1( "Initial offset = %d\n", choffset );
		printt1( "sline = %d\n", sline );
		/* number of lines from the start of the actual line to where
	 	 * the cursor is */
		n_lines = (choffset / edwin->_maxx) - sline;
		/* Get which line of the 'line' we are on */
		n_cols = (choffset % edwin->_maxx);

		printt2( "n_lines = %d, n_cols = %d\n", n_lines, n_cols );

		/* Make columns number of characters from actual beginning of line */
		n_cols -= ole_indent(n_lines,sline);

		printt1("Cursor is %d chars into text on that line\n",n_cols);

		choffset = line_borders[n_lines] + n_cols;

		printt1("Offset into string = %d\n", choffset );	
		/* Now subtract off the length of the preceding text */

		choffset -= ptlen;
		printt1("Offset into edit text = %d\n", choffset );

		if (choffset > edtxtlen)
			choffset = strlen(edtext);

		if (choffset < 0) choffset = 0;

		olcursor += choffset;	
	}

	printt1("choffset winds up %d\n", choffset);

	
	/* make it relative to edited part */
	for (;;) {	/* main one line editor loop */
		extern int inbuf_count;
		if( strlen(edtext) != edtxtlen ) {
			printt2( "serious error: ed %d chars instead of %d\n",
				strlen(edtext), edtxtlen );
			edtxtlen = strlen(edtext);
		}

		/* Handle Type-Ahead */
		/* do not display if there is typeahead coming */
		if( inbuf_count > KBUF_SIZE / 2 || ! get_lookahead()  )
			ole_update(); /* Update the screen */
		 else
			needs_update = TRUE;


		printt3( "ole loop, edtext '%s', len = %d, cursor %d\n",
			edtext, edtxtlen, olcursor-edtext);

		setmac(2);
		cmd = readkey();
		setmac(1);

		ret_cmd = gcmd = 0;

		/* Should we pass this key back to the main loop ? */
		if (OLE_Accept(flags, termchars, cmd)) {
			printt0( "OLE_Accepted\n");
			OLE_Done(flags, TRUE);
			/* Should we pass this character back, or should
			 * we exit, but ignore it */
			setlachar(gcmd ? 0 : cmd);
			break;
		}

		/* Process the character (i.e. insert it, or whatever) */
		docmd(cmd);

		/* See if we should now exit, based on the edit text */
		if (OLE_Done(flags, FALSE)) {
			printt1( "OLE_Done-ed edtext = %s\n", edtext );

			/* If ret_cmd has not been set, set it to
			 * the last key that was entered
			 */
			if (ret_cmd == 0)
				ret_cmd = cmd;
			break;
		}
	}

	somode = oldsomode;

	for ( p=retbuf, q=edtext; *q; ) *p++ = *q++;
	*p = 0;
	return retbuf;
}

/*
 * one line accept - return nonzero if we should terminate, based on
 * the flags and termchars given by our caller and what is at the
 * end of the buffer.  Called before keystroke interpreted.
 *
 * In the current implementation, we return only if the character is
 * nonprinting and not an oledit command.
 */
OLE_Accept(fl, tc, xcmd)
int fl;		/* flags to decide */
reg char *tc;	/* terminating characters */
int xcmd;	/* last key the user typed */
{
	register int cmd;
	extern char *scaninptr;

	cmd = xcmd;
	printt4( "OLE_Accept, fl %o, tc '%s', cmd '%c' (%o)\n",
		fl, tc ? tc : "", cmd, cmd);
	if (tc && strchr(tc, cmd)) {
		/* Throw this character away */
		gcmd = TRUE;
		return TRUE;
	}

	/* If arrow key and still within token, process locally */
	if (cmd == KEY_LEFT ) return olcursor <= edtext;
	if (cmd == KEY_RIGHT) return *olcursor == 0;
#ifdef HAS_POINTING
	if ( cmd == KEY_COORD && gptr_yp( edwin, FALSE) == sline ) {
		int xdif = (olcursor - edtext) + gptr_xp(edwin, TRUE);
		if( xdif < 0 || xdif > strlen(edtext) )
			return TRUE;
		}
#endif

	/* Any other key understood by OLE keeps you in */
	if (isolechar(cmd))
		return FALSE;
	if( !isascii(cmd) )
		return TRUE;
	if (isprint(cmd))
		return FALSE;
	
	return TRUE;
}

/*
 * one line done - return nonzero if we should terminate, based on
 * the flags and termchars given by our caller and what is at the
 * end of the buffer.  Called after keystroke interpreted.
 */
OLE_Done(fl, addspace)
int fl;		/* flags to decide */
int addspace;
{
	struct token ta[80];
	register int ntok;	/* number of tokens returned by scanner */
	extern char tokentext[];
	char *ptr;
	char c;

	/* We only want to tokenize the input if in TOP mode,
	 * if we are entering an expression for the first time
	 * (i.e. not editing it)
	 */ 

	if( ( fl & OLE_TOP ) == 0 ) {
		printt0( "OLE_Done() - not in TOP mode\n" );
		return FALSE;
	}

	if( ( fl & OLE_STR ) != 0 ) {
		printt0( "OLE_Done - string, return FALSE\n" );
		return FALSE;
	}

	/* if we don't have any text, return */
	if( edtext[0] == 0 ) return FALSE;

	if( addspace ) {
		strcat( edtext, " " );
	}

	/* Scan the text, breaking it up into tokens */
	domalloc = FALSE;
	expscan(edtext, ta, tbuf);

	/* If we added a space remove it */
	if( addspace ) { 
		edtext[strlen(edtext)-1] = 0;
	}

	printt3( "text after expscan (%s) (%s) @ %x\n", edtext, tbuf, tbuf );

	for (ntok=0; ta[ntok].token_no != TOK_EOF; ntok++) ;
	printt3( "OLE_Done finds %d tokens %d %d\n",
		ntok, ta[0].token_no, ta[1].token_no);

	/* If in TOP mode, and just finished punct token, take it now. */
	/* (only if we are not entering an expression) */

	if( ((fl & OLE_EXP) == 0) && ntok > 0) {

		if( !addspace ) {
			strcat(edtext," ");
			domalloc = FALSE;
			expscan(edtext,ta,tbuf);
			edtext[strlen(edtext)-1] = 0;
			for (ntok=0; ta[ntok].token_no != TOK_EOF; ntok++) ;
		}
	
		/* We are not on an expression, so terminate on
		 * any token
	 	 */	
		printt1( "Pushing %d tokens onto the stack\n", ntok );
		while( ntok > 0 ) {
			pushtoken( &ta[ntok-1] );
			ntok--;
		}
		ret_cmd = TOK_NOP;

		/* Tell oledit to quit */
		return TRUE;
	}

	/*
	 * If in EXP mode, we terminate on any token that can't be in
	 * an expression.
	 */
	if (fl & OLE_EXP) {
		int c;	/* character offset in edtext of end of exp */
		printt0( "OLE_Done - expression\n" );

#ifdef DB_OLEDIT
		if (tracing) {
			int i;

			fprintf(trace, "expscan returns ");
			for (i=0; i<ntok; i++)
				fprintf(trace, " %s '%s',",
					tokname(ta[i].token_no), ta[i].ttext);
			fprintf(trace, " total %d tokens\n", ntok);
		}
#endif DB_OLEDIT
		/* 2nd to last token is.  This can happen if last token is
		 * a 1 character token and 2nd to last is multi char.
		 */
		if (ntok > 1 && isterm(ntok-2, ta)) {
			printt0( "2nd last token terminates expr\n" );
			pushtoken(&ta[ntok-1]);
			pushtoken(&ta[ntok-2]);
			ret_cmd = TOK_NOP;
			c = ta[ntok-2].charoff;

			/* We have pushed the terminated tokens,
			 * wipe out the text of those tokens in
			 * the expression (so that they aren't in the
			 * expression when it is finally parsed)
			 */ 
			edtext[c++] = ' ';
			edtext[c] = 0;
			return TRUE;
		} else

		/* Last token is a non-expression token */
		if (ntok > 0 && isterm(ntok-1, ta)) {
			printt0( "Last token terminates expr\n" );
			pushtoken(&ta[ntok-1]);
			ret_cmd = TOK_NOP;
			c = ta[ntok-1].charoff;

			/* Again, wipe out text of last token */
			edtext[c++] = ' ';
			edtext[c] = 0;
		} else
			return FALSE;

		/* We have gotten a non-expression token while
		 * expecting a token
		 * If the first token is a non-expression token
		 * then don't even try to parse it as a possible 
		 * expression, and leave the token on the stack
		 */
		if( isexptok( ta[0].token_no ) == 0 ) {
			NonExprToken = TRUE;
		}
		return TRUE;
	}
	return FALSE;
}

#ifndef A_UNDERLINE
#define wattron(win, at) wstandout(win)
#define wattroff(win, at) wstandend(win)
#endif

ole_update()
{

/* This is really awful, but...  If the window that the one line editor
 * was called on is the editor window, use the new ole_outln routine
 * to update the window.  If it was not, then it was probably called on the
 * command line window.  In this case, call the old display routine.
 */

/*
 * For GEM, commands are given in a dialog box, so none of this is needed
 */
#ifndef GEM
	if( flags & OLE_CMD )
		displ();
	else
#endif
		ole_outln();


}

#ifndef GEM
/* Update the display */
displ()
{
	register char *p;
	int crow=0, ccol=0;
	char *startp;			/* where display of line begins */
	buf_el *ptstart;		/* prec text start */
	int y, x;
	int howwide = edwin->_maxx-5;   /* max number of chars per line */
	int howfar = olcursor - edtext; /* number of chars into edit text */
	extern buf_el Attr_Map[];

	printt2( "max x = %d, sline = %d\n", edwin->_maxx, sline );	
	/* MAKE THIS ALL CONDITIONAL on all-printable type-ahead */
	startp = edtext;
	ptstart = prectext;
	wmove(edwin, sline, 0);   /* position the cursor at beginning of 
				   * starting line 
				   */

	/* If the previous text + amount to display up to cursor is greater
         * than the screen width, then display the last howwide chars
         * of the edited line
         * ptlen + howfar = length up to cursor
         */

	if( ptlen + howfar > howwide ) {
                /* Check to see if we are to display more than we can */
		if( howfar > howwide ) {
			/* Display as many chars as possible */
			startp = olcursor - howwide;
			ptstart = prectext + ptlen;
			}
		 else
			/* Otherwise make cursor be on far right */
			ptstart = prectext + ptlen + howfar - howwide;
		}

	/* kludge, but this whole thing is */
	if( edwin == curWindows[CMDWIN] )
		wattron( edwin, _STANDOUT );
	wattrstr(edwin, ptstart);

	/* Turn on the OLEDIT colour */
	if( somode ) {
#ifdef COLPOS
		wattroff( edwin, AC_WHITE );
#endif
		wattron(edwin, Attr_Map[COL_OLEDIT]);
		}

	getyx( edwin, crow, ccol );
	printt2( "after prec: row = %d, col = %d\n", crow, ccol );
	/* Find out where the cursor will be on the new text */
	ccol += olcursor - startp;
	printt1( "new ccol: %d\n", ccol );
 	waddstr( edwin, startp );

	if (somode) wattroff(edwin, Attr_Map[COL_OLEDIT]);
	
	/* Put the text following that which is being edited onto the screen */
	wattrstr(edwin, foltext);

	/* Find out where the cursor is */
	getyx(edwin, y, x);
	printt2( "after: row = %d, col = %d\n", y, x );
	/* do something about folding, and folding on last line */
	/* If we haven't gone up to the end of the line, clear to eol */
	if (x < edwin->_maxx)
		wclrtoeol(edwin);

	/* Position the cursor at the appropriate spot */
	wmove(edwin, crow, ccol);			/* restore cursor */
	wrefresh(edwin);
}

#endif GEM

ole_outln()
{

register buf_el *src_cpy;
register buf_el *dst_cpy;
struct scr_line *ole_ln;
unsigned char *ed_cpy;
int i;

	extern buf_el Attr_Map[];
	extern buf_el *lb_cursor;
	extern buf_el line_buffer[];

redraw_it:
	ole_ln = curr_window->w_lines;   /* pointer to list of lines */

	dst_cpy = line_buffer;
	/* Copy previous text */
	src_cpy = prectext;
	while( *src_cpy )
		*dst_cpy++ = *src_cpy++;

	ed_cpy = edtext;
	/* Copy the edited text */
	while( *ed_cpy ) 	/* copy the edit text */
		*dst_cpy++ = *ed_cpy++ | Attr_Map[ COL_OLEDIT ];

	src_cpy = foltext;
	/* Copy the following text */
	while( *src_cpy )	/* copy the following text */
		*dst_cpy++ = *src_cpy++;

	*dst_cpy = 0;		/* store a null terminator */
	lb_cursor = dst_cpy;	/* set the line buffer cursor there */

	/* Previous text len + offset */
	srch_real = ptlen + (olcursor - edtext);
				/* Calculate the rel. cursor position */
	printt2( "Cursor is %d chars into string, len = %d\n", srch_real,
			ptlen+edtxtlen+ftlen );
	printt1( "line_buffer is %d long\n", lb_cursor - line_buffer );
	if( (olcursor - edtext) > edtxtlen ) {
		printt0( "serious error: cursor past EOL in ole_outln\n" );
		srch_real = ptlen;
	}
	cur_line = sline; /* Set which line to display on */
	if( out_line( &ole_ln[ sline ], curr_window, OLEOUT,
			 curr_window->w_height ) ) {
		printt0( "scrolled in ole_outln... redisplay\n" );
		goto redraw_it;
	}
	wmove( edwin, srch_row, srch_real );	
	wrefresh( edwin );
}

/* do the command. */
docmd(cmd)
int cmd;
{
	register int n;			/* scratch integer */

	printt1( "docmd(%o)\n", cmd);
	/*
	 * Treat backspace like ctrl H.  This should just be another case
	 * label below, but KEY_BACKSPACE is CNTL('H') on some implementations
	 * (in particular on QNX) and we can't have two identical labels
	 * on a case.
	 */
	if (cmd == KEY_BACKSPACE)
		cmd = CNTL('H');

	switch (cmd) {
	/* cursor motion commands */

	case AL_IEDIT:
		/* No-op command, just to get us into the editor. */
		break;

	/* forward one character */
	case KEY_RIGHT:
		if (*olcursor)
			olcursor++;
		break;

	/* back one character */
	case KEY_LEFT:
		if (olcursor > edtext)
			olcursor--;
		break;

#ifdef HAS_POINTING:
	case KEY_COORD:
		/* check for errors here? */
		olcursor += gptr_xp(edwin, TRUE);
		break;

#endif

	/* self inserting character */
	default:
		inschar(cmd);
		break;

	/* delete char forward */
	case CNTL('D'):
#ifdef KEY_DC
	case KEY_DC:
#endif
		delchars(1);
		break;
	
	/* delete char backward */
	case CNTL('H'):
	case 0177:	/* DEL */
		if (olcursor <= edtext)
			return;		/* already at left margin */
		olcursor--;
		delchars(1);
		break;
	
	/* delete backward one word */
	case CNTL('W'):
		n = 0;
		/* back up over white space */
		while (olcursor>edtext && (*olcursor==' ' || *olcursor=='\t'))
			n++, olcursor--;
		/* back up over the word */
		while (olcursor > edtext && *olcursor!=' ' && *olcursor!='\t')
			n++, olcursor--;
		delchars(n);
		break;
	
	case AL_ID:
		complete();
		break;
	}
}

/* Delete n characters after the cursor */
delchars(n)
int n;
{
	register char *p, *q;

	if (*olcursor == 0)
		return;		/* off the end */

	edtxtlen -= n;

	for (p=olcursor,q=p+n; *q; ) {
		*p++ = *q++;
		}
	*p++ = 0;
}

/* Insert the character ch before the cursor */
inschar(ch)
char ch;
{
	register char *p, *q;

	/* If combined length of preceding text, edit text and following text
         * exceeds the length of the line_buffer, give an error message
	 */

	if( (ptlen + edtxtlen + ftlen) >= (MAX_LEN_LINE-5) ) {
		printt3( "ptlen = %d, edlen = %d, ftlen = %d\n",
			ptlen, edtxtlen, ftlen );
		printt2( "line len = %d, curlen = %d\n", MAX_LEN_LINE,
			ptlen + edtxtlen + ftlen );
		warning( ER(225,"Can't insert any more text, line too long.") );
		return;
	}

	for (p=olcursor; *p; p++)
		;
	q = p+1;
	while (p >= olcursor)
		*q-- = *p--;
	*olcursor++ = ch;
	edtxtlen++;
	printt2( "inschar(%o), olcursor %d\n", ch, olcursor-edtext);
}

int
olechars[] =
{
	KEY_RIGHT,	/* cursor right */
	KEY_LEFT,	/* cursor left */
	CNTL('D'),	/* delete char */
#ifdef KEY_DC
	KEY_DC,	/* delete char */
#endif
	CNTL('H'),	/* backspace (delete char to left) */
	0177,	/* DEL (delete char to left) */
	KEY_BACKSPACE, /* delete char to left */
	CNTL('W'), 	/* delete word to left */
	AL_ID, /* complete the partly typed id */
	0
};

/*
 * Return TRUE if the character is handled by the one line editor.
 */
isolechar(cmd)
int cmd;
{
	register int i;
	register int gch;
	for( i = 0; gch = olechars[i]; i++ )
		if( cmd == gch )
			return TRUE;
	return FALSE;
}

/* Return TRUE if ta[ntok] terminates the expression. */
isterm(ntok, ta)
int ntok;
struct token ta[];
{
	int stn;	/* save token number */
	int et;		/* value from isexptok */
	register int r;		/* return value from expparse */

	stn = ta[ntok].token_no;
	et = isexptok(stn);
	printt3( "isterm ntok %d, stn %s, et %d\n", ntok, tokname(stn), et);
	/* 0 means it can't possibly be in the expression, so terminate */
	if (et == 0)
		goto rettrue;
	/* 3 means it most certainly could be in an expression */
	if (et == 3)
		return FALSE;
	/* 1 means it could be in an expression, but can't be the 1st token */
	if (et == 1)
		if (ntok == 0)
			goto rettrue;
		else
			return FALSE;

	if( et == 4 && ntok == 0 ) 
			goto rettrue;

	/* 2 means it could be, but terminate if parens match to the left */
	ta[ntok].token_no = TOK_EOF;
	r = t_expparse(ta, (nodep *)NULL);
	ta[ntok].token_no = stn;
	printt1( "isterm calls expparse and gets %d\n", r);
	if (r)
		goto rettrue;
	else
		return FALSE;
rettrue:
	return TRUE;
}

/* Push the given token onto the keyboard input.
 * This is not really a general purpose routine.
 */
pushtoken(t)
struct token *t;
{
	extern struct token ptok[];

	printt2("pushtoken tn %s, text '%s'\n", 
			tokname(t->token_no), Safep(t->ttext));
	/* struct assignment */
	blk_move( &(ptok[nptok++]), t, sizeof(struct token) );
}

/*
 * Before returning a token, call the scanner to ensure it will be
 * passed to in_token.
 */
#ifdef notdef
callscan( text )
char *text;
{
	register int tn;		/* token number from scanner */
	char tbuf[MAX_LEN_LINE];
	int tmplac;	/* temporary copy of lookaheadchar */
	extern char tokentext[];

	printt2( "callscan text '%s', lachar %o\n", text, lookaheadchar);
	if (text[0] == 0)
		return;

	strcpy(tbuf, text);
	strcat(tbuf, " ");	/* force token to be terminated */
	ScanPtr = tbuf;
	tn = bscan();
	printt1( "CallScan('%s')\n", Safep( text ) );
	printt2( "CallScan() - token %d, tokentext '%s'\n",
			tn, Safep( tokentext ) );
	printt3( "callscan gets %s, tmplac %o, lachar %o\n",
		tokname(tn), tmplac, lookaheadchar);

	if (tn == TOK_EOF || tn == TOK_ILLCH)
		return;

	ret_cmd = tn;
	printt1( "callscan returns ret_cmd %d\n", ret_cmd);
	return;
}
#endif notdef
/*
 * Table of expresion values.  Any token not listed cannot appear in
 * an expression.  Possible values:
 * 1: can appear in an expression, but not at the beginning.
 * 2: can appear in an exp, but only if parens don't match before it.
 * 3: can appear in an expression, even at the beginning.
 * 4: can appear in an expression, but not at the beginning, 
 *    and only if parens don't match before it (1 & 2)
 */
bits8
exptoks[] = {
TOK_EOF, 1,
TOK_AND, 1,
TOK_DIV, 1,
#ifdef TURBO
TOK_XOR, 1,
TOK_SHL, 1,
TOK_SHR, 1,
#endif
TOK_DOTDOT, 1,
TOK_ID, 3,
TOK_IN, 1,
TOK_INT, 3,
TOK_MOD, 1,
TOK_NIL, 3,
TOK_NOT, 3,
TOK_NUMB, 3,
TOK_OR, 1,
TOK_STRING, 3,
TOK_LPAREN, 3,
TOK_RPAREN, 2,
TOK_COMMA, 4,		/* Changed from 2 */
TOK_COLON, 1,
TOK_STAR, 1,
TOK_PLUS, 1,
TOK_SLASH, 1,
TOK_MINUS, 3,
TOK_LBRACKET, 3,	/* Changed from 1 */
TOK_RBRACKET, 2,
TOK_EQ, 1,
TOK_LT, 1,
TOK_LE, 1,
TOK_NE, 1,
TOK_GT, 1,
TOK_GE, 1,
TOK_UPARROW, 3,
TOK_CHAR, 3,
TOK_DOT, 1,
0, 0,
};

/*
 * Return nonzero if the token is one that can appear in an expression.
 */
isexptok(tok)
int tok;
{
	register bits8 *t;

	t = table_lookup(exptoks, tok, 2);
	printt2( "isexptok(%s) returns %d\n", tokname(tok), t != NULL);
	if (t)
		return *t;
	return FALSE;
}

/* Like strcpy, but copies from a buf_el string to a char string */
becpy(dest, src)
register char *dest;
register buf_el *src;
{
	printt0("becpy: ");
	while (*dest++ = *src++) {
		printt1( "<%o>", src[-1]);
	}
	printt0("\n");
}

/* Like strcpy, but copies from a char string to a buf_el string
 * ICK - the arguments are backwards from strcpy.
 */
#ifndef GEM
charToBE(charp, bep, xattr)
register char	*charp;
buf_el	*bep;
short xattr;
{
	register short attr = xattr;
	while( *charp )
		*bep++ = *charp++ | attr;
	*bep = 0;
}
#endif

/* Complete a partially typed identifier */
complete()
{
	register char *beg;
	register int len;
	register char *res;
	char *sym_complete();

	for (beg=olcursor-1; beg >= edtext && (isalnum(*beg)||*beg=='_'); beg--)
		;
	beg++;	/* it went back one too far */
	printt3("complete, cursor %x, id '%s', len %d\n", (int)cursor, beg, len);
	len = olcursor - beg;
	res = sym_complete(cursor, beg, len);
	/* using help might have redisplayed losing edwin.  KLUDGE */
	edwin = curWindows[MAINWIN];
	printt1("complete gets '%s' returned\n", res);
	if (res == NULL)
		return;
	res += len;
	while (*res)
		docmd(*res++);
}

/* Calculate the indent for a specific line */
ole_indent( off, line )
int off,line;
{
	struct scr_line *ole_ln;
	int i;
	
	ole_ln = curr_window->w_lines;

	i = calc_main_indent( ole_ln[line].ind_level, edwin->_maxx);

	if( off ) {
		return( i + (TABSIZE/2) );
	} else {
		return( i );
	}
}

