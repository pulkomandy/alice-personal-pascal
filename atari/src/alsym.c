
#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "alctype.h"
#include "typecodes.h"
#include "menu.h"
#include "flags.h"
#ifdef PARSER
#include "input.h"
#endif

int blat[] = {
	HAS_EDITOR,
#ifdef RELEASE
	RELEASE,
#else
	0,
#endif
	HAS_IOFUNCS
	};

#ifdef DB_ALSYM
# define PRINTT
#endif
#include "printt.h"

/* and now the symbol lookup routines */

#ifndef OLMSG
char rdmsg[] = "redeclared`Symbol %s redeclared";
#endif

/* an undefined symbol for slookup to return a pointer to */
/* NOTE THIS MUST BE IN TREE SPACE */

nodep with_symtab = NIL;		/* with statement we looked in */


/*
 * slookup
 * This routine takes a name and finds a
 * symbol table entry for it.  It returns a pointer to that entry.
 * Since field names can be overloaded with other names and one
 * another, things become a royal pain eventually.  We will ignore
 * this for now.
 */
#ifndef OLMSG
char undstring[] = "undef`Symbol '%s' has not been declared";
#endif

#ifndef SAI
symptr
slookup(nam, create, table, moan, symtouse)
char	*nam;		/* name */
int	create;		/* flag indicating if we should create if undef */
nodep table;		/* lookup table or not */
int	moan;		/* complain if not found */
symptr	symtouse;	/* a symbol we can make an undef from */
{
	register symptr ptr;	/* pointer to move in the table */
	register nodep symtab;	/* header of symbol tables */
	extern int smallflag;	/* parser's small model flag */
	bits8	nhash	= hash(nam);

	printt5("slookup(%s, %s, %lx, %s, %lx)\n", nam,
		create == CREATE ? "create" : "nocreate",
		(long)table,
		moan == MOAN ? "moan" : "nomoan", (long)symtouse);

	if (table != NIL)
		symtab = table;
	 else 
#ifndef PARSER
		symtab = next_symtab(cursor, TRUE);
#else PARSER
		symtab = next_symtab(NIL, TRUE);
#endif PARSER

	do {
		ptr = search_symtab(symtab, nam, nhash, NOREDECLS);
		if (ptr != NIL) {
			if( sym_dtype(ptr) == T_PROGRAM )
				continue;
			if (sym_dtype(ptr) == T_UNDEF && moan == MOAN) {
#ifdef PARSER
				/* turboid(nam); */
#else
				warning(ER(190,undstring), nam);
#endif
				}
#ifdef PARSER
			/* large mode symbol, small model flag */
			if( smallflag && sym_mflags(ptr) & SF_LARGE )
				continue;
			if (symtab == LibSymtab) {
				LibRefd = TRUE;
				or_sym_mflags(ptr, SF_LIBREF);
				}
#endif
			return ptr;
		}
	} while ((symtab = next_symtab(symtab, FALSE)) != NIL);

#ifndef PARSER
	if (create == CREATE) {
		symptr newmem;
		if (moan == MOAN)
			warning( ER(190,undstring), nam );
		/* nil cursor as there is no type to associate */
		if( symtouse ) {
			newmem = symtouse;
			/* use scope of undef to save old type */
			s_sym_scope( newmem, sym_dtype(newmem) );
			s_sym_dtype( newmem, T_UNDEF );
			or_sym_mflags( newmem, SF_REFERENCED );
			}
		 else
			newmem = s_declare(nam, T_UNDEF, NIL);
		return sym_link(newmem, sym_kid(cur_work_top));
	} else
		return Tptr(null_undef);
#else
	if (create == CREATE) {
		if (moan == MOAN)
			turboid(nam);
		return s_declare(nam, T_UNDEF, GlobalSymtab);
		}
	else
		return Tptr(null_undef);
#endif
}


/*
 * Return true if a name is currently in use in this scope
 */
int
inUse(name)
char	*name;
{
	return slookup(name, NOCREATE, NIL, NOMOAN,NIL) != Tptr(null_undef);
}


/* 
 * Find the "real" symbol table.  Sometimes we pass slookup a symbol table
 * that is actually a special node like a VAR_FIELD or WITH that points to
 * an actual symbol table somehow.  This routine takes these special nodes
 * and returns the symbol table referred to.
 */
nodep 
find_r_symtab(symtab)
nodep symtab;
{
	register NodeNum sttype;

	if( !symtab )
		return NIL;
	sttype = ntype(symtab);


	/* if a special record scope return that */

	/* THIS DOESN't WORK IN THE PARSER */
	if( sttype == N_INIT_STRUCT )
		return sym_kid( kid2(symtab) );
	if( sttype == N_ST_WITH || sttype == N_VAR_FIELD ) 
#ifndef PARSER
		{
		register nodep recvt;

		recvt = gexp_type( kid1(symtab) );
		if( ntype(recvt) == N_TYP_RECORD )
			return sym_kid( recvt );
		 else
			return NIL;
		}
#else
		{
		extern nodep var_type();
		return sym_kid( var_type( kid1(symtab) ) );
		}
#endif
	 else
		return symtab;
}

