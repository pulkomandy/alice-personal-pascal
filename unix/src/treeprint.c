/*
 *DESC: The master routine that turns a subtree into text
 */

#include "alice.h"
#include <curses.h>
#include "typecodes.h"
#include "class.h"
#include "flags.h"
#include "workspace.h"

#ifdef DB_TREEPRINT
# define PRINTT
#endif
#include "printt.h"

#define bufaddc( ch ) *lb_cursor++ = ch | chmask
#define pr_column (lb_cursor - line_buffer)

#define NL 10

extern workspace *curr_workspace;
extern buf_el *OLE_StartCursor, *OLE_EndCursor;

static leng_check(), rvbufadd(), tab_it();

/*
 * Routines to print out a sample tree
 */
int srch_row, srch_scol, srch_width;	/* position of cursor on screen */
int srch_real;				/* real start of representor */
buf_el chmask = 0;			/* mask for standout mode */
extern buf_el Attr_Map[MAX_COLOURS];	/* colour table */
extern bits8 Symc_map[];		/* symbol to colour map */
curspos srch_cursor;			/* cursor being looked for */
bits8 out_flag;				/* mode of output */
int form_column;			/* column of most recent node */
curspos former_node;			/* most recent node for column search */
int indent_end = TRUE;			/* default indent your ends */

#ifdef IBMQNX
unsigned qat;
#define set_attr(attr) qat = attr,chmask = ((chmask | qat) & 0x0f00)|(qat & 0xf000)
#else
# ifdef msdos
#  define set_attr(attr) chmask = (chmask & (A_BOLD|A_BLINK|A_STANDOUT|A_REVERSE)) | attr
# else
#  define set_attr(attr) chmask |= attr
# endif
#endif

/* 
  #define astandout() chmask = (chmask & 0x8800)|((chmask << 4) & 0x7000)
   */
#define astandout() chmask |= A_STANDOUT

#define clr_attr(attr) chmask &= ~(attr)
#define asg_attr(attr)	chmask = attr

#define sav_attr(saveit) saveit = chmask;
#define rst_attr(saveit) chmask = saveit;

buf_el line_buffer[MAX_LEN_LINE];
buf_el *lb_cursor;


static char def_build[23];     /* build default strings */
/*
 * treeprint has several functions.  The primary one is printing the tree
 * into a buffer.  This has out_flag == 0.  With out_flag == LOOK_CURSOR,
 * we get the secondary function.
 * The secondary function is to define the physical parameters of a tree
 * cursor position.  This function sets three externals to indicate
 * things about the cursor.  srch_real is the column the "point" of the
 * cursor will be found in.
 * Higher level routines set the external "srch_row" which indicates what
 * row of the screen this all occurs on.
 *
 * The third function is finding what alice object is at a given column.
 * it is doen when out_flag == LOOK_COLUMN.  the external "sought_column"
 * contains the column we are looking for.
 * The alice object on or to the left of sought_column is returned in
 * "pointed_node" by the routine found_column()
 * found_column also marks how far to the left we were, and puts it in
 * point_offset
 */
