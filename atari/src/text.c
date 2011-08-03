
/*
 * Textual changes.  These routines are called when the user wants to
 * make a change textually, often to characters within a token (identifiers,
 * comments, strings) or within an expression.  These routines all call the
 * one line editor (oledit) under various circumstances.
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "scan.h"
#include "token.h"
#include "ole.h"
#include "class.h"
#include "keys.h"
#include "alctype.h"

#ifdef GEM
#include "gemcmd.h"
#endif

#ifdef DB_TEXT
# define PRINTT
#endif
#include "printt.h"

char	EditableClasses[] = {
	C_EXP,
	C_VAR,
	C_DECL_ID,
	C_HIDECL_ID,
	C_COMMENT,
	C_PNAME,
	C_FLD_NAME,
	C_PROC_ID,
	C_FUN_ID,
	C_CONSTANT,
	0
};

/*
 * AL_IEDIT is a special kludge.  When the user presses the EDIT key to go
 * into the one line editor, the key is mapped into AL_IEDIT.  This key
 * is a no-op to the one line editor.
 *
 * The basic idea is that when the cursor is on something that can be edited
 * (especially a stub) and you type a key that is known to be understood by
 * oledit, the routines drop you into oledit and let it handle the key.  They
 * leave you in oledit for more editing.  Such keys include printing text,
 * delete character, and left and right arrow keys, under certain
 * circumstances.  IEDIT gets you dropped into the editor, then oledit treats
 * it as a no-op, with the net effect that hitting EDIT has gotten you into
 * the one line editor.
 *
 * As the code progressed, the code that put you into the editor on certain
 * keys was disabled, leaving only the EDIT key to do this.  The macro
 * below (startedit) decides whether to put you into oledit given a particular
 * keystroke you've just typed.  However, IEDIT still works the same.
 */

static char null_string[] = { 0 };

extern char *oledit();
extern buf_el *OLE_CurText;
extern buf_el *OLE_PrcText;
extern buf_el *OLE_FolText;

/* 
 * These variables are set up by three routines (save_indent, save_ptext,
 * save_ctext, save_ftext) called from out_line and treeprint to remember the
 * text of the line with the cursor on it, in three pieces: before the
 * cursor (ptext=preceding text), in the cursor (ctext), after the
 * cursor (ftext=following text).
 */
/*static*/ int savestate = 0;		/* 0=init, 1=bef curs, 2=after curs */
/*static*/ buf_el *savecp;		/* remember previous value of cp */
/*static*/ buf_el *savep;		/* remember where cursor is/was */
buf_el *sptext, *sctext, *sftext;	/* save pointers to before & after */
int saveindent;				/* # chars to indent this line */
int nptok;				/* number of tokens queued */
struct token ptok[5];			/* push token stack */
int savecol;				/* col cursor was in at last IEDIT */

buf_el BENULL[] = {0, 0};		/* "" in buf_el */

int	NonExprToken;

extern inchar tokentext[];
extern int ret_cmd;
extern nodep  oletree;

/*
 * This routine is called from the main loop.  It gets a top level command
 * (eventually) while offering the user the one line editor commands.
 */

int
topcmd( row, scol, ncols, ocursnode )
int row;	/* row on screen */
int scol;	/* starting column */
int ncols;	/* number of columns */
curspos ocursnode;	/* pointer to cursor */
{
	int i;
	extern int got_colon;

	i = ntopcmd( row, scol, ncols, ocursnode );

	if( (i == TOK_EQ) && got_colon ) {
		printt0("Colon followed by equal, return nop\n" );
		i = TOK_NOP;
	}
	
	got_colon = FALSE;
	return i;
}

