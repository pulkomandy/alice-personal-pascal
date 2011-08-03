/*
 * Routines to output a segment of the screen to a window
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "flags.h"
#include "break.h"
#include "search.h"

#ifdef DB_OUTPUT
# define PRINTT
#endif
#include "printt.h"

#define bufaddc( ch ) *lb_cursor++ = ch | chmask
extern buf_el line_buffer[MAX_LEN_LINE];
extern buf_el *lb_cursor;
extern int sline;

/*
 *  display a set of lines in a window
 *  
 */

int cur_line;		/* while printing */


#define FULL_SPEED TRUE
#define STAND_STILL FALSE

#ifdef TIMEIT
struct tbuffer {
	long utime;
	long stime;
	long cutime;
	long cstime;
	} obuf, nbuf;
#endif

int	ascii_output	= FALSE;	/* doing an ascii listing */
FILE	*asc_file;			/* ascii output file */
static bp_index;			/* back patch index */
static nodep *bparray;			/* where is backpatch array */

text_out( cline )
char *cline;
{
	char fname[MAX_LEN_LINE];
	char *p, *strrchr();
	extern FILE *qfopen();
	nodep backpatch[MAX_BACKPATCH];	/* where to backpatch */
	int i;				/* loop though backpatches */

#ifdef GEM
	BusyBee( TRUE );
#endif

	bparray = backpatch;	/* set external */
	bp_index = 0;

	filename(fname, cline, FALSE, (char *)0, FALSE);
	if( (p = strrchr( fname, '.' )) && case_equal( p+1, FileExt) ) 
		strcpy( p+1, TextExt);

	if( asc_file = qfopen( fname, "w" ) ) {
#ifdef DEMO
		slmessage( ER(226,"Printing to file %s (sort of)"), fname );
		fprintf( asc_file, "Sorry, this command is not included in the demonstration verison\n" );
		fprintf( asc_file, "In the real version, this would save a printable version of your program\n" );
		fprintf( asc_file, "in this file.  You could then list it, or even compile it with a normal\n" );
		fprintf( asc_file, "compiler.\n" );
		fprintf( asc_file, "\nSee the startup menu for how to get your copy of this system\n" );
#else
		ascii_output = TRUE;
		slmessage( ER(227,"Printing to file %s"), fname );

		clear_break();
		disp_lines( curr_window, 0, 99, cur_work_top, 0, 0 );
		ascii_output = FALSE;
#endif DEMO
		fclose( asc_file );
		}
	 else
		error( ER(87,"badasc`Can't open file %s"), fname );
	for( i = 0; i < bp_index; i++ )
		s_ntype( backpatch[i], N_HIDE );
#ifdef GEM
	BusyBee( FALSE );
#endif

}


disp_lines(xwin, physline, nlines, startnode, pcode_index, subline )
alice_window *xwin;		/* the window in which to draw */
unsigned physline;		/* which line in the window to begin */
unsigned nlines;		/* number of lines to display. 0 = all */
reg curspos startnode;		/* actual line being listed */
short pcode_index;		/* which print code to use */
bits8 subline;			/* which line to start on if multi-line */
{
	register alice_window *win = xwin;
	register int stop_line;		/* what line to stop printing on */
	struct scr_line ourline;	/* to scan along with */
#ifdef TIMEIT
	long retime, times();
	
	retime = times( &obuf );
#endif

	printt4("disp_lines(%x, %d, %d, %x,", win, physline, nlines, startnode);
	printt2(" %d, %d)\n", pcode_index, subline);

	cur_line = physline;
	stop_line = nlines ? physline+nlines : win->w_height;
	if( stop_line > win->w_height )
		stop_line = win->w_height;

	ourline.ind_level = calc_indent( tparent(startnode) );
	ourline.sub_line = subline;
	ourline.listptr = tparent(startnode);
	ourline.listindex = get_kidnum(startnode);
	ourline.pcode = pcode_index;


	/*
	 * Start searching before or after the current line, but
	 * not if would move off ends of workspace.
	 */
	if (Searching) {
		if (!((Searching == SRCH_FWD) ? go_to_next(&ourline, 1,1 ) :
						go_to_prev(&ourline)))
			goto noSearch;
		}

	while( cur_line < stop_line || ascii_output || Searching ) {
		if( !out_line( &ourline, win, FULL_SPEED, stop_line ) &&
				(cur_line < stop_line) ) {
			/* clear out the bottom part of the window */
			if( !(ascii_output || Searching)) {
				wmove( win->w_desc, cur_line, 0 );
				wclrtobot( win->w_desc );
				while( cur_line < stop_line )
					win->w_lines[cur_line++].listptr = NIL;
				}
			/* and quit */
			break;
			}
		}
noSearch:
	if (Searching)
		srchDone();

#ifdef TIMEIT
	fprintf( stderr, "Real time %d,", (int)(times( &nbuf ) - retime) );
	fprintf( stderr, " from %d + %d\n", (int)(nbuf.utime - obuf.utime),
		(int)(nbuf.stime - obuf.stime) );

#endif
}

