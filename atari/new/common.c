#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "jump.h"
#include "flags.h"
#include "typecodes.h"
#include "class.h"
#ifdef PROC_INTERP
#include "process.h"
#endif

#ifdef SAI
# undef DB_SUBR
#endif
#ifdef DB_COMMON
# define PRINTT
#endif
#include "printt.h"

/* routine to put cursor on a hide parent if it accidentally reaches
   something inside */

#ifndef SAI

nodep 
hidebound( tree )
nodep tree;
{
	register nodep lasthide;	/* final result here */
	register nodep ascend;		/* for going up */

	/* parent makes comment not included */
	ascend = lasthide = tree;

	while( ascend ) {
		if( (ntype(ascend) == N_HIDE || ntype(ascend) == N_LIBRARY)
					&& tparent(tree) != ascend )
			lasthide = ascend;
		ascend = tparent(ascend);
		}
	return lasthide;
}

#endif /* not SAI */

/* check to see if a sym is ok for this class */
ok_sym( sym, class, loc )
symptr sym;		/* what symbol we are checking */
ClassNum class;		/* class to check for */
nodep loc;		/* location of the ID */
{
	register type_code *tvec;
	register type_code otype;	/* what type the symbol is */

	tvec = classtypes(class);
	otype = sym_dtype( sym );

	if( otype == T_FUNCTION && class == C_VAR ) {
		register int mysloc;
		/* checks that left hand side is valid */
		return ntype(tparent(loc))==N_ST_ASSIGN &&
			kid1(tparent(loc))==loc &&
			is_ancestor( tparent(sym), loc );
	
		}
	 else
			if( tvec )
				while( * tvec )
					if( * tvec++ == otype )
						return TRUE;

#ifndef SAI
#ifdef DB_COMMON
	if( tracing ) {
		tvec = classtypes( class );
		fprintf( trace, "ok_sym fails\n" );
		fprintf( trace, "tvec = %lx\n", (long) tvec );
		fprintf( trace, "typecodes allowed:\n" );
		while( * tvec ) {
			fprintf( trace, "  typecode: %ld\n", (long) * tvec );
			tvec++;
		}
		fprintf( trace, "dtype = %ld\n", (long) otype );
	}
#endif
#endif SAI
	return FALSE;
}
/* block move routine. */


blk_move( to, from, bytes )
reg char *to;		/* destination block start */
reg char *from;		/* source block start */
reg int bytes;		/* how many to move */
{
#ifdef msdos
# ifndef LARGE
	if( bytes )
		memcpy( to, from, bytes );
	/* dos_blk( to, get_DS(), from, get_DS(), bytes ); */
# else
	dos_blk( to, from, bytes );
# endif
#else
# ifdef QNX
	copy( to, from, bytes );
# else
	if( to < from )
		while( bytes-- )
			*to++ = *from++;
	 else {
		to += bytes;
		from += bytes;
		while( bytes-- )
			*--to = *--from;
		}
# endif qnx
#endif msdos
}

#ifndef QNX

max(x, y)
int	x;
int	y;
{
	if (x > y)
		return x;
	else
		return y;
}
min(x, y)
int	x;
int	y;
{
	if (x < y)
		return x;
	else
		return y;
}
#endif

#ifdef QNX
strncpy(dest, src, num)
{
	copy( dest, src, num );
}
#endif QNX

jmp_buf exit_buf;
jmp_buf err_buf;

/* check if the tree is not a variable */

static bits8 var_types[] = {
	T_INIT, T_VAR, T_FORM_REF, T_FORM_VALUE, T_FUNCTION, T_ABSVAR,
	T_MEMVAR, T_PORTVAR, T_FILENAME, 0 };

not_variable( tree )
nodep tree;
{
	register NodeNum actarg;

	actarg = ntype(tree);
	return (actarg != N_ID  || !strchr( var_types, sym_dtype(kid_id(tree))) )
		&& actarg != N_VAR_ARRAY &&
		actarg != N_VAR_FIELD && actarg != N_VF_WITH &&
		actarg != N_VAR_POINTER;
}


#ifdef QNX
/* this is coming in release 2 I am told */
/* strchr is defined to return a pointer to the end of the string if
 * c is NULL.
 */
char *
strchr(_str, _c)
char	*_str;
char	_c;
{
	register char	*str	= _str;
	register char	c	= _c;

	do {
		if (*str == c)
			return str;
	} while (*str++ != 0);

	return (char *)NULL;
}

#endif

nodep 
find_true_parent(xthenode )
nodep xthenode;		/* who we are finding the parent of */
{
	register nodep thenode;
	thenode = xthenode;

	while( ntype_info(ntype( (thenode = tparent( thenode )) ) ) & F_NOSTOP )
		;
	return thenode;
}

#ifndef PARSER
#ifdef HAS_WORKSPACE
clean_curws()
{
	clr_2flag(curr_workspace, WS_DIRTY);
#ifndef SAI
	redraw_status = TRUE;
#endif
}
#else HAS_WORKSPACE
clean_curws()
{
}
#endif HAS_WORKSPACE
#endif
#ifdef msdos
#include <spawn.h>
#endif

#ifdef TOS
/*
 * simulate spawnlp for gem
 */
