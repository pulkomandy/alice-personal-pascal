#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "command.h"
#include "flags.h"
#include "dbflags.h"
#define CLIST(arg) arg+64
#include "keys.h"
#include "typecodes.h"
#include "token.h"
#include "ole.h"
#include "class.h"

#ifdef DB_COMMAND
# define PRINTT
#endif
#include "printt.h"

/*
 *DESC: The master command line interpreter.
 */



/*
 * Carry out a command from the user.  cmd is the token number typed
 * by the user.  The text of the token is in tokentext.
 */
extern char *M_insert[];
extern char *M_delete[];
extern char *M_change[];
extern char *M_io[];
extern char *M_misc[];
extern char *M_run[];
extern char *M_move[];
extern char *M_helpkey[];
extern char *M_master[];
extern char *M_range[];

extern alice_window main_win;
extern unsigned find_class(), find_type();
extern char trace_name[];

static not_stop_here(), logon(), logoff(), waitForSpace();

extern int	error_present;
	extern char tokentext[];

/*
 * Carry out the given built in command.  cmd is the command number
 * from "command.h".  args is a character string with any arguments.
 */
bi_command(cmd, args)
int cmd;
char *args;
{
	extern int count_suspend;
	int dummy_y, dummy_x;

	switch(cmd) {
	case CM_BACK:
		do {
			cursor = uc_prevpos(cursor);
		} while( ntype_info( ntype(cursnode) ) & F_NOSTOP );
		break;
	case CM_CLEAR:
		if (!error_present)
			redraw_status = TRUE;
#ifdef unix
		clearok( curscr, TRUE );
#endif
		wclear(curr_window->w_desc);
		clear_em(TRUE);
		display(TRUE);
		break;
	case CM_DELETE:
		grab_range(R_FORCE|R_LEAF|R_NOLIST);
		chprune(sel_node);
		break;
	case CM_DELLIST:
		do_delete( args != 0 );
		break;
	case CM_EABOVE:
		grab_range(0);		/* clear selection */
		exp_list(0);
		break;
	case CM_EBELOW:
		grab_range(0);		/* clear selection */
		exp_list(1);
		break;
	case CM_THELP:
		token_help();
		break;
	case CM_PHELP:
		pas_help(args);
		break;
	case CM_EXIT:
		done(args);
		break;
	case CM_FWD:
		{
		nodep savcur;
		savcur = cursor;
		do {
			cursor = uc_nextpos(cursor);
		} while( ntype_info( ntype(cursnode) ) & F_NOSTOP );
		if( c_at_root(cursor) )
			cursor = savcur;
		}
		break;
	case CM_NEXT:
		{
		nodep savcur;

		savcur = cursor;
		markPopBack(cursor);
		do {
			cursor = uc_nextpos(cursor);
		} while (not_stop_here(cursor,savcur));
		}
		break;
	case CM_PREV:
		{
		nodep savcur;
	
		markPopBack(cursor);
		savcur = cursor;
		do {
			cursor = uc_prevpos(cursor);
		} while( not_stop_here(cursor,savcur));
		}
		break;
	case CM_PARENT:
		if( !c_at_root( cursor ) )
			cursor = find_true_parent(cursor);
		break;



#ifdef DEBUG


	case CM_TRACE:
		getIfNull( args, "File? ", trace_name, 20 );
		dtrace = fopen( trace_name, "w" );
		message(ER(195,"turning on trace, file '%s'"), trace_name);
#ifdef unix
		if( args && *args == 'u' )
			setbuf( dtrace, (pointer)NULL );
#endif
		break;
	case CM_UTRACE:
		dtrace = fopen( trace_name, "w" );
		message(ER(196,"turning on unbuffered trace, file '%s'"), trace_name);
#ifndef qnx
		/* test microsoft thing */
		setbuf(dtrace, (char *)0 );
#endif
		break;
#endif
	case CM_HIDE:
		tryhide();
		break;
	case CM_REVEAL:
		grab_range(0);	/* clear selection */
		tryreveal();
		break;
	case CM_NLOAD:
		aload(LOAD, args);
		break;
	case CM_MERGE:
		aload(MERGE, args);
		break;
	case CM_SAVE:
		grab_range(0);
		imm_abort();
		save(0, setfname(args, FALSE, FileExt, FALSE) );
		break;
	case CM_AUTOSAVE:
		{
		extern int autsave, ascount;
		char nmbuf[40];

		imm_abort();

		getIfNull( args, "Save Interval (0 for no Autosave) ? ",
			nmbuf, sizeof(nmbuf) );
		autsave = atoi( nmbuf );
		ascount = 0;
		}
		break;
	case CM_TEXT:
		grab_range(0);
		text_out(args);
		break;
	case CM_WSEXEC:
		exec_ws( args );
		break;
	case CM_RDAMNIT:
		count_suspend = 0;
	case CM_RUN:
		grab_range(0);		/* clear selection */
		if( !count_suspend ) {
#ifdef ICON
			slmessage( ER(197,"Program is running - CTRL-RUBOUT to halt it") );
#else
			slmessage( ER(197,"Program is running") );
#endif
			if (!do_run_prog(cur_work_top, work_debug(curr_workspace))) {
				waitForSpace();
				}
			break;
			}
		/* else fall through to */
	case CM_CONTINUE:
		slmessage( ER(288,"Resumed program execution") );
		sing_step(work_debug(curr_workspace));
		waitForSpace();
		break;
	case CM_CHECK:
		{
		extern int had_type_error;
		block_typecheck(cur_work_top);
		display(TRUE);
#ifdef GEM
		if( !had_type_error ) 
			form_alert( 1, 
                   "[2][Typecheck completed|No errors detected][OK]" );

#else
		if( !had_type_error )
			message( ER(282,"Typecheck completed. No errors detected.") );
#endif

		}
		break;
	case CM_UNDO:
		undo(FALSE);	/* not a cleanup */
		break;
	case CM_REDO:
		redo();
		break;
	case CM_EDIT:
		/*
		 * The "edit the current subtree textually" command pushes
		 * the EDIT key onto the input, which forces invocation of
		 * the one line editor.
		 */

		/* We cannot edit a list */
		grab_range( R_FORCE|R_LEAF|R_NOLIST );
		/* set the cursor to be the selected node */
		cursor = sel_node;

		getyx( curr_window->w_desc, dummy_y, dummy_x );

		/* Test to see if it is valid to edit the selected 
		 * text in this manner 
		 */

		if( OLE_CanEdit( cursor ) ) {

				/* Now we treeprint the current line into
				 * a buffer (line_buffer).  This also
				 * sets up pointers within line_buffer
				 * to where the cursor starts and ends.
				 */

			OLE_InitEdit( cursor );

				/* Now that we have the original text in
				 * the line_buffer, we chop it up into
				 * three parts: the preceding text, the
				 * following text, and the text that
				 * is being edited.
				 */

			OLE_ChopUpText( cursor, FALSE );

				/* Now that the text is in three parts, call
				 * the one line editor to edit the cursor
				 * text.
				 */

			OLE_Edit( cursor, OLE_EDITING, "", dummy_y, dummy_x );

				/* Now we have the new text, we have to see
				 * if it fits into what was there before
				 * if it doesn't OLE_Replace will issue an
				 * appropriate error.
				 */
			OLE_Replace( cursor );
		}
		 else {
			error( ER(120,"cantedit`You cannot edit a %s as text")
					,NodeName( ntype( cursor ) ) );
		}	
		break;
	case CM_STEP:
		sing_step(DBF_ON|STEP_SINGLE);
		break;
	case CM_SUPERSTEP:
		slmessage(ER(199,"Superstep Execution") );
		if( count_suspend )
			do_superstep();
		 else
			sing_step(DBF_ON|STEP_SINGLE);
		break;
	case CM_M_INSERT:
		com_menu(M_insert, FALSE);
		break;
	case CM_M_DELETE:
		com_menu(M_delete, FALSE);
		break;
	case CM_M_IO:
		com_menu(M_io, FALSE);
		break;
	case CM_M_CHANGE:
		com_menu(M_change, FALSE);	
		break;
	case CM_HELP:
		if( args ) {
			helpfile( strchr( args, '/' ) ? "" : "psymbol/", args );
			}
		 else
			com_menu(M_helpkey, FALSE);
		break;
	case CM_ERRHELP:
		do_errhelp(args);
		break;
	case CM_SYMBOL:
		sym_menu(cursor);
		break;
	case CM_MENU:
		com_menu(M_master, FALSE);
		break;
	case CM_NOP:
		break;
	case CM_SELECT:
		if( !anchor ) {
			do_select();
			break;
			}
		/* else fall through into */
	case CM_UNSEL:
		grab_range(0);
		break;
#ifdef HAS_MENU
	case CM_M_RANGE:
		com_menu(M_range, FALSE);
		break;
	case CM_M_MISC:
		com_menu(M_misc, FALSE);
		break;
	case CM_M_MOVE:
		com_menu(M_move, FALSE);
		break;
	case CM_M_DEBUG:
		deb_menu();
		break;
#endif
	case CM_TOKEN:
		key_token();
		break;
#ifdef DB_DUMP
	case CM_DUMP:
		dump(cur_work_top);
		break;
#endif
	case CM_SETFOLLOW:
#ifdef GEM
		do_gfon();
		break;
#else
		work_debug(curr_workspace) |= DBF_CURSOR;
#endif
	case CM_BUGON:
		prep_r_window();
		break;
	case CM_CLRFOLLOW:
#ifdef GEM
		do_gfoff();
#else
		work_debug(curr_workspace) &= ~DBF_CURSOR;
#endif
		break;
	case CM_BUGOFF:
		imm_abort();
		del_r_window();
		break;
	case CM_LSIB:
		if( get_kidnum( cursor ) )
			cursor = c_lsib(cursor);
		break;
	case CM_RSIB:
		cursor = c_rsib(cursor);
		break;
	case CM_CHILD:
		if( n_num_children(cursor) )
			cursor = realcp(kid1(cursor));
		break;
	case CM_BLOCK: 
		{
		register nodep ascend;
		extern nodep my_block();

		ascend = my_block(cursparent);
		markPopBack(cursor);
		cursor = ascend ? ascend : cur_work_top;
		}
		break;

	case CM_SETFILE:
		setfname(args, TRUE, FileExt, FALSE);
		break;
	/* save as text, then execute dos command */
	case CM_COMPILE:
		text_out( (char *) 0 );
	case CM_SHELL:
	case CM_DOS:
		/* use exec if dos command */
		do_shell(args, cmd != CM_SHELL);
		break;
	/* Window management commands */
	/*
	case CM_W_SPLIT:
	case CM_W_CREATE:
	case CM_W_DESTROY:
	case CM_W_NEXT:
	case CM_W_CHANGE:
	case CM_W_BIG:
		bug("Window commands not implemented");
	*/

	case CM_OPTION:
		{
		char nmbuf[40];

		extern int change_stacksize;
		getIfNull( args, "Option line? ",
			nmbuf, sizeof(nmbuf) );
		do_cloption( nmbuf, TRUE );
		if( change_stacksize )
			init_interp();
		display(TRUE);
		}
		break;
	case CM_WORKSPACE:
		imm_abort();
		go_to_workspace(args);
		break;
	case CM_GET:
		graft_ws(args, cursor);
		break;
	case CM_COPY:
		cp_to_ws(args);
		break;
	case CM_MOVE:
		{
#ifdef notdef
		if (sel_node == cur_work_top)
			error(ER(49,"pruneall`You can't delete an entire program!"));
#endif
		cp_to_ws(args);
		if( sel_last < 0 ) {
			cursor = sel_node;
			grab_range( R_LEAF | R_FLIST );
			if( sel_last < 0 ) {
				chprune( sel_node );
				break;
				}
			}
		del_range(FALSE);
		}
		break;

	/* Searching commands */
#ifndef OLD_STUFF
	case CM_S_FWD:
		srchFwd(args);
		break;
	case CM_S_BACK:
		srchBack(args);
		break;
	case CM_S_AGAIN:
		srchAgain();
		break;
#else
#ifdef HAS_SEARCH
	case CM_S_FWD:
		search(TRUE, FALSE, FALSE);
		break;
	case CM_S_BACK:
		search(FALSE, FALSE, FALSE);
		break;
	case CM_S_TFWD:
		search(TRUE, TRUE, FALSE);
		break;
	case CM_S_TBACK:
		search(FALSE, TRUE, FALSE);
		break;
	case CM_S_AGAIN:
		search(TRUE, FALSE, TRUE);
		break;
#endif HAS_SEARCH
#endif OLD_STUFF

	/* macro commands */
	case CM_A_DEF:
		macdef(args);
		break;
	case CM_IMM:
		{
		int savcodech;
		extern Boolean CodeChanged;

		savcodech = CodeChanged;
		grab_range(0);
		do_immediate();
		CodeChanged = savcodech;
		}
		break;
	case CM_EXEC:
		grab_range(R_FORCE|R_NOLIST);
		prep_r_window();
		prep_to_run();
		slmessage( ER(200,"Immediate mode execution") );
		ex_immediate(sel_node,work_debug(curr_workspace));
		break;
	case CM_BPOINT:
		set_bpoint();
		break;
	case CM_CPOINT:
		clr_point();
		break;
#ifdef HAS_MARK
	case CM_MARK:
		mark(args);
		break;
	case CM_GO:
		go(args);
		break;
	case CM_POPBACK:
		popBack();
		break;
#endif HAS_MARK
	case CM_LOWER:
		do_lower(FALSE);
		break;
	case CM_M_SPECIALTIES:
		do_transmog();
		break;
	case CM_RAISE:
		do_raise();
		break;
	case CM_COMOUT:
		do_comout();
		break;
	case CM_SPOP:
		if( count_suspend )
			pop_susp();
		 else
			error( ER(27,"notsusp`Program not currently suspended") );
		break;
	case CM_TBACK:
		if( count_suspend )
			tracepop();
		 else
			error( ER(27,"notsusp`Program not currently suspended") );
		break;
	case CM_DECL:
		{
		register symptr gosym;
		int std;
		NodeNum ctyp;

		ctyp = ntype(cursor);
		if( args ) 
			gosym = slookup( args, NOCREATE, NIL, NOMOAN, NIL );
		 else
			gosym = (ctyp == N_ID || ctyp == N_VF_WITH) ?
						kid_id(cursor) : 0;

		if( ctyp != N_DECL_ID ) {
			if( !gosym || (std = sym_dtype(gosym )) == T_UNDEF ||
					(sym_scope(gosym) == 0 && 
					std != T_LABEL ))
				error( ER(28,"nodecl`There is no declaration for this item"));
		 	else {
				markPopBack( cursor );
				cursor = NCAST gosym;
				}
			}
		break;
		}
	case CM_PGDOWN:
		page_down();
		break;

	case CM_PGUP:
		page_up();
		break;
	
	case CM_NEW:
		imm_abort();
		clear_ws(args);
		break;
	case CM_LASTERR:
		repeat_err();
		break;

	case CM_RECOVER:
		warning( ER(201,"noundo`Memory Recovered.  You can't UNDO past this point") );
		zap_history();
		redraw_status = TRUE;
		break;

	/* Turn output logging on/off */
	case CM_LOG:
		logon(args);
		break;
	case CM_NOLOG:
		logoff();
		break;
#ifdef IBMPC
	case CM_VIEW:
		viewGraphics();
		break;
#endif

#ifdef GEM
	case CM_GSLIDE:
		do_gslide();
		break;
	case CM_GCLOSE:
		do_gclose();
		break;
	case CM_GFOLLOW:
		do_gfollow();
		break;
	case CM_GABOUT:
		do_gabout();
		break;
	case CM_GFULLED:
		do_gfull();
		break;
	case CM_GREDRAW:
		do_gredraw();
		break;
	case CM_GTOP:
		do_gtop();
		break;
	case CM_GMOVED:
		do_gmoved();
		break;
	case CM_GSIZED:
		do_gsized();
		break;
#endif

	default:
		warning(ER(202,"badcommand`Unknown command '%s'"), tokentext);
#ifdef DB_COMMAND
		if (tracing) fprintf(dtrace, "Unknown command %d'%s'\n", cmd,
				tokentext);
#endif
	}
}