/*
 * Search through this symbol table for the name, return the matching symbol
 * table entry or NIL.
 */
symptr
search_symtab(symtab, name, nhash, redecls)
nodep symtab;			/* symbol table to search */
char	*name;				/* name to search for */
bits8	nhash;				/* hash code for name */
int	redecls;			/* stop on redeclarations? */
{
	register symptr	ptr;

	printt4("search_symtab(%lx, %s, %d, %s)\n", (long)symtab, name, nhash,
		redecls ? "redecls" : "noredecls");

	if( !(symtab = find_r_symtab(symtab) ) )
		return (symptr) NIL;

	for (ptr = (symptr)kid1(symtab); ptr !=(symptr)NIL; ptr = sym_next(ptr)) {
		if (sym_nhash(ptr) == nhash &&
		    case_equal(sym_name(ptr), name) &&
		    (redecls == ((node_flag(ptr) & NF_REDECLARED) != 0))) {
			printt2("returning, ptr=%lx(%s)\n", (long)ptr, sym_name(ptr));
			return ptr;
		}
	}
	printt0("returning NIL\n");
	return (symptr)NIL;
}

#ifndef PARSER
nodep	LookupRec;
#endif

/*
 * Link the N_DECL_ID node into the symbol table.  Return the node, which
 * is used by slookup.
 */
symptr
sym_link(_sp, symtab)
symptr	_sp;		/* pointer to a N_DECL_ID node */
nodep symtab;	/* the node that points to the symbol tab for def */
{
	register symptr	sp = _sp;	/* for QNX C compiler */
	register symptr ptr;		/* to scan symbol table for end */
	reg symptr	pptr;		/* pointer to previous symbol */

	printt2("sym_link(%x, %x)\n", sp, symtab);

	symtab = find_r_symtab(symtab);

	/* check for empty symbol table */
	if ((pptr = (symptr)kid1(symtab)) == (symptr)NIL) {
		printt0("empty symtab\n");
		s_kid1(symtab, NCAST sp);
		or_node_flag(sp, NF_IS_DECL);
		s_sym_next(sp, (symptr)NIL);
		return sp;
	}

	/* search through the table looking for duplicates */
	for (ptr = pptr; ptr != (symptr)NIL; ptr = sym_next(ptr)) {
		if (sym_nhash(ptr) == sym_nhash(sp) &&
		    case_equal(sym_name(ptr), sym_name(sp)) && 
		    !(node_flag(ptr) & NF_REDECLARED)) {
			/*
			 * If an UNDEF node exists for this name, unlink
			 * it and link in the new one.  Change references
			 * from the UNDEF node to the new node, and
			 * free the UNDEF node.
			 */
			if (sym_dtype(ptr) == T_UNDEF) {

				printt1("%x was T_UNDEF\n", ptr);

#ifndef PARSER
				mark_line(NCAST sp, HUGE_CHANGE);
#endif

				/*
				 * Unlink the UNDEF node by linking in
				 * the new one.
				 */
				if (ptr == (symptr)kid1(symtab))
					s_kid1(symtab, NCAST sp);
				else
					s_sym_next(pptr,  sp);
				s_sym_next(sp, sym_next(ptr));

#ifndef PARSER
				LookupRec = NIL;
#endif
				sym_assign(cur_work_top, ptr, sp);

				or_node_flag(sp, NF_IS_DECL);
				return sp;
			}
			/*
			 * What to do about redeclarations.
			 *
			 * If the new decl 'sp' was the real declaration once
			 * (it could be returning due to undo), then
			 * let it be the real declaration again.  Otherwise
			 * keep the current declaration as the real one and
			 * mark the new one as a redeclaration.
			 */
			else if (sp != ptr) {
				/* link it into the table after real one */
				s_sym_next(sp, sym_next(ptr));
				s_sym_next(ptr, sp);

				if (node_flag(sp) & NF_IS_DECL) {
					or_node_flag(ptr, NF_REDECLARED);
					clr_node_flag(ptr, NF_IS_DECL );
#ifndef PARSER
					mark_line(NCAST ptr, SMALL_CHANGE);
#endif
					return sp;
				}

				or_node_flag(sp, NF_REDECLARED);

				warning(ER(113,rdmsg), sym_name(sp));
			
				return Tptr(null_undef);
			} else {
				printt3("%x (%s) declared twice in symtab %x\n",
				    (int)sp, sym_name(sp), symtab);
				return sp;
			}
		}
		pptr = ptr;
	}
	/* pptr now points to the end of the table and can add */
	s_sym_next(pptr, sp);
	s_sym_next(sp, (symptr)NIL);

	printt2("added %x at end of symtab, pptr=%x\n", sp, pptr);

	or_node_flag(sp, NF_IS_DECL);
	return sp;
}

