#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "input.h"

#ifndef RELEASE
# define PRINTT
#endif
#include "printt.h"

FILE *dtrace;			/* debugging output */
nodep forest[MAXTREES];		/* parent of PROGRAM */
nodep cursor;			/* quick cursor to ref in builtin */
int listing = 0;		/* nonzero if user wants a listing */
#ifdef LOADABLE_TEMPLATES
char *templ_fname = ALICETPL;
#endif

int	Errors	= 0;

#ifdef foo
int	turbo_flag	= FALSE;	/* accept Turbo Pascal extensions by default */
#else
int	turbo_flag	= TRUE;	/* accept Turbo Pascal extensions by default */
#endif

extern workspace main_ws;
extern int smallflag;			/* small model flag */
workspace *curr_workspace = &main_ws;	/* the current workspace */

main(argc,argv)
int argc;
char **argv;
{
	int res;
	char trace_name[30];
	char *inf_name = NULL;
	char *outf_name = NULL;
#ifdef TURBO
	char *lib_name = 0;
	int gotalib = FALSE;
#endif
	register int c;
	extern FILE *yyin;
	extern char	*yyfilename;
	int	silent	= FALSE;

#ifdef LOADABLE_TEMPLATES
	LoadNodes(ALICETPL);
#endif
	dtrace = NULL;
	root = tree( N_NULL, NIL );
	buildin();

	for (argv++, argc--; argc > 0; argv++, argc--) {
		if (argv[0][1] == '=') switch( c = argv[0][0] ) {
#ifndef RELEASE
			/* Set trace file names, open them */
			case 't':	/* trace file */
			case 'u':	/* unbuffered trace file */
			case 'f':	/* file name only, for later */
				if (argv[0][2])
					strcpy(trace_name, argv[0]+2);
				else
					sprintf(trace_name, "atr%d",
#ifndef UNIX
								1);
#else
								getuid());
#endif
				if (c != 'f')
					dtrace = fopen(trace_name, "w");
#ifndef qnx
				if (c == 'u')
					setbuf( dtrace, (char *)NULL );
#endif
				break;
#endif RELEASE
			case 'i':	/* input file */
				inf_name = argv[0] + 2;
				break;
			case 'o':	/* output file */
				outf_name = argv[0] + 2;
				break;
#ifdef TURBO
			case 'l':	/* library file */
				lib_name = argv[0] + 2;
				gotalib = TRUE;
				if (!lib_name[0])
					lib_name = 0;
				break;
#endif
		} else if( argv[0][0] == '+' ) {
			switch( argv[0][1] ) {
				case 's': /* silent */
					silent = TRUE;
					break;
				case 'L':
					smallflag = FALSE;
					break;
				case 'l':
					listing = TRUE;
					break;
				default:
					error( "Invalid option +%c\n", argv[0][1] );
			}
		} else if (argv[0][0] == '-') {
			switch (argv[0][1]) {
#ifdef TURBO
			case 't':	/* turbo off */
				turbo_flag = FALSE;
				lib_name = 0;
				break;
#endif
			case 'L':
				smallflag = TRUE;
				break;
			default:
				error( "Invalid option -%c\n", argv[0][1] );
			}
		}
		else {
			/* Not a flag, push it back */
			argc++; argv--;
			break;
		}
	}

	if ( argc >= 2 ) {
		inf_name = argv[1];
		argc--; argv++;
	}

	if ( argc >= 2 ) {
		outf_name = argv[1];
		argc--; argv++;
		}

	if( inf_name && outf_name == NULL ) {
		char *dotp;

		outf_name = checkalloc( strlen(inf_name) + 4);
		strcpy( outf_name, inf_name );
		dotp = strchr( outf_name, '.' );
		if( dotp )
			strcpy( dotp, ".ap" );
		 else
			strcat( outf_name, ".ap" );
		}


	if (inf_name == NULL || outf_name == NULL) {
		fprintf(stderr, "use: apin [+silent] [+listing] [-turbo] [l=library] [+Large] [i=]textfile [[o=]file.ap]\n");
		exit(1);
	}

	if (!silent) {
		printf("APIN Version 1.3.1 (%s)\n",smallflag ? "Small":"Large");
		printf("input file '%s', output file '%s'\n", inf_name, outf_name);
		fflush(stdout);
	}

#ifdef TURBO
	if( !gotalib && smallflag )
		lib_name = TURBOLIB;
	if (lib_name)
		LoadLibrary(lib_name);
#endif

	if (strcmp(inf_name, "")==0 || strcmp(inf_name, "-")==0) {
		yyin = stdin;
		yyfilename = "(stdin)";
		}
	else {
		yyin = fopen(inf_name, "r");
		yyfilename = inf_name;
		if (yyin == NULL) {
#ifndef unix
			fprintf(stderr, "Can't open %s\n", inf_name);
#else
			perror(inf_name);
#endif unix
			exit(2);
		}
	}

	work_fname(curr_workspace) = outf_name;
	SymtabStack[0].symtab = CurSymtab = CurBlkSymtab = &master_table;
	SymtabStack[0].isBlock = TRUE;

#ifdef TURBO
	if (LibSymtab)
		pushSymtabStack(LibSymtab, FALSE);
#endif

	res = yyparse();
	printt1("yyparse returned %d\n", res );

	if (Errors)
		fatal("");

#ifdef TURBO
	checkUnsupported();
#endif

	cur_work_top = root;

#ifdef TURBO
	if (turbo_flag) {
		dump(root);
		fixupDecls(decl_kid(root), sym_kid(root));
		printt0("after fixup,\n");
		dump(root);

		linkLib(root);
		printt0("after linkLib,\n");
		dump(root);
		}
#endif

	save(0, FALSE);	/* do not do the safetest */
	exit( 0 );
}
