#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "command.h"
#include "flags.h"
#include "dbflags.h"
//#define CLIST(x)	64+x
#include "keys.h"
#include "typecodes.h"
#include "token.h"
#include "ole.h"
#include "class.h"
#include "menu.h"
#include <ctype.h>
#include "search.h"

#ifdef DB_COMMAND
# define PRINTT
#endif
#include "printt.h"

#ifdef GEM
#include "alrsc.h"
extern long gl_menu;
#endif

#include "command.h"

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


extern int	error_present;
	extern char tokentext[];
	
static waitForSpace();


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
#ifdef GEM
		draw_all();
#endif

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
		trace = fopen( trace_name, "w" );
		message(ER(195,"turning on trace, file '%s'"), trace_name);
#ifdef unix
		if( args && *args == 'u' )
			setbuf( trace, (pointer)NULL );
#endif
		break;
	case CM_UTRACE:
		trace = fopen( trace_name, "w" );
		message(ER(196,"turning on unbuffered trace, file '%s'"), trace_name);
#ifndef qnx
		/* test microsoft thing */
		setbuf(trace, (char *)0 );
#endif
		break;
#endif
	case CM_HIDE:
		tryhide();
		break;
	case CM_REVEAL:
		grab_range(0);	/* clear selection */
		tryreveal();
#ifdef GEM
		count_lines();
#endif
		break;
	case CM_NLOAD:
		aload(LOAD, args);
		break;

#ifdef HAS_LIBRARY
	case CM_LIBRARY:
		aload(LIBRARY, args);
		break;
#endif

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

		getIfNull( args, LDS( 411,
			"Save Interval (0 for no Autosave) ? "),
			nmbuf, sizeof(nmbuf) );
		autsave = atoi( nmbuf );
		ascount = 0;
		}
		break;
	case CM_TEXT:
		grab_range(0);
#ifdef GEM
		dummy_x = SetFreq( 1 );
#endif
		text_out(args);
#ifdef GEM
		SetFreq( dummy_x );
#endif
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
			form_alert( 1, LDS( 412, 
                   "[2][Typecheck completed.|No errors detected.][ OK ]" ));

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
#ifndef GEM
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
#endif GEM
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
#ifndef GEM
	case CM_MENU:
		com_menu(M_master, FALSE);
		break;
#endif
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
#ifndef GEM
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
#endif GEM
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
#ifdef GEM
		do_gbugon();
#else
		prep_r_window();
#endif
		break;
	case CM_CLRFOLLOW:
#ifdef GEM
		do_gfoff();
#else
		work_debug(curr_workspace) &= ~DBF_CURSOR;
#endif
		break;
	case CM_BUGOFF:
#ifdef GEM
		do_gbugoff();
#else
		imm_abort();
		del_r_window();
#endif
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
#ifdef GEM
		{
		extern char *cpl_string;
		/*
		 * If there are no args, then set them to be the compile
		 * string
		 */
		if( !args ) {
			if( !cpl_string )
				error( ER( 362, 
			"nocpl`No compile string defined" ));
			args = cpl_string;
		}
	}
#endif
	case CM_SHELL:
	case CM_DOS:
		/* use exec if dos command */
#ifdef TOS
		do_shell( args, TRUE );
#else
		do_shell(args, cmd != CM_SHELL);
#endif
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
		getIfNull( args, LDS( 414, "Option line? " ),
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
#ifdef GEM
		do_gsearch( SRCH_FWD );
#else
		srchFwd(args);
#endif
		break;
	case CM_S_BACK:
#ifdef GEM
		do_gsearch( SRCH_BACK );
#else
		srchBack(args);
#endif
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
	case CM_VIEW:
		viewGraphics();
		break;

#ifdef GEM
	case CM_CHELP:
		do_chelp( args );
		break;
	case CM_GFREEMEM:
		do_gfreemem();
		break;
	case CM_GCDIR:
		do_cdir( args );
		break;
	case CM_GSAVEAS:
		setfname(args, TRUE, FileExt, FALSE);
		grab_range(0);
		imm_abort();
		save(0, setfname(args, FALSE, FileExt, FALSE) );
		break;
	case CM_GNEWWS:
		do_gnewws();
		break;
	case CM_GDEBUG:
		do_gdebug();
		break;
	case CM_GLOG:
		do_glog();
		break;
	case CM_GKEYS:
		do_gkeys();
		break;
	case CM_GSLIDE:
		do_gslide();
		break;
	case CM_GCOUNT:
		count_lines();
#ifndef RELEASE
		checkhelp();
#endif
		break;
	case CM_GCLOSE:
		do_gclose();
		break;
	case CM_GFOLLOW:
		do_gfollow();
		break;
	case CM_GBUFFER:
		do_gbuffer();
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
	case CM_GSEARCH:
		do_gsearch( 0 );
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
#ifndef RELEASE
	case CM_GSNAP:
		take_snap();
		break;
#endif
#endif

	default:
		warning(ER(202,"badcommand`Unknown command '%s'"), tokentext);
#ifdef DB_COMMAND
		if (tracing) fprintf(trace, "Unknown command %d'%s'\n", cmd,
				tokentext);
#endif
	}
}