treeprint(xthenode, prcode_index, listcode, def, plist )
nodep xthenode;	  /* current node being listed */
int prcode_index;	/* print code index for this line */
char listcode;	  /* selects bytes between list items */
char *def;		/* the default code */
listp plist;	/* parameter list for call, only present then */
{
	register nodep thenode = xthenode;
#ifdef LOADABLE_TEMPLATES
	char far *temp_ptr;	 /* pointer into string for template */
#else
	char *temp_ptr;	 /* pointer into string for template */
#endif
	bits8 special;		/* what default we use */
	NodeNum ourtype, newtype;/* type of node we have */
	nodep newnode;		/* new child of this node */
	nodep tempnode;		/* temporary node in strange indirection */
	bits8 ourprec,newprec;	/* operator binding strength */
	register int kidnum;		/* child index number */
	buf_el cmsave;		/* save attribute mask */
	int setecol = 0;	/* should we set end of cursor column */
	bits8 noerror = TRUE;	/* are we not displaying an error */
	extern int ascii_output;	/* text command in action */


	stackcheck();
	/* loose a level of indentation to make it fit better */

	if( thenode == NIL )
		return;		/* in case of damaged tree */

	printt5("treeprint(thenode=%lx, prcode_index=%d, listcode=%d, def=%lx, plist=%lx)\n",
		(long)thenode, prcode_index, listcode, (long)def, (long)plist);
	printt2("out_flag %d, pr_column %d\n", out_flag, pr_column);
#ifdef QNX
	if( 0 ) {
		kl_terminate: goto terminate;
		}
#endif
	/* save cursor attribute coming in */
	sav_attr(cmsave);
	asg_attr( Attr_Map[COL_DEFAULT] );	/* clear current attributes */
	/* lose a level of indentation to make it fit better */

/* if we are looking for the cursor, save the preceding text now that
 * we know we are starting to print the node that the cursor is on
 */
if( out_flag ) {
	if( out_flag == LOOK_CURSOR )
		if( setecol = (srch_cursor == thenode) ) {
			OLE_StartCursor = lb_cursor;
			/* save_ptext(line_buffer, lb_cursor); */
			/* srch_scol = pr_column; */
			}
	}

special = FALSE;
/* def_build[0] = (char)0; Why was this here? */
kidnum = 0;
ourtype = ntype(thenode);
ourprec = bind_strength(ourtype);

if( is_a_list(thenode) ) {

	if( listcode == 'P' && plist ) {
		/* we got a list of parameters and it is time to go to town
		   on them */
		printt1("treeprint, plist=%lx\n", plist);
		kidnum = argp_call( plist, (pint)thenode );
		}
	else if (listcount(FLCAST thenode) > 0) {
		treeprint( kid1(thenode), 0, listcode, def, LNIL );
		kidnum = 1;
		}
	 else
		kidnum = 0;		/* empty list, falls out */

	for(; kidnum < listcount(FLCAST thenode) ; kidnum++ ) {
		switch( listcode ) {
			case ':':
				if( ntype(node_kid(thenode,kidnum-1)) == N_FLD_INIT)
					goto addsemi;
			case ',':
			case 'P':
				if( kidnum )
					rvbufadd( ", ", thenode, kidnum );
				break;
			case ';':
			addsemi:
				rvbufadd( "; ", thenode, kidnum );
				break;
			case 'l':	/* should not happen */
				bug( ER(269,"Got l list code\n") );
				break;
			case 0 :
				break;
			default:	
				bufaddc( listcode );
				break;
			}
		treeprint( node_kid(thenode,kidnum), 0, listcode, def, LNIL );
		if( pr_column > MAX_LEN_LINE - 30 ) {
			warning( ER(236,"toolong`Line on screen too long") );
#ifdef QNX
			goto kl_terminate;
#else
			goto terminate; 
#endif
			}
		}
	}
 else {

	if( node_flag(thenode) & NF_STANDOUT )
		astandout();

	temp_ptr = print_codes(ourtype)[prcode_index];
	while( *temp_ptr ) {
		if( *temp_ptr == NL )
			break;
		if( *temp_ptr == '\t' )
			tab_it(HTABSIZE);
		 else if( *temp_ptr == '!' ) {

			char newcode;	/* generated list code */
			++temp_ptr;
			if( *temp_ptr >= '1' && *temp_ptr <= '9' ) {
				kidnum = *temp_ptr++ - '1';
				newnode = node_kid( thenode, kidnum );
				}
			 else
				kidnum = -1;
			switch( *temp_ptr ) {

			 case 'T':
				if( indent_end )
					tab_it( HTABSIZE );
				break;
			 case 'A': /* conditional attribute */
				if( chmask )
					break;
			 case 'a': /* attribute */
				++temp_ptr;
				if( noerror )
					set_attr( Attr_Map[ *temp_ptr - 'a' ] );
				break;
			 case 'k':
				if( noerror )
					set_attr( Attr_Map[COL_KEYWORD] );
				break;
			 case 'f':
				{
				symptr routname;

				routname =  (symptr) kid1(thenode) ;

				if( not_a_stub(routname) && sym_mflags(routname) & SF_ISFORWARD ) {
					while( *temp_ptr++ != '!' ||
							*temp_ptr != 'o' )
						;
					clr_sym_mflags(routname,SF_ISFORWARD);
					}
				}
				break;
			 case '>':
#ifdef PCODE_INDENT
				++ind_level;
#endif
				break;
			 case '<':
#ifdef PCODE_INDENT
				if(ind_level < 1)
					fprintf(stderr, "Too far left in node %d\n", ourtype );
				 else
					--ind_level;
#endif
				break;
			case 'H':
				if( ascii_output )
					bufadd( "{+Hide end}" );
				break;
			case '{':	/* comment parameter */
				{
				extern int ascii_output;
				int longcom;
				int nt = ntype(newnode);
				char far *comstr;

				/* detect space comment */
				
#ifdef HYBRID
				extern unsigned st_segment;
				comstr = fpmake(str_kid(0,newnode),st_segment);
#else
				comstr = str_kid(0,newnode);
#endif

				longcom = !((nt == N_T_COMMENT &&
					STAR(comstr) == ' '&& STAR(comstr+1)==0)
					|| (ascii_output && nt == N_STUB));
				if (longcom)
					bufaddc( '{' );
				treeprint(newnode,0,' ',(char *)0,LNIL);
				if( longcom )
					bufaddc( '}' );
				}
				break;
			case 'o':	/* optional parameter */
				if( kidnum >= 0 && is_a_stub(newnode) &&
							newnode != cursor ){
					/* skip over optional template */
					while( *temp_ptr++ != '!' ||
							*temp_ptr != 'o' )
						;
					}
				break;
				
			case 'p':
				{
				int paren_it;

				newtype = ntype(newnode);
				newprec = bind_strength(newtype);
				paren_it = (ourprec && newprec && 
					(newprec > ourprec || 
					(kidnum && newprec==ourprec)));
				if( paren_it )
					bufaddc( '(' );
				treeprint( newnode, 0, ' ',
					special ? def_build : (char *)0, LNIL );
					if( paren_it )
						bufaddc( ')' );
				special = FALSE;
				}
				break;
			case 'P':	/* parameter list */
				/* had better damn well be a list */
				newcode = *++temp_ptr;
				printt2("P, newnode=%lx, ntype(newnode)=%d\n",
					newnode, ntype(newnode));
				if( listcount( FLCAST newnode ) ) {
				   bufaddc('(');
				   if( (ntype(thenode) == N_ST_CALL ||
				       ntype(thenode) == N_EXP_FUNC) &&
				       ntype(kid1(thenode)) == N_ID  &&
				       (tempnode=tparent(kid_id(kid1(thenode)))) ) {
				       /* the following treeprint grabs
					  the declaration list for the routine,
					  with that complex indirection.  It
					  must be modified as the tree is */
					printt2("tempnode=%lx, ntype(tempnode)=%d\n",
						tempnode, ntype(tempnode));
					 if( is_a_list(tempnode) )
						tempnode = tparent(tempnode);
					printt2("tempnode=%lx, ntype(tempnode)=%d\n",
						tempnode, ntype(tempnode));
					 treeprint( newnode, 0, 'P', (char *)0,
					   FLCAST kid2(tempnode));
				   } else
				       	 treeprint( newnode, 0, newcode,
					   special ? def_build : (char *)0, LNIL );
				   bufaddc(')');
				   special = FALSE;
				   }
				else
				   bufaddc(' ');
				break;
				

			 case 'l':	/* empty only KLUDGE */
			 case ';':      /* list of statements */	
			 case ',':      /* comma list */
			 case ':':
				treeprint( newnode, 0, *temp_ptr,
					special ? def_build : (char *)0, LNIL );
				special = FALSE;
					break;
			 case '(':
				{
#ifdef LOADABLE_TEMPLATES
				char far *p1, *p2;
#else
				char *p1, *p2;
#endif

				p2 = def_build;
				for(p1=temp_ptr+1;*p1!=')'&&*p1;++p1)
					*p2++ = *p1;
				*p2 = (char) 0;
				temp_ptr = p1;
				special = TRUE;
				}
				break;
			 case 'C':		/* char constant */
				{
				unsigned thechar = (unsigned int)newnode;
				if( thechar >= ' '&& thechar < 127 ) {
					bufaddc( 047 );
					bufaddc( thechar );
					/* double a single quote */
					if( thechar == 047 )
						bufaddc( 047 );
					bufaddc( 047 );
					}
				 else {
					char statbuf[5];
					sprintf( statbuf, "#%u", thechar&0xff );
					bufadd( statbuf );
					}

				break;
				}
			 case 'S':	/* constant string */
				{
				unsigned char qtype;
				unsigned char *ourst;
				ourst = (unsigned char *)newnode;
				qtype = *ourst;
				if( ascii_output )
					leng_check( strlen(ourst)+1 );
				bufaddc( qtype );
#ifdef TURBO
				ourst += 2;	/* skip quote and len */
#else
				ourst++;
#endif
				while( *ourst ) {
					bufaddc( *ourst );
					/* if this char is quote, double it */
					if( *ourst++ == qtype )
						bufaddc( qtype );
					}
				bufaddc( qtype );
				}
				break;
			 case 'E':	/* possible extra segment string */
#if defined(ES) || defined(HYBRID)
				esbufadd((char *)newnode);
#else
				bufadd( (char *)newnode );
#endif
				break;
			 case 'I':      /* symbol table reference */
				{
				symptr oursym;
				oursym = (symptr)kid1(thenode);
				if( noerror )
				 set_attr(Attr_Map[Symc_map[sym_dtype(oursym)]]);
				bufadd( sym_name(oursym) );
				}
				break;
			 case 'D':	/* A symbol declaration */
				if( noerror )
					set_attr(Attr_Map[
					 Symc_map[sym_dtype((symptr)thenode)]]);
				bufadd( sym_name( (symptr)thenode) );
				break;
			 case 'N':
				bufadd( (char *)(kid_id(thenode)) );
				break;
			 case 's' : /* a stub */
				{
				extern int ascii_output;
				ClassNum stubnum = int_kid( 0, thenode );
				if( ascii_output ) {
					if( stubnum == C_BLCOMMENT ||
					    stubnum == C_COMMENT )
						break;
					bufadd( "{+ " );
					}
#ifdef A_UNDERLINE
				if( noerror )
					set_attr(Attr_Map[COL_STUB]);
#else
				bufadd( "<<" );
#endif
				if( def && !ascii_output) 
					bufadd( def );
				 else
					bufadd( classname( stubnum ) );
#ifdef A_UNDERLINE
#else
				bufadd( ">>" );
#endif
				if( ascii_output )
					bufaddc( '}' );
				break;
				}
#ifdef notused
			 case '!' :
				bufaddc( '!' );
				break;
#endif
			 case '\t' :	/* special double size tab */
				tab_it(2*HTABSIZE);
				break;
			 case 'R':	/* semicolon after record */
				if( look_multi(thenode) == 0 )
					bufaddc( ';' );
				break;
				
			 case 'c' : /* cursor is here */
				{
				int cur_column;

				cur_column = pr_column;
#ifdef DB_TREEPRINT
				if (tracing)
					fprintf(dtrace,
				"treeprint does !c, setecol %d, pr_column %d\n",
					setecol, cur_column);
#endif
				if( cur_column > MAX_LEN_LINE - 30 ) {
					warning( ER(236,"toolong`Line on screen too long") );
#ifdef QNX
					goto kl_terminate;
#else
					goto terminate; 
#endif
					}
				if( node_flag(thenode) & NF_ERROR) {
					noerror = FALSE;
					set_attr(Attr_Map[COL_ERROR]);
					}
				if( setecol ) {
					srch_real = cur_column;
					/*save_ptext(line_buffer, lb_cursor);*/
					}
				if( out_flag == LOOK_COLUMN ) {
				    if( cur_column > sought_column ) {
					found_column();
					}
				     else {
					/* save away the most recent */
					former_node = thenode;
					form_column = cur_column;
#ifdef DB_TREEPRINT
					if(tracing)fprintf(dtrace,
					 "Set former node to %x, column to %d\n",
						former_node, form_column );
#endif
						}
					}
				}
				break;
			case 'n':
				break;
			default :
				bug( ER(270,"treeprint:Bad template char %d\n"), *temp_ptr );
				break;
				}
			}
			 else {
				bufaddc( *temp_ptr );
				}
			++temp_ptr;
			}
		if( prcode_index == 0 && node_flag( thenode ) & NF_BREAKPOINT ){
			tab_it( 2*HTABSIZE );
			bufadd( "{+Breakpoint" );
			if( !work_debug(curr_workspace) )
				bufadd( " (Debug ONLY)" );
			bufaddc( '}' );
			}
		}
	;
terminate:
	rst_attr(cmsave);
	/* mark the end of the cursor location check */
	if( out_flag ) 
		if( out_flag == LOOK_CURSOR && setecol ) {
			/* srch_width = pr_column - srch_scol; */
			/* save the text that is under the cursor */
			OLE_EndCursor = lb_cursor;
			/* save_ctext(lb_cursor); */
			}
		else if( out_flag == LOOK_COLUMN && pr_column > sought_column ) 
				found_column();
	*lb_cursor = (char)0;		/* set null at end */
}