/*
 * Create an N_ID or N_VF_WITH pointing at this name.
 */

nodep 
symref( nam )
char	*nam;		/* name table entry for this string */
{

	register symptr lookres;

	with_symtab = NIL;

	lookres = slookup(nam, CREATE, NIL, MOAN,NIL);

	if( with_symtab != NIL && sym_dtype(lookres) == T_FIELD )
		{
		printt2( "vf_with of %x, table %x\n", lookres, with_symtab);
		return tree( N_VF_WITH, NCAST lookres, with_symtab );
		}
	 else
		return tree(N_ID, NCAST lookres );

}

#ifdef PARSER
nodep 
fld_symref( nam, rec )
char	*nam;		/* name table entry for this string */
nodep rec;		/* record variable */
{
	symptr ste;	/* symbol table entry for rec */
	nodep ty;	/* type of ste */
	nodep st;	/* symbol table for record fields */
	nodep  var_type();
	symptr lookres;

	ty = var_type(rec);
	if (ntype(ty) != N_TYP_RECORD)
		fatal("left hand side of '.' is not a record");

	st = kid2(ty);
	printt3("ty=%lx, ntype(ty)=%d, st=%lx\n", (long)ty, ntype(ty), (long)st);

	lookres = slookup( nam, 0, st, MOAN,NIL );
	return tree( N_VAR_FIELD, rec, tree( N_ID, lookres ));
}
#endif PARSER

#ifndef PARSER

/* We don't need menus, etc for the StandAlone Interp */
#ifndef SAI

/*
 * Routine called when HELP is pressed in a symbol menu
 */
symhelp( selected, menu )
int selected;		/* which entry help is desired on */
struct menu_data *menu;
{
	if( selected )
		helpfile( "psymbol/", menu->item[selected] );
}

extern char menu_nop[];

/*
 * Generate a menu of possible symbols at the given location.
 * This routine gives both user and builtin symbols.  If there is more
 * than one user symbol, builtins show up in a private menu
 */
		
sym_menu( loc )
nodep loc;
{
	char *ret;
	extern inchar tokentext[];
	extern char *sym_complete();

	if( ret = sym_complete( loc, "", 0 ) )  {
		strcpy( tokentext, ret );
		in_token(N_ID,cursor);
		}
}

#ifndef GEM

/*
 * Do symbol completion at a given location
 */
char *
sym_complete( loc, prefix, len )
nodep loc;
char *prefix;
int len;
{
	register nodep symtab;	/* header of symbol tables */
	struct menu_data m;
	reg int ret;		/* selected var */
	char *bistr;
	int onlyone;		/* only one symbol from normal pass */
	ClassNum oclass;
/*	extern alice_window main_win; */
	extern inchar tokentext[];

	bistr = LDS( 401, "Built in Names" );

	new_menu( &m, LDS( 402, "Symbols" ) );
	add_menu_item( menu_nop );
	
	symtab = next_symtab(loc, TRUE);
	oclass = ex_class(loc);		/* what class we are at */
	do {
		if( symtab == NIL )
			break;
		if( madd_table( symtab, oclass, prefix, len, loc ) == ERROR )
			break;
		/* advance to the next symbol table */
	} while( (symtab = next_symtab( symtab, FALSE )) != Tptr(master_table));
	onlyone = 0;

	/*
	 * If there is only one entry, then don't bother popping a menu
	 */
	if( m.num_items == 2 )
		onlyone = 1;
	 else {
		/*
		 * Add on the choice of getting the list of builtins
		 */
		add_menu_item( bistr );
		if( m.num_items > 2 )
			ret = pop_menu( curr_window->w_desc, &m, (funcptr)NIL );
	 	else
			ret = m.num_items-1;
		}
	if( ret == m.num_items-1 || onlyone ) {
		if( !onlyone ) {
			new_menu( &m, bistr );
			add_menu_item( menu_nop );
			}
		madd_table(  Tptr(master_table), oclass, prefix, len, loc );
		if( m.num_items == 2 )
			ret = 1;
		 else
			ret = pop_menu( curr_window->w_desc, &m, symhelp );
		}
	if( ret > 0 ) 
		return m.item[ret];
	 else
		return (char *)0;

}

#else GEM

/*
 * Do symbol completion at a given location
 */
