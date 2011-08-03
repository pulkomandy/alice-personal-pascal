
#define INCLUDEIO

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "jump.h"
#include "menu.h"
#include "keys.h"
#include "alctype.h"
#include "mem.h"

#ifdef DB_MAIN
# define PRINTT
#endif
#include "printt.h"

#ifdef ICON
#define ICON_DISP 1
#endif

/* Set up the stack size on the ST */
#ifdef ATARI520ST
#ifdef MarkWilliams

#include <osbind.h>

/*
 * Define stack size
 */

long _stksize = 0x1800;

/*
 * QFopen directory to search
 */
char *apdir = (char *)0;

/*
 * Compile String
 */
char *cpl_string = (char *)0;

/*
 * Install Exception vectors or not
 */
int ExcInstall = TRUE;

#endif
#endif

extern FILE *qfopen();

#ifndef GEM
struct al_window main_win;	/* initial window */
#endif

curspos cursor;			/* main node the cursor points at */
int novice;			/* is the user a novice */
extern int in_execws;		/* inside special exec */
extern int CheckSpeed;

#ifdef MC68000
stacksize max_stack_size = MAX_STACK_SIZE;
stacksize lc_stack_size = LOC_STACK_SIZE;
#else
unsigned int max_stack_size = MAX_STACK_SIZE;
unsigned int lc_stack_size = LOC_STACK_SIZE;
#endif
int imm_only = FALSE;
int cantredraw = TRUE;
int can_dump = FALSE;

FILE *etrace = 0;			/* execution debug */
FILE *trace = 0;		/* for debugging - use if non-null	*/
int ed_level = 0;
#ifdef DEBUG
char trace_name[20] = "atrace";	/* editor trace file name */
#endif

int redraw_status = TRUE;	/* set if status is to be redrawn */

extern buf_el Attr_Map[MAX_COLOURS];

extern int	ascii_output;

#ifdef ICON
extern int	TBGearRatio;	/* track ball gear ratio */
#endif

#ifdef TURBO
int turbo_flag = TRUE;
#endif

WINDOW	*curWindows[MAXWINDOWS];
#ifdef DEMO
WINDOW *demo_win;
int demo_mode = 0;
#else
#define demo_mode 0
#endif

extern int multi_monitor;

#ifdef unix
int montype = 1;
#else
int montype = 0;		/* monitor type. 1=underline 2=colour */
#endif

int maccount = 0;		/* number of macros defined */
int win_default;		/* default window flags */

#ifdef QNX
char *confname = "/config/ap.init";
#else
# ifdef msdos
char *confname = "?:\\ap.ini";
# else
#  ifdef ATARI520ST
char *confname = "?:\\ap.ini";
#else
char *confname = "/u/alice/ap.init";
#  endif
# endif
#endif

char *origconf;

#ifdef LOADABLE_TEMPLATES
#ifdef ATARI520ST
char *templ_fname = "?:\\aptempla.suf";
#else
char *templ_fname = ALICETPL;
#endif
#endif

int user_conf = FALSE;
int error_count = 0;
int gotenvname = 0;		/* did we get a name from the environment */
extern int indent_end;
#if !(defined(HYBRID) || defined(ES) || defined(LARGE) )
#define meminit()
#endif
#ifndef COLPOS
#define setscol(col)
#endif

#ifdef ATARI520ST
char *help_prefix = "?:\\helpfile";
#else
#ifdef DEMO
char *help_prefix = "?:/demohelp";/* prefix used to get help huffman files */
#else
char *help_prefix = "?:/helpfile";	/* prefix used to get help huffman files */
#endif
#endif

char print_names = TRUE;
FILE *confile = NULL;
int minit_done = FALSE;		/* memory init done */

#ifdef LARGE
long TotalMem = 0l;
#endif
char	*LoadFile	= 0;

