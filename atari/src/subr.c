#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "jump.h"
#include "keys.h"
#include "class.h"
#include "dbflags.h"

#ifdef DB_SUBR
# define PRINTT
#endif
#include "printt.h"

#ifdef SAI
extern int sai_out;
#endif

#ifdef PARSER
arg_error( str, val ) 
char *str;
char *val;
{
	fprintf( stderr, "Error:");
	fprintf( stderr, str, val );
}
#endif

int error_present =0;  /* Nonzero if there's an error message shown */
char last_ebuf[MX_WIN_WIDTH + 50];
char *last_error = (char *)0;   /* copy of most recent error message */
char last_ecode[20] = "";/* help code for error message */

/*
 * A fatal error has occurred - print error message and go back
 * for another command.
 */

#ifndef GEM

/*VARARGS1*/
error(msg, p1, p2, p3, p4)
ErrorMsg msg;
int p1, p2, p3, p4;
{
#ifdef notdef
	message( "%r", &msg );
#else
	message(msg, p1, p2, p3, p4);
#endif
#ifndef SAI
	flushla();
#endif

#ifdef SAI
	sai_print( "Execution aborted\n" );
	done((char *)0);
#else
	err_return();
#endif SAI

}

#else GEM

/*
 * Non-Critical error, just put it on the info line of the edit
 * window
 */
ncerror( format, p1, p2, p3, p4 )
ErrorMsg format;
char *p1, *p2, *p3, *p4;
{
	message( format, p1, p2, p3, p4 );
	err_return();
}

/*
 * A Real error happened, put up a dialog box
 */
error( format, p1, p2, p3, p4 )
ErrorMsg format;
char *p1, *p2, *p3, *p4;
{
	flushla();

	ErrAlert( format, p1, p2, p3, p4 );

	err_return();
}

/*
 * Put the message in an alert box, chopping it up to fit on multiple
 * lines, and if there is help, offer a help button
 */

ErrAlert( format, p1, p2, p3, p4 )
ErrorMsg format;
char *p1, *p2, *p3, *p4;
{
	/*
	 * Copy it into the error buf
	 */
#ifdef OLMSG
	sprintf( last_ebuf, getERRstr( format ), p1, p2, p3, p4 );
#else
	sprintf( last_ebuf, format, p1, p2, p3, p4 );
#endif

	PureAlert();
}

PureAlert()
{
	char *mptr;
	int has_help = FALSE;
	char msg[100];

	mptr = strchr( last_ebuf, '`' );

	/*
	 * There is a help file associated
	 */
	if( mptr ) {
		*mptr = 0;
		strcpy( last_ecode, last_ebuf );
		*mptr = '`';
		mptr++;
		has_help = TRUE;
	} else {
		/* No help file associated */
		strcpy( last_ecode, "" );
		mptr = last_ebuf;
	}
	last_error = last_ebuf;

	/*
	 * Now that we have the string, if it is too long, chop it
	 * up by putting in '|' bars
	 */
	ChopStr( mptr );

	/*
	 * If there is help, offer to look at it
	 */
#ifndef SAI
	if( has_help ) {
		sprintf( msg, LDS( 398, "[2][%s][ OK | HELP ]"), mptr );
		if( form_alert( 1, msg ) == 2 ) {
			helpfile( "perror/", last_ecode );
		}
	} else
#endif SAI
	{
		sprintf( msg, LDS( 399, "[2][%s][ OK ]"), mptr );
		form_alert( 1, msg );
	}
}

#define CHOP_WIDTH 30

ChopStr( str )
char *str;
{
	char *ptr;

	/*
	 * If we have already inserted '|' bars, then just return
	 */
	if( strchr( str, '|' ) ) return;

	for( ;; ) {

		if( strlen( str ) < CHOP_WIDTH ) return;

		/*
		 * String is too long, chop it down to multiple lines
		 */	
		ptr = &str[CHOP_WIDTH - 1];

		while( ptr != str && *ptr != ' ' ) ptr--;

		if( ptr == str ) {
		/*
		 * We have a problem, we have CHOP_WIDTH continuous chars
		 * arbitrarily put an '|' bar at the end of the section
		 */
			str[CHOP_WIDTH - 1] = '|';
			str += CHOP_WIDTH;

		} else {
			*ptr = '|';
			str = ptr + 1;
		}
     	}
}
#endif GEM