char *
sym_complete( loc, prefix, len )
nodep loc;
char *prefix;
int len;
{
	register nodep symtab;	/* header of symbol tables */
	char *str;
	struct menu_data m;
	int ret;		/* selected var */
	int onlyone;		/* only one symbol from normal pass */
	char *bistr;

	ClassNum oclass;
/*	extern alice_window main_win; */
	extern inchar tokentext[];

	bistr = LDS( 403, " <Built in Names> " );

	/* Give it a title: */
	new_menu( &m, "Symbols" );

	/*
	 * Add all the symbols that match the prefix
	 */	
	symtab = next_symtab(loc, TRUE);
	oclass = ex_class(loc);		/* what class we are at */
	do {
		if( symtab == NIL )
			break;
		if( madd_table( symtab, oclass, prefix, len, loc ) == ERROR )
			break;
		/* advance to the next symbol table */
	} while( (symtab = next_symtab( symtab, FALSE )) != Tptr(master_table));

	onlyone = FALSE;

	/*
	 * If there is only one entry, then don't bother popping a menu
	 */
	if( m.num_items == 1 )
		onlyone = TRUE;
	 else {
		/*
		 * Add on the choice of getting the list of builtins
		 */
		add_menu_item( bistr );
		if( m.num_items > 1 )
			pop_menu( &m, (funcptr)NIL, TRUE, &str, &ret );
	 	else
			str = bistr;
	}
	/* If we picked 'built in symbols' or there was only one item
	 * in that matched his prefix...
	 */
	if( str == bistr || onlyone ) {
		if( !onlyone ) {
			/*
			 * We must have picked 'built in symbols...'
			 * Change the title of the menu
			 */
			new_menu( &m, LDS( 405, "Builtin Symbols" ) );
		}
		/*
		 * Add on any built-in symbols that match the prefix
		 */
		madd_table(  Tptr(master_table), oclass, prefix, len, loc );
		/*
		 * If we still only have one item,
		 * then return that string
		 */
		if( m.num_items == 0 )
			return (char *)0;

		if( m.num_items == 1 )
			str = m.item[0];
		 else
			pop_menu( &m, symhelp, TRUE, &str, &ret );
	}

	return str;
}

#endif GEM

/* add a symbol table to a menu of symbols */

static
madd_table( symtab, oclass, prefix, len, loc )

nodep symtab;		/* symbol table to add */
ClassNum oclass;
char *prefix;
int len;		/* length of prefix */
nodep loc;		/* location of id */
{
	register symptr ptr;

	ptr = (symptr) kid1(find_r_symtab(symtab));
	/* search all symbols in the block for the same pointer */
	/* ok sym's third argument the id's location */
	while( ptr != (symptr) NIL ) {
		if( ok_sym( ptr, oclass, loc ) && sym_dtype(ptr) != T_UNDEF &&
				cless_comp( sym_name(ptr), prefix,len ) )
			if( add_menu_item(sym_name(ptr)) == ERROR )
				return ERROR;
		ptr = sym_next(ptr);
		}
	return 0;
}
#endif

/* caseless compare, true if equal, write in assembler? */

cless_comp( xstr, xprefix, len )
char *xstr;		/* string with symbol */
char *xprefix;		/* my guy */
int len;
{
	register int dlen = len;
	register char *str = xstr;
	register char *prefix = xprefix;

	while( dlen-- )
		if( (*str++ | 0x20) != (*prefix++ | 0x20) )
			return FALSE;
	return TRUE;
}
#endif PARSER
/*
 *    Declaration routines.
 *	These routines declare the appropriate item in the local symbol
 *	table and return a pointer to the tree showing the declaration
 */


/*
 *	s_declare
 *
 *	This routine, a shadow of its former self, merely creates a
 *	symbol node and stuffs its fields.  Linking into and grafting
 *	onto is performed by graft.
 *
 *	lname	- name table entry for the symbol
 *	symtype	- type of the symbol itself
 *	symtab	- what symbol table it goes in, only given in parser
 */

symptr
s_declare(lname, symtype, symtab)
char	*lname;		/* string table entry for name */
type_code symtype;	/* what type the actual symbol is */
reg nodep symtab;	/* the node that points to the symbol tab for def */
{
	register symptr	ptr;
	symptr	newn;
#ifdef PARSER
	register symptr	pptr;
#endif
	printt3("s_declare(%s, %d, %lx)\n", lname, symtype, (long)symtab);
	newn = fresh(struct symbol_node);
#ifdef PARSER
	pptr = (symptr) kid1(symtab);
	printt2("symtab %lx, kid1 %lx\n", (long)symtab, (long)pptr);
	if (pptr == (symptr) NIL) {
		ptr = newn;
		s_kid1(symtab, NCAST ptr);
		printt2("new symtab, set %lx -> %lx\n", (long)symtab, (long)ptr);
	} else {
		for (ptr=pptr; ptr; ptr=(sym_next(ptr))) {
			printt1("%5s ", sym_name(ptr));
			if (case_equal(sym_name(ptr), lname)) {
				nodep p;
				int nt;
				if (sym_dtype(ptr) == T_UNDEF) {
					s_sym_dtype(ptr, symtype);
					return ptr;
					}
				p = tparent(ptr);
				nt = ntype(p);
				if (nt == N_DECL_PROC && kid4(p) == NIL)
					/* It's been declared forward */
					return ptr;
				else if (nt == N_DECL_FUNC && kid5(p) == NIL)
					/* Ditto */
					return ptr;
				else if(sym_dtype((symptr)ptr) == T_FILENAME )
					/* it's a file in the param list */
					return ptr;
				else
		error(ER(113,"redeclared`Symbol %s redeclared"), lname);
			}
			pptr = ptr;
		}
		/* pptr now points to the end of the table, to which we add */
		printt2("\ndidn't find it, set %lx -> %lx\n",
			(long)pptr, (long)newn);
		ptr = sym_next(pptr) = newn;
	}
	s_sym_name(ptr, allocstring(lname));		/* allocs again */
#else
	ptr = newn;
	s_sym_name(ptr, allocstring(lname));
#endif 	PARSER
	s_sym_nhash(ptr, hash(lname));
	s_sym_next(ptr, (symptr)NIL);
	s_ntype(ptr, N_DECL_ID);
	s_tparent(ptr, NIL );
	s_sym_type( ptr, NIL );
#ifdef PARSER
	s_node_flag(ptr, NF_IS_DECL);
#else
	s_node_flag(ptr, 0);
#endif
	s_2flag( ptr, 0 );
	s_sym_value(ptr, (pint)0 );
	s_sym_mflags( ptr, 0 );
	s_sym_saveid( ptr, 0 );	 /* ? what have I broken ? */
	/* if, three years from now, you are looking at this comment,
	  (feb 86) it is here because jan thinks random additions are
	  dangerous.  So there. */
	s_sym_dtype(ptr, symtype);
	return ptr;
}