/* add a string to the line buffer */
 
bufadd( xstr )
unsigned char *xstr;
{
	register unsigned char *str = xstr;
	extern int ascii_output;
	printt1( "Bufadd asked to add string %s\n", str );
	/* and out top bit in case we highlight */
	if( ascii_output )
		leng_check( strlen(str) );
	while( (*lb_cursor++ = *str++ | chmask) & 0x7f )
		;
	/* do not want the zero to be a char */
	lb_cursor--;	/* so point at it */
}
#ifdef ES
esbufadd( str )
reg unsigned char *str;
{
	/* and out top bit in case we highlight */
	while( (*lb_cursor++ = @str++ | chmask) & 0x7f )
		;
	/* do not want the zero to be a char */
	lb_cursor--;	/* so point at it */
}

#endif
#ifdef HYBRID
#include "extramem.h"
esbufadd( str )
offset str;
{
	unsigned char far *tstr;
	extern int ascii_output;

	FP_SEG(tstr) = st_segment;
	FP_OFF(tstr) = str;
	if( ascii_output ) {
		char far *slcalc;
		slcalc = tstr;
		while( *slcalc++ )
			;
		leng_check( slcalc - tstr );
		}
	/* and out top bit in case we highlight */
	while( (*lb_cursor++ = *tstr++ | chmask) & 0x7f )
		;
	/* do not want the zero to be a char */
	lb_cursor--;	/* so point at it */
}