/* return from runtime error */

#ifndef SAI
re_return(){
	extern int win_default;
	if( !(work_debug( curr_workspace ) & DBF_ON ) ) {
		slmessage( ER(233,"A runtime error has occured (see above).  Hit any key to continue") );
#ifndef GEM
		readkey();
#endif
		redraw_status = TRUE;
		reswflags( curWindows[MAINWIN], win_default );
		display(TRUE);
		}
	err_return();
}
#else
re_return() {
	sai_print( LDS( 400, "A runtime error has occured\n" ) );
	done((char *)0);
}
#endif SAI


/*
 * Print an error message on the error line.
 */

message( msg, p1, p2, p3, p4 )
ErrorMsg msg;
int p1, p2, p3, p4;
{
	nonum_message(getERRstr(msg), p1, p2, p3, p4);
}

/*
 * Put up a message on the info line, but strip it of any help file
 * info
 */

nonum_message(xmsg, p1, p2, p3, p4)
char * xmsg;
int p1, p2, p3, p4;
{
	register char *msg;
	register char *mgp;
	msg = xmsg;

	sprintf(last_ebuf, msg, p1, p2, p3, p4);

	if( mgp = strchr( last_ebuf, '\n' ) )
		*mgp = 0;
	if( mgp = strchr( last_ebuf, '`' ) ) {
		*mgp = 0;
		strcpy( last_ecode, last_ebuf);
#ifdef GEM
		*mgp = '`';
#endif
		last_error = mgp + 1;
	}
	else {
		last_error = last_ebuf;
	}
	if (strlen(last_error) >= MX_WIN_WIDTH - 1)
		strcpy(&last_error[MX_WIN_WIDTH - 4], "...");

	pure_message( last_error );
}

#ifdef SAI
pure_message( msg )
char *msg;
{
	char linbuf[100];
	if( msg == NULL ) return;
	sprintf( linbuf, "%s\n", msg );
	sai_print( linbuf );
}
#else
#ifdef GEM
/*
 * Just put the plain text on the info line of the current edit window
 */
pure_message( msg )
char *msg;
{
	char line[100];

#ifdef DEBUG
	if (tracing) {
		fprintf(trace, "Error: ");
		fprintf(trace, msg);
		fprintf(trace, "\n");
		fflush(trace);
	}
#endif

	STMessage( curr_window->w_desc, msg );
	error_present = 4;

}
#else
pure_message( msg )
char *msg;
{
	int i, width;
	extern int windows_made;
	WINDOW *win = curWindows[ERRWIN];

	if (msg == NULL)
		return;	/* bullet proof */
#ifdef DEBUG
	if (tracing) {
		fprintf(trace, "Error: ");
		fprintf(trace, msg);
		fprintf(trace, "\n");
		fflush(trace);
	}
#endif
	if (stdscr && win && windows_made ) {
		suspendGraphics();		/* sigh */

		/* curses has been started up */
		wmove(win, 0, 0);
		wstandout(win);
		waddstr(win, msg);
		width = win->_maxx;
		for( i = strlen(msg); i < width; i++ )
			waddch( win, ' ' );
		wstandend(win);
		error_present = 2;	/* gone after two commands */
		touchwin(win);/* NECESSARY! */
		wrefresh(win);
	} else {
		fprintf(stderr, "\n\n%s\n", msg );
		fflush(stderr);
	}
}
#endif
#endif

repeat_err(){
	if( last_error )
#ifdef GEM
		PureAlert( last_ebuf );
#else
		pure_message( last_ebuf );
#endif
	 else
		warning( ER(234,"noerr`You have not had an error yet!") );
}

#ifndef GEM
redraw_errline()
{
	if (error_present > 0)
		pure_message(last_error);
	else
		pfkeyslabel();
}
#endif

/* Clear any message that's in the error message window */
clear_em(redo)
int redo;
{
#ifndef SAI
	printt1("clear_em, error_present %d\n", error_present);

	if( redo )
		error_present = 1;
	if( error_present > 0 ) {
		if( !--error_present ) {
#ifdef GEM
			STMessage( curr_window->w_desc, "" );
#else
			pfkeyslabel();
			wrefresh( curWindows[MAINWIN] );
#endif
			}
		}
#endif
}