/*
 *  buildin - installs built in symbol table
 */

#endif /* not SAI */


#ifdef ES_TREE


buildin()
{
	register symptr ptr;
	register char	*p;
	extern char *bstr_data;
	int	strsize;


	printt0("buildin\n");

	/*
	 * Read in builtin strings (they are packed together).
	 *
	 * STRINGS contains a word describing the number of bytes the
	 * packed-together strings occupy, then the strings themselves.
	 */
	
	p = bstr_data;
	for( ptr = Tptr(ndlast); ptr != (symptr)NIL; ptr = sym_next(ptr) ) {
		s_sym_name(ptr, p);
		p += strlen(p) + 1;
		printt1( "Building in symbol %s\n", sym_name(ptr) );
		s_sym_nhash(ptr, hash(sym_name(ptr)));
		}
	for( ptr = Tptr(SP_number); ptr != (symptr)NIL; ptr = sym_next(ptr) ) {
		s_sym_name(ptr, p);
		p += strlen(p) + 1;
		printt1( "Building in symbol %s\n", sym_name(ptr) );
		s_sym_nhash(ptr, hash(sym_name(ptr)));
		}
	s_sym_name(Tptr(null_undef), LDS( 407, "Undefined" ) );
}
#else

#ifndef PARSER
rfloat our_pi = FLOATNUM(3.14159265358979323846);
#endif

buildin()
{
	register symptr ptr;
 	extern struct symbol_node ndpi;

	/* pi setup for dumb Mark Williams.  Put in builtin when
	  we abandon this compiler */

#ifdef notdef
	printt1( "address of our_pi as long %lx\n", (pint)&our_pi );
	s_sym_value( (&ndpi), (pint)&our_pi );
#endif

	/* There are two builtin tables, one visible to the user, and
	 * one not.  The invisible one contains things like special built
	 * in types (ordinal) and built in field names (TypeCode)
	 */

	for( ptr = Tptr(ndlast); ptr != (symptr)NIL; ptr = sym_next(ptr) ) {
		printt1( "Building in symbol %s\n", sym_name(ptr) );
		s_sym_nhash(ptr, hash(sym_name(ptr)));
		}

	for( ptr = Tptr(SP_number); ptr != (symptr)NIL; ptr = sym_next(ptr) ) {
		printt1( "Building in symbol %s\n", sym_name(ptr) );
		s_sym_nhash(ptr, hash(sym_name(ptr) ));
		}
	s_sym_name(Tptr(null_undef), LDS( 407, "Undefined" ) );	

}
#endif

#ifndef SAI

#ifdef PARSER

nodep 
rscheck( asymtab )
nodep asymtab;
{
	register nodep thesymtab = asymtab;

	if( thesymtab && ntype(thesymtab) == N_ST_WITH ) {
		printt1( "Set with_symtab to %x\n", thesymtab );
		with_symtab = thesymtab;
		}
	return thesymtab;
}

nodep 
next_symtab(asymtab)
reg nodep asymtab;
{
	register int	i;

	if (asymtab == NIL)
		return rscheck(CurSymtab);
	if (asymtab == Tptr(master_table))
		return NIL;
	if (asymtab == SymtabStack[0].symtab)
		return Tptr(master_table);
	for (i = 1; i < MAX_BLOCK_DEPTH; i++)
		if (asymtab == SymtabStack[i].symtab)
			return rscheck(SymtabStack[i-1].symtab);
	error(ER(273,"bug`next_symtab can't find next symbol\n"));
}

#else


