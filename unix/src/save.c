/*
 *DESC: Write .ap files to the disk
 */

/* Is this correct? */
#define INTERPRETER 1


#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "flags.h"
#include "menu.h"

#if defined(SAI) || defined(MAP)
# define NOT_EDITOR
#endif

#ifdef PROC_INTERP
# include "process.h"
#endif
#ifdef INTERP
# undef HAS_SAVE
#endif
#ifdef DEMO
# undef HAS_SAVE
#endif
#ifndef LISTER

#define NL	'\012'

static	short	Syms;

static	FILE	*Savef;

#if defined( msdos ) || defined( TOS )
#define OPst "wb"
#else
#define OPst "w"
#endif

#if defined(PARSER) || defined(MAP)
# define NOT_EDITOR
#endif

static saveMisc();
static treeCountSyms();

save(flags, safetest)
int	flags;
int safetest;		/* should we test if it exists? */
{
	extern int ascount;
#ifdef HAS_SAVE
	char	*name = work_fname(curr_workspace);

#ifndef NOT_EDITOR
	if( safetest && !access( name, 0 ) && !com_menu( M_save, TRUE ) ) 
		return;
#endif


	if ((Savef = fopen(name, OPst)) != NULL)
#ifdef NOT_EDITOR
		;
#else 
		slmessage(ER(231,"Saving to %s"), name);
#endif
	else {
#ifdef NOT_EDITOR
		fprintf(stderr, "Can't open %s\n", name);
#else
		warning(ER(232,"cantopen`Can't open %s, file not saved"), name);
#endif
		return ERROR;
	}

	saveMisc(flags);

	putword(built_count, Savef);

	Syms = 0;
	treeCountSyms(cur_work_top);
	putword(Syms, Savef);

	treeSave(cur_work_top);

	fclose(Savef);
#ifndef NOT_EDITOR
	clean_curws();
	ascount = 0;
#endif
	return 0;
}

static
saveMisc(flags)
int	flags;
{
#ifdef SAVEDEBUG
	fprintf(Savef, "Save file version %d\n", SAVE_VERSION);
#else
	efputc('A', Savef);
	efputc('P', Savef);
	putword(SAVE_VERSION, Savef);
	putword(flags, Savef);
	/*
	 * Later we should output sizeof(int), but beware.  Putword
	 * then would have to write out a full set of bytes (little
	 * endian) but not THIS putword and the one above, which would
	 * still have to write out 2 byte words.  In theory load will
	 * handle files with any size int but it's not been tested.
	 * Note that integer constants are stored as strings so they
	 * don't need the larger word, in fact very little does.
	 */
	putword(2, Savef);
#endif
}

/*
 * Walk the tree, and when we find a symbol table, set the sym_saveid
 * to the current symbol number.
 */
static
treeCountSyms(_np)
nodep _np;
{
	register nodep np = _np;
	register int	i;
	register symptr	sp;
	int	n_kids;

	if (ntype_info(ntype(np)) & F_SYMBOL)
		for (sp = (symptr)kid1(sym_kid(np)); sp; sp = sym_next(sp))
			s_sym_saveid(sp, Syms++);

	n_kids = reg_kids(np);
	for (i = 0; i < n_kids; i++)
		treeCountSyms(node_kid(np, i));
}

/*
 * Save a symbol table
 */
static
saveSyms(_symtab_node)
nodep _symtab_node;
{
	register nodep symtab_node = _symtab_node;
	register symptr	sp;

#ifndef MAP
	if (!symtab_node || ntype(symtab_node) != N_SYMBOL_TABLE)
		bug(ER(265,"not a symbol table!"));
#endif

#ifdef SAVEDEBUG
	fprintf(Savef, "saveSyms\n");
#else
	efputc(N_SYMBOL_TABLE, Savef);
	efputc(node_flag(symtab_node), Savef);
	efputc(node_2flag(symtab_node), Savef );
#endif
	for (sp = (symptr)kid1(symtab_node); sp != (symptr)NIL; sp = sym_next(sp)) {
#ifdef SAVEDEBUG
		fprintf(Savef, "(symbol table declaration)\n");
		fprintf(Savef, "\tflags = %x\n", node_flag(sp));
		fprintf(Savef, "\tdecl type = %x\n", sym_dtype(sp));
		fprintf(Savef, "\tname %s\n", sym_name(sp));
#else
		if( ntype(sp) != N_DECL_ID ){
			warning( ER(320,"Corrupt symbol table") );
			break;
			}
		efputc(N_DECL_ID, Savef);	/* USED TO BE NTYPE */
		efputc(node_flag(sp), Savef);
		efputc(node_2flag(sp), Savef);
		putword(sym_dtype(sp), Savef);
		if (fprintf(Savef, "%s\012", sym_name(sp)) < 0)
			writeError(Savef);
		/* don't save hash */
		putword((int)sym_value(sp), Savef);
#endif
	}
#ifdef SAVEDEBUG
	fprintf(Savef, "(end of symbol table)\n");
#else
	efputc(0, Savef);
#endif
}