#endif

int column_base = 0;
static
leng_check( slen )
int slen;
{
	register int cpos = pr_column;
	if( cpos - column_base + slen > TURBO_MAXLINE ) {
		*lb_cursor++ = '\n';
		column_base = cpos;
		}
}


/* Add to the buffer in reverse video */
static
rvbufadd( str, xthenode, kidnum )
char *str;
nodep xthenode;
int kidnum;
{
	register nodep thenode = xthenode;
	buf_el atsave;
	

	if( node_flag(node_kid(thenode,kidnum-1)) & node_flag(node_kid(
				thenode,kidnum)) & NF_STANDOUT ) {
		sav_attr(atsave);
		astandout();
		bufadd( str );
		rst_attr(atsave);
		}
	 else
		bufadd( str );
		
}
/* do a tab in the output */
static
tab_it(tsize)
int tsize;
{
	register int tc;

	tc = tsize - ((int)(pr_column) % tsize);
	while( tc-- )
		bufaddc( ' ' );
}

/* scanning function for declarations in the case of a parameter list */
#define MAX_SLEN 40
int
argp_func(xthedecl, parm_no, xtype_parent, plist )
symptr xthedecl;		/* what symbol is declared */
int parm_no;		/* which parameter */
nodep xtype_parent;	/* the formal node */
listp plist;	/* actual parameter list */
{
	register symptr thedecl = xthedecl;
	register nodep type_parent = xtype_parent;
	/* we must print the prelude, treeprint the arguments */
	NodeNum partype;
	char holdbuf[MAX_SLEN];
	char *pref, *nam, *typename;
	char *def;

	stackcheck();

	if( parm_no >= listcount(plist) )
		return parm_no;
	printt2("type_parent=%x, ntype(type_parent)=%d\n", (int)type_parent,
		ntype(type_parent));
	partype = ntype(type_parent);
	if( parm_no > 0 )
		rvbufadd( ", ", plist, parm_no );
	nam = pref = "";
	if( thedecl && not_a_stub( thedecl ) ) {
		printt1( "decl %X exists and was not a stub\n", (long)thedecl );
		if( partype == N_FORM_REF )
			pref = "var ";
		else if( partype == N_FORM_FUNCTION )
			pref = "func ";
		else if( partype == N_FORM_PROCEDURE )
			pref = "proc ";
		if( is_a_symbol(thedecl) ) 
			nam = sym_name( thedecl );
		 else {
			/* special kludge for built in names */
			/* string loaded into extra segment */
#ifdef ES_TREE
			nam = getSegString((char *)thedecl + 1, tr_segment );
#else
			nam = (char *)thedecl + 1;
#endif
			}
		}
	 else {
		printt1( "decl %x was a stub or zero\n", (int) thedecl );
		}
	if( partype == N_FORM_VALUE || partype == N_FORM_REF ) {
		nodep rname;
		if( ntype((rname = kid2(type_parent))) == N_ID )
			typename = sym_name(kid_id(rname));
		else if (ntype(rname) == N_STUB)
			typename = "Untyped";
		else
			/* predefined type in builtin */
			typename = sym_name( (symptr)rname ); 
		}
	else if( partype == N_STUB )
		typename = "Unknown";
	else
		typename = "";	/* no indication on these */
	/* instead of the holdbuf on the stack, is there some free buffer? */

	if( *nam && strlen(pref)+strlen(nam)+strlen(typename) < MAX_SLEN-3 )  {
		sprintf( def = holdbuf, "%s%s : %s", pref, nam, typename );
		}
	 else
		def = typename;
	
	treeprint( node_kid( plist, parm_no ), 0, ',', def, LNIL );
	return ++parm_no;
}
