#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "token.h"
#include "menu.h"
#include "flags.h"
#include "class.h"
#include "typecodes.h"
#include "alctype.h"
#include "huff.h"

extern char tokentext[];
static int *tk_menu;
extern char Null_Str[];

#ifndef GEM
char *hit_spc = "Hit SPACE to resume ALICE editing";
#endif GEM

/* give help about actions taken on tokens */
int
acthelp( selected, menu )
int selected;
struct menu_data *menu;
{
#ifdef HAS_HELP
	nodep dummy;
	register int action;
	char afn[5];

	action = get_action( tk_menu[selected], &dummy, cursor );
	if( action >= 128 ) {
		char *nname;
		if( nname = NodeName(action & 127) )
			message( ER(208,"This will build %s '%s' structure"),
				strchr( "aeiou", *nname ) ? "an" : "a",
					nname );
		}
	 else {
#ifdef DEMO
		helpfile( "pactions/", "default" );
#else
		sprintf( afn, "%d", action );
		helpfile( "pactions/", afn );
#endif
		}
#endif HAS_HELP
}

/*
 * Routine called when user asks what tokens can be typed now.
 */
int
token_help()
{
#ifdef HAS_HELP
	register ActionNum *p;
	extern char *tokname();

	struct menu_data m;
	register int childclass;	/* used as class or type */

	int rv;			/* return variable */

	unsigned tn;		/* token number */

	int tno[128];		/* token number for action */
	int upkid, cpcode;	/* scan through lists */
	nodep old_ascend, ascend;	/* scan up parents */
	extern int phys_pcode;


	tk_menu = tno;
	new_tk_menu( &m );

	if (is_a_stub(cursor) ){
		childclass = (int)kid1(cursor);
		if (childclass >= CLIST(0))
			childclass -= CLIST(0);
#ifdef DB_HELP
		if (tracing) fprintf(trace, "childclass %d\n", childclass);
#endif
		p=Class_Actions[childclass];
		while( *p ) {
			tn = *p++ & 0377;	/* pick up token number */
			p++;			/* skip over action */
			m_token_add( tn, &m );
			}
		ascend = tparent(cursor);
		}
	else {
		ascend = cursor;
		}
	upkid = -1;
	cpcode = phys_pcode;
	old_ascend = NIL;
	while( ascend  ) {
		extern unsigned find_type();
		if( old_ascend )
			upkid = get_kidnum( old_ascend );
		find_type(0, ntype(ascend), upkid, cpcode, &m ); /* lint is confused? */
		old_ascend = ascend;
		cpcode = -1;
		ascend = tparent(ascend);
		}
		
	/* now scan up through nodes we are on */


#ifdef DB_HELP
	if (tracing) fprintf(trace, "menu with %d items\n", m.num_items);
#endif
	/* action help routine gives help for action codes */
	/* set external so it can help */
#ifdef GEM
	pop_menu( &m, 0L, FALSE, (char **)0, &rv );
#else
	rv = pop_menu(curr_window->w_desc, &m, acthelp );
#endif

#ifdef DB_HELP
	if (tracing) fprintf(trace, "got %d\n", rv);
#endif
#ifdef GEM
	if( rv < 0 )
		return;
#else
	if (rv <= 0)
		return;
#endif

	rv = tno[rv];

	/* For some tokens, the text matters - pick something vanilla */
	/* actuall for the constants we should use the prompt function */
	switch(rv) {
	case TOK_ID: sym_menu(cursor);		return;
#ifdef promptit
	case TOK_INT: getprline( LDS( 424, "Integer? " ), tokentext, 20); break;
	case TOK_NUMB: getprline( LDS( 425, "Real Number? "), tokentext, 30); break;
	case TOK_CHAR: getprline( LDS( 426, "Character? " ), tokentext+1, 1) ;
				tokentext[0] = tokentext[2] = 047;break;
	case TOK_STRING: getprline( LDS( 427, "String? "), tokentext+1, MX_WIN_WIDTH ); 
					tokentext[0] = '"'; break;
	case TOK_CMNT: getprline( LDS( 429, "Comment? "), tokentext, MX_WIN_WIDTH); break;
#else
	case TOK_INT:
	case TOK_NUMB:
	case TOK_CHAR:
	case TOK_STRING:
	case TOK_CMNT:
		warning( ER( 283,
			"You can enter a '%s' just by typing it in right now" ),
			tokname(rv));
		return;
#endif
	default: strcpy(tokentext, "&");	/* should be a "don't care" */
	}
#ifdef DB_HELP
	if (tracing) fprintf(trace, "returned %d, tokentext '%s'\n", rv, tokentext);
#endif
	in_token(rv,cursor);
#endif HAS_HELP
}

