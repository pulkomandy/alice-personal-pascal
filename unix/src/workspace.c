
/*
 *
 *DESC:  Routines to deal with workspaces - special named subtrees
 *  Workspaces are kept in an array of workspace structs.
 *  Two main workspaces are the main program workspace and
 *  the deleted tree workspace
 *
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "class.h"
#include "typecodes.h"
#include "menu.h"
#include "flags.h"
#include "dbflags.h"
#include "jump.h"
#include "extramem.h"
#ifdef DB_WS
#define PRINTT
#endif
#include "printt.h"

/*
 * Tree copy routine, copies or modifies trees.
 * During copy, we may have to modify symbol references that are out
 * of tree into name references, or vice versa.
 */


workspace *curr_workspace;		/* the current workspace */
workspace *ws_list;	 		/* workspace list, master workspace */
alice_window *curr_window;

#ifndef PARSER

#ifndef SAI
imm_ws()
{
	register nodep wtop;
	cursor = work_node(curr_workspace);
	dec_block( N_IMM_BLOCK );
	cursor = realcp( kid2((wtop = work_node(curr_workspace))) );
	work_scursor(curr_workspace) = wtop;
	work_debug(curr_workspace) = 0;
	prep_r_window();		/* setup for debug */
}
#endif

init_ws()
{
	extern alice_window main_win;

	curr_window = &main_win;
	curr_workspace = main_win.wspace = ws_list = Tptr(main_ws);
	linkup( curr_workspace, 0, make_stub( C_ROOT ) );
	work_name(curr_workspace) = "main";
	work_debug( curr_workspace ) = 0;
	
	printt1( "Initialize workspaces - main_ws is %x\n", Tptr(main_ws) );
	/* an init_pwork will be done later if we decide to clip */
}


/*
 * tree copy routine - copies trees and handles symbol references
 * within them so that we can copy between real workspaces and fragment
 * workspaces
 */

nodep 
tcopy( xloc, dcopy, reref )
nodep xloc;		/* location of tree to copy */
int dcopy;		/* are we doing a copy, else a move */
int reref;		/* map N_NAME to N_ID in this symbol table, if
			 * false try the reverse
			 */
{
#ifdef HAS_WORKSPACE
	register nodep ntree;	/* new tree */
	register nodep loc = xloc;
	NodeNum loctype;
	register int index;	/* loop through children */
	int maxkid;
	int nmsiz;

	loctype = ntype(loc);
	if( dcopy ) {
		if( loctype == N_DECL_ID ) 
			nmsiz = sizeof( struct symbol_node );
		 else {
			nmsiz = nodeSize(loc);
			}
		ntree = NCAST talloc( nmsiz );
		printt2( "tcopy node %x onto new mem %x\n", loc, ntree );
		t_blk_move( ntree, loc, nmsiz );

		switch (loctype) {
		case N_DECL_ID:
			clr_node_flag(ntree,NF_IS_DECL|NF_REDECLARED);
			/* copy over the memory */
			s_sym_name( (symptr)ntree, allocstring(
				sym_name( (symptr) ntree) ) );
			s_sym_type((symptr)ntree, NIL);	
			break;
		case N_SYMBOL_TABLE:
			s_kid1(ntree, NIL);
			break;
		case N_LIST:
			s_listacount( ntree, listcount(ntree) );
			break;
		case N_T_COMMENT:
		case N_CON_INT:
		case N_CON_REAL:
		case N_EXP_ERROR:
#if defined(ES) || defined(HYBRID)
			s_str_kid(0, ntree, allocESString(getESString(str_kid(0, loc))));
			break;
#endif
		case N_CON_STRING:
			{
			char	*from, *to;

			from = str_kid(0, loc);
			to = checkalloc(3 + strlen(from + 2));
			to[0] = from[0];
			to[1] = from[1];
			strcpy(to + 2, from + 2);
			s_str_kid(0, ntree, to);
			break;
			}
			}
		}
	 else
		ntree = loc;
	/* if it is a string ref, map it to a symref if requested */
#ifdef notdef
	if( loctype == N_NAME ) {
		if( reref )  {
			/* cursor must be set - find table explicitly? */
			printt1("Map name %s to symbol node", kid_id(ntree));
			s_kid_id(ntree, slookup(kid_id(ntree), CREATE, NULL,
						 MOAN, NIL));
			s_ntype( ntree, N_ID );
			printt1( "%x", kid_id(ntree) );
			}
		return ntree;
		}
#endif
	if( loctype == N_ID || loctype == N_VF_WITH ) {
		if( !reref ) {
			/* cursor must be set - find table explicitly? */
			printt2("Map symbol ref %x to name %s\n", kid_id(ntree), sym_name(kid_id(ntree)) );
			/* this is the kid_id */
			s_str_kid( 0, ntree, 
				allocstring(sym_name( kid_id(ntree) ) ) );
			s_ntype( ntree, N_NAME );
			}
		return ntree;
		}
	/* descend the children and do the appropriate settings */
	maxkid = reg_kids( loc );

	/*
	 * We must also link up the symbol table kid.  This took a while
	 * to find...I hope there aren't any more problems like this lurking
	 * around.
	 */
	if (ntype_info(loctype) & F_SYMBOL)
		maxkid++;

	for( index = 0; index < maxkid; index++ ) {
		nodep nkid;

		nkid = tcopy( node_kid( loc, index ), dcopy, reref );
		if( dcopy ) {
			linkup( ntree, index, nkid );
			}
		}
	return ntree;
#endif HAS_WORKSPACE
}

