/*
 * Routines to load programs and libraries
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

#ifdef EDITOR
extern inchar tokentext[MAX_LEN_LINE];
#define lt_bf ((char *)tokentext)
#else
char lt_bf[MAX_LEN_LINE];
#endif

FILE	*loadf;
static int	sym_no;
static symptr	*builtin_ptrs	= 0;
static Boolean	*BuiltinLookedUp;			/* for merge */
static Boolean	Merge;
static symptr	*sym_ptrs	= 0;
static unsigned	load_flags;
static int	WSDestroyed;
static nodep	GlobalSymtab;
int built_size;		/* Number of builtins allocated in setup_builtin_ptr */

extern funcptr	mem_cleanup;
extern int	cleanup();
extern FILE	*qfopen();
extern char	*get_string();
extern nodep	load_tree();
extern char	getbyte();
extern int	getword();

nodep	_load();

#if defined(msdos) || defined(TOS)
#define OPstr "rb"
#else
#define OPstr "r"
#endif

#define NL 10

#ifdef HAS_LIBRARY
char    *codeptr;		/* Ptr to actual lib code */
char	*codeblk;		/* Ptr to Info Block */
#endif

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

#if defined(EDITOR) || defined(SAI)
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

	mem_cleanup = cleanup;

	filename(name, file_args, TRUE, FileExt, TRUE);

	dotp = strrchr(name, '.');
	if (dotp && case_equal(dotp+1, TextExt))
		textToAP(name, dotp);

	ltree_root = (nodep)NIL;

	if ((loadf = qfopen(name, OPstr)) == NULL) {
		warning(ER(218,"badsavefile`Can't open save file \"%s\""),
			name);
		return TRUE;	/* failure */
	}

#ifndef SAI
	popAllSusp();		/* free suspended states */
#endif

	WSDestroyed = FALSE;