static bits8 tokmap[128/BITS_IN_BYTE];

new_tk_menu( m )
struct menu_data *m;
{
#ifdef HAS_HELP
	register int i;
	extern char menu_nop[];

	new_menu( m, LDS( 430, "Possible Input" ));
#ifndef GEM
	add_menu_item( menu_nop );
#endif
	/* clear allocated token map */
	for( i = 0; i < sizeof(tokmap); i++ )
		tokmap[i] = 0;
#endif
}

m_token_add( token, m )
unsigned token;
struct menu_data *m;
{
#ifdef HAS_HELP
	/* quit if already allocated */
	if( tokmap[ token >> 3 ] & 1 << (token & 7) )
		return;

	tk_menu[m->num_items] = token;
	add_menu_item( tokname(token) );
	tokmap[ token >> 3 ] |= 1 << (token & 7);
#endif
}
/* symbols you can get help on */

static type_code helpsyms[] = {
T_BTYPE, T_BBTYPE, T_FILENAME, T_BTFUNC, T_BTPROC, T_BPROC, T_BFUNC,
T_WRITELN, T_READLN, T_PROGRAM, T_BCONST, T_BENUM, 0 };

/*
 * Routine called when user asks what areas of pascal he can get help in
 */

#ifdef GEM
#define MENU_FIRST	0
#else
#define MENU_FIRST	1
#endif

int
pas_help(args)
char *args;
{
#ifdef HAS_HELP
	
	struct menu_data nc_menu;
	
	register nodep ascend;	/* go up tree collecting menu items */
	register char *nname;	/* name for each nodep */
	int selected;		/* which type of help */
	int amiret;		/* return from add menu */
	char tymask[128];	/* mask of help types */
	int tyindex;		/* index into mask */
	char *htype;		/* type of help */
	int bshelp = FALSE;	/* built in symbol in list */
	extern char menu_nop[];

	if( args ) {
		helpfile( "pnode/", args );
		return;
		}

	for( tyindex = 0; tyindex < sizeof(tymask); tyindex++ )
		tymask[tyindex] = FALSE;
	new_menu( &nc_menu, LDS( 431, "Pascal Help") );

#ifndef GEM
	add_menu_item( menu_nop );
#endif

	/* loop up tree, gather classes etc. */

	if( is_a_stub(cursor) ) {
		add_menu_item( classname( ex_class(cursor) ) );
		ascend = cursparent;
	}
	 else
		ascend = cursor;

	do {
		NodeNum astype;

		astype = ntype(ascend);
		while( ascend && ntype_info(astype) & F_NOSTOP ) {
			ascend = tparent(ascend);
			astype = ntype(ascend);
		}
		if( !ascend || is_root(ascend) )
			break;

		nname = tymask[astype] ? (char *)NULL : NodeName(astype);
		tymask[astype] = TRUE;
		/* what we really want to detect is being a builtin symbol.
		   In the future, the scope should detect this, or a bit
		   In the meantime, we get builtin routines */
		if( astype == N_ID )
			if( strchr( helpsyms, sym_dtype(kid_id(ascend)) ) ) {
				add_menu_item( sym_name(kid_id(ascend)));
				bshelp = TRUE;
				}
		ascend = tparent(ascend);

		if( !nname )
			continue;
		amiret = add_menu_item( nname );
	} while( amiret != ERROR );

#ifdef GEM
	pop_menu( &nc_menu, (funcptr)NULL, FALSE, (char **)0, &selected );
#else
	selected = pop_menu( curr_window->w_desc, &nc_menu, (funcptr)NULL );
#endif

	htype = "pnode/";
	if( selected == MENU_FIRST ) {
		if( is_a_stub(cursor ) )
			htype = "pclass/";
		else if( ntype(cursor) == N_ID && bshelp )
			htype = "psymbol/";
	}
	if( selected >= MENU_FIRST && selected < nc_menu.num_items ) {
		helpfile( htype, nc_menu.item[selected] );
	}
#endif HAS_HELP
}

#ifdef DIRECT_HELP

#define helpwrefresh(win)
#define helpdelwin(win)


static int	HWStartRow;
static int	HWHeight;

WINDOW *
helpwin( height, width, startrow, startcol )
int height, width, startrow, startcol;
{
	HWStartRow = startrow;
	HWHeight = height;

	return (WINDOW *)1;
}


#ifdef HYBRID
static cbufptr buf = 0;
extern char far *choosealloc();

static
gtbuf() {
	if( !buf )
		buf = (cbufptr)choosealloc( MX_WIN_WIDTH * sizeof(chtype) );
}
#endif