#ifndef SAI
workspace *
get_workspace( wsname, create )
char *wsname;
int create;		/* should we create it if not there */
{
#ifdef HAS_WORKSPACE
	register workspace *traverse;
	register workspace *new_work;
	workspace *oldws;
	char	*allocnstring();

	/* skip to end of workspace list */
	printt2( "Look for ws %s, create=%d\n", wsname, create );
	for(oldws = traverse = ws_list; traverse;
				traverse = work_next((oldws = traverse)))
		if( !strncmp( wsname, work_name(traverse), MAX_WS_NLEN-1) )
			return traverse;
	if( create ) {
		/* LOTS OF this zeroing code can go if malloc gives 0s */
		work_next(oldws) = new_work = fresh(workspace);
		printt1("Creating new workspace at %x\n", new_work );
		/*s_ntype(new_work, N_NULL); */	/* alloced at 0 */
		/*s_tparent(new_work, NIL); */
		linkup(new_work, 0,make_stub(C_ROOT) );
		/*work_fname(new_work) = NULL; */
		work_name(new_work) = allocnstring(wsname, MAX_WS_NLEN);
		printt2("Linking new ws %x with old ws %x\n", new_work, oldws);
		work_prev(new_work) = oldws;
		/* work_next(new_work) = NULL;*/
		}
	 else
		error( ER(181,"badwork`Workspace %s does not exist"), wsname );

	return new_work;
#endif HAS_WORKSPACE
}

#endif SAI
extern	int	count_suspend;

#ifndef SAI
go_to_workspace( wsname )
char *wsname;			/* command line argument */
{
#ifdef HAS_WORKSPACE
	register workspace *tospace;

	/* find and possibly create the new workspace */
	/* do not offer fragments */
	tospace = get_nws( wsname, TRUE, 0  );
	printt2( "Got to workspace %s at %x\n", work_name(tospace), tospace );
	if( node_2flag( tospace ) & WS_FRAGMENT )
		error( ER(182,"fragment`Can't edit code fragments (yet)") );

	/* save away the attributes of this window */
	work_cursor(curr_workspace) = cursor;
	work_scursor(curr_workspace) = curr_window->start_cursor;
	/* turn off debugging */
	popAllSusp();
	if (work_debug(curr_workspace) & DBF_ON)
		del_r_window();

	/* if it was created, put a program blank in it */
	if( is_a_stub(work_node(tospace)) ) {
		int	our2Flag = node_2flag(curr_workspace);

		init_pwork( tospace );		/* dirties our ws */
		s_node_flag(tospace, 0 );
		s_2flag(tospace,0);

		s_2flag(curr_workspace, our2Flag);
		}
	
	/* set the current workspace an init params for our window */
	curr_workspace = tospace;
	cursor = work_cursor(tospace); 
	curr_window->start_cursor = work_scursor(tospace);
	curr_window->start_pcode = 0;
	curr_window->start_sline = 0;
	curr_window->wspace = tospace;
	curr_window->big_change = TRUE;

	redraw_status = TRUE;

	zap_history();
#endif HAS_WORKSPACE
}

#endif SAI
popAllSusp()
{
	while (count_suspend > 0)
		pop_susp();
}

#ifndef SAI

clear_ws(args)
char *args;
{
	extern char *	M_clearws[];

	if (args || !com_menu(M_clearws, TRUE)) {
		prune(work_node(curr_workspace));
		go_to_workspace(work_name(curr_workspace));
		work_fname(curr_workspace) = NULL;
		clean_curws();
		}
}

	/* copy some selected code into a named workspace */