#include <osbind.h>
spawnlp( md, path, arg1, tail, env )
int md;
char *path;
char *arg1;
char *tail;
char *env;
{
	char the_tail[129];
	char *str;
	char *fname;

	str = the_tail+1;

	/* replace % by the base name of the file */
	if( tail ) while( *tail ) {
		if( tail[0] == '%' && tail[1] == '%' ) {
			fname = curr_workspace->ws_fname;
			if( fname == (char *)0 ) continue;
			while( *fname ) {
				if( *fname == '.' ) break;
				*str++ = *fname++;
			}
			tail += 2;
		} else
			*str++ = *tail++;
	}
	*str++ = 0;

	the_tail[0] = strlen( the_tail+1 );

	return Pexec( 0, path, the_tail, env );
}
#define P_WAIT 0
#endif TOS

safeshell( str, pause, exec )
char *str;
int pause;
int exec;		/* do a spawn type call */
{
	char *semi;

#ifndef SAI
	int ret;
	regtty( TRUE, FALSE );
#ifdef QNX
	ret = shell( str );
#else
# if defined(msdos) || defined(TOS)
	if( exec ) {

		char *argloc;
	    for(;;) {
		if( semi = strchr( str, ';' ) ) {
			*semi = 0;
		}
		if( argloc = strchr( str, ' ' )  )
			*argloc++ = 0;	/* stomp on arg */

		ret = spawnlp( P_WAIT, str, str, argloc, (char *)0 );

		if( semi == (char *)0 ) break;
		str = semi + 1;
	    }
	}
	 else
# endif
	ret = system( str );
#endif
	printt2( "Called system with str %s, got return %d\n", str, ret );
	regtty( FALSE, pause );
	return ret;
#endif SAI
}

regtty( doit, pause )
int doit;
{
	if( doit ) {
		echo();
		nocrmode();
		}
	 else {
		noecho();
		crmode();
		}
#if !defined(SAI)
#ifdef GEM
	if( pause ) {
		form_alert( 1, LDS( 417, 
			"[2][Click on OKAY to|return to ALICE][ OK ]"));
	}
#else
	if( pause ) {
		fprintf( stderr, getERRstr( ER(194,"Hit SPACE to continue") ) );
		readkey();
		}
#endif
#endif

#ifdef msdos
	msdoscursor( doit );
#endif
}
/*
 * set constant value for interpreter.  loc is the node whose contents are
 * set, cntype the type of node, str the text of the constant value
 */

s_conval(xloc, cntype, str )
nodep xloc;
NodeNum cntype;
reg unsigned char *str;
{
	register nodep loc = xloc;
#ifdef FLOATING
	extern rfloat	atof();
#endif

	if( cntype == N_CON_STRING ) {
		s_str_kid( 0, loc, strc_alloc( str ) );
		s_int_kid( 1*STR_KID, loc, strlen(str + 1));
		return;
		}
	if( cntype == N_CON_CHAR )
		s_int_kid(0, loc, (unsigned)str[1] );
	 else
		s_str_kid( 0, loc, allocESString(str) );
	if( cntype == N_CON_INT ) {
		s_int_kid( 1*STR_KID, loc, getint(str) );
		if( badint(str, TRUE) )
			or_node_flag( loc, NF_BADCONST );
		}
	else if( cntype == N_CON_REAL ) {
#ifdef FLOATING
		s_fl_kid(loc, atof(str));
#else
		warning( ER(204,"unfinished`Warning : No real numbers in this version") );
#endif
		}
}

getint( str )
char *str;
{
	if( str[0] == '$' ) {
		unsigned res = 0;
		register char *cp = str;

		while( *++cp ) 
			res = (res << 4) + ((*cp > '9') ? 9 : 0) + (*cp & 0xf);
		return res;
		}
	if( badint(str, TRUE ))
		return 0;
	 else
		return atoi(str);
}
	
/* get an integer and worry about overflow */
badint( str, givew )
char *str;		/* digit string */
int givew;		/* give warning */
{
	long res, atol();

	res = atol(str);
	if( res > 32767L ||  res < -32768L ) {
		if( givew )
			warning(ER(275,"badint`Integer is not in range -32767..32767"));
		return TRUE;
		}
	return FALSE;
}

	

block_typecheck( block )
nodep block;
{
	c_comp_decls(1, block, TC_FULL | TC_DESCEND | TC_MOVECURSOR );
	/* beep to indicate done */
}

#ifdef QNX
efputc(c, fp)
int	c;
FILE	*fp;
{
	/* QNX putc returns the character written */
	putc(c, fp);
}

int
fgetc(fp)
FILE	*fp;
{
	return getc(fp);
}
#else
efputc(c, fp)
int	c;
FILE	*fp;
{
	if (fputc(c, fp) == EOF)
#ifdef ATARI520ST
		if( c != EOF )
#endif
			writeError(fp);
}
#endif

writeError(fp)
FILE	*fp;
{
	if (fp)
		fclose(fp);
	error(ER(30,"badwrite2`error writing file"));
}

#ifdef SAI	/* while curses is being repaired */

#ifndef A_ATTRIBUTES
#define A_ATTRIBUTES  0x80
#endif

reswflags( win, newflgs )
WINDOW *win;
short newflgs;
{
	win->_flags &= ~A_ATTRIBUTES;
	win->_flags |= (newflgs & A_ATTRIBUTES );
}

#endif