#define HN_SAME	(char *)1
#define HN_NONE (char *)0

/* show the tree for a new position */
struct {
	char *word;
	int ttype;
	char *hname;
} cmdtab[] = {
	/* Note: binary search used, these must be in alpha order */
	"again",	CM_S_AGAIN,	"Search Again",
	"autosave",	CM_AUTOSAVE,	HN_SAME,
	"back",		CM_BACK,	HN_NONE,
/*	"big",		CM_W_BIG, */
	"block",	CM_BLOCK,	HN_SAME,
	"breakpoint",	CM_BPOINT,	"Set Breakpoint",
#ifdef GEM
	"buffer",	CM_GBUFFER,	"Buffer Graphics",
#endif
	"bugoff",	CM_BUGOFF,	"Debug On",
	"bugon",	CM_BUGON,	"Debug Off",
#ifdef GEM
	"cd",		CM_GCDIR,	"Change Directory",
#endif
#ifndef GEM
	"change",	CM_M_CHANGE,	HN_NONE,
#endif
/*	"changewin",	CM_W_CHANGE, */
	"check",	CM_CHECK,	"Typecheck Program",
#ifdef GEM
	"chelp",	CM_CHELP,	HN_NONE,
#endif
	"child",	CM_CHILD,	HN_NONE,
	"clear",	CM_CLEAR,	"Redraw Screen",
	"clearfollow",	CM_CLRFOLLOW,	"Cursor Following Off",
	"clearpoint",	CM_CPOINT,	"Clear Breakpoint",
	"clip",		CM_DELETE,	HN_SAME,
	"comout",	CM_COMOUT,	"Comment Out",
	"compile",	CM_COMPILE,	"Invoke Compiler",
	"continue",	CM_CONTINUE,	HN_SAME,
	"copy",		CM_COPY,	HN_SAME,
/*	"create",	CM_W_CREATE, */
	"createws",	CM_GNEWWS,	"Create Workspace",
#ifndef GEM
	"cursor",	CM_M_MOVE,	HN_NONE,
	"debug",	CM_M_DEBUG,	HN_NONE,
#endif
	"decl",		CM_DECL,	"Go to Declaration",
	"delete",	CM_DELLIST,	HN_SAME,
#ifndef GEM
	"delmenu",	CM_M_DELETE,	HN_NONE,
#endif

/*	"destroy",	CM_W_DESTROY, */
#ifdef msdos
	"dos",		CM_DOS,		HN_SAME,
#endif
#ifdef DB_DUMP
	"dump",		CM_DUMP,	HN_SAME,
#endif
	"edit",		CM_EDIT,	"Edit as text",
	"enclose",	CM_LOWER,	HN_SAME,
	"errhelp",	CM_ERRHELP,	"Error Help",
	"execute",	CM_EXEC,	"Execute Statement",
	"execws",	CM_WSEXEC,	"Execute Workspace",
	"exit",		CM_EXIT,	"Exit Alice",
	"expr",		CM_EXPR,	HN_NONE,
	"extend",	CM_EBELOW,	HN_SAME,
	"filename",	CM_SETFILE,	"Set Filename",
#ifdef GEM
	"freemem",	CM_GFREEMEM,	"Memory Left",
#endif

	"fwd",		CM_FWD,		HN_NONE,

/*
 * All the GEM specific commands
 */
#ifdef GEM
	"gemabout",	CM_GABOUT,	HN_NONE,
	"gemclose",	CM_GCLOSE,	HN_NONE,
	"gemcount",	CM_GCOUNT,	HN_NONE,
	"gemdebug",	CM_GDEBUG,	HN_NONE,
	"gemfollow",	CM_GFOLLOW,	HN_NONE,
	"gemfulled",	CM_GFULLED,	HN_NONE,
	"gemkeys",	CM_GKEYS,	HN_NONE,
	"gemlog",	CM_GLOG,	HN_NONE,
	"gemmoved",	CM_GMOVED,	HN_NONE,
	"gemredraw",	CM_GREDRAW,	HN_NONE,
	"gemsaveas",	CM_GSAVEAS,	HN_NONE,
	"gemsearch",	CM_GSEARCH,	HN_NONE,
	"gemsized",	CM_GSIZED,	HN_NONE,
	"gemslide",	CM_GSLIDE,	HN_NONE,
#ifndef RELEASE
	"gemsnap",	CM_GSNAP,	"Snapshot Taker",
#endif
	"gemtop",	CM_GTOP,	HN_NONE,
#endif

	"get",		CM_GET,		"Get Workspace",
	"go",		CM_GO,		"Go to Mark",
	"help",		CM_HELP,	HN_SAME,
	"hide",		CM_HIDE,	HN_SAME,
	"immediate",	CM_IMM,		"Immediate Mode",
	"insert",	CM_EABOVE,	HN_SAME,
#ifndef GEM
	"insmenu",	CM_M_INSERT,	HN_NONE,
	"io",		CM_M_IO,	HN_NONE,
#endif

	"lasterror",	CM_LASTERR,	"Help on Last Error",

#ifdef HAS_LIBRARY
	"library",	CM_LIBRARY,	"Install Library",
#endif

	"load",		CM_NLOAD,	HN_SAME,
	"log",		CM_LOG,		"Log Output to file",
	"lsib",		CM_LSIB,	HN_NONE,
	"map",		CM_A_DEF,	"Define Macro",
	"mark",		CM_MARK,	"Set Mark",
#ifndef GEM
	"menu",		CM_MENU,	HN_NONE,
#endif
	"merge",	CM_MERGE,	HN_SAME,
#ifndef GEM
	"misc",		CM_M_MISC,	HN_NONE,
#endif

	"move",		CM_MOVE,	"Move to Workspace",
	"new",		CM_NEW,		"Clear Workspace",
	"next",		CM_NEXT,	HN_NONE,
/*	"nextwin",	CM_W_NEXT, */
	"nolog",	CM_NOLOG,	"Turn logging off",
	"nop",		CM_NOP,		HN_SAME,
	"option",	CM_OPTION,	"Set Option",
	"pagedown",	CM_PGDOWN,	"Page Down",
	"pageup",	CM_PGUP,	"Page Up",
	"parent",	CM_PARENT,	HN_SAME,
	"phelp",	CM_PHELP,	HN_SAME,
	"pop",		CM_SPOP,	"Pop Suspended State",
	"popback",	CM_POPBACK,	"Go to Previous Spot",
	"prev",		CM_PREV,	HN_NONE,
	"raise",	CM_RAISE,	HN_SAME,
#ifndef GEM
	"range",	CM_M_RANGE,	HN_NONE,
#endif

	"recover",	CM_RECOVER,	"Recover Memory",
	"redo",		CM_REDO,	HN_SAME,
	"reveal",	CM_REVEAL,	HN_SAME,
	"rsearch",	CM_S_BACK,	"Reverse Search",
	"rsib",		CM_RSIB,	HN_NONE,
#ifdef OLD_STUFF
	"rtsearch",	CM_S_TBACK,	HN_NONE,
#endif
	"run",		CM_RDAMNIT,	"Pop & Run Program",
	"runit",	CM_RUN,		"Run Program",
	"save",		CM_SAVE,	HN_SAME,
	"search",	CM_S_FWD,	HN_SAME,
	"select",	CM_SELECT,	HN_NONE,
	"setfollow",	CM_SETFOLLOW,	"Cursor Following On",
	"shell",	CM_SHELL,	"Shell Command",
	"specialties",	CM_M_SPECIALTIES, "Special Changes",

/*	"split",	CM_W_SPLIT, */
	"step",		CM_STEP,	"Single Step",
	"superstep",	CM_SUPERSTEP,	"Super Step",
	"symbol",	CM_SYMBOL,	HN_SAME,
	"text",		CM_TEXT,	"Save as Text",
	"token",	CM_TOKEN,	HN_NONE,
	"toklist",	CM_THELP,	"What can I type here ?",
#ifdef TOS
	"tos",		CM_DOS,		"TOS Command",
#endif

#ifdef DEBUG
	"trace",	CM_TRACE,	HN_NONE,
#endif DEBUG
	"traceback",	CM_TBACK,	"Who called me ?",
#ifdef OLD_STUFF
	"tsearch",	CM_S_TFWD,	HN_NONE,
#endif
	"type",		CM_TYPEROT,	HN_NONE,
	"undo",		CM_UNDO,	HN_SAME,
	"unselect",	CM_UNSEL,	HN_NONE,
#ifdef DEBUG
	"utrace",	CM_UTRACE,	HN_NONE,
#endif DEBUG

#ifndef GEM
	"view",		CM_VIEW,	HN_SAME,
	"ws",		CM_WORKSPACE,	HN_SAME,
#endif
	0,		0,		HN_NONE,
};