#ifndef SAI
	if (merge == LOAD) {
		/*
		 * If we are loading a file (as opposed to merging it)
		 * or loading a library, free the old workspace,
		 * to try to get more memory.
		 *
		 * If the current workspace hasn't been saved out, then
		 * confirm the load.
		 */
		if (work_dirty(curr_workspace) ) {
#ifdef GEM
			if( form_alert( 1, LDS( 440, 
 "[2][You have not saved|this workspace|Load anyway ?][Yes|No]" )) == 2 ) {
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

	/* Go and load the file */
	top = _load( merge == LIBRARY );


#ifndef SAI
	{ extern int can_dump; can_dump = TRUE; }
	dump(top);

	/* We are merging the file */
	if (merge == MERGE) {
		merge_file(top);
		}
	/* We are installing a library */
	else 
#ifdef HAS_LIBRARY
		if( merge == LIBRARY )
			install_library(top, name);
	else
#endif
		{
		/* We are loading a completely new file */
		load_file(top);
		newname(name);
		}
#else
	load_file(top);
	newname(name);
#endif

#ifdef HAS_LIBRARY
	/* Go through the tree and load in any libraries that
	 * might be there
	 */
	search_lib( cur_work_top );
#endif

	cleanup( OK );

	block_typecheck( cur_work_top );

#ifdef GEM
#ifndef SAI
	count_lines();
#endif
#endif
	return FALSE;
}

#endif


#ifdef PARSER
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
	cleanup(OK);
}

AbandonRestOfLibrary()
{
	cleanup(OK);
}

#endif

nodep
_load( islibrary )
int islibrary;
{
	nodep n;

#ifdef GEM
	BusyBee( TRUE );
#endif
	load_misc();		/* loads flags, among other things */

	setup_builtin_ptrs();
	alloc_sym_ptrs();

	sym_no = 0;

#ifdef EDITOR
	GlobalSymtab = sym_kid(cur_work_top);
#endif
	n = load_tree((nodep)NIL, 0, islibrary );

#ifdef GEM
	BusyBee( FALSE );
#endif
	return n;
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
Boolean	err;
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
	if (err == ERR || err == ABORT ) {
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
		cleanup(ERR);
		error(ER(67,"badformat`bad save format (%s)"), where);
# endif
#endif
	}
}

/*
 * NOTE: We should check to see if we are trying to load a library
 * file into the main workspace.  If we are, issue an error
 */
static
load_misc()
{
	int	version;
	char    *nsf;
	char	c;

	nsf = LDS( 444, "not an ALICE Pascal save file" );

	/* Now we can read in the file */
	check_format( getbyte(loadf) == 'A', nsf);

	/* See whether or not it is a binary library or a pascal library */
	c = getbyte(loadf);
	check_format( c == 'P' || c == 'L', nsf);

#ifdef HAS_LIBRARY
	/* If a binary library, load it in now */
	if( c == 'L' ) {
		extern char *libfload();

		/*
		 * Go and load the binary part of the library
		 */
		codeblk = libfload( loadf, &codeptr );

	} else
		codeblk = (char *)0;

#endif HAS_LIBRARY

	version = getword(loadf);
	check_format(version >= SAVE_VERSION, LDS( 445, "unsupported version" ));
	if( version > SAVE_VERSION )
		warning(ER(191,"badformat`Save format %d is a later release, load at own risk"));

	load_flags = getword(loadf);

	Dumpload( "Version number %d, flags %x\n", version, load_flags );

	check_format(getword(loadf) == sizeof(int), LDS( 446, 
			"wrong sized integers"));
}

static
setup_builtin_ptrs()
{
	register symptr	sp;
	register int	i;

	built_size = max(getword(loadf), built_count);

	printt1("%d builtin symbols\n", built_size);

	builtin_ptrs = (symptr *)checkalloc(sizeof(symptr) * (built_size + 1));

	for (i = 0; i <= built_size; i++)
		builtin_ptrs[i] = Tptr(UnknownSymbol);

	for (sp = (symptr)kid1(Tptr(master_table));sp!= NULL; sp = sym_next(sp)) {
		/* Extra cautious, we've been bitten before! */
		i = -sym_saveid(sp);
		if (i >= 0 && i < built_size)
			builtin_ptrs[i] = sp;
		}
	for (sp = Tptr(SP_number); sp != NULL; sp = sym_next(sp)) {
		/* Extra cautious, we've been bitten before! */
		i = -sym_saveid(sp);
		if (i >= 0 && i < built_size)
			builtin_ptrs[i] = sp;
		}

#ifdef EDITOR
	if (Merge) {
		BuiltinLookedUp = (Boolean *)checkalloc(sizeof(Boolean) *
 					(built_size + 1));
		for (i = 0; i <= built_size; i++)
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
load_syms(symtab_node, parent, islibrary)
nodep symtab_node;
nodep parent;
int islibrary;	/* Flag that says whether or not we are loading a library */
{
	register symptr	sp;
	register symptr	lastsp;
	register bits8	type;

#ifdef EDITOR
	stackcheck();
#endif

	s_ntype(symtab_node, getbyte(loadf));

	check_format(ntype(symtab_node) == N_SYMBOL_TABLE, LDS( 447,
			"bad symbol table"));

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
		get_string( lt_bf, loadf );
		if( sym_dtype( sp ) == T_ENUM_ELEMENT ) 
		     s_sym_name(sp, allocstring( lt_bf ) );
		 else
		     s_sym_name(sp, "<var>" );
#else
		s_sym_name(sp, allocstring(get_string(lt_bf, loadf)));
		printt1("loaded %s\n", sym_name(sp));
#endif SAI
		s_sym_nhash(sp, hash(sym_name(sp)));
		s_sym_value(sp, (pint)getword(loadf));

#ifdef HAS_LIBRARY
		/* If we are loading a library, and the symbol refers to
		 * a T_LFUNC or a T_LPROC, then make the sym_value point
		 * to the actual code of the binary proc/func
		 */
		if( sym_dtype( sp ) == T_LFUNC || sym_dtype(sp) == T_LPROC )
			s_sym_value( sp, sym_value(sp) + codeptr );
	
#endif HAS_LIBRARY
		s_sym_saveid(sp, sym_no);
		Dumpload("I  Symbol %s, dtype=%d\n",sym_name(sp),sym_dtype(sp));
		/*s_sym_type(sp, (nodep )NULL); */
		/*sym_mflags is also null */
#ifdef HAS_LIBRARY
		/*
		 * If we are loading a library, then set the SF_LIBRARY
		 * bit on the mflags, so that when we save the file out,
		 * 'save' knows whether each symbol is a regular symbol
		 * or a library symbol
		 */
		if( islibrary )
			s_sym_mflags( sp, SF_LIBRARY );

#endif HAS_LIBRARY

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

nodep 
load_tree(parent, whatkid, islibrary)
nodep parent;
unsigned int whatkid;		/* what child is this, who laid to rest? */
int islibrary;		/* Whether we are loading from a library or not */
{
	register nodep np;
	register int	i;
	int	n_kids;
	bits8	type;
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
		cleanup(ERR);
		error(ER(68,"badnode`strange type %d in file"), type);
#endif
#endif
		}

	if (t_is_standard(type)) {
		np = tree(type, NIL, NIL, NIL, NIL, NIL, NIL, NIL);
		printt1( "New tree in load is %x\n", (int)np );
		s_node_flag(np, getbyte(loadf));
		s_2flag( np, getbyte(loadf) );

		/* Load any extra stuff */
		load_extra( node_2flag( np ) );

		switch(type) {
		case N_HIDE:
			n_kids++;
			Dumpload( "\n", 0, 0 );
			break;
#ifdef HAS_LIBRARY
		/*
		 * Load in a library node and the file that is associated
		 * with it
		 */
		case N_LIBRARY:
			/* library load - just load the filename, etc */
			library_load(np);
			/* n_kids is 1, comment will be loaded */
			break;
#endif
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
		case N_DECL_PROC:
		case N_DECL_FUNC:
		case N_PROGRAM:
		case N_TYP_RECORD:
		case N_IMM_BLOCK:
		case N_FORM_PROCEDURE:
		case N_FORM_FUNCTION:
			s_node_kid(np, kid_count(type),
				   NCAST talloc(sizeof(struct standard_node)));
			load_syms(sym_kid(np), np, islibrary );
			break;
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
		load_tree(np, i, islibrary);
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
	int ch1, ch2;

	ch1 = getc(fp);
	ch2 = getbyte(fp);
	rw = (ch1 & 0xff) | (ch2 << 8);

	/*
	 * If your machine has 32 bit integers, negative integers will
	 * end up as integers greater than 32767.  It might be better to
	 * have defined a signed-16-bit type...
	 */
#ifdef INT32BITS
	if (rw & 32768)
		rw -= 65536;
#endif
	return rw;
}


#ifndef OLMSG
static char PreemEOF[] = "premature end of file";
#endif

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
		check_format(FALSE, LDS( 448, PreemEOF ));
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
		check_format(FALSE, LDS( 448, PreemEOF ));

	return c;
}

#ifdef HAS_LIBRARY

/*
 * Pointer to the binary loaded
 */
char *codeptr;

/*
 * This is the routine that is called from the regular load stuff
 * to load a library - NOTE: it does not actually load the library
 * at this time, it just loads the filename of the library file
 * The library will be loaded after the main program has been
 * loaded, and will then be grafted on.
 */

library_load( np )
nodep np;	/* Library Node - Kids not filled in yet */
{
	/* Read the filename of the library and store it in kid 2 */
	s_str_kid(2,np, allocESString(get_string(lt_bf, loadf)));

}

/*
 * Create a new LIBRARY node - used by 'install library' command
 */
lib_declare( blocknode )
nodep blocknode;	/* block to hang declaration off */
{
	bits8 *tokent;
	register listp decl_list;	/* list of declarations */
	register nodep decblock;    /* block of declarations as we like 'em */
	bits8 glob_decl;	/* what kind of declaration we look for */
	int num_decl;		/* number of declaration types */
	register int i;		/* loop through listed declaration classes */
	int decstub;		/* on a declarations stub */
	extern nodep find_true_parent();

	glob_decl = N_LIBRARY;

	decl_list = decl_kid(blocknode);
	num_decl = listcount(decl_list);
	/* Make sure we are on a declaration stub at the main level */
	decstub = is_a_stub(cursor) && int_kid(0,cursor) == C_DECLARATIONS &&
		( ntype(find_true_parent( cursor )) == N_PROGRAM );
	/*
	 * Declaration of a library node.  If the cursor
	 * is on a declaration stub, add the node there, otherwise
	 * add it to the end of the declaration list.
	 */
	if (!decstub) {
		cursor = node_kid(decl_list, num_decl-1);
		if (not_a_stub(cursor))
			exp_list(1);
	}

	/*
	 * Now we have to graft on the library node
	 */
	do_exp_prod( N_LIBRARY, 0 );

	/* sets 'cursor' to point to the library node */
}

/*
 * Graft the library file onto the library node in the main tree
 * 'cursor' should be pointing to the library node
 */
add_library( top )
nodep top;
{
	int nk;
	int index;
	nodep nlist;
	int lkid;

	/*
	 * The list of declarations to add is in kid4 of the library
	 * file
	 */
	nlist = kid4( top );

	/*
	 * Set the 'code pointer' of the library node to be the loaded
	 * binary library code block.  If it is just a regular pascal
	 * library, then codeblk will be NIL
	 */
	s_kid4( cursor, codeblk );

	/*
	 * Graft each previously deleted node onto the newnode's list,
	 * which is its 'lkid'th child.  This assumes node_kid(cursor,1)
	 * is a list.
	 */
	cursor = kid1(kid2(cursor));
	for (index = listcount(nlist) - 1; /* test in middle */; index--) {
		graft(node_kid(nlist, index), cursor);
		if (index == 0)
			break;
		exp_list(0);
		}

	/*
	 * Now we should free up the storage associated with the other
	 * goop in the tree
	 */

}

#endif HAS_LIBRARY

/*
 * If the NF2_EXTRA flag is on, load the extra stuff
 */
load_extra( flag )
int flag;
{
	if( flag & NF2_EXTRA ) {
		int extras;
		/* ignore extra bytes we don't understand */
		extras = getbyte(loadf);
		while( extras-- )
			getbyte(loadf);
	}
}

#ifdef HAS_LIBRARY

/*
 * Search through a file and see if there are any libraries to
 * load
 */
int
search_lib( top )
nodep top;
{
	register int i;
	register int n_kids;
	int flag;

	if( !top ) return FALSE;

	if( (ntype( top ) == N_LIBRARY) && !kid2( top ) ) {
		/* This is a library node - go and load it in and graft
		 * it onto the tree
	 	 */
		char *name;
		FILE *saveld;
		nodep lib;

		saveld = loadf;

		name = getESString( kid3( top ) );
		loadf = qfopen( name, OPstr );
		if (loadf == NULL) {
			warning(ER(321,"badsavefile`Can't open library \"%s\""),
			name);
			fresh_stub( top, 1 );
			s_node_flag( top, NF_ERROR );
			/* Turn on the error flag for the node */
			loadf = saveld;
			return TRUE;	/* failure */
		}
		
		/*
		 * Okay, we now have the opened file... go and load it
		 */
		lib = _load( TRUE );

		/*
		 * Now graft it onto the main tree
		 */
		cleanup( OK );

		loadf = saveld;

		/* First we must create a list stub in kid2 of the node */
		fresh_stub( top, 1 );
		cursor = top;

		/* And go and add the library */
		add_library( lib );

		/* Now recurse through the decls just loaded */
		if( search_lib( kid2( top ) ) ) {
			/* If there were errors in the kids, turn on
			 * the error flag here
			 */
			s_node_flag( top, NF_ERROR );
			return TRUE;
		}
		return FALSE;

	}

	/*
	 * Otherwise, check all the children nodes
	 */
	n_kids = regl_kids( top );
	flag = FALSE;
	for( i = 0; i < n_kids; i++ ) {
		/* If there was an error - return TRUE */
		if( search_lib( node_kid( top, i ) ) )
			flag = TRUE;
	}
	return flag;
}

/*
 * Given the load tree and the filename, graft the library onto the
 * main workspace
 */
install_library( top, name )
nodep top;
char *name;
{
	nodep libnode;

	/* Create a library node and put the cursor on it */
	lib_declare( cur_work_top );

	/* Store the filename of the library */
	s_str_kid( 2, cursor, allocESString( name ) );

#ifdef DEBUG
	printt0("Dump of library tree\n" );
	dump( top );
#endif

	/*
	 * Add_library graft's top onto the current cursor position
	 */
	add_library( top );
}
#endif HAS_LIBRARY