int line_borders[MAX_LINES];	/* splits used in line output */

/* the routine to output a line at cur_line and move the pointer */

out_line(xline, xwin, code, stopline )
struct scr_line *xline;		/* description of line to output */
alice_window *xwin;		/* the window we output into */
int code;			/* code describing progress */
int stopline;			/* physical line to stop */
{
	register struct scr_line *line = xline;
	register alice_window *win = xwin;
	/* first output the line into the buffer */
	/* if the parent is a list we do a bit of special work.
	   Something has to be done about the default here */
	nodep outnode;			/* node we are printing */



	int main_indent, sub_indent;	/* number of characters of indent */
	int split_index;		/* to loop while splitting */

	int line_size;			/* how long the line is */
	int line_room;			/* how much room after indent */
	int vert_room;			/* how many lines to put it on */
	int line_no;			/* for looping through lines */
	extern int column_base;		/* for when treeprint line folds */
	struct scr_line *ole_line;
#ifdef QNX
	int ndone_kludge = TRUE;	/* do the treeprint, goto doesn't
					   work properly under Quantum */
#endif

	if( code != OLEOUT ) lb_cursor = line_buffer;	/* start of line */
	column_base = 0;
	main_indent = calc_main_indent( line->ind_level, win->w_width ); 

	/* stop indenting after a certain distance from the right */
	save_indent(main_indent);

	sub_indent = main_indent + TABSIZE / 2;
	out_flag = code > FULL_SPEED ? code : 0;

	if( code != OLEOUT ) {
		

	outnode = line_headnode((*line));
	pcod_adjust( line );
#ifndef DEMO
	if( ascii_output  ) {
		if( ntype(outnode) == N_FORWARD ) {
			symptr routname;
			routname = kid_id( kid1( outnode ) );
			if( tparent( routname ) ) {
				treeprint(tparent(routname),0,' ', (char *)NULL,
						LNIL );
				bufadd( " forward;" );
				*lb_cursor = 0;
				or_sym_mflags( routname, SF_ISFORWARD);
#ifdef QNX
				ndone_kludge = FALSE;
#else
				goto line_done; /* do not treeprint exception */
#endif
				}
			}
	  	else if( ntype(outnode) == N_HIDE && bp_index < MAX_BACKPATCH ){
			 if( node_2flag(outnode) & NF2_KEEP_HIDDEN ) {
				bufadd("{");
				treeprint(kid1(outnode),0,' ',NIL,NIL);
				bufadd( "}" );
				goto line_done;
				}
			  else {
				fprintf(asc_file, "{+Class %s}{+Hide}",
					classname( ex_class(outnode) ) );
				s_ntype(outnode, N_REVEAL);
				bparray[bp_index++] = outnode;
				}
			}
		else if( ntype(outnode) == N_REVEAL && !line->pcode ) {
			fprintf( asc_file, "{+Class %s}",
					classname(ex_class(outnode) ) );
			}
		}
#endif DEMO
	   if( is_a_stub(outnode) ) {
		/* go to the parent and find the default */
		/* outnode will not be nil if at the root */

		/* figure standout here based on parent */
		treeprint( outnode, 0, ' ', (char *)NULL, LNIL );
		line->pcode = 0;	/* stubs only one line */
		}
	 else {
#ifdef QNX
		if( ndone_kludge )
#endif
		treeprint( outnode, line->pcode, ' ', (char *)NULL, LNIL );
		}

	/* now if ascii output, print buffer and go to next */
#ifndef DEMO
	if( ascii_output ) {
		int i, col;
		char c;

	 line_done:
		c = '\n';
		col = 0;
		for(;;) {
			if( c == '\n' )
				for( i = 0; i < line->ind_level; i++ )
					efputc( '\t', asc_file );
			c = line_buffer[col++] & A_CHARTEXT;
			if( c )
				efputc( c, asc_file );
			 else
				break;
			}

		efputc( '\n', asc_file );
 		if( got_break() ) {
			clear_break();
			message( ER(212,"break`Break!") );
			return FALSE;
			}
		return go_to_next( line, 1,1 );
		}
#endif DEMO
#ifndef OLD_STUFF
	if (Searching) {
		char	srch_line[MAX_LEN_LINE];

		becpy(srch_line, line_buffer);
		if (matchPattern(srch_line, 0) >= 0) {
			Searching = SRCH_FOUND;
			cursor = node_kid(line->listptr, line->listindex);
			return 0;
			}
		else if (Searching == SRCH_FWD)
			return go_to_next( line, 1,1 );
		else
			return go_to_prev(line);
		}
#endif

	}  /* End of stuff not to execute for OLEOUT */

	/* line_buffer now contains the text of the line
	   we must split it up and output it based on the type of
	   printing we are doing */
	
	line_size = lb_cursor - line_buffer;
	line_room = win->w_width - main_indent;
	printt2( "line size = %d, room = %d\n", line_size, line_room );

#ifdef notdef
	if (out_flag == LOOK_CURSOR)
		save_ftext(lb_cursor);
#endif notdef

	split_index = 0;
	line_borders[0] = 0;
	/* calculate the boundaries upon which the line is folded. */
	/* This could do with being more intelligent */
	do {
		split_index++;
		line_borders[split_index] = line_borders[split_index-1]
			+ line_room;
		printt2( "line_bord %d = %d\n", split_index, line_borders[split_index] );
		line_size -= line_room;
		line_room = win->w_width - sub_indent;
		printt2( "size = %d, room = %d\n", line_size, line_room );
	} while( line_size >= 0 );
	line_borders[split_index] += line_size;

	/* split_index now contains the number of lines */

	/* Make sure that all the lines about to be displayed will actually
	 * appear on the screen (only for the one-line editor).  This is
	 * so that the user can see all of the line that he is editing
	 */
	printt1( "nlines = %d\n", split_index );
	if( (code == OLEOUT) && ( (sline + split_index) > win->w_height ) ) {
		/* looks like we have to scroll a bit */
		printt1( "scroll: sline = %d\n", sline);
		while( sline + split_index > win->w_height ) {
			/* decrement the startline (sline) and delete the
			 * top line */
 			sline--;
			printt1( "Scrolling... sline now = %d\n", sline );
			do_del_line( win, 0, (struct scr_line *)0 );
		}
		/* reset the line pointers for the new screen after the scroll 
		*/
		set_sc_fline( win, &(win->w_lines[0]) );
		/* and return TRUE so that the one-line editor calls
		 * treeprint again to actually display the line
		 * this time, without the hassle of scrolling the screen
		 */
		return( TRUE );
	}

/* If we are looking for where the cursor is on the screen, then calculate
 * which line it is on, etc.  If we came from the OLEDITOR, we still have
 * to calculate the line and column of the cursor so that we can position it
 * there afterwards
 */
	if( (code == LOOK_CURSOR) || (code == OLEOUT) ) {
		/* map columns for cursor into start and end */
		printt1( "Search real = %d\n", srch_real );
		for( line_no = 0; line_no < split_index; line_no++ ) {
			printt2( "line_borders[%d] = %d\n", line_no+1,
					line_borders[line_no+1] );
			if( srch_real < line_borders[line_no+1] ) {
				srch_row = cur_line + line_no - line->sub_line;
				srch_real = srch_real - line_borders[line_no]
					+ main_indent +(line_no ? TABSIZE/2:0);
				if( code == LOOK_CURSOR ) return(TRUE);
				goto ole_cont; 
				}
		}
		/* This checks to see if srch_real was set to the end of the
		 * buffer.  If so, it then calculates the row/col.
		 * This is for the one-line editor.
		 */
		if( srch_real == line_borders[line_no] ) {
			srch_row = cur_line + line_no - 1 - line->sub_line;
			srch_real = srch_real - line_borders[line_no-1] +
					main_indent +((line_no-1) ? TABSIZE/2:0);
			goto ole_cont;
		}
		bug( ER(263,"Can't find search start in out_line") );
	}
	/* if full speed, indicate that the size of this area is equal
	   to the number of lines we have to print */
ole_cont:
	/* Now we have to actually display the line in line_buffer */
	vert_room = code == FULL_SPEED ? split_index : line->sc_size;
	line->sc_size = split_index;
	
	printt2( "Fit line %d lines long into space %d in size\n", split_index,
				vert_room );
	/* if we are printing the last line in a sub-line, set this
	   appropriately */

	if( line->sub_line == MAX_LINES )
		line->sub_line = split_index - 1;

	/* loop printing as many lines as we are sure will fit, or all of
	   them if they do.  If we go off the edge of the screen, bypass
	   all special code */
	for( line_no = line->sub_line; line_no < split_index &&
			line_no < vert_room ; ) {
		if( cur_line >= stopline ){
			printt0("Going past end of screen\n" );
			return(FALSE);
		}
		printline( win, main_indent, line_no++, line );
	}

	printt1( "out_line: cur_line = %d\n", cur_line );	
	printt1( "         stop_line = %d\n", stopline );

/* the output line has been shortened, delete the extra ones */
	if( (line_no < vert_room) && (cur_line < stopline) ) {
		/* go over the extra lines, deleting them */
		while( line_no++ < vert_room ) {
			printt1( "Deleting line at %d\n", cur_line );
			scr_delete( win, cur_line );
			}
		}
/* if the output line is bigger than it was, we have to insert lines */
	 else if( split_index > vert_room ) {
		/* loop, inserting lines, bumping line->sc_size and going
		   on until the whole thing is printed or we have hit the
		   end of the screen */
		while( (line_no < split_index) && (cur_line < stopline) ) {
			scr_insert( win, cur_line );
			/* Now that we have opened up a line, print on it */
			printline( win, main_indent, line_no++, line );
			}
		}
	/* we have now filled out this line */
	
	if( code == OLEOUT ) return( FALSE );
	
	/* now find the next line of printing */

	if( cur_line >= stopline )
		return( FALSE );
	if( code == FULL_SPEED )
		return( go_to_next( line, split_index, split_index ) );
	return( FALSE );
}

