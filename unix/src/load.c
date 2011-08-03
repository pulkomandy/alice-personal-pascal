/*
 *DESC: Routines to load programs and libraries
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "flags.h"
#include "menu.h"
#include "class.h"
#include "typecodes.h"
#include "dbflags.h"
#ifdef PARSER
# include "input.h"
#endif

#ifdef DB_LOAD
# define PRINTT
#endif
#include "printt.h"

FILE	*loadf;
static int	sym_no;
static symptr	*builtin_ptrs	= 0;
static Boolean	*BuiltinLookedUp;			/* for merge */
static Boolean	Merge;
static symptr	*sym_ptrs	= 0;
static unsigned	load_flags;
static int	WSDestroyed;
static nodep	GlobalSymtab;

extern funcptr	mem_cleanup;
extern int	cleanup();
extern FILE	*qfopen();
extern char	*get_string();
extern nodep	load_tree();
extern char	getbyte();
extern int	getword();

nodep	_load();

#if defined(msdos) || defined(ATARI520ST)
#define OPstr "rb"
#else
#define OPstr "r"
#endif

#define NL 10

nodep ltree_root = (nodep)NIL;		/* root of loaded tree */

#if defined(SAI) || defined(PARSER) || defined(DUMP_LOAD)
/* Nothing */
#else 
#define EDITOR	1
#endif

#ifdef DUMP_LOAD
extern int dump_indent;
#else
#define Dumpload(a,b,c)
#endif

/* pre-declarations of static status */
static merge_file();
static load_misc();
static setup_builtin_ptrs();
static free_builtin_ptrs();
static alloc_sym_ptrs();
static free_sym_ptrs();

#if defined(EDITOR) || defined(SAI)

static destroy_ws();