/*
 * Get the next symbol table
 * When searching for a symbol, we must check all symbol tables
 * associated with all scopes the location is contained in.  This
 * takes a given scope and returns the next highest scope, all the
 * way up to the builtin table.
 * It also creates the special scopes needed within a with, field ref
 * or record initializer.  After all scopes are done, nil is returned
 */
static nodep 
next_symtab( asymtab, firstime )
reg nodep asymtab;
int firstime;	/* called from within the tree.  - what a terrible argument name! */
{
	register nodep ascend;

	printt1("next_symtab(%x)\n", asymtab);

	if (asymtab == Tptr(master_table) || !asymtab ||
	    (!firstime && (ntype(asymtab) == N_VAR_FIELD ||ntype(asymtab)==N_INIT_STRUCT)))
		return NIL;

	ascend = asymtab;
	if( ntype(ascend) == N_SYMBOL_TABLE ) {
		ascend = tparent(tparent(ascend));
		if( !ascend )
			return NIL;
		}
	/* DO NOT stop on N_TYP_RECORD */
	while(/*is_a_subscope(ascend)||*/ !(ntype_info(ntype(ascend)) & F_SYMBOL)) {
		register nodep ascparent;
		register NodeNum asctyp;
		if (is_root(ascend))
			return Tptr(master_table);
		ascparent = tparent(ascend);
		asctyp = ntype(ascparent);

		/* I HATE WITHS THEY ARE A KLUDGE ON A KLUDGE */

		/* this code stops on a with and causes the scan of the
		   symbol table associated.  It stores the with node in
		   a special external in case the symbol is found there
		   so it can be stored for runtime purposes */

		if( (asctyp == N_VAR_FIELD || asctyp == N_ST_WITH) &&
				get_kidnum(ascend) == 1) {
			extern nodep get_damn_exp_type();
			nodep t = get_damn_exp_type(kid1(ascparent));

			/* make sure there is a valid record here before
			 * we stop thinking there is a symbol table
			 */

			printt2("with or field, ntype(%x)=%d\n", t, ntype(t));
			if( ntype(t)!= N_TYP_RECORD ){
				ascend = ascparent;
				continue;
				}

			/* if it is a with, mark that this is the case */
			if( asctyp == N_ST_WITH ) {
				with_symtab = ascparent;
				}
			return ascparent;
			}
		 else if( asctyp == N_FLD_INIT && get_kidnum(ascend)==0 ) {
			/* left side of field initializer */
			nodep ret = tparent(tparent(ascparent));
			nodep t = kid2( ret );
			if( ntype(t) != N_TYP_RECORD ) {
				ascend = ascparent;
				continue;
				}
			return ret;
			}


		ascend = ascparent;
	}

	printt2("Found table at %x type %s\n", (int)ascend, NodeName(ntype(ascend)));

	return sym_kid(ascend);
}
#endif

#ifndef PARSER

/*
 * If the node was a symbol, or might have symbol kids, call the supplied
 * function with the symbol node as an argument.
 */

if_sym_call(_np, funcp)
nodep _np;
int	(*funcp)();
{
	register nodep np = _np;
	register int	i;
	int		nodeType;
	int		n_kids;

	stackcheck();
	if (!np) {
		printt0("if_sym_call on nil tree");
		return;
	}
	nodeType = ntype(np);
	printt3("if_sym_call(%x, %lx), ntype(np)=%d\n", np, (long)funcp, nodeType);

	/* Here we descend records where before, for some reason, unbenownst
	   to man or beast, we did not.  Lord help us. */

	/* if we *do* descend the record we're screwed.  If we don't,
	   we are fucked.  nice choice */

	if (nodeType == N_DECL_ID)
		(*funcp)(np);
	else if (ntype_info(nodeType) & F_DECLARE /* aaaarrrgggghhh */) {
		n_kids = regl_kids(np);
		for (i = 0; i < n_kids; i++)
			if_sym_call(node_kid(np, i), funcp);
	}
	/*
	 * N_DECL_PROC and N_DECL_FUNC and N_FORM_FUNCTIN and N_FORM_PROCEDURE
	 * don't have F_DECLARE set, because
	 * we don't want to act on all of their local declarations.
	 * Unfortunately this means we must specially handle their
	 * N_DECL_ID routine name kid specially.
	 */
	else if (highType(nodeType)) {
		if_sym_call(kid1(np), funcp);
		}
}

/* Is this the sort of node that has symbols underneath it for if_sym_call */

int
highType(type)
int	type;
{
	return (type == N_DECL_PROC || type == N_DECL_FUNC ||
		type == N_FORM_PROCEDURE || type == N_FORM_FUNCTION);
}

bits8 higherDecls[] = {
	T_PROCEDURE, T_FUNCTION, T_FORM_PROCEDURE, T_FORM_FUNCTION, 0
};

/* Predicates for lookupFor() */
static
isRecord(np)
nodep np;
{
	return ntype(np) == N_TYP_RECORD;
}

static
isBlock(np)
nodep np;
{
	return ntype_info(ntype(np)) & F_SCOPE;
}
/*
 * Return the appropriate symbol table node for this declaration.
 */