/* put the line in the window */
/* print a single physical line in the specified window */
printline(xwin, indent_level, line_no, xline )
alice_window *xwin;		/* the window it goes into */
int indent_level;		/* how many chars */
int line_no;			/* which line */
struct scr_line *xline;		/* the line struct for entry in the table */
{
	register alice_window *win = xwin;
	register struct scr_line *line = xline;
 	int pokewhere;		/* where */
	buf_el removed;			/* what we removed */

	/* clear the change flag for this line */
	line->sc_change = FALSE;
	/* copy over line information */
	scrl_copy( win->w_lines[cur_line], *line );
	/* tell it which line it is (within the logical line) 
	 * if it is the first line, it is line zero 
         */
	win->w_lines[cur_line].sub_line = line_no;

	/* if it consists of multiple physical lines, then the indent level
         * is increased by half a tab 
         */
	if( line_no > 0 )
		indent_level += TABSIZE/2;
	
#ifndef notdef
	wmove( win->w_desc, cur_line, 0 );
	wclrtoeol( win->w_desc );
	wmove( win->w_desc, cur_line, indent_level );
#else notdef
	/* move the cursor to the beginning of the line */
	wmove( win->w_desc, cur_line, indent_level );
	wclrfrombeg( win->w_desc );
#endif notdef
	/* find out the end of the string for this line and insert a 
         * terminator, saving the character that was there */
	pokewhere = line_borders[line_no] + win->w_width - indent_level;
	if( pokewhere < MAX_LEN_LINE ) {
		removed = line_buffer[pokewhere];
		line_buffer[pokewhere] = 0;	/* set end of line to zero */
		}
#ifdef A_UNDERLINE
	wattrstr(
#else
	waddstr( 
#endif
	/* print the string on the screen, starting at the appropriate point */
	win->w_desc, &(line_buffer[line_borders[line_no]]) );
	/* it is possible we have to check for line fold here?? */
	/* if so, compare x from getyx to cur_line and see */
#ifdef notdef
	wclrtoeol( win->w_desc );
#endif notdef

#ifdef DB_OUTPUT
	if (tracing) {
		int i;
		fprintf(trace, "output line %d: \"", line_no);
		for (i=line_borders[line_no]; line_buffer[i]; i++) {
			if (line_buffer[i] & A_ATTRIBUTES)
				putc('\'', trace);
			putc(line_buffer[i]&A_CHARTEXT, trace);
		}
		fprintf(trace, "\"\n");
	}
#endif
	/* restore first char of next line */
	if( pokewhere < MAX_LEN_LINE )
		line_buffer[pokewhere] = removed;
	cur_line++;
}

pcod_adjust( line )
struct scr_line *line;
{
	register int pc_calc;
	NodeNum lhtype;