main(argc, argv)
char **argv;
{
	int	init_stat = TRUE;

#ifdef DEBUG
	if( argc > 1 && !strcmp( argv[1], "t=" ) )
		trace = stdout;
#endif

#ifdef LARGE
	TotalMem = getFreeParas() * 16L;
#endif

	/* before we do anything else, load the error messages */
#ifdef QNX


	if (argc == 0) {
		fprintf(stderr, "Use: %s [file] [C=confile] [s=varstacksize] [f=checking freq]\n             [l=locstacksize] [m#=macrostring]* [c#=fontcode]* [-indent]\n", argv[0] );
		exit(1);
	}

	if (argv[0][0] == '/' )
		far_load( argv[0], 1 );
	 else {
		extern char tokentext[];
		sprintf( tokentext, "/user_cmds/%s", argv[0] );
		far_load( tokentext, 1 );
		}

#endif

	/*
	 * Put aside a little extra memory in case we run out
	 */
#ifndef HYBRID
	ExtraSysMem = mmalloc(EXTRASYSMEM);
#endif

	if( !errset() ) {
#ifdef msdos
		msdoscursor( TRUE );
#endif
		fprintf( stderr, LDS( 450, "Error on startup of ALICE\n" ));
		exit(1);
		}

#ifdef QNX
	/* Grow the segment */
	GrowMem();
#endif


#ifdef ES 
	esinit();
#endif

	/*
	 * Evaluate command line and .init file options
	 */
#ifdef HYBRID
	{
	int ret;
	ret = initSegAlloc();
	printt1( "init of segalloc returns %d\n", ret );
	if( ret ) {
		fprintf( stderr, "Failure initializing memory\n" );
		alexit(1);
		}
	}
#endif
#ifdef msdos
	/* init stack checking variable */
	{
	int ret;
	extern unsigned stacklimit;
	ret = stackinit();
	printt2( "stack limit is %x stack is %x\n", stacklimit, ret );
	}
#endif
	/* now we load the templates */
	origconf = confname;

	while( TRUE ) {
		char *argline;
		char lbuf[MAX_LEN_LINE];

#ifdef msdos
		if( !gotenvname ) {
			char *envres;
			char *getenv();
			gotenvname = TRUE;
			if( envres = getenv( "APINIT" ) ) {
				confname = envres;
				origconf = confname;
				}
			}
#endif msdos

		if( confname != NULL ) {
			if( confile )
				fclose(confile);
			confile = qfopen( confname, "r" );
			/* no warning for default config file */
			if( user_conf && !confile )
				error( ER(69,"Can't open config file %s"), confname );
			confname = (char *)NULL;
			}
		if( confile ) {
/* fix */		if( fgets( lbuf, MAX_LEN_LINE-1, confile ) ) {
				argline = lbuf;
				RemoveNL( lbuf );
				}
			  else {
				fclose(confile);
				confile = (FILE *)NULL;
				continue;
				}
			}
		 else {
			/* no file open, get a command line arg */
			argv++;
			argc--;
			argline = argv[0];
			if( argc <= 0 )
				break;
			}
		do_cloption( argline, confile != 0 );
		 
		}


#ifdef GEM
	GemInit();		/* Initialize GEM (resource file, etc) */
#endif

	printt0( "About to do initscr\n" );
	initscr();

#ifndef GEM
	win_default = stdscr->_flags;
#else
	win_default = AC_BLACK;
#endif

#ifdef msdos
	ersetup();	/* error handler vector */
#endif
	set_mtype(0);
#ifdef LOADABLE_TEMPLATES
	if( LoadNodes(templ_fname) ) {
		printt0( "Templates loaded...\n" );
		}
	 else {
		endwin();
		fprintf( stderr, LDS( 451, "Error in loading templates\n" ));
		alexit(1);
		}
#endif

	here(3);
	checkfar(3);
#ifdef XDEBUG
	if( ed_level ) {
#ifdef ICON
		etrace = trace;
#else
		etrace = fopen( "etrace", "w" );
#endif
#ifdef unix
		if( ed_level < 0 ) {
			setbuf( etrace, (pointer)NULL );
			ed_level = -ed_level;
			}
#endif unix
		}
#endif

	init_ws();
	buildin();

	checkfar(4);
	initWindows();

	set_wscur( curr_window,  cur_work_top );

	initGraphics();

#ifndef GEM	
	if (print_names) {
		setscol( AC_RED|A_BOLD );
		center( 3, "ALICE: The Personal Pascal" );
		setscol( AC_YELLOW );
		center( 5,"Copyright (c) 1985 by Looking Glass Software Limited" );
#ifdef msdos
#ifndef ICON_DISP
		center( 6, "Published by Software Channels Inc." );
#endif
#endif
		setscol( AC_CYAN | A_BOLD );
		center( 8,"Designed by Brad Templeton" );
#ifndef ICON_DISP
		center( 10, "Additional work by Jim Gardner, Jan Gray and David Rowley");
#else
		center( 10, "Additional work by Jim Gardner, Jan Gray, David Rowley & Rob Ferguson" );
#endif
		setscol( AC_RED | A_BOLD );
#ifdef DEMO
		center( 11, "Demonstration Copy, Version 1.3.2 (No Save, Limited Memory)" );
		if( !demo_mode )
			center(12,"For evaluation only - choose selection \"F\"" );
#else
# ifdef LARGE
		center( 12, "Release SCI-1.3.1 LARGE" );
# else
		center( 12, "Release SCI-1.3.2 BETA" );
# endif
# ifdef msdos
		center( 13, "Software Channels Support - 416/967-4290" );
# endif
#endif
		if( !demo_mode ) {
#ifdef ICON_DISP
			center( 13,"Custom Version for the Ontario Ministry of Education");
#endif
		}
		setscol( AC_WHITE );
		refresh();
	} 

#else GEM
	
#endif GEM

#ifdef GEM
	CalcMemLeft();
#endif
	init_interp();

#ifdef msdos
	dos_break(FALSE);
#endif

	here(5);
	checkfar(5);

	if( !errset() )
		if( error_count++ > 10 ) {
			printf( "too many errors on startup, bye-bye" );
		
#ifdef msdos
			msdoscursor( TRUE );
			errestore();
#endif
			endwin();
			alexit(1);
			}
			/* first level error return point */

	if (LoadFile) {
		meminit();
		minit_done = !(init_stat = aload(LOAD, LoadFile)) ;
		}
#ifdef QNX
	/* flush input buffer */
	while( getbc(TRUE) )
		getbc(FALSE);
#endif

#ifdef GEM
	if( init_stat )
		init_pwork( curr_workspace );
#else

	while (init_stat) {
		switch( com_menu( M_init, TRUE ) ) {
		case 0:
			init_pwork(curr_workspace);
			init_stat = FALSE;
			break;
		case 1:
			init_pwork(curr_workspace);
			meminit();
			minit_done = !(init_stat = aload(LOAD, (char *)0));
			break;
		case 2:
			done((char *)0);
		case 3:
			imm_ws();		/* immediate mode */
			init_stat = FALSE;
			imm_only = TRUE;
			break;
		case 4:
			helpfile( "misc/", "intro" );
			init_stat = TRUE;
			break;
		case 5:
#ifndef ICON_DISP
#ifdef DEMO
			helpfile( "misc/", "dgetone" );
#else
			helpfile( "misc/", "getone" );
#endif
			init_stat = TRUE;
			break;
		case 6:
#endif
#ifdef DEMO
			demo_mode = 1;
			init_demo();
			make_std_windows();
			refresh_scr();
			init_stat = TRUE;
#endif
			break;
	
		}
	}

#endif

	can_dump = TRUE;
	redraw_status = TRUE;
	pfkeyslabel();
	werase( curr_window->w_desc );

	clean_curws();
	if( maccount < 5 ) {
		extern char *keymacros[];
		warning( ER(219,"nomac`Few macros defined. (Can't find %s) Press ESC to quit"), origconf );
		keymacros[AL_HELP] = "\200help\n";
		keymacros[033] = "\200exit\n";
		keymacros[CNTL('X')] = "\200";
		}
#if defined(HYBRID) || defined(ES) || defined(LARGE)
	/* set up memory counters */
	if( !minit_done )
		meminit();
#endif
#ifdef CHECKSUM
	csuminit();
#endif

#ifdef GEM
	/* Install the exception vectors */
	if( ExcInstall ) installExceptions();

	if( setjmp( err_buf ) == 2 ) {
		/*
		 * Some sort of exception happened, handle it
		 */
		printExceptions();
	}
#else
	errset(); 		/* set for exception handling return */
#endif

	ascii_output = FALSE;
	in_execws = FALSE;		/* not in special exec */
	catchsigs();

	printt0("Ready to call mainloop after errset\n");

	cantredraw = FALSE;

	mainloop();

	/* Normally never get here - exit command calls done directly. */
	endwin();
	alexit(0);
}