/* show the tree for a new position */
struct {
	char *word;
	int ttype;
} cmdtab[] = {
	/* Note: binary search used, these must be in alpha order */
	"again",	CM_S_AGAIN,
	"autosave",	CM_AUTOSAVE,
	"back",		CM_BACK,
/*	"big",		CM_W_BIG, */
	"block",	CM_BLOCK,
	"breakpoint",	CM_BPOINT,
	"bugoff",	CM_BUGOFF,	/* what a great command! */
	"bugon",	CM_BUGON,
	"change",	CM_M_CHANGE,
/*	"changewin",	CM_W_CHANGE, */
	"check",	CM_CHECK,
	"child",	CM_CHILD,
	"clear",	CM_CLEAR,
	"clearfollow",	CM_CLRFOLLOW,
	"clearpoint",	CM_CPOINT,
	"clip",		CM_DELETE,
	"comout",	CM_COMOUT,
	"compile",	CM_COMPILE,
	"continue",	CM_CONTINUE,
	"copy",		CM_COPY,
/*	"create",	CM_W_CREATE, */
	"cursor",	CM_M_MOVE,
	"debug",	CM_M_DEBUG,
	"decl",		CM_DECL,
	"delete",	CM_DELLIST,
	"delmenu",	CM_M_DELETE,
/*	"destroy",	CM_W_DESTROY, */
#ifdef msdos
	"dos",		CM_DOS,
#endif
#ifdef DB_DUMP
	"dump",		CM_DUMP,
#endif
	"edit",		CM_EDIT,
	"enclose",	CM_LOWER,
	"errhelp",	CM_ERRHELP,
	"execute",	CM_EXEC,
	"execws",	CM_WSEXEC,
	"exit",		CM_EXIT,
	"expr",		CM_EXPR,
	"extend",	CM_EBELOW,
	"filename",	CM_SETFILE,
	"fwd",		CM_FWD,

/*
 * All the GEM specific commands
 */
#ifdef GEM
	"gemabout",	CM_GABOUT,
	"gemclose",	CM_GCLOSE,
	"gemfollow",	CM_GFOLLOW,
	"gemfulled",	CM_GFULLED,
	"gemmoved",	CM_GMOVED,
	"gemredraw",	CM_GREDRAW,
	"gemsized",	CM_GSIZED,
	"gemslide",	CM_GSLIDE,
	"gemtop",	CM_GTOP,
#endif

	"get",		CM_GET,
	"go",		CM_GO,
	"help",		CM_HELP,
	"hide",		CM_HIDE,
	"immediate",	CM_IMM,
	"insert",	CM_EABOVE,
	"insmenu",	CM_M_INSERT,
	"io",		CM_M_IO,
	"lasterror",	CM_LASTERR,
	"load",		CM_NLOAD,
	"log",		CM_LOG,
	"lsib",		CM_LSIB,
	"map",		CM_A_DEF,
	"mark",		CM_MARK,
	"menu",		CM_MENU,
	"merge",	CM_MERGE,
	"misc",		CM_M_MISC,
	"move",		CM_MOVE,
	"new",		CM_NEW,
	"next",		CM_NEXT,
/*	"nextwin",	CM_W_NEXT, */
	"nolog",	CM_NOLOG,
	"nop",		CM_NOP,
	"option",	CM_OPTION,
	"pagedown",	CM_PGDOWN,
	"pageup",	CM_PGUP,
	"parent",	CM_PARENT,
	"phelp",	CM_PHELP,
	"pop",		CM_SPOP,
	"popback",	CM_POPBACK,
	"prev",		CM_PREV,
	"raise",	CM_RAISE,
	"range",	CM_M_RANGE,
	"recover",	CM_RECOVER,
	"redo",		CM_REDO,
	"reveal",	CM_REVEAL,
	"rsearch",	CM_S_BACK,
	"rsib",		CM_RSIB,
#ifdef OLD_STUFF
	"rtsearch",	CM_S_TBACK,
#endif
	"run",		CM_RDAMNIT,
	"runit",	CM_RUN,
	"save",		CM_SAVE,
	"search",	CM_S_FWD,
	"select",	CM_SELECT,
	"setfollow",	CM_SETFOLLOW,
	"shell",	CM_SHELL,
	"specialties",	CM_M_SPECIALTIES,
/*	"split",	CM_W_SPLIT, */
	"step",		CM_STEP,
	"superstep",	CM_SUPERSTEP,
	"symbol",	CM_SYMBOL,
	"text",		CM_TEXT,
	"token",	CM_TOKEN,
	"toklist",	CM_THELP,
#ifdef DEBUG
	"trace",	CM_TRACE,
#endif DEBUG
	"traceback",	CM_TBACK,
#ifdef OLD_STUFF
	"tsearch",	CM_S_TFWD,
#endif
	"type",		CM_TYPEROT,
	"undo",		CM_UNDO,
	"unselect",	CM_UNSEL,
#ifdef DEBUG
	"utrace",	CM_UTRACE,
#endif DEBUG
#ifdef IBMPC
	"view",		CM_VIEW,
#endif
	"ws",		CM_WORKSPACE,
	0,		0,
};

