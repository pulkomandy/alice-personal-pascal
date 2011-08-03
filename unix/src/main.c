
#define INCLUDEIO

/*
 *DESC: Main entry point for ALICE, option processing, initializing etc.
 */

#include "alice.h"
#include <curses.h>
#include <string.h>
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

extern FILE *qfopen();

#ifdef ATARI520ST
#ifdef MarkWilliams
long _stksize = 0x1800;
#endif
#endif

struct al_window main_win;	/* initial window */

curspos cursor;			/* main node the cursor points at */
int novice;			/* is the user a novice */
extern int in_execws;		/* inside special exec */
extern int CheckSpeed;

unsigned int max_stack_size = MAX_STACK_SIZE;
int imm_only = FALSE;
unsigned int lc_stack_size = LOC_STACK_SIZE;
int cantredraw = TRUE;
int can_dump = FALSE;

FILE *etrace = 0;			/* execution debug */
FILE *dtrace = 0;		/* for debugging - use if non-null	*/
int ed_level = 0;
char trace_name[20] = "atrace";	/* editor trace file name */
int redraw_status = TRUE;	/* set if status is to be redrawn */
extern buf_el Colour_Map[MAX_COLOURS], Mono_Map[MAX_COLOURS],
		Attr_Map[MAX_COLOURS];

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

int maccount = 0;		/* number of macros defined */
int win_default;		/* default window flags */

#ifdef QNX
char *confname = "/config/ap.init";
#else
# ifdef msdos
char *confname = "?:\\ap.ini";
# else
char *confname = "ap.init";
# endif
#endif
char *origconf;

#ifdef LOADABLE_TEMPLATES
char *templ_fname = ALICETPL;
#endif

int user_conf = FALSE;
int error_count = 0;
int gotenvname = 0;		/* did we get a name from the environment */
extern int indent_end;
#if !(defined(HYBRID) || defined(ES) || defined(LARGE) )
#define meminit()
#endif

#ifdef DEMO
char *help_prefix = "?:/demohelp";/* prefix used to get help huffman files */
#else
char *help_prefix = "?:/helpfile";	/* prefix used to get help huffman files */
#endif
char print_names = TRUE;
FILE *confile = NULL;
int minit_done = FALSE;		/* memory init done */

#ifdef LARGE
long TotalMem = 0l;
#endif
char	*LoadFile	= 0;

static center( int line, char *string );

main(argc, argv)
char **argv;
{
	int	init_stat = TRUE;
	extern unsigned int getFreeParas();

#ifdef DEBUG
	if( argc > 1 && !strcmp( argv[1], "t=" ) )
		dtrace = stdout;
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

#ifdef Q201
	if (argv[0][0] == '/' )
		far_load( argv[0], 1 );
	 else {
		extern char tokentext[];
		sprintf( tokentext, "/user_cmds/%s", argv[0] );
		far_load( tokentext, 1 );
		}
#endif

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
		fprintf( stderr, "Error on startup of ALICE\n" );
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

	printt0( "About to do initscr\n" );
	initscr();
	win_default = stdscr->_flags;

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
		fprintf( stderr, "Error in loading templates\n" );
		alexit(1);
		}
#endif

	here(3);
	checkfar(3);
#ifdef XDEBUG
	if( ed_level ) {
		etrace = fopen( "etrace", "w" );
#ifdef unix
		if( ed_level < 0 ) {
			setbuf( etrace, (pointer)NULL );
			ed_level = -ed_level;
			}
#endif unix
		}
#endif

	init_predef();
	init_ws();
	buildin();
	set_wscur( &main_win,  cur_work_top );

	checkfar(4);

	initWindows();

	initGraphics();

	
	if (print_names) {
		setscol( AC_RED|A_BOLD );
		center( 3, "ALICE: The Personal Pascal" );
		center( 5,"Copyright (c) 1985,2000 by Brad Templeton" );
		setscol( AC_CYAN | A_BOLD );
		center( 8,"Designed by Brad Templeton" );
		center( 10, "With Jim Gardner, Jan Gray and David Rowley");
		setscol( AC_RED | A_BOLD );
		center( 12, "Release LGS-1.3" );
		center( 14, "ALICE Pascal is now free, but unsupported." );
		center( 15, "http://www.templetons.com/brad/alice.html" );
		setscol( AC_NORMAL );
		refresh();
	} 


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
		char *dot;
		int stat;