alexit(val)
int val;
{
#ifdef HYBRID
	wrapupSegAlloc();
#endif
	exit(val);
}

/* Routine called upon exiting the editor */
done(args)
char	*args;
{
#ifdef GEM
	char *str;
	extern int LoRes;
#endif

	if (!(args && args[0])) 
		if (!imm_only && anyDirtyWS()) {
#ifdef GEM
			if( LoRes )
				str = LDS( 452, 
"[2][You have not saved one|of your workspaces|Quit anyway ?][ Yes | No ]");
			else
				str = LDS( 453, 
"[2][You have not saved one|of your workspaces|Quit anyway ?][ Quit | Don't Quit ]");

			if( form_alert( 2, str ) == 2 ) return;
		}
#else
			if (!com_menu(M_quit, TRUE))
				return;
#endif

#ifndef GEM
	if( stdscr ) {
		wclear( stdscr );
		wmove(stdscr, 0, 0);
		wrefresh(stdscr);
		endwin();
		}
#endif

#ifdef GEM
	/*
	 * Free up the curses windows, and the gem windows too
	 */
	GWinFree();
	GemTerm();
	if( ExcInstall ) restoreExceptions();

#endif

#ifdef msdos
	dos_break(TRUE);
	errestore();
#endif msdos

#ifdef PROC_EDITOR
	quitit();
#endif
	exit(0);
}