/* A bug in Alice has been found - create core to help track it down */
/*VARARGS1*/
bug(msg, p1, p2, p3, p4)
ErrorMsg msg;
int p1, p2, p3, p4;
{

	char msgbuf[80];

	strcpy(last_ebuf, LDS( 470, "Internal error: " ));
#ifdef notdef
	sprintf( last_ebuf + strlen(last_ebuf), "%r", &msg );
#else
	sprintf(last_ebuf+strlen(last_ebuf), getERRstr(msg), p1, p2, p3, p4);
#endif

#ifdef DEBUG
	/* While debugging, bugs leave a core se we can track it down */
	pure_message(last_error = last_ebuf );
	if(tracing)fflush(trace);
	if (stdscr)
		endwin();
	if(tracing)fflush(stdout);
	dump(cur_work_top);
	if(tracing)fflush(trace);
	if(etrace) fflush(etrace);
#ifdef PROC_EDITOR
	quitit();
#endif
	alexit(1);
#else DEBUG
	/* In production mode, bugs just act like user errors */
	pure_message(last_error);
#ifndef SAI

#ifdef GEM
	if( form_alert( 1, LDS( 471, 
 "[2][An Internal Error has occurred|Continue ?][ OK | Abort ]" )) == 1 )
		err_return();
#else
	getprline( "Continue? (y/n)", msgbuf, sizeof(msgbuf) );
	space_strip( msgbuf );
	if( msgbuf[0] == 'y' )
		err_return();
#endif

#endif SAI

#ifdef msdos
	msdoscursor( TRUE );
#endif

#ifdef SAI
	done();
#else SAI

#ifdef PROC_EDITOR
	quitit();
#endif
	alexit(1);

#endif SAI
#endif DEBUG
}

#ifdef msdos

msdoscursor( on )
int on;
{
#ifndef SAI
	if( on ) {
		register WINDOW *win = curWindows[CMDWIN];
		if( stdscr && win ) {
			wclear( win );
			wrefresh(win);
			}
		_poscurs( LINES-2, 0 );
		}

	 else
		_poscurs( 255, 255 );
#endif SAI
}
#endif

/* tree segment block move routine. */
t_blk_move( _to, _from, bytes )
reg char *_to;		/* destination block start */
reg char *_from;		/* source block start */
reg int bytes;		/* how many to move */
{
#ifdef msdos
# ifdef ES_TREE
	dos_blk( _to, tr_segment, _from, tr_segment, bytes );
# else
	blk_move( _to, _from, bytes );
# endif
#else
	register char far *to = tfarptr( _to );
	register char far *from = tfarptr( _from );

	if( to < from )
		while( bytes-- )
			STAR(to)++ = STAR(from)++;
	 else {
		to += bytes;
		from += bytes;
		while( bytes-- )
			STAR(--to) = STAR( --from );
		}
#endif
}

#ifdef msdos
dos_blk( destoff, destseg, srcoff, srcseg, bytes )
unsigned int destoff, destseg, srcoff, srcseg, bytes;
{
	long dest, src;


	dest = ((long)destseg << 4 ) + destoff;
	src = ((long)srcseg << 4 ) + srcoff;

	if( dest < src )
		bmove( destoff, destseg, srcoff, srcseg, bytes, FALSE );
	 else
		bmove( destoff+bytes-sizeof(char), destseg,
			srcoff+bytes-sizeof(char), srcseg, bytes, TRUE );
}
#endif
	

/*
 * routine to lookup entries in byte tables.
 * assumes - tables of bytes with related information.
 * terminated by zero entry.
 * returns pointer to one past the lookup token or zero.
 *
 * CAUTION: if the table contains a zero key, that will
 * terminate the search.
 */

bits8 *
table_lookup( table, key, size )
bits8 *table;	/* table to look in */
reg bits8 key;	/* what are we looking for? */
reg int size;	/* how long are the entries */
{
	register bits8 *srch_ptr;		/* for going through */

	if( table ) 
		for( srch_ptr = table; *srch_ptr; srch_ptr += size )
			if( *srch_ptr == key )
				return( srch_ptr + 1 );
	return (bits8 *)NULL;
}