/*
 * Binary search for built in command.  destroys the string, lower casing it
 */
int
look_cmd(str)
char *str;
{
	register int m;
	reg int h, l, r, i;
	char lowbuf[100];
	char *stp;

	/* if null command, make it nop */
	if( !str )
		return CM_NOP;

	for( i = 0; str[i] && i < sizeof(lowbuf)-1; i++ )
		lowbuf[i] = str[i] | 0x20;
	lowbuf[i]=0;

	l=0; h=sizeof(cmdtab) / sizeof(cmdtab[0]) - 2;
	while (l <= h) {
		m = (l+h)/2;
		r = strcmp(cmdtab[m].word, lowbuf);
		if (r < 0)
			l = m+1;
		else if (r > 0)
			h = m-1;
		else
			return cmdtab[m].ttype;
	}
	return 0;
}

funcptr cmd_preproc = 0;

char *comargs;		/* command arguments */

do_command(cmd)
unsigned cmd;
{
	char firstword[150];
	register char *p, *q;

#ifndef QNX
	if( cmd_preproc && (*cmd_preproc)(cmd,tokentext) )
			return;
#endif
	if (cmd == TOK_CMD || !cmd) {
#ifdef DEBUG
		if (tracing) fprintf(dtrace, "\nCommand: %s\n", tokentext);
#endif
		/* strip leading spaces */
		for( q = tokentext; *q == ' '; q++ )
			;
		for (p=firstword; *q>' '; )
			*p++ = *q++;
		while( *q == ' ' )
			q++;
		if( *q == 0 )
			q = 0;
		*p = 0; 
		cmd = look_cmd(firstword);
		comargs = q;
		bi_command(cmd, q);
	} else {
		if( cmd != TOK_NOP )
			in_token(cmd,cursor);
	}
}