static nodep 
my_symtab(_decl)
symptr	_decl;
{
	register symptr	decl = _decl;

	register type_code dtype;

	printt2("my_symtab(%x), ntype=%d\n", decl, ntype(decl));

	if (ntype(decl) != N_DECL_ID) {
		printt1("%x is not a declaration!\n", decl);
		return NIL;
	}

	dtype = sym_dtype(decl);
	if (dtype == T_ENUM_ELEMENT) {
		printt0("enum\n");
		return sym_kid((symptr)lookupFor(isBlock, "block", decl));
	}
	else if (dtype == T_FIELD) {
		printt0("field\n");
		return sym_kid(lookupFor(isRecord, "record", decl));
	}
	else if (strchr(higherDecls, dtype) != NULL) {
		printt0("higher decl\n");
		return next_symtab(next_symtab(tparent(decl),TRUE),FALSE);
	}
	else
		return next_symtab(tparent(decl),TRUE);
}

/* search up in the tree from startp looking for a given condition */

nodep 
lookupFor(pred, what, startp)
int	(*pred)();
char	*what;
nodep startp;
{
	register nodep up;

	for (up = startp; is_not_root(up) && !(*pred)(up); up = tparent(up))
		;
	if (is_root(up))
		bug(ER(245,"didn't find %s"), what);
	return up;
}


int	DontAddDecl	= FALSE;
/*
 * To add a declaration, link it into the symbol table using sym_link(), and
 * possibly change ID references to the new declaration.
 */
add_decl(_decl)
symptr	_decl;
{
	register symptr	decl = _decl;
	register nodep sp;
	symptr		old_decl;
	int	wasundef;

	printt1("add_decl(%x)\n", decl);

	if (DontAddDecl)
		return;

	/* if adding an undef, it must be a deleted one */
	if( wasundef = (sym_dtype(decl) == T_UNDEF) ) {
		s_sym_dtype( decl, sym_scope(decl) );
		}
	sp = my_symtab(decl);
	/*
	 * Link the symbol into the symbol table.  If the decl is
	 * the current declaration for that name and there are other
	 * declarations of that name, refer them to the new declaration.
 	 * 	***	**T_FIELD**     ***
	 */
	old_decl = slookup(sym_name(decl), NOCREATE, sp, NOMOAN,NIL);
	printt1("old_decl=%x\n", old_decl);
	/* if we got the same guy, but we aren't putting in an undef,
	 * return.  Otherwise, we are linking in a guy who was an undef
	 * made with the same memory
	 */
	if( decl == old_decl ) {
		if( !wasundef )
			return;
		sym_unlink( decl, sym_kid(cur_work_top) );
		mark_line( NCAST decl, HUGE_CHANGE );
		}
	sym_link(decl, sp);
	if (decl != old_decl && (node_flag(decl) & NF_IS_DECL)
				&& (old_decl != Tptr(null_undef))
		/*&& (sym_dtype(decl) != T_FIELD)*/) {
		mark_line(NCAST decl, HUGE_CHANGE);
		LookupRec = sym_dtype(decl) == T_FIELD ? tparent(sp) : NIL;
		sym_assign(my_block(sp), old_decl, decl);
	}
}

/*
 * Unlink declaration from symbol table
 */
static
sym_unlink(_decl, _sp)
symptr	_decl;				/* declaration */
nodep	_sp;				/* symbol table pointer */
{
	register symptr far *ptrptr;
	register symptr decl	= _decl;
	reg nodep sp		= _sp;		/* symbol table pointer */
	symptr	newsym;

	for (ptrptr = (symptr far*)kid1adr(sp);
		STAR(ptrptr) != (symptr )NIL && STAR(ptrptr) != decl;
#ifdef ES_TREE
		ptrptr = & ((realsymptr)tfarptr(*ptrptr))->s_next) 
#else
	    	ptrptr = &(sym_next( STAR(ptrptr) )) )
#endif
	     ;
	/* it should NOT be possible to go off the end */
	if ( STAR(ptrptr) != (symptr )NIL)
		STAR(ptrptr) = sym_next(decl);
	else
		bug(ER(246,"del_decl: can't find symtab entry %x to delete in %x"),
		    decl, sp);
}

/*
 * Remove the declaration from the symbol table, and possibly
 * remove ID references from it.
 */