treeSave(_np)
nodep _np;
{
	register nodep np = _np;
	register int	i;
	int	n_kids;
#ifdef PARSER
#define	stackcheck()
#else
	stackcheck();
#endif

#ifdef SAVEDEBUG
	fprintf(Savef, "ntype=%d=%s: ", ntype(np), NodeName(ntype(np)));
#else
	efputc(ntype(np), Savef);
#endif
	if (is_standard(np)) {
#ifdef SAVEDEBUG
		fprintf(Savef, "with flags %x ", node_flag(np));
#else
		/* all saved blocks are non-typechecked */
		efputc(node_flag(np) & ~NF_TCHECKED, Savef);
		efputc(node_2flag(np), Savef);
#endif
		}

	switch(ntype(np)) {
	case N_STUB:
#ifdef SAVEDEBUG
		fprintf(Savef, "(stub): %s\n", classname(int_kid(0,np)));
#else
		putword(int_kid(0,np), Savef);
#endif
		break;
	case N_CON_CHAR:
		putword(int_kid(0,np), Savef );
		break;
	case N_T_COMMENT:
	case N_EXP_ERROR:
	case N_CON_INT:
	case N_CON_REAL:
		if (fprintf(Savef, "%s\012", getESString(str_kid(0,np))) < 0)
			writeError(Savef);
		break;
	case N_CON_STRING:
		{
		register char *svstr;

		svstr = str_kid(0,np);

		/* skip the length byte to preserve save file */
#if defined(TURBO) && !defined(PARSER)
		if (fprintf(Savef, "%c%s\012", svstr[0], svstr+2) < 0)
#else
		if (fprintf(Savef, "%s\012", svstr) < 0)
#endif
			writeError(Savef);
		}
		break;
	case N_DECL_ID:
#ifdef SAVEDEBUG
		fprintf(Savef, "(declaration references symbol %d)\n",
			sym_saveid((symptr)np));
#else
		putword(sym_saveid((symptr)np), Savef);
#endif
		break;
	case N_VF_WITH:
		/* we need to save or somehow recalcuate on load the record */
	case N_ID:
#ifdef SAVEDEBUG
		fprintf(Savef, "(id references symbol %d)\n",
			sym_saveid((symptr)kid_id(np)));
#else
		putword(sym_saveid((symptr)kid_id(np)), Savef);
#endif
		break;
	case N_LIST:
#ifdef SAVEDEBUG
		fprintf(Savef, "(list with %d elements)\n", listcount(FLCAST np));
#else
		putword(listcount(FLCAST np), Savef);
#endif
		break;
	case N_PROGRAM:
	case N_DECL_PROC:
	case N_DECL_FUNC:
	case N_TYP_RECORD:
	case N_IMM_BLOCK:
	case N_FORM_PROCEDURE:
	case N_FORM_FUNCTION:
#ifdef SAVEDEBUG
		fprintf(Savef, "which has symbol table and %d kids\n", n_kids);
#endif
		saveSyms(sym_kid(np));
		break;
	}

	n_kids = reg_kids(np);
	for (i = 0; i < n_kids; i++)
		treeSave(node_kid(np, i));
#else HAS_SAVE
	error(ER(112,"Sorry, no saving of programs in this version") );
#endif HAS_SAVE
}

putword(word, fp)
int word;
FILE *fp;
{
	efputc(word, fp);
	efputc(word >> 8, fp);
}

#endif /* not LISTER */

#ifdef PARSER
writeError(fp)
FILE	*fp;
{
	fclose(fp);

	fatal("Error while writing save file");
}

efputc(a, f)
char a;
FILE *f;
{
#ifdef QNX
	if( fput( &a, 1, f ) != 1 )
#else
	if (putc(a, f) == EOF)
#endif
		writeError(f);
}
#endif