#ifdef COLPOS
setscol( colour )
unsigned colour;
{
	extern int mon_is_colour;

#ifndef GEM
	if( mon_is_colour )
		reswflags( stdscr, colour );
#endif

}
#endif

static
center( line, string )
int line;
char *string;
{
	int pos = (COLS - strlen(string))/2;
	pos = pos < 0 ? 0 : pos;
	move( line + (demo_mode ? 2 : 0), pos );
	addstr( string );
}

char eschars[] = { 'n', 'c', 'h', 'u', 'd',
		'l', 'r', 'b', 's', '\\', 'f', 0 };
bits8 escmaps[] = { '\n', AL_ICOMMAND, KEY_BACKSPACE, KEY_UP, KEY_DOWN,
		KEY_LEFT, KEY_RIGHT, 4 /*ctrl d */, AL_ID, '\\', AL_FNAME };

macroize( str )
char *str;
{
	/* this routine actually steps on the string */

	register char *src, *dest;

	src = dest = str;

	while( *src ) {
		if( *src == '\\' ) {
			char *res;
			if( res = strchr( eschars, *++src ) )
				*dest++ = escmaps[res-eschars];
			 else
				*dest++ = *src;
			if( !*src )
				break;
			++src;
			}
		 else
			*dest++ = *src++;
		}
	*dest = 0;
}


int bugcount = 0;
int didbreak;
#ifdef unix
#include <signal.h>

sigoccurred(sn)
int sn;
{
	if (++bugcount > 20)
		done((char *)0);
	catchsigs();
	if (sn == SIGINT) {
		didbreak = TRUE;
		bugcount = 0;
		}
	 else
		error(ER(70,"Internal error: UNIX signal %d"), sn);
}

static int gotdeath = 0;

static
abfush(sn)
{
	if( gotdeath > 2 )
		abort();
	++gotdeath;
#ifdef DEBUG
	if( trace ) {
		fflush(trace);
		}
	if( etrace )
		fflush( etrace );
#endif
#ifndef GEM
	wstandend( stdscr );
	waddstr( stdscr, "Oh No!" );
	wrefresh(stdscr);
#endif

	endwin();
	abort();	
}