static
not_stop_here(cur,savcur)
register nodep cur;
nodep savcur;
{
	register int cclas;
	return is_a_list(cur) ||
		((not_a_stub(cur) ||(cclas = int_kid(0,cur)) == C_COMMENT
				|| cclas == C_BLCOMMENT ) &&
		!(node_flag(cur) & NF_ERROR) &&
		!(ntype(cur) == N_ID && sym_dtype(kid_id(cur)) == T_UNDEF)
		&& cur != savcur);
}

FILE	*LogFile = 0;

static
logon(args)
char	*args;
{
	char	*fname = (args && strlen(args) > 0) ? args : LOGFILE;

	LogFile = fopen(fname, "w");

	if (LogFile)
		warning(ER(203,"logging output to \"%s\""), fname);
	else
		error(ER(29,"Log - can't open file \"%s\""), fname);
}

static
logoff()
{
	if (LogFile) {
		fclose(LogFile);
		LogFile = 0;
	}
}

static
waitForSpace()
{
	extern int	error_present;

	if (!(work_debug(curr_workspace) & DBF_ON)) {
		slmessage(ER(198,"Program halted - hit SPACE to resume editing"));
		/* If he hit break, flush it */
		/* reading a key decrements the error present flag,
		 * causing the runtime error to leave the screen, so
		 * we want to keep it at the same value, that's why */
#ifndef GEM
		while (readkey() != ' ')
			error_present++;
#endif
		display(TRUE);
		}
}