#ifndef GEM
static
helpwaddstr( win, row, col, str, pad )
WINDOW *win;
int row, col;
preg1 char *str;
int	pad;				/* 0 == no padding */
{
	register int	i;
	register int	l;
	int	llen;
	/* buffer must be in window segment */
#ifdef HYBRID
	gtbuf();
#else
	chtype	buf[MX_WIN_WIDTH];
#endif

	llen = MX_WIN_WIDTH-1 - col;
	l = min(strlen(str), llen);

#ifdef COLPOS
	for (i = 0; i < l; i++)
		buf[i] = str[i] | AC_WHITE;
#else
	or (i = 0; i < l; i++)
		buf[i] = str[i];
#endif
#ifdef DB_HELP
	if( tracing )
		fprintf( trace, "buffer %lx, len %d\n", buf, l );
#endif

	draw_chars(HWStartRow + row, col, buf, l);
}

#endif GEM

#ifdef msdos
#define	NWCORNER	0xc9 | AC_WHITE
#define	NECORNER	0xbb | AC_WHITE
#define	SWCORNER	0xc8 | AC_WHITE
#define	SECORNER	0xbc | AC_WHITE
#define	HORIZ		0xcd | AC_WHITE
#define	VERT		0xba | AC_WHITE
#else
#ifdef ICON
#define NWCORNER	3
#define	NECORNER	2
#define	SWCORNER	4
#define	SECORNER	5
#define	HORIZ		0
#define	VERT		1
#endif
#endif

helpbox( win, bc1, bc2, height )
WINDOW	*win;
char	bc1, bc2;				/* ignored */
int	height;
{
	register int	i;
#ifdef HYBRID
	gtbuf();
#else
	chtype	buf[MX_WIN_WIDTH];
#endif

	for (i = 1; i < MX_WIN_WIDTH-1; i++)
		buf[i] = HORIZ;
	buf[0] = NWCORNER;
	buf[MX_WIN_WIDTH-1] = NECORNER;
	draw_chars(HWStartRow, 0, buf, MX_WIN_WIDTH);

	buf[0] = SWCORNER;
	buf[MX_WIN_WIDTH-1] = SECORNER;
	draw_chars(HWStartRow + HWHeight - 1, 0, buf, MX_WIN_WIDTH);

	for (i = 1; i < MX_WIN_WIDTH-1; i++)
		buf[i] = ' ';
	buf[0] = VERT;
	buf[MX_WIN_WIDTH-1] = VERT;
	for (i = 1; i < HWHeight - 1; i++)
		draw_chars(HWStartRow + i, 0, buf, MX_WIN_WIDTH);
}

cls()
{
	register int	i;
#ifdef HYBRID
	gtbuf();
#else
	chtype	buf[MX_WIN_WIDTH];
#endif

	for (i = 0; i < COLS; i++)
		buf[i] = ' ';

	for (i = 0; i < LINES; i++)
		draw_chars(i, 0, buf, COLS);
}

#else

#define helpwin newwin
#define helpwrefresh wrefresh
#define helpdelwin delwin
#define helpbox( win, bc1, bc2, height ) box( win, bc1, bc2 )
#define	cls()		printf("\014\n "); fflush(stdout);

#ifndef GEM
helpwaddstr( win, row, col, str )
WINDOW *win;
int row, col;
char *str;
{
	wmove( win, row, col );
	waddstr( win, str );
}
#endif

#endif

#ifndef GEM