#ifdef QNX
char *
strrchr(_s, _c)
char	*_s;
char	_c;
{
	register char	*s	= _s;
	register char	c	= _c;
	register char	*where	= NULL;

	for (;;) {
		if (*s == c)
			where = s;
		if (*s++ == NULL)
			return where;
	}
}

int
strncmp(_s1, _s2, n)
char	*_s1;
char	*_s2;
int n;
{
	register char   *s1 = _s1;
	register char   *s2 = _s2;
	register int	i;

	for (i = 0; i < n; i++, s1++, s2++)
		if (*s1 < *s2)
			return -1;
		else if (*s1 > *s2)
			return 1;
		else if (*s1 == '\0')
			return 0;
	return 0;
}

getuid()
{
	return 1;
}

#endif QNX

statusLine()
{
/*
 * For GEM, write the suspend info to the title of the pg_out window
 */
#if !defined(SAI)
	extern int count_suspend;
	extern int call_depth;
	extern int call_current;
	extern unsigned	MemUsed;
	extern int code_damaged;
	char *fn;
	nodep sblock, get_sblock();
	char *rname;
	register NodeNum btype;
	register WINDOW *win = curWindows[CMDWIN];
	nodep bkid;

	fn = work_fname(curr_workspace);

#ifndef GEM
	curOff(win);
	wmove(win, 0, 0);
#ifdef ICON_WIN_MGR
	wstandend(win);
#else
	wstandout( win );
#endif

#endif GEM

	if( count_suspend && (sblock = get_sblock()) ) {
		char line[80];

#ifdef GEM
		sprintf( line, LDS( 473, "Frame %2.2d/%2.2d in " ),
			call_current, call_depth );
#else
		sprintf( line, "%s(%1d) frame %2.2d/%2.2d in ",
			code_damaged ? "Suspend" : "SUSPEND",
			count_suspend, call_current, call_depth );
#endif
		bkid = kid1(sblock);
		btype = ntype(sblock);
		if( (btype == N_DECL_PROC || btype == N_DECL_FUNC) &&
					not_a_stub(bkid) )
			rname = sym_name( (symptr)kid1(sblock) );
		else if( btype == N_PROGRAM )
			rname = LDS( 475, "<Program>" );
		else if( btype == N_IMM_BLOCK )
			rname = LDS( 476, "<Immediate>" );
		else
			rname = "???";
#ifdef GEM
		sprintf( line+strlen(line), "%s %s(%d)", rname,
			code_damaged ? LDS( 477, "Suspend" ) :
				LDS( 482, "SUSPEND"),
			count_suspend );

		PgTitle( line );
#else
		sprintf( line+strlen(line), "%-12.12s", rname );
		wprintw( win, line );
#endif
		}
	 else {
#ifndef GEM
#if defined(HYBRID) || defined(ES) || defined(LARGE)
			char *mlstring();
			char mlbuf[10];

			wprintw( win, "%-26s  %-11s",
				"ALICE: The Personal Pascal", mlstring(mlbuf) );
#else
			wprintw( win, "%-26s", "ALICE: The Personal Pascal" );
			wprintw( win, "  Used: %2uK", (512+MemUsed)/1024);
#endif
#endif GEM
		}

#ifdef GEM
	SetCurWinTitle();
#else
	wprintw(win, " File%c %-16.16s WS: %-9.9s",
		node_2flag(curr_workspace) & WS_DIRTY ? '?' : ':',
		fn ? fn : "<None>", work_name(curr_workspace));
	wclrtoeol(win);
	wstandend(win);
	redraw_status = FALSE;
	wrefresh(win);
#endif GEM

#endif SAI
}
ErrorMsg last_slmsg;
/*VARARGS1*/
slmessage( str, strarg )
ErrorMsg str;
char *strarg;
{
#if !defined(SAI) && !defined(GEM)
	register WINDOW *win = curWindows[CMDWIN];

	last_slmsg = str;

	if (okayWithGraphics()) {
		wmove(win, 0, 0);
		wstandout( win );
		wprintw( win, getERRstr(str), strarg );
		wclrtoeol(win);
		wstandend(win);
		wrefresh(win);
		redraw_status = TRUE;
		}
#endif SAI && GEM
}