/*
 * Binary search for built in command.  destroys the string, lower casing it
 */
int
look_cmd(str)
char *str;
{
	register int m;
	reg int h, l, r;
	char *stp;

	for( stp = str; *stp;  )
		*stp++ |= 0x20;	/* make lower case */
	/* if null command, make it nop */
	if( !*str )
		return CM_NOP;

	l=0; h=sizeof(cmdtab) / sizeof(cmdtab[0]) - 2;
	while (l <= h) {
		m = (l+h)/2;
		r = strcmp(cmdtab[m].word, str);
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

	if( cmd_preproc && (*cmd_preproc)(cmd,tokentext) )
			return;
	if (cmd == TOK_CMD || !cmd) {
#ifdef DEBUG
		if (tracing) fprintf(trace, "\nCommand: %s\n", tokentext);
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

logon(args)
char	*args;
{
	char	*fname = (args && strlen(args) > 0) ? args : LOGFILE;

	LogFile = fopen(fname, "w");

	if (LogFile) {
		warning(ER(203,"logging output to \"%s\""), fname);
#ifdef GEM
		menu_icheck( gl_menu, RMLOG, 1 );
#endif
	} else
		error(ER(29,"Log - can't open file \"%s\""), fname);
}

logoff()
{
	if (LogFile) {
		fclose(LogFile);
#ifdef GEM
		menu_icheck( gl_menu, RMLOG, 0 );
#endif
		LogFile = 0;
	}
}

/*
 * Toggle the logfile status
 */
#ifdef GEM
do_glog()
{
	extern FILE *LogFile;
	char line[50];

	if( LogFile ) {
		logoff();
	} else {
		getprline( LDS( 415, "Name of output file:" ), line, 50 );
		logon( line );
	}
}
#endif

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

#ifdef GEM

do_chelp( args )
char *args;
{
	int i;
	struct menu_data m;
	char *s;
	char *t;
	char *helpstr;

	if( args && *args ) {
		helpfile( "command/", args );
		return;
	}

	new_menu( &m, LDS( 416, "Command Help" ) );

	i = 0;
	while( cmdtab[i].word ) {
		if( cmdtab[i].hname == HN_SAME ) {
			s = cmdtab[i].word;
			*s = *s + 'A' - 'a';
			add_menu_item( s );
		} else if( cmdtab[i].hname != HN_NONE )
			add_menu_item( cmdtab[i].hname );

		i++;
	}
	pop_menu( &m, (char *)0, TRUE, &s, (int *)0 );

	i = 0;

	helpstr = "nop";

	while( cmdtab[i].word ) {
		t = cmdtab[i].word;
		if( *t >= 'A' && *t <= 'Z' )
			*t = *t - 'A' + 'a';

		/* Look for the string returned */
		if( s == cmdtab[i].word || s == cmdtab[i].hname )
			helpstr = cmdtab[i].word;

		i++;
	}

	if( s )
		helpfile( "command/", helpstr );

}

#endif GEM