del_decl(_decl)
symptr	_decl;			/* node that goes */
{
	register symptr decl = _decl;
	register symptr far *ptrptr;
	reg nodep sp;		/* symbol table pointer */
	symptr		newsym;

	printt1("del_decl(%x)\n", decl);

	if (node_flag(decl) & NF_IS_DECL)
		mark_line(NCAST decl, HUGE_CHANGE);
	else
		mark_line(NCAST decl, SMALL_CHANGE);

	/*
	 * Remove the declaration from its symbol table.
	 */
	sp = my_symtab(decl);
	printt1("sp is %x\n", sp);
	sym_unlink(decl, sp);
	/*
	 * If the declaration was the "real" declaration, change
	 * references from decl to another declaration.  Check
	 * for existing REDECLARED symbols, otherwise search higher up
	 * symbol tables for the symbol.  If a declaration doesn't exist
	 * create a T_UNDEF one.	*** **T_FIELD** ***
	 */
	if (node_flag(decl) & NF_IS_DECL) {
		newsym = search_symtab(sp, sym_name(decl), hash(sym_name(decl)),
					 REDECLS);
		if (newsym != (symptr)NIL) {
			clr_node_flag(newsym, NF_REDECLARED);
			or_node_flag(newsym, NF_IS_DECL);
		}
		if (!newsym)
			newsym = slookup(sym_name(decl), CREATE,
				 next_symtab(tparent(sp),FALSE), NOMOAN,decl);
		LookupRec = NIL;
		if( decl != newsym )
			sym_assign(my_block(sp), decl, newsym);
	}
}

/* externals to speed up symbol exchange */

static symptr sw_newid, sw_oldid;
#endif

/* change all references to one ID to another one */

sym_assign(xblock, oldid, newid )
nodep xblock;	/* starting block */
symptr oldid;	/* id that is being replaced */
reg symptr newid;	/* id that is replacing it */
{
#ifndef PARSER
	register listp dlist;	/* declaration list for this block */
	register int index;		/* loop through decls */
	register nodep block = xblock;
	int	type	= ntype(block);

	stackcheck();

	printt4("sym_assign(%x=%s, %x, %x)\n", (int)block, NodeName(ntype(block)),
		 (int)oldid, (int)newid);

	if (is_a_hide(block)) 
		dlist = kid2(block);
	/*
	 * A hack: formal procedures and functions don't have any
	 * list of declarations.
	 */
	else if (type == N_FORM_PROCEDURE || type == N_FORM_FUNCTION)
		dlist = NIL;
	else
		dlist = decl_kid(block);

	sw_oldid = oldid;
	sw_newid = newid;

	if (dlist)
		for( index = 0; index < listcount(dlist); index++ ) {
			nodep ourdecl;

			ourdecl = node_kid( dlist, index );
			if( ntype(ourdecl) == N_DECL_PROC ||
					ntype(ourdecl) == N_DECL_FUNC ||
					is_a_hide(ourdecl) ) 
				sym_assign( ourdecl, oldid, newid );
			 else
				do_sym_swap( ourdecl );
				
			}
	for( index = 0; index < kid_count(ntype(block)); index++ )
		if( node_kid(block,index) != dlist )
			do_sym_swap( node_kid(block,index) );
#endif
}


#ifndef PARSER
do_sym_swap(xsnode )
nodep xsnode;
{
	register int	index;
	reg int	maxkid;
	NodeNum snt;
	NodeNum pnt;		/* parent node type */
	register nodep snode = xsnode;

	stackcheck();
	printt2("do_sym_swap(%x=%s)\n", (int)snode, NodeName(ntype(snode)));

	snt = ntype(snode);
	if( snt == N_ID || snt == N_VF_WITH) {
		if( kid_id(snode) == sw_oldid ) {
			if( LookupRec ) {
				NodeNum pnt;
				nodep snparent;
				snparent = tparent(snode);
				pnt = ntype(snparent);
				if( !(pnt == N_VAR_FIELD &&
				  get_damn_exp_type(kid1(snparent))==LookupRec
				  || pnt == N_FLD_INIT &&
				  kid2(tparent(tparent(snparent))) ==LookupRec))
					return;
				}

				
			s_kid_id(snode, sw_newid);
			printt3("kid_id(%x) set to %x [%x]\n",snode, sw_newid,
				kid_id(snode));
			}
		}
	 else {
		maxkid = regl_kids(snode);
		for( index = 0; index < maxkid; index++ )
			do_sym_swap( node_kid(snode,index) );
		}
}
#endif PARSER
#endif /* not SAI */


/* compare strings for equality regardless of case, assuming alphanumeric */
#define CASEBIT 0x20
case_equal( xstr1, xstr2 )
char *xstr1;
char *xstr2;
{
	register char *s1 = xstr1;
	register char *s2 = xstr2;

	while( *s1 )
		if( ( *s1++ | CASEBIT) != ( *s2++ | CASEBIT ) )
			return FALSE;
	/* zero in s1, if also end of s2 strings were equal */
	return *s2 == 0;
}

/* find the hash bucket for a character string */
	
hash( xname )
char *xname;
{
	register unsigned ht;
	register char *name = xname;
	
	for( ht = 0; *name; ++name )
		ht += ((unsigned)*name) | CASEBIT;

	return ht & 0xff;
}

/*
 * This routine is called if there is a stack overflow.
 * The stack is reset a bit first, to allow room for the calls
 */

stackerr()
{
	/* gastly error */

	error( ER(8,"complex`Nasty Error - expression or program too complex") );
}