#ifdef unix
		if( (dot = strrchr( LoadFile, '.' )) &&
				case_equal(dot, ".pas") ) {
			stat = convert_to_ap( LoadFile );
			if( stat ) {
				warning( "Can't covert Pascal source %s",
					LoadFile );
				goto init_loop;
				}
			 else
				strcpy( dot, ".ap" );
			}
#endif /*unix*/
		meminit();

		minit_done = !(init_stat = aload(LOAD, LoadFile)) ;
		}
#ifdef QNX
	/* flush input buffer */
	while( getbc(TRUE) )
		getbc(FALSE);
#endif

    init_loop:
#ifdef GEM
	init_pwork( curr_workspace );
	init_stat = FALSE;
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

#endif GEM

	can_dump = TRUE;
	redraw_status = TRUE;
	pfkeyslabel();
	werase( main_win.w_desc );

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

	errset(); 		/* set for exception handling return */
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
	if (!(args && args[0])) 
		if (!imm_only && anyDirtyWS())
			if (!com_menu(M_quit, TRUE))
				return;

	if( stdscr ) {
		wclear( stdscr );
		wmove(stdscr, 0, 0);
		wrefresh(stdscr);
		endwin();
		}
#ifdef msdos
	dos_break(TRUE);
	errestore();
#endif msdos

#ifdef PROC_EDITOR
	quitit();
#endif
	exit(0);
}


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
int escmaps[] = { '\n', AL_ICOMMAND, KEY_BACKSPACE, KEY_UP, KEY_DOWN,
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

void
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

static void
abfush(sn)
{
	if( gotdeath > 2 )
		abort();
	++gotdeath;
#ifdef DEBUG
	if( dtrace ) {
		fflush(dtrace);
		}
	if( etrace )
		fflush( etrace );
#endif
	wstandend( stdscr );
	waddstr( stdscr, "Oh No!" );
	wrefresh(stdscr);
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
	int	argval;
	int	isplus;
	extern char *keymacros[];

	if( !argline )
		error( ER(71,opterstr), "<null>" );

	printt1( "Process option line %s\n", argline );

	if( *argline == '#' ) return;

	if (argstr = strchr(argline, '=')) {
		argstr++;
		argval = atoi(argstr);
		switch( c = argline[0] ) {
#ifdef ICON
			case 'g':	/* trackball gear ratio */
				TBGearRatio = argval;
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
				templ_fname = allocstring(argstr);
				break;
#endif
			case 'L':
				/* screen lines */
				LINES = argval;
				break;
			case 'c':
				macnum = argline[1] - 'a';
				Attr_Map[macnum] = argval<<8;
				set_mtype( -1 );	/* cancel mon setup */
				break;
			case 'm':

				if( isdigit( argline[1] ) ) 
					macnum = strtol( argline+1, NULL,
						argline[1] == '0' ? 8 : 10 );
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
					dtrace = fopen(trace_name, "w");
#ifndef qnx
				if (c == 'u')
					setbuf( dtrace, (pointer)NULL );
#endif
				break;
#endif DEBUG
			default:
				error( ER(71,opterstr), argline );
			}
		}
	else if( (isplus = argline[0] == '+') || argline[0] == '-' ) {
		switch( argline[1] ) {
#ifdef COLPOS
			case 'c': /* colour */
				/* monitor type -1 means keep default */
				set_mtype(isplus ? 2 : -1 );
				break;
			case 'u': /* underline */
				set_mtype( 1 );
				break;
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
		fprintf(dtrace, "[At line %d in %s]\n", line, file);
}
#endif

#ifdef unix

#include <sys/wait.h>

convert_to_ap( char *fname )
{
	int kid;
	int status;

	if( kid = fork() ) {
		if( kid < 0 )
			return -1;
		wait( &status );
		return (WIFEXITED(status)) ? (WEXITSTATUS(status)) : -1;
		}
	 else {
		execlp( "apin", "apin", "+s", fname, NULL );
		fprintf( stderr, "Can't run apin program to convert .pas to .ap" );
		exit(1);
	 	}
	return 0;
}

#endif /* unix */