aload(merge, file_args)
int	merge;
char	*file_args;
{
	nodep	top;
	char	*dotp;
	char	name[PATHLEN];
	extern char	*strrchr();

	Merge = merge;

	/* Clear the selection */
#ifndef SAI
	grab_range(0);
	imm_abort();
#endif


	filename(name, file_args, TRUE, FileExt, TRUE);

	dotp = strrchr(name, '.');
	if (dotp && !strcmp(dotp+1, TextExt))
		textToAP(name, dotp);

	ltree_root = (nodep)NIL;

	if ((loadf = fopen(name, OPstr)) == NULL) {
		warning(ER(218,"badsavefile`Can't open file \"%s\"\n"),
			name);
		return TRUE;	/* failure */
	}

	mem_cleanup = cleanup;

#ifndef SAI
	popAllSusp();		/* free suspended states */
#endif

	WSDestroyed = FALSE;

#ifndef SAI
	if (merge != MERGE) {
		/*
		 * If not a merge operation, free the old workspace,
		 * to try to get more memory.
		 *
		 * If the current workspace hasn't been saved out, then
		 * confirm the load.
		 */
		if (work_dirty(curr_workspace) ) {
#ifdef GEM
			if( form_alert( 1,
 "[2][You have not saved this workspace|Load anyway ?][Yes|No]" ) == 2 ) {
#else
			if( !com_menu(M_load, TRUE)) {
#endif
				fclose(loadf);
				return TRUE;
			}
		}
		destroy_ws();
	}
	zap_history();

	slmessage( ER(313,"Loading \"%s\""), name );
#endif

	top = _load();

#ifndef SAI
	{ extern int can_dump; can_dump = TRUE; }
	dump(top);
	if (merge == MERGE) {
		merge_file(top);
		}
	else {
		load_file(top);
		newname(name);
		}
#else
	load_file(top);
	newname(name);
#endif

	block_typecheck( cur_work_top );

	cleanup(TR_OK);
	return FALSE;
}

#endif

#ifdef PARSER
FILE *
qfopen( fnam, opts )
char *fnam, *opts;
{
	extern FILE *fopen();
	return fopen( fnam, opts );
}

LoadLibrary(name)
char	*name;
{
	printt1("LoadLibrary(%s)\n", name);

	ltree_root = (nodep)NIL;

	if ((loadf = qfopen(name, OPstr)) == NULL)
		fatal("can't open library \"%s\"\n", name);
	LibTree = _load();
	LibSymtab = sym_kid(LibTree);
}

LoadRestOfLibrary()
{
	int	i;
	int	n_kids;

	printt0("LoadRestOfLibrary\n");

	n_kids = kid_count(N_PROGRAM);
	for (i = 0; i < n_kids; i++)
		load_tree(LibTree, i);
	cleanup(TR_OK);
}

AbandonRestOfLibrary()
{
	cleanup(TR_OK);
}

#endif

nodep
_load()
{
	load_misc();		/* loads flags, among other things */

	setup_builtin_ptrs();
	alloc_sym_ptrs();

	sym_no = 0;

#ifdef EDITOR
	GlobalSymtab = sym_kid(cur_work_top);
#endif
	return load_tree((nodep)NIL, 0);
}

#ifdef EDITOR
static
destroy_ws()
{
	if (work_node(curr_workspace))
		prune(work_node(curr_workspace));
	WSDestroyed = TRUE;
}

static
restore_ws()
{
	if (WSDestroyed) {
		if (work_debug(curr_workspace) & DBF_ON)
			del_r_window();
		init_pwork(curr_workspace);
		zap_history();
	}
}

#endif EDITOR

long errloc;

cleanup(err)
int	err;
{
	extern long ftell();
#ifdef EDITOR
	extern nodep WhereDeclsChanged;
	extern nodep CheckAt;

	WhereDeclsChanged = (nodep)NIL;
	CheckAt = (nodep)NIL;
#endif EDITOR

	mem_cleanup = (funcptr)0;

	free_builtin_ptrs();
	free_sym_ptrs();
	errloc = ftell(loadf);
	fclose(loadf);

#ifdef EDITOR
	/*
	 * Since we have freed the original program tree, we must
	 * leave the poor user *something*...
	 */
	if (err == TR_ERR || err == ABORT ) {
		treefree( ltree_root );
		restore_ws();
		newname( (char *)0 );
		}
	ltree_root = (nodep)NIL;

	zap_history();

	show_whole_doda();
	if( err == ABORT ) 
		error( ER(66,"loadmem`Not enough memory to load program") );
#endif
}

#ifndef PARSER
# ifdef SAI
load_file(top)
nodep	top;
{
	cur_work_top = top;
	linkup(NCAST curr_workspace, 0, cur_work_top);
}
#else
load_file(top)
nodep top;
{
	nodep symtab; 	/* symbol table */
	register symptr symp;
	symptr prev, next;	/* symbols in search */

	graft(top, work_node(curr_workspace));
	set_wscur(curr_window, cursor = cur_work_top = top);
	curr_window->big_change = TRUE;
	/* clear out the undefineds */
	symtab = sym_kid( top );	/* better be N_PROGRAM */
	prev = (symptr)0;
	for( symp = (symptr)kid1(symtab); symp; symp = next ) {
		next = sym_next( symp );
		if( sym_dtype( symp ) == T_UNDEF
				&& !(sym_mflags(symp) & SF_REFERENCED) ) {
			/* delete this guy */
			if( prev )
				s_sym_next( prev, next );
			 else
				s_kid1( symtab, NCAST next );
			s_tparent( symp, NIL );
			treefree( symp );
			}
		 else
			prev = symp;
		}

	zap_history();
	clean_curws();
}
#endif SAI
#endif PARSER

#ifdef EDITOR
/*
 * A merge is done just like a load, but instead
 * of destroying the previous contents of this workspace, we
 * merge the tree into the current program
 */
static
merge_file(top)
nodep top;
{
	register int	i;
	register listp	lp;
	int	kid;

	/*
	 * Merge the comments, decls and statement kids of top into the
	 * current workspace.
	 */
	for (kid = 2; kid <= 4; kid++) {
		lp = node_kid(top, kid);
		cursor = kid1(node_kid(cur_work_top, kid));
		exp_list(0);
		graft(node_kid(lp, 0), cursor);
		for (i = 1; i < listcount(lp); i++) {
			printt1("before exp_list, cursor = %x\n", cursor);
			exp_list(1);
			printt1("before graft, cursor = %x\n", cursor);
			graft(node_kid(lp, i), cursor);
		}
	}

	set_wscur( curr_window, cursor = cur_work_top );
	curr_window->big_change = TRUE;
}
#endif EDITOR

static
check_format(okay, where)
int	okay;
char	*where;
{
	if (!okay) {
#ifdef PARSER
		fatal("Bad library save format (%s)", where);
#else
# ifdef SAI
		fprintf(stderr, "Bad save format (%s)\n", where);
		done();
# else
		cleanup(TR_ERR);
		error(ER(67,"badformat`bad save format (%s)"), where);
# endif
#endif
	}
}

static char	notSavefile[]	= "not an ALICE Pascal save file";

static int intsize = 2;		/* size of ints in load file */

static
load_misc()
{
	int	version;

	check_format(getbyte(loadf) == 'A', notSavefile);
	check_format(getbyte(loadf) == 'P', notSavefile);

	intsize = 2;

	version = getword(loadf);
	check_format(version >= SAVE_VERSION, "unsupported version");
	if( version > SAVE_VERSION )
		warning(ER(191,"badformat`Save format %d is a later release, load at own risk"));

	load_flags = getword(loadf);

	Dumpload( "Version number %d, flags %x\n", version, load_flags );

	intsize = getword(loadf);
	check_format(intsize <= sizeof(int), "integers too large");
}

static
setup_builtin_ptrs()
{
	register symptr	sp;
	register int	size;
	register int	i;

	size = max(getword(loadf), built_count);

	printt1("%d builtin symbols\n", size);

	builtin_ptrs = (symptr *)checkalloc(sizeof(symptr) * (size + 1));

	for (i = 0; i <= size; i++)
		builtin_ptrs[i] = Tptr(UnknownSymbol);

	for (sp = (symptr)kid1(Tptr(master_table));sp!= NULL; sp = sym_next(sp)) {
		/* Extra cautious, we've been bitten before! */
		i = -sym_saveid(sp);
		if (i >= 0 && i < size)
			builtin_ptrs[i] = sp;
		}
	for (sp = Tptr(SP_number); sp != NULL; sp = sym_next(sp)) {
		/* Extra cautious, we've been bitten before! */
		i = -sym_saveid(sp);
		if (i >= 0 && i < size)
			builtin_ptrs[i] = sp;
		}

#ifdef EDITOR
	if (Merge) {
		BuiltinLookedUp = (Boolean *)checkalloc(sizeof(Boolean) * (size + 1));
		for (i = 0; i <= size; i++)
			BuiltinLookedUp[i] = FALSE;
		}
#endif
}

static
free_builtin_ptrs()
{
	if (builtin_ptrs) {
		dmfree(builtin_ptrs);
		builtin_ptrs = 0;
		}

#ifdef EDITOR
	if (BuiltinLookedUp) {
		dmfree(BuiltinLookedUp);
		BuiltinLookedUp = 0;
		}
#endif
}

static
alloc_sym_ptrs()
{
	int i;
	int size = getword(loadf);
	sym_ptrs = (symptr *)checkalloc(sizeof(symptr) * size);
	for (i = 0; i < size; i++)
		sym_ptrs[i] = Tptr(UnknownSymbol);
}

static
load_syms(symtab_node, parent)
nodep symtab_node;
nodep parent;
{
	register symptr	sp;
	register symptr	lastsp;
	register bits8	type;
	char ls_buffer[MAX_LEN_LINE];

#ifdef EDITOR
	stackcheck();
#endif

	s_ntype(symtab_node, getbyte(loadf));

	check_format(ntype(symtab_node) == N_SYMBOL_TABLE, "bad symbol table");

	s_tparent(symtab_node, parent);
	s_node_flag(symtab_node, getbyte(loadf));
	Dumpload( "ISymbol table :\n",0,0 );
	s_2flag(symtab_node, getbyte(loadf));

	lastsp = NULL;
	while ((type = getbyte(loadf)) == N_DECL_ID) {
#ifdef DUMP_LOAD
		sp = (symptr)checkalloc(sizeof(struct symbol_node));
#else
		sp = (symptr)talloc(sizeof(struct symbol_node));
#endif
		sym_ptrs[sym_no] = sp;
		s_ntype(sp, type);
		s_node_flag(sp, getbyte(loadf));
		s_2flag(sp, getbyte(loadf));
		s_sym_dtype(sp, getword(loadf));
#ifdef SAI
		/* If we are the SAI, don't allocate memory for the sym names */
		get_string( ls_buffer, loadf );
		if( sym_dtype( sp ) == T_ENUM_ELEMENT ) 
		     s_sym_name(sp, allocstring( ls_buffer ) );
		 else
		     s_sym_name(sp, "<var>" );
#else
		s_sym_name(sp, allocstring(get_string(ls_buffer, loadf)));
		printt1("loaded %s\n", sym_name(sp));
#endif SAI
		s_sym_nhash(sp, hash(sym_name(sp)));
		s_sym_value(sp, (pint)getword(loadf));
		s_sym_saveid(sp, sym_no);
		Dumpload("I  Symbol %s, dtype=%d\n",sym_name(sp),sym_dtype(sp));
		/*s_sym_type(sp, (nodep )NULL); */
		/*sym_mflags is also null */
		s_sym_next(sp, lastsp);
		if( node_2flag( sp ) & NF2_EXTRA ) {
			int extras;
			/* ignore extra bytes we don't understand */
			extras = getbyte(loadf);
			while( extras-- )
				getbyte(loadf);
			}
		lastsp = sp;
		sym_no++;
	}

	s_kid1(symtab_node, NCAST lastsp);
}

static
free_sym_ptrs()
{
	if (sym_ptrs) {
		dmfree(sym_ptrs);
		sym_ptrs = 0;
	}
}

#ifdef EDITOR
extern inchar tokentext[MAX_LEN_LINE];
#define lt_bf ((char *)tokentext)
#else
char lt_bf[MAX_LEN_LINE];
#endif

nodep 
load_tree(parent, whatkid)
nodep parent;
unsigned int whatkid;		/* what child is this, who laid to rest? */
{
	register nodep np;
	register int	i;
	int	n_kids;
	int	type;
	int	temp;
	extern listp sized_list();

#ifdef EDITOR
	stackcheck();
#endif

	type = getbyte(loadf);
	n_kids = kid_count(type);
	printt3( "Loading for type %d, kid_count %d, parent %x\n", type, n_kids,
			parent );
#ifdef DUMP_LOAD
	dump_indent++;

	Dumpload( "INode <%s>(%d):", NodeName(type), n_kids );
#endif

	if (!valid_node(type)) {
badnode:
#ifdef PARSER
		fatal("strange type %d in library", type);
#else
#ifdef SAI
		fprintf(stderr, "Internal error: strange type %d in file", type);
		done();
#else
		cleanup(TR_ERR);
		error(ER(68,"badnode`strange type %d in file"), type);
#endif
#endif
		}

	if (t_is_standard(type)) {
		np = tree(type, NIL, NIL, NIL, NIL, NIL, NIL, NIL);
		printt1( "New tree in load is %x\n", (int)np );
		s_node_flag(np, getbyte(loadf));
		s_2flag( np, getbyte(loadf) );

		if( node_2flag( np ) & NF2_EXTRA ) {
			int extras;
			/* ignore extra bytes we don't understand */
			extras = getbyte(loadf);
			while( extras-- )
				getbyte(loadf);
			}

		switch(type) {
		case N_HIDE:
			n_kids++;
			Dumpload( "\n", 0, 0 );
			break;
		case N_STUB:
			{
			int theclass;
			s_int_kid(0, np, theclass = getword(loadf));
			Dumpload( " Class %d\n", theclass,0);
			}
			break;
		case N_T_COMMENT:
		case N_EXP_ERROR:
#ifdef SAI
			get_string( lt_bf, loadf );
			s_str_kid(0,np, "<comment>" );
#else
			s_str_kid(0,np, allocESString(get_string(lt_bf, loadf)));
#endif
			Dumpload( " String \"%s\"\n", lt_bf, 0 );
			break;
		case N_CON_CHAR:
			{
			register int thec;
			s_int_kid(0,np,thec = getword(loadf));
			Dumpload( " Char %d '%c'\n", thec,thec);
			}
			break;
		case N_CON_INT:
		case N_CON_REAL:
		case N_CON_STRING:
#ifdef PARSER
			s_str_kid(0,np,allocstring(get_string(lt_bf, loadf)));
#else
			s_conval(np, type, get_string(lt_bf, loadf));
#endif
			Dumpload( " Constant [%s]\n", lt_bf, 0 );
			break;
		case N_PROGRAM:
		case N_DECL_PROC:
		case N_DECL_FUNC:
		case N_TYP_RECORD:
		case N_IMM_BLOCK:
		case N_FORM_PROCEDURE:
		case N_FORM_FUNCTION:
			{
			nodep thetab;

			thetab = (nodep)fresh(struct standard_node);
			s_node_kid(np, n_kids, thetab );
			load_syms(thetab, np);
			printt2( "symbol table %x should be %x\n", thetab, sym_kid(np) );
			break;
			}
		case N_VF_WITH: /* should not be a standard nodep */
			{
			symptr tsym;
			tsym = sym_ptrs[getword(loadf)];
			Dumpload( " Ref to '%s'\n", sym_name(tsym), 0 );
			or_sym_mflags( tsym, SF_REFERENCED );
			s_kid_id( np, tsym );
			}
			break;
		default:
			Dumpload( "\n", 0, 0 );
			break;
		}
	} else switch(type) {
	case N_LIST:
		n_kids = getword(loadf);
		np = FNCAST sized_list(n_kids);
		Dumpload( " - List size %d\n", n_kids, 0 );
		break;
	case N_DECL_ID:
		{
		int symword;
		symword = getword(loadf);
		printt1( "getting decl_id symbol number %d\n", symword );
		np = NCAST sym_ptrs[symword];	/* always +ve */
		Dumpload( " Decl of '%s'\n", sym_name( (symptr)np ), 0 );
		}
		break;
	case N_ID:
		{
		int temp;
		temp = getword(loadf);
		if (temp >= 0) {
			symptr tsym;
			tsym = sym_ptrs[temp];
			Dumpload(" Ref to %d/'%s'\n",temp,sym_name(tsym));
			or_sym_mflags( tsym, SF_REFERENCED );
			np = tree(type, NCAST sym_ptrs[temp], NIL);
			}
		else {
			temp = -temp;
#ifdef EDITOR
			if (Merge && !BuiltinLookedUp[temp]) {
				symptr	decl	= builtin_ptrs[temp];

				BuiltinLookedUp[temp] = TRUE;
				/* This has problems if you try to look up
				 * a builtin field... */
				decl = slookup(sym_name(builtin_ptrs[temp]),
						NOCREATE, GlobalSymtab,
						NOMOAN, NIL);
				if (decl != Tptr(null_undef))
					builtin_ptrs[temp] = decl;
				}
#endif
			np = tree(type, NCAST builtin_ptrs[temp], NIL);
			Dumpload(" Ref Built %d/'%s'\n",temp,sym_name(builtin_ptrs[temp]));
			}
		}
		break;
	case N_NULL:
		np = tree(N_NULL, NIL);
		break;
	default:
		/* "can't happen" now that we test valid_node() */
		goto badnode;
	}

	if( parent )
		linkup( parent, whatkid, np );
	 else
		ltree_root = np;

#ifdef PARSER
	/*
	 * We are loading a library, and only wish to read as far as
	 * the symbol table for now.  Later we may load the subtrees...
	 */
	if (ntype(np) == N_PROGRAM)
		return np;
#endif

	for (i = 0; i < n_kids; i++)
		load_tree(np, i);
#ifdef DUMP_LOAD
	dump_indent--;
#endif

	return np;
}

/*
 * Return an integer
 */
int
getword(fp)
FILE *fp;
{
	register int	rw;
	int i;
	register int ch;

	rw = 0;
	for( i = 0; i < intsize; i++ ) {
		ch = getc(fp) & 0xff;
		rw |= ch << (i*8);
		}
	if( ch & 0x80 && i < sizeof(int) )
		return rw | ( -1 << (i*8) );
	 else
		return rw;

}


static char PreemEOF[] = "premature end of file";

/*
 * Stupid fgets doesn't strip off the ending newline!
 * The stupid Icon thinks newline ('\n') is 0x1e!
 */
char *
get_string(buf, f)
char	*buf;
FILE	*f;
{
	register char	*s = buf;
	int	ch;

	while ((ch = getc(f)) != NL && ch != EOF) {
		*s++ = ch;
		}
	if (ch == EOF)
		check_format(FALSE, PreemEOF);
	*s = '\0';

	return buf;
}

char
getbyte(fp)
FILE	*fp;
{
	int	c;

	c = getc(fp);
	if (c == EOF)
		check_format(FALSE, PreemEOF);

	return c;
}