catchsigs()
{
	signal(SIGINT, sigoccurred);
/*	signal(SIGQUIT, abfush); */
#ifdef SIGSEGV
	signal(SIGSEGV, abfush);
#endif
#ifdef SIGBUS
	signal(SIGBUS, abfush);
#endif
	signal(SIGILL, abfush);
}
#else unix
catchsigs()
{
	/* Should insert other OS dependent interrupt handling here */
}
#endif

#ifndef GEM
set_mtype( val )
int val;
{
#ifdef msdos
	extern buf_el Colour_Map[MAX_COLOURS], Mono_Map[MAX_COLOURS];

	if( val )
		montype = val;
	if( !montype ) {
		extern int can_underline;
		extern int mon_is_colour;

		/* here 1 means monochrome and 2 means colour montitor */
		if( can_underline )
			montype = 1;
		if( mon_is_colour )
			montype = 2;
		}
#endif
	if( montype == 1 )
		blk_move( Attr_Map, Mono_Map, sizeof(Attr_Map) );
	else if( montype == 2 )
		blk_move( Attr_Map, Colour_Map, sizeof(Attr_Map) );
}

#else GEM
set_mtype( mode )
int mode;
{
	extern buf_el HiResMap[], MedResMap[], LoResMap[];

	int rez = Getrez();

	/*
	 * If the -c option was specified, use the HiRes color map
	 */
	if( mode == 1 )
		rez = 2;

	switch( rez ) {
		case 2:
			blk_move( Attr_Map, HiResMap, sizeof( Attr_Map ) );
			break;
		case 1:
			blk_move( Attr_Map, MedResMap, sizeof( Attr_Map ) );
			break;
		case 0:
			blk_move( Attr_Map, LoResMap, sizeof( Attr_Map ) );
			break;
	}
}
#endif GEM

#ifndef OLMSG
static char opterstr[] = "Bad option %s\n";
#endif
int change_stacksize;
int turbo_relax;		/* relax var parameter typecheck */
extern int turbo_io;		/* reset & text files of turbo pascal */