cp_to_ws(wsname)
char *wsname;
{
#ifdef HAS_WORKSPACE
	register workspace *tospace;
	nodep graftnode;

	grab_range( R_FORCE | R_LEAF );

	if( ntype( sel_node ) == N_PROGRAM )
		error( ER(281,"fragment`Copying of an entire program to a workspace not permitted") );

	/* offer all kinds of workspaces */
	tospace = get_nws( wsname, TRUE, -1 );
	if( tospace == curr_workspace )
		error( ER(183,"copy`Can't copy onto the current workspace") );
	s_2flag( tospace, WS_FRAGMENT );
	if( not_a_stub(work_node( tospace ))  ) {
		warning( ER(237,"stomp`Previous contents of workspace %s deleted"),
			work_name(tospace) );
		prune( work_node(tospace) );
		}
	/* now get the code and move it there */
	if( sel_last < 0 ) {
		graftnode = tcopy( sel_node, TRUE, NULL );
		}
	 else {
		/* copy over a list */
		int index;
		graftnode = FNCAST list_range();
		for( index = 0; index < listcount(FLCAST graftnode); index++ )
			linkup( graftnode,index,tcopy(node_kid(graftnode,index),
						TRUE, NULL ) );
		}
	
	/* we now have the list or node copied in graftnode */
	graft( graftnode, work_node(tospace) );
#endif HAS_WORKSPACE
}

/* get a named workspace for inclusion or goto */

workspace *
get_nws(prename, cflag, getwhat)
char *prename;		/* command line name */
int cflag;		/* creation flag */
int getwhat;		/* what ws to get.  -1 for all, WS_FRAGMENT for frags*/
{
#ifdef HAS_WORKSPACE
	char wsname[MAX_WS_NLEN];	/* workspace name */
	extern char menu_nop[];
	int noprompt;

	if( !prename ) {
		struct menu_data m;
		register workspace *traverse;
		int ret;

		new_menu( &m, "Workspaces" );
		if( cflag )
			add_menu_item( "New Workspace");
		add_menu_item( menu_nop );
		for(traverse = ws_list; traverse; traverse=work_next(traverse))
			if( getwhat < 0 || !((getwhat ^ node_2flag(traverse))
						& WS_FRAGMENT) )
				add_menu_item( work_name(traverse) );
		ret = pop_menu( curr_window->w_desc, &m, (funcptr)0 );
		if( ret == cflag )
			err_return();
		 else
			prename = 0;
		if( cflag && ret == 0 )
			/*noprompt = FALSE*/;
		 else
			prename = m.item[ret];
		}
	 else
		space_strip( prename );	 /* is this risky? */
	/* prename might have been set, so check it again */
	if( !prename  ) {
		getprline( "Workspace Name? ", wsname, MAX_WS_NLEN );
		space_strip( wsname );
		}
	return get_workspace( prename ? prename : wsname, cflag );
#endif HAS_WORKSPACE
}

	/* do a get into the specified location */

graft_ws( wsname, xloc )
char *wsname;		/* argument to command */
nodep xloc;		/* stub to put it on */
{
#ifdef HAS_WORKSPACE
	workspace *cpws;
	register nodep loc = xloc;
	register listp insnode;
	nodep cursymtab;

	if( not_a_stub( loc ) ) 
		error( ER(184,"get`You may only insert a code portion on a placehoder") );
	/* offer fragments only */
	cpws = get_nws( wsname, FALSE, WS_FRAGMENT );
	insnode = FLCAST work_node( cpws );
	if( is_a_stub(insnode) )
		error( ER(185,"empty`Workspace %s is empty"), work_name(cpws) );
	if( is_a_list( insnode ) ) {
		if( not_a_list( tparent(loc) ) )
			error( ER(186,"inslist`You can't insert a list of things here") );
		 else {
			int index;
			/* loop performing insertion */
			hist_room( 2 * listcount(insnode) + 1 );
			cursor = copy_graft( kid1(insnode), loc);
			for( index = 1; index < listcount(insnode); index++){
				exp_list(1);	/* expand out */
				cursor = copy_graft( node_kid(insnode,index),
						cursor);
				}
			}
		}
	 else
		copy_graft( insnode, loc );
#endif HAS_WORKSPACE
}