static int
ntopcmd( row, scol, ncols, ocursnode )
int row;	/* row on screen */
int scol;	/* starting column */
int ncols;	/* number of columns */
reg curspos ocursnode;	/* pointer to cursor */
{
	register char *rv;
	int cmd, r, tm;
	register int t = ntype(ocursnode);
	int clexp;				/* class expected */
	int oleflags = OLE_TOP;			/* flags to pass to oledit */
	char *termch = NULL;			/* terminating chars */
	int strflag = 0;	/* 0=none, 1=char, 2=string, 3=comment */
	int blok = FALSE;	/* leading blanks are not ignored */
	int choffset = 0;	/* cursor character offset to pass to oledit */
	int chopped = 0;	/*true if current token chopped (e.g. "var") */
	char rb[MAX_LEN_LINE];	/* place for result to go */

	extern int ret_cmd;
	extern nodep  oletree;

	printt3("topcmd, cursnode %x, type %d (%s)\n",
		 (int)ocursnode, t, NodeName(t));

	printt3("row %d, scol %d, ncols %d\n", row, scol, ncols);

	/* If any tokens have been pushed onto the stack, return it
	 */

	if (nptok > 0) 
		return pop_token();
	
	/*
	 * Check to see if we're on a string or comment node.
	 * If so, don't ignore leading blanks.
	 */

	/* Get the class of the node */
	clexp = ex_class(cursnode);

	/* Ignore leading spaces on non-comments */
	if (iscomclass(clexp)) blok = TRUE;

	printt4("clexp %d (%s), blok %d, type %d\n",
		clexp, Class_Names[clexp], blok, ntype(ocursnode));

	/* We are on an expression stub and are entering an expression
	 * for the first time
	 */
	if ((clexp == C_EXP) && (ntype(cursnode) == N_STUB) ){
		oleflags |= OLE_EXP;
		printt1("oleflags=%o\n", oleflags);
	}

	/*
	 * Get the first keystroke before going into oledit.
	 * This is done (1) in case of a command, (2) so the user
	 * will see the regular (not oledit) display - the oledit
	 * display messes with the highlighting on the current line.
	 */

	 /* Make conditional on type-ahead of any sort */
	wmove(curr_window->w_desc, row, scol); 
	wrefresh(curr_window->w_desc);

	/* If we are skipping spaces... */
	if( blok == FALSE ) do {
		cmd = readkey();
		} while ( (cmd==KEY_BACKSPACE || cmd==' ' || 
				cmd==AL_NOP || cmd==0));
	else
		cmd = readkey();

#ifdef GEM
	if( is_gem(cmd) ) {
		/* A Gem Command, stuff the appropriate string into
		 * tokentext
		 */
		GemCmd( cmd );
		return TOK_CMD;
	}
#endif

	/* clear_em used to be here */

	printt2( "text key '%c' (%o)\n", cmd, cmd);

	switch (cmd) {

	/* Got a CTRL-X (or whatever), get the name of the command */
	case AL_ICOMMAND:
		getbotline();
		return TOK_CMD;

	case '{':	/* } for vi's % paren matching command */
		if (t != N_STUB)
			break;
		strcpy(tokentext, null_string );/* start with empty comment */
		setlachar(' ');
		return TOK_CMNT;
	case KEY_UP:
	case KEY_DOWN:
	case KEY_LEFT:
	case KEY_RIGHT:
#ifdef HAS_POINTING
	case KEY_COORD:
#endif
		new_phys(cmd);
		return TOK_NOP;
	
	case AL_ID:
		sym_menu(cursor);
		return TOK_NOP;
	case '\n':
	case '\r':
		tokentext[0] = cmd;
		tokentext[1] = 0;
		return TOK_CR;
	}

	/* Special handling of printing chars on certain tree nodes */

	if (isprint(cmd)) {
		if (t == N_STUB) {
			/* comment/string stub */
			r = int_kid(0,cursor);
			if (iscomclass(r) ){
				strcpy(tokentext, null_string);	
				/* empty comment/str */
				/* Force oledit next time around */
				setlachar(cmd);
				return TOK_CMNT;
			}
		}
		printt2("comcheck: cmd %o, t %d\n", cmd, t);
		if ( iscomtype(t) && (strlen(getESString(str_kid(0,cursor)))
						== 0 )) {
			oleflags = OLE_STR;
			if (t == N_T_COMMENT) {
				strflag = 3;
				/* { */
				termch = "}";
			}
		}
	}
	/*
	 * If we got one of the terminating characters, just have
	 * it call the scanner directly.  This also applies if we're
	 * on a subtree that can't be edited textually, like "for", or
	 * if we're on an editable non-stub subtree but the user didn't
	 * hit the EDIT key.
	 *
	 * READ THIS: This if statement has been the source of most of
	 * the recent bugs in text and the one line editor.  It has been
	 * fiddled with a lot.  The basic idea is to decide whether the
	 * user is editing existing text, typing in new text, or typing
	 * in a command.  If typing in a command, we go into the if (setting
	 * chopped) and treat the result as a command, not as something
	 * to be inserted here.  (The command typed, e.g. "var", will be
	 * displayed INSTEAD OF the current token, until it's completely
	 * typed, mainly because it's easier to code this way, although
	 * human factors arguments probably would like this too.)
	 *
	 * There are three primary cases to consider: an expression,
	 * an identifier, and a string.  These can be detected by the
	 * setting of oleflags.
	 */

	/* Chop up the text */
	OLE_ChopUpText( cursnode, !iscomclass(clexp) );

	/* Since we are editing stuff here for the first time */

	*OLE_CurText= 0;	/* Don't edit existing text */

	/* Sanity check - don't know what to do with what user typed. */
	if( cmd > 127 || !isprint(cmd)){
		printt1( "illegal text key (%o)\n", cmd);
		beep();
		return TOK_NOP;
	}

	/*
	 * Actually call the one line editor.  We already have the first
	 * character, in cmd.  We've decided there may be more, so push
	 * it back onto the input and invoke oledit.
	 */
	printt1( "start text with key (%o)\n", cmd);

	setlachar(cmd);	/* push it back so we get it next time */
			/* (in the editor) */
	ret_cmd = TOK_NOP;

	OLE_InitEdit( cursnode );

	NonExprToken = FALSE;

	OLE_Edit( cursnode, OLE_TOP | oleflags, termch, row, scol );

	/* If we got a command token at the beginning of typing in an
	 * expression, just return it.
	 */
	if( NonExprToken )
		return pop_token();
	
	/* Consider sticking the resulting new string back in the tree */

	/* If we are entering an expression */
	if( oleflags & OLE_EXP ) {

		/* add in the typed in expression */
		OLE_Replace( cursnode );

		/* If we terminated because of a bad token, return it */
		/* It is already pushed onto the stack */
		return pop_token();
	}

	/* Handle the entering of new strings, comments, etc */
	if( oleflags & OLE_STR ) {
		OLE_Replace( cursnode );
		return TOK_NOP;
	}
	/* if we had returned to us a TOK_NOP, and OLE_Edit pushed tokens
	 * onto the stack, we may as well remove one now 
	 */
	if ( (nptok > 0) && (ret_cmd == TOK_NOP) ) {
		/* A token has been pushed onto the input, use it */
		return pop_token();
	}
	/* Otherwise treat as a token typed by the user */
	if (ret_cmd > 0)
		return ret_cmd;
	else
		return TOK_NOP;
}

static int
pop_token()
{
	if( nptok == 0 ) {
		printt0( "pop_token(), no tokens pending, returns TOK_NOP\n" );
		return TOK_NOP;
	}
	strcpy( tokentext, ptok[--nptok].ttext );
	printt2( "pop_token() returns token %d, text = '%s'\n",
			ptok[nptok].token_no, Safep( tokentext ) );
	return ptok[nptok].token_no;
}

/* Return TRUE if character ch might begin an expression */
static int
expbegchar(ch)
int ch;
{
	if (isalnum(ch)) return TRUE;
	if (strchr("-+(['\"", ch)) return TRUE;
	return FALSE;
}

/* Return TRUE if class is something a comment goes into */
static
iscomclass(cl)
int cl;
{
	return (cl == C_COMMENT) || (cl == C_BLCOMMENT);

}

/* Return TRUE if type is something a comment goes into */
static
iscomtype(ty)
int ty;
{
	return (ty == N_T_COMMENT) || (ty == N_ST_COMMENT) || ty == N_NOTDONE;
}