do_cloption( argline, mustmalloc )
char *argline;
int mustmalloc;		/* argline stuff will go away */
{
	register char *argstr;
	char	c;
	int	macnum;
#ifdef MC68000
	long	argval;
	extern	long atol();
#else
	int	argval;
#endif
	int	isplus;
	extern char *keymacros[];

	if( !argline )
		error( ER(71,opterstr), "<null>" );

	printt1( "Process option line %s\n", argline );

	if( *argline == '#' ) return;

	if (argstr = strchr(argline, '=')) {
		argstr++;

		c = argline[0];
#ifdef MC68000
		argval = atol( argstr );
#else
		argval = atoi(argstr);
#endif

		switch( c ) {
#ifdef ICON
			case 'g':	/* trackball gear ratio */
				TBGearRatio = argval;
				break;
#endif
#ifdef GEM
			case 'd':
				apdir = allocstring( argstr );
				break;
#endif
			case 'C':
				confname = argstr;
				user_conf = TRUE;
				break;
			case 's':
				max_stack_size = argval;
				change_stacksize = TRUE;
				break;
			case 'l':
				lc_stack_size = argval;
				change_stacksize = TRUE;
				break;
#ifdef LOADABLE_TEMPLATES
			case 'T':	 /* template file */
#ifndef DEBUG
			case 't':
#endif
				templ_fname = allocstring(argstr);
				break;
#endif
#ifndef GEM
			case 'L':
				/* screen lines */
				LINES = argval;
				break;
#endif
			case 'c':
				macnum = argline[1] - 'a';
				Attr_Map[macnum] = argval<<8;
#ifndef GEM
				set_mtype( -1 );	/* cancel mon setup */
#endif
				break;
			case 'm':
				if( isdigit( argline[1] ) )
					macnum = atoi( argline+1 );
				 else if( argline[1] == 047 ) /* ' */
					macnum = argline[2];
				 else if( argline[1] == '^' )
					macnum = argline[2] & 0x1f;
				 else
					error( ER(72,"opt`Invalid m#= option") );
				if (macnum >= 0 && macnum < HIGHKEY) {
					macroize(argstr);
					keymacros[macnum] = mustmalloc ?
						allocstring(argstr) :
						argstr;
					maccount++;
				} else
					error(ER(73,"opt`macro %d out of range"), macnum);
				break;
			case 'i': /* init string */
				macroize(argstr); macpush(argstr);
				break;
			case 'h':
				help_prefix = allocstring(argstr);
				break;
#ifdef DEMO
			case 'd': /* automatic demo */
				demo_mode = argval;
				break;
#endif
			case 'f': /* checking frequency */
				CheckSpeed = argval;
				break;
			/* Set debugging levels */
#ifdef XDEBUG
			case 'x':	/* execution debug only */
				ed_level = argval;
				break;
#endif
#ifdef DEBUG
			case 'b':	/* execution and md */
				ed_level = argval;
			/* Set trace file names, open them */
			case 't':	/* trace file */
			case 'u':	/* unbuffered trace file */
				if (*argstr)
					strcpy(trace_name, argstr);
				else
					sprintf(trace_name, "atr%d", getuid());
				if (c != 'f')
					trace = fopen(trace_name, "w");
#ifndef qnx
				if (c == 'u')
					setbuf( trace, (pointer)NULL );
#endif
				break;
#endif DEBUG
#ifdef GEM
			case 'p':
				cpl_string = allocstring( argstr );
				break;
#endif
			default:
				error( ER(71,opterstr), argline );
			}
		}
	else if( (isplus = argline[0] == '+') || argline[0] == '-' ) {

		switch( argline[1] ) {

#ifdef COLPOS
			case 'c': /* colour */
				/* monitor type -1 means keep default */
#ifdef GEM
				if( !isplus )
					set_mtype( 1 );
#else
				set_mtype(isplus ? 2 : -1 );
#endif
				break;
#ifndef GEM
			case 'u': /* underline */
				set_mtype( 1 );
				break;
#endif

#endif
			case 'q':	/* quick: no init menu */
				print_names = FALSE;
				break;
			case 'i':
				indent_end = isplus;
				break;
#ifdef KEYFILE
			case 'k': {
				extern FILE *keyfile;
				extern int keyfwrite;
				keyfwrite = isplus;
				keyfile = fopen( "keylog", isplus ? "wb":"rb" );
				}
				break;
#endif
#ifdef ECURSES
#ifndef QNX
			case 'f':
				{
				extern int does_flicker;
				does_flicker = isplus;
				}
				break;
#endif
#endif

#ifdef DEMO
			case 'd':
				demo_mode = 1;
				break;
#endif
#ifdef multi_works
			case 'm':
				multi_monitor = isplus;
				break;
#endif

			case 's':
				{
				extern bool noforce;
				noforce = !isplus;
				}
				break;
#ifdef TURBO
			case 't':
				turbo_flag = isplus;
				break;
			case 'b':
				turbo_io = isplus;
				break;
			case 'v':
				turbo_relax = !isplus;
				break;
#endif
#ifdef GEM
			case 'x':
				ExcInstall = isplus;
				break;
#endif GEM
			default:
				error( ER(71,opterstr), argline );
			}
		}
	else
		LoadFile = allocstring(argline);
}

#ifdef QNX

unsigned malloc_total = 0;

GrowMem()
{
	char *ptrs[45];
	char *c;
	int i;
	int amount = 20000;

	i = 0;
	while( (amount > 10) && (i < 45) ) {
		
		while( (i < 45) && ((c = mmalloc(amount)) != NULL) ) {
			malloc_total += amount;
			ptrs[i++] = c;
		}
		amount /= 2;
	}
	i--;
	while( i >= 0 ) {
		dmfree( ptrs[i] );
		i--;
	}
}
#endif QNX

#ifdef HERE
gotHere(file, line)
char	*file;
int	line;
{
	if (tracing)
		fprintf(trace, "[At line %d in %s]\n", line, file);
}
#endif