nodep 
copy_graft( xwhat, xwhere )
nodep xwhat;
nodep xwhere;		/* what stub we graft on */
{
#ifdef HAS_WORKSPACE
	register nodep what = xwhat;
	register nodep where = xwhere;
	register ClassNum insclass;
	nodep ret;

	insclass = ex_class(where);
	if( is_a_stub(what) )
		if( int_kid(0,what) == insclass )
			return where;
		 else
			error( ER(187,"badget`Can't have a %s placeholder here"),
				classname( int_kid(0,what) ) );
	printt3("Check_graft of %x onto %x class %d\n", what, where, insclass);
	if( strchr( Valid_Nodes[insclass], ntype(what) ) ) {
		extern int DontAddDecl;
		nodep	ocurs;
		nodep	block;
/* THIS IS A HUGE BOTCH AND SHOULD BE REDONE */
		ret = tcopy( what, TRUE, TRUE );
		DontAddDecl = TRUE;
		graft(ret, where);
		DontAddDecl = FALSE;

		ocurs = cursor;
		fixNames(ret, 1);		/* regular DECL_IDs only */
		cursor = ocurs;
		fixNames(ret, 2);		/* field DECL_IDs only */
		cursor = ocurs;
		block = my_block(ret);
		c_comp_decls(my_scope(block), block, TC_NOSTUBS);
		fixNames(ret, 3);
		return cursor = ocurs;
		}
	 else
		error( ER(188,"badget`You can't put %s nodes onto a %s placeholder"),
			NodeName( ntype(what) ),  classname( insclass ) );

#endif HAS_WORKSPACE
}

#endif SAI

fixNames(_np, pass)
nodep	_np;
{
	register nodep np = _np;
	register int	i;
	int	type	= ntype(np);
	int	nkids	= reg_kids(np);

	if (type == N_DECL_ID) {
		if (pass == 1 && sym_dtype((symptr)np) != T_FIELD ||
		    pass == 2 && sym_dtype((symptr)np) == T_FIELD)
			add_decl(np);
		}
	else if (type == N_NAME) {
		symptr lookres;
		extern nodep with_symtab;

		with_symtab = NIL;

		cursor = np;		/* eww, gross */
		lookres = slookup(str_kid(0,np), CREATE, NIL, MOAN, NIL );

		if( with_symtab && sym_dtype(lookres) == T_FIELD ) {
			nodep newvfw;
			int kidn;
			nodep npparent;

			npparent = tparent(np);
			kidn = get_kidnum(np);
			/* free the existing node */
			tfree(np);
			newvfw = tree(N_VF_WITH, NCAST lookres, with_symtab );
			linkup( npparent, kidn, newvfw );
			}
		 else {
			s_kid_id(np, lookres);
			s_ntype(np, N_ID);
			}
		}
	else if (pass == 3 || !(type == N_VAR_FIELD || type == N_ST_WITH))
		for (i = 0; i < nkids; i++)
			fixNames(node_kid(np, i), pass);
}
#endif PARSER

/*
 * Set up the tree to be an empty program, so we can start editing.
 */
#ifndef SAI
init_pwork(xws)
workspace *xws;
{
	register workspace *ws = xws;
	nodep savcur;
	extern char tokentext[];	/* input tokens here */

	printt3("New program in workspace %x name %s, work node %x\n",
			ws, work_name(ws), work_node(ws) );
	cursor = work_node(ws);	/* cursor is on unexpanded node */
	dec_block(N_PROGRAM);		/* default setup */
	savcur = cursor = realcp(kid2(work_node(ws)));
			/* the first file parameter */
	exp_list(1);
	strcpy( tokentext, "input" );
	cursor = savcur;
	contok( N_T_COMMENT );
	strcpy( tokentext, "output" );
	contok( N_T_COMMENT );
	work_cursor(ws) = cursor =
			realcp(kid5(work_node(ws)));	/* start on code */
	work_scursor(ws) = work_node(ws);
	work_debug(ws) = 0;
}

#endif SAI
#ifdef HAS_WORKSPACE
dirty_curws()
{
	extern int	redraw_status;

	if (!(node_2flag(curr_workspace) & WS_DIRTY))
		redraw_status = TRUE;
	or_2flag(curr_workspace, WS_DIRTY);
}


#ifndef SAI
int
anyDirtyWS()
{
	register workspace	*wp;
	register int	found;
	char	WSNames[MX_WIN_WIDTH + MAX_WS_NLEN + 3];

	WSNames[0] = '\0';
	found = FALSE;
	for (wp = ws_list; wp; wp = work_next(wp)) {
		if (!(node_2flag(wp) & WS_FRAGMENT) &&
		    (node_2flag(wp) & WS_DIRTY)) {
			if (found)
				strcat(WSNames, ", ");
			else
				found = TRUE;
			strcat(WSNames, work_name(wp));
			if (strlen(WSNames) >= MX_WIN_WIDTH - 1)
				break;
		}
	}
	if (found) {
		message(ER(238,"You haven't saved workspace %s"), WSNames);
		return TRUE;
	} else
		return FALSE;
}
#endif SAI

#else HAS_WORKSPACE

#ifdef Moved to common.c
clean_curws()
{
}
#endif Moved to common.c

dirty_curws()
{
}

anyDirtyWS()
{
	return FALSE;
}

#endif HAS_WORKSAPCE