helpfile( hdir, subject )
char *hdir;		/* class of help needed */
char *subject;		/* subject of the help */
{
#ifdef HAS_HELP
	FILE *hfdesc;		/* help file */
	FILE *hopen();		/* returns helpfile */

	int llen;
	int hwidth, hheight;
	int gflag;			/* flag indicating get another line */
	int line_ctr;			/* line in help window */
	register WINDOW *hwin;
	char inbuf[MX_WIN_WIDTH];
	int ret;		/* menu return */
	char *argp;
	struct menu_data m;
	char *comp;
	char *menu_actions[MX_FILE_MENU];	/* file menu store */
	char *menutitle;			/* title of menu */
	register int menu_dex = 0;

	/* special label to avoid having to do infinite recursion */
restart_help:
	if( subject[0] == 'H' ) {
		++subject;
		hdir = "";
		}
	if( hfdesc = hopen( hdir, subject ) ) {

		hwidth = llen = COLS;
#ifdef ICON_WIN_MGR
		line_ctr = -1;
#else
		line_ctr = 0;
#endif
		hheight = LINES;
		hfgets( inbuf, MX_WIN_WIDTH, hfdesc );
		if( inbuf[0] == HELP_ESCAPE ) {
			hheight = atoi( inbuf + 1 ) + 2;
#ifdef notdef
			hwidth = COLS;
#else
			if( argp = strchr( inbuf, ' ' ) )
				hwidth = atoi( argp+1 ) + 2;
#endif
			gflag = FALSE;
			}
		 else {
			hclose( hfdesc );
#ifdef HUFFHELP
			close_help();
#endif
			message( ER(286,"Garbled help file." ) );
			return;
			}

		if( hheight > LINES )
			hheight = LINES;
		if( hwidth > COLS )
			hwidth = COLS;
#ifdef ICON_WIN_MGR
		hwin = pnewwin(hheight - 2, hwidth - 2,
				(LINES - hheight)*CHAR_HEIGHT,
				(llen - hwidth)*CHAR_WIDTH + CHAR_WIDTH/2,
				FALSE );
#else
		hwin = helpwin(hheight, hwidth, LINES - hheight,
				llen - hwidth );
#endif

#ifndef DIRECT_HELP
		if( hwin == (WINDOW *)ERR) {
			warning( ER(209,"Not enough memory for help screen") );
			hclose( hfdesc );
			return;
		}
#endif

#ifndef DIRECT_HELP
		curOff( hwin );
#endif
#ifdef DB_HELP
		if(trace)fprintf(trace,"hwin=%x, New Window for help rows %d cols %d, srow %d, scol %d\n", hwin, hheight, hwidth, LINES - hheight, llen - hwidth );
#endif
#ifdef DIRECT_HELP
#else
		werase( hwin );
#endif
		if( hheight + 1 < LINES )
			helpbox( hwin, '|', '-', hheight );
		 else {
#ifdef DIRECT_HELP
			cls();
#endif
			line_ctr = -1;
			}
		while( gflag || hfgets( inbuf, MX_WIN_WIDTH, hfdesc ) ) {
			RemoveNL( inbuf );
#ifdef DB_HELP
			if (trace)
				fprintf(trace, "read \"%s\"\n", inbuf);
#endif
			gflag = FALSE;
			if( inbuf[0] == HELP_ESCAPE ) {
				switch( inbuf[1] ) {
					case 'e':
						/* until jim takes them out */
						break;
					case 'm':
						menutitle =allocstring(inbuf+2);
						menu_dex = 0;
						new_menu( &m, menutitle );
						break;
					case 'h': /* a new file */
						goto out_while;
						
					case 'i':
						if(menu_dex >=MX_FILE_MENU-1)
#ifdef DB_HELP
							error(ER(48,"menu too long"));
#else
							break;
#endif DB_HELP

						comp = strchr(inbuf,',' );
						*comp = 0;
						menu_actions[menu_dex] = 
							allocstring(comp+1);
						add_menu_item(allocstring(
						  inbuf+2));
						++menu_dex;
		
						break;
					case 'p':
						helpwrefresh(hwin);
						ret = pop_menu( hwin, &m,(funcptr)NULL );
						strcpy( subject = inbuf, 
							menu_actions[ret] );

						/*free up memory used in menu */
						dmfree(menutitle);
						while(menu_dex--){
							dmfree(m.item[menu_dex]);
							dmfree(menu_actions[menu_dex]);
							}
						hclose(hfdesc);
						helpdelwin(hwin);

						if( ret >= 0 ) {
#ifdef DB_HELP
							if(trace)fprintf(trace,"More help subject of %s\n", subject );
#endif
							if( subject[0] == 'H' ){
#ifdef notdef /* screenimage? */
								refresh_scr();	
#endif
							/* avoid tail recursion
							  to save stack space */
								goto restart_help;
								}
							/* else */
							menuact( Null_Str, subject );
							}
						goto term_and_redisplay;
					default:
						bug( ER(252,"Funny Control code %c in help file"), inbuf[1] );
					}
				}
			 else {
				helpwaddstr( hwin, ++line_ctr, 1, inbuf );
				}
			}
	out_while:
#ifdef msdos
		wattron( hwin, A_BOLD );
#else	
		wstandout( hwin );
#endif
		helpwaddstr( hwin, ++line_ctr, (hwidth-strlen(hit_spc))/2, hit_spc );
		helpwrefresh( hwin );
#ifdef msdos
		wattroff( hwin, A_BOLD );
#else
		wstandend( hwin );
#endif
		while( readkey() != ' ' )
			;
		hclose(hfdesc);
		helpdelwin(hwin);
						 
		}
	 else {
give_warning:
#ifdef msdos
		warning( ER(210,"Help on %s not available.  Check to see if Help files are on your disk"), subject );
#else
		warning( ER(211,"Help on %s not available yet (%s%s)"),
			subject, hdir, subject );
#endif
	}
	term_and_redisplay:
		refresh_scr(FALSE);
#endif HAS_HELP
}

#endif GEM