pfkeyslabel()
{
#if !defined(SAI) && !defined(GEM)

	wmove(curWindows[ERRWIN], 0, 0);
#ifdef ICON_WIN_MGR
	wstandend(curWindows[ERRWIN]);
#else
	wstandout(curWindows[ERRWIN]);
#endif
	waddstr(curWindows[ERRWIN], PFKEYNAMES);
	wclrtoeol(curWindows[ERRWIN]);
	wrefresh(curWindows[ERRWIN]);

#endif SAI && GEM
}

#ifdef I_CHECKSUM

struct checkArea {
	unsigned	chksum;
	unsigned char	*start;
	unsigned char	*end;
} CheckAreas[] = {
	0,	&User_spare,	0x1000,
	0,	0x1000,		0x2000,
	0,	0x2000,		0x3000,
	0,	&G_Stub,	&/*ZAAAP...was built_count*/
	0,	&NodeStart,	&NodeEnd,
	0,	0,		0
};

extern int	checklevel;		/* 0 = off, 1 = data, 2 = data & code */
static unsigned codeChkSum;
static FILE	*con;

/*
 * Checksum the code and constant data, and moan if they change.
 */
checksum(whence)
int	whence;
{
	register unsigned	*up;
	register unsigned char	*p;
	register unsigned	sum;
	struct checkArea	*ap;

	if (!checklevel)
		return;

	if (!con) {
		con = fopen("[1]$tty2", "w");
		fprintf(con, "Checksum enabled\n");
	}

	/*
	 * Checksum the code.
	 */
#ifdef ICON
	if (checklevel > 1) {
		sum = 0;
		asm("push es");
		asm("mov ax,cs");
		asm("mov es,ax");
		for (up = 0; up < (unsigned *)0xfa00; ) {
			sum += @up++;		/* Unroll the loop, as */
			sum += @up++;		/* this is a slow operation! */
		}
		asm("pop es");
		if (sum != codeChkSum && whence)
			fprintf(con, "\007checksum(%d) code, was %d is %d\n",
				whence, codeChkSum, sum);
		codeChkSum = sum;
	}
#endif

	/*
	 * Checksum the data.
	 */
	for (ap = CheckAreas; ap->end != NULL; ap++) {
		sum = 0;
		for (p = ap->start; p < ap->end; p++)
			sum += *p;
		if (ap->chksum != sum && whence)
			fprintf(con, "\007checksum(%d) %x--%x was %d is %d\n",
				whence, ap->start, ap->end, ap->chksum, sum);
		ap->chksum = sum;
	}
}
#endif CHECKSUM

/* find the expected class for a cursor position */
ClassNum
ex_class(xcp )
curspos xcp;		/* cursor location class is desired of */
{
	register curspos cp = xcp;
	register ClassNum rclass;

/*	if (is_a_stub(cp))	this was too clever for words
		return (int) kid1(cp); */
	if( is_a_list( tparent(cp) ) ) {
		cp = tparent( cp );
		}
	rclass = kid_class( ntype(tparent(cp)), get_kidnum( cp ) ) & CLASSMASK;
	if( rclass == C_PASSUP )
		if (tparent(cp))
			rclass = ex_class( tparent(cp) );
		else
			bug(ER(267,"ex_class fails "));
	return rclass;
}

/* safe string for printf */

#ifdef DEBUG
char *
Safep( str )
char *str;
{
	return str ? str : "**Null-String**";
}
#endif

/* remove leading and trailing spaces */

space_strip( str )
char *str;
{
	register char *p1, *p2;

	p1 = str;
	while ( *p1 == ' ' )
		++p1;
	p2 = str + strlen(str) - 1;
	if( strlen(str) ) {
		while( *p2 == ' ' )
			p2--;
		p2[1] = 0;
		}
	if( p1 != str )
		blk_move( str, p1, strlen(p1)+1 );
}
		

#if defined(msdos) && defined(LARGE)

fp_error()
{
	run_error( ER(314, "fper`Floating point error (overflow)" ) );
}
#endif