	printt3("pcod_adjust(%lx), line->pcode=%d, line_headnode(*line)=%lx\n",
		line, line->pcode, line_headnode(*line));

	lhtype = ntype(line_headnode(*line));

	if( line->pcode == MAX_PCODE ) {
		for( pc_calc = 0;
			print_codes(lhtype)[pc_calc];
			pc_calc++ )
				;
		line->pcode = pc_calc - 1;
		}
}

#ifndef GEM

buf_el Colour_Map[] = {
#ifdef IBMPC
AC_WHITE,	       /* default */
A_BOLD|AC_BLUE,	 /* stub */
A_BOLD|AC_RED,	  /* error */
A_BOLD|AC_WHITE,	/* keywords */
AC_WHITE,	       /* variables */
AC_WHITE,		/* named constants */
AC_YELLOW,	      /* comments */
AC_WHITE,	       /* char constant */
AC_WHITE,	       /* integer constant */
AC_WHITE,	       /* real constant */
AC_WHITE,	       /* string */
AC_GREEN|A_BOLD,	/* routine names */
AC_CYAN,		/* built in routines */
AC_WHITE,	       /* field names */
A_REVERSE|AC_MAGENTA,  /* undefined symbols */
AC_WHITE,	       /* labels */
AC_CYAN|A_BOLD		/* one line edit */
#else
0
#endif
};

#ifdef IBMPC
#define DEFC AC_WHITE
#else
#define DEFC 0
#endif

#ifndef A_BOLD
#define A_BOLD 0
#endif
/* map for colour card, b&w monitor  */
#ifdef ICON
# ifdef ICON_WIN_MGR
buf_el Attr_Map[MAX_COLOURS] = {
DEFC,		      /* default 0 a */
A_UNDERLINE,	     /* stub 1 b */
A_BOLD|_STANDOUT,	     /* error 2 c */
A_BOLD,		      /* keywords 3 d */
DEFC,		      /* variables 4 e */
DEFC,			/* named constants 5 f */
0x400,			/* comments 6 g */
DEFC,		      /* char constant 7 h */
DEFC,		      /* integer constant 8 i */
DEFC,		      /* real constant 9 j */
DEFC,		      /* string 10 k*/
DEFC,		      /* routine names 11 l */
DEFC,		      /* built in routines 12 m */
DEFC,		      /* field names 13 n */
DEFC|_STANDOUT,		/* undefined symbols 14 o */
DEFC,		      /* labels 15 p */
0x4000		     /* one line edit 16 MOVE THIS */
};
# else ICON_WIN_MGR
buf_el Attr_Map[MAX_COLOURS] = {
DEFC,		     /* default */
UL_UNDERLINE,	     /* stub */
REV_BIT,	     /* error */
FONT_BOLD,	      /* keywords */
DEFC,		      /* variables */
DEFC,		      /* named constants */
FONT_ITALIC,	      /* comments */
DEFC,		      /* char constant */
DEFC,		      /* integer constant */
DEFC,		      /* real constant */
DEFC,		      /* string */
DEFC,		      /* routine names */
DEFC,		      /* built in routines */
DEFC,		      /* field names */
REV_BIT,	     /* undefined symbols */
DEFC,		      /* labels */
UL_DOTTED		  /* one line edit */
};
# endif
#else
buf_el Attr_Map[MAX_COLOURS] = {
DEFC,		      /* default */
#ifdef GEM
A_UNDERLINE|DEFC,
#else
A_BOLD|DEFC,		 /* stub */
#endif
DEFC|_STANDOUT,	     /* error */
DEFC,		      /* keywords */
DEFC,		      /* variables */
DEFC,		      /* named constants */
DEFC,		      /* comments */
DEFC,		      /* char constant */
DEFC,		      /* integer constant */
DEFC,		      /* real constant */
DEFC,		      /* string */
DEFC,		      /* routine names */
DEFC,		      /* built in routines */
DEFC,		      /* field names */
DEFC|_STANDOUT,	     /* undefined symbols */
DEFC,		      /* labels */
A_BOLD|DEFC		  /* one line edit */
};
#endif

#ifndef A_UNDERLINE
#define A_UNDERLINE 0
#endif
/* map for monochrome card */
buf_el Mono_Map[MAX_COLOURS] = {
DEFC,		      /* default 0 a */
A_UNDERLINE,	     /* stub 1 b */
DEFC|_STANDOUT,	     /* error 2 c */
DEFC,		      /* keywords 3 d */
DEFC,		      /* variables 4 e */
DEFC,			/* named constants 5 f */
A_BOLD|DEFC,		/* comments 6 g */
DEFC,		      /* char constant 7 h */
DEFC,		      /* integer constant 8 i */
DEFC,		      /* real constant 9 j */
DEFC,		      /* string 10 k*/
DEFC,		      /* routine names 11 l */
DEFC,		      /* built in routines 12 m */
DEFC,		      /* field names 13 n */
DEFC|_STANDOUT,		/* undefined symbols 14 o */
DEFC,		      /* labels 15 p */
A_UNDERLINE	     /* one line edit 16 MOVE THIS */
};

#endif GEM

/* map from symbol types to our colour types */
bits8 Symc_map[] = {
0,
15, 5, 5, 4, 4, 
4, 4, 14,
13, 4, 4, 4, 11, 11,
11, 11, 12, 12, 12, 12, 12, 12,
5, 5, 4, 4,
/* BENUM, MEM, PORT */
5, 4, 4
};


#ifdef GEM

#define DEFC	AC_BLACK

buf_el LoResMap[] = {
DEFC,	       		/* default */
A_BOLD|AC_BLUE,	 	/* stub */
A_BOLD|AC_RED,	  	/* error */
A_BOLD|DEFC,		/* keywords */
AC_BLACK,	       	/* variables */
AC_BLACK,		/* named constants */
AC_LBLACK,		/* comments */
AC_BLACK,	        /* char constant */
AC_BLACK,	        /* integer constant */
AC_BLACK,	        /* real constant */
AC_BLACK,	        /* string */
AC_GREEN|A_BOLD,	/* routine names */
AC_CYAN,		/* built in routines */
AC_BLACK,	        /* field names */
A_REVERSE|AC_MAGENTA,   /* undefined symbols */
AC_BLACK,	        /* labels */
AC_CYAN|A_BOLD,		/* one line edit */
0, 0, 0
};

buf_el MedResMap[] = {
DEFC,	       		/* default */
A_UNDERLINE|DEFC, 	/* stub */
AC_RED,		  	/* error */
A_BOLD|DEFC,		/* keywords */
AC_BLACK,	       	/* variables */
AC_BLACK,		/* named constants */
DEFC,			/* comments */
AC_BLACK,	        /* char constant */
AC_BLACK,	        /* integer constant */
AC_BLACK,	        /* real constant */
AC_BLACK,	        /* string */
AC_GREEN,		/* routine names */
AC_GREEN|A_BOLD,	/* built in routines */
AC_BLACK,	        /* field names */
A_REVERSE|AC_RED,	/* undefined symbols */
AC_BLACK,	        /* labels */
AC_GREEN|A_DOTUNDERLINE,/* one line edit */
0, 0, 0
};

buf_el HiResMap[] = {
DEFC,	       		/* A - default */
A_UNDERLINE|DEFC, 	/* B - stub */
A_REVERSE|DEFC,	  	/* C - error */
A_BOLD|DEFC,		/* D - keywords */
DEFC,		       	/* E - variables */
DEFC,			/* F - named constants */
DEFC,			/* G - comments */
DEFC,		        /* H - char constant */
DEFC,		        /* I - integer constant */
DEFC,		        /* J - real constant */
DEFC,		        /* K - string */
DEFC,			/* L - routine names */
DEFC,			/* M - built in routines */
DEFC,		        /* N - field names */
A_REVERSE|DEFC,		/* O - undefined symbols */
DEFC,		        /* P - labels */
DEFC|A_DOTUNDERLINE,	/* Q - one line edit */
0, 0, 0 
};

buf_el Attr_Map[] = {
DEFC,	       	/* default */
DEFC,		/* stub */
DEFC,	  	/* error */
DEFC,		/* keywords */
DEFC,	       	/* variables */
DEFC,		/* named constants */
DEFC,		/* comments */
DEFC,	        /* char constant */
DEFC,	        /* integer constant */
DEFC,	        /* real constant */
DEFC,	        /* string */
DEFC,		/* routine names */
DEFC,		/* built in routines */
DEFC,	        /* field names */
DEFC,   	/* undefined symbols */
DEFC,	        /* labels */
DEFC,		/* one line edit */
0, 0, 0
};

#endif GEM
