/*
 * This file contains routines associated with the CProc family of
 * builtins.  Tables of system pointers, pointers to useful routines,
 * and several special routines are here.
 */

#ifndef CNTL
#include "alice.h"
#include "interp.h"
#include "curses.h"
#include "workspace.h"
#include "window.h"
#include "extramem.h"
#endif

#ifdef TOS
#include <osbind.h>
#include <linea.h>
extern long comload();
extern int system();
extern long FindBitMap();
extern CoMap(), MapCo(), WinMap(), MapWin(), DrawWin();
extern draw_all(), HndlSlot(), WinSlot(), CurWin(), GemWin(), GetWork();
extern SizeWin(), SetWork(), wresize(), copy_gwin(), GWinFree(), do_gfull();
extern STMessage(), PgTitle(), win_align(), MkWindow(), FreeSlot(), FindSlot();
extern newAlloc(), newFree();

long
al_linea0()
{

	/* Initialize Line-A and then return the address of
	 * something...
	 */
	linea0();
	return (long)(&la_init);

}
#endif

#ifdef DB_CFUNC
# define PRINTT
#else
# undef PRINTT
#endif
#include "printt.h"

#define MAXIARGS 20

extern int has_segments, what_computer;

extern int fread();

#ifdef msdos

int fnfunc( stream )
FILE *stream;
{
	return fileno(stream);
}

extern long filelength();


#ifdef LARGE
# define bfread		fread
# define ex_checkalloc	checkalloc
#endif

#if defined(SAI) && !defined(HYBRID)
#define  bfread 	fread
#endif

pointer
comload( fname )
char *fname;
{
	FILE *cf;
	FILE *qfopen();
	char far * *arena;
	long flen;

	if( cf = qfopen( fname, "rb" ) ) {
		arena = (char far **)checkalloc( sizeof( char far *) );
		flen = filelength( fileno( cf ) );
		if( flen > 0l && flen < 65535l ) {
			if( *arena = ex_checkalloc( (int)flen ) )
				bfread( *arena, (int)flen, 1, cf );
			}
		fclose( cf );
		return (pointer)arena;
		}
	return (pointer)0;
}

static
rawcall( off, seg, ar1, ar2, ar3, ar4,ar5,ar6,ar7,ar8,ar9 )
unsigned off,seg,ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9;
{
	funcptr p;

	FP_OFF(p) = off;
	FP_SEG(p) = seg;
	return (*p)(ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9);

}
#endif msdos

#ifdef GEM
static
rawcall( p, ar1, ar2, ar3, ar4,ar5,ar6,ar7,ar8,ar9 )
funcptr p;
int ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9;
{
	return (*p)(ar1,ar2,ar3,ar4,ar5,ar6,ar7,ar8,ar9);
}
#endif

static longop( lfunc, l1, l2, res )
int lfunc;	
long l1, l2;
long *res;
{
	switch( lfunc & 15 ) {
		case 1:
			*res = l1 * l2;
			break;
		case 2:
			*res = l1 / l2;
			break;
		case 3:
			*res = l1 + l2;
			break;
		case 4:
			*res = l1 - l2;
			break;
/*
 * Added comparison operator 04/14/87 - DWR
 */
		case 5:
			*res = (long)(l1 < l2);
			break;

		}
	do_undef( res, U_SET, sizeof(long) );
	if( lfunc >= 16 )
		*(rfloat *)res = (rfloat) *res;
}

extern int fprintf(), dos_blk(), fclose();

typedef pint (*ptrfunc)();

extern pint
atoi(),
sprintf(),
new_menu(),
add_menu_item(),
pop_menu(),
grab_range(),
validint(),
validreal(),
getprline(),
n_num_children(),
display(),
graft(),
do_command(),
prune(),
exp_list(),
del_list(),
change_ntype();

#ifdef notdef /* SAI */
extern pint fprintf();
#endif

extern pint
fresh_stub(),
run_error(),
blk_move(),
nonum_message(),
hash(),
macroize(),
macpush();


#ifdef QNX
extern pint fprintf();
#endif

extern pint fseek(), fflush();
extern int rand();

#ifdef QNX
extern pint getc(), get_date();
#define fgetc getc
#else
extern pint fgetc(), time();
#endif

extern pint 
waddch(),
waddstr(),
wmove(),
winsertln(),
wdeleteln(),
wattron(),
wattroff(),
delwin(),
box(),
wrefresh(),
wclear(),
wclrtoeol(),
wclrtobot(),
touchwin(),
fwrite(),
fillchar();

#if defined(Microsoft) && defined(msdos)
extern pint
intdos(),
int86(),
int86x(),
segread(),
inp(),
outp(),
changeDir(),
getCDir(),
safeshell(),
callblock(),
unlink();
#else
# ifdef msdos
extern pint
intcall(),
in(),
inb(),
out(),	
outb(),
unlink(),
system();
# endif msdos
#endif Microsoft && msdos

#ifdef QNX
extern pint pnewwin(),
shell(),
atoh(),
cd(),
attach(),
detach(),
create(),
get_date(),
io_in(),
io_out(),
send(),
receive(),
reply(),
get_ticks();
#endif QNX

#ifdef unix
extern pint fork();
#endif unix

extern char 	*malloc();
#ifdef HYBRID
extern char far *ex_checkalloc();
extern offset segAlloc();
extern int segFree();
#endif HYBRID

extern FILE	*qfopen();

pint do_nop(){}

#ifdef SAI
#define Tinyi(x) (ptrfunc)0
#define Tinyp(x) (pointer)0
#else
#define Tinyi(x) x
#define Tinyp(x) x
#endif SAI

/*macro above for null def in standalone interpreter */

ptrfunc cfunc_array[] = {
atoi,			/* 0 */
sprintf, 		/* 1 */
(ptrfunc)longop,	/* 2 */
(ptrfunc)strchr,	/* 3 */
(ptrfunc)malloc,	/* 4 */
(ptrfunc)allocstring,	/* 5 */
new_menu,		/* 6 */
add_menu_item,		/* 7 */
pop_menu,		/* 8 */
validint,		/* 9 */
validreal,		/* 10 */
(ptrfunc)run_error,	/* 11 */
Tinyi(grab_range),	/* 12 */
Tinyi((ptrfunc)table_lookup),/* 13 */
Tinyi(getprline),	/* 14 */
Tinyi(n_num_children),	/* 15 */
(ptrfunc)blk_move,	/* 16 */
Tinyi(display),		/* 17 */
Tinyi(do_command),	/* 18 */
Tinyi(graft),		/* 19 */
Tinyi(prune),		/* 20 */
Tinyi(exp_list),	/* 21 */
Tinyi(del_list),	/* 22 */
Tinyi(change_ntype),	/* 23 */
Tinyi(fresh_stub),	/* 24 */
Tinyi((ptrfunc)tree),	/* 25 */
Tinyi((ptrfunc)newlist),	/* 26 */
Tinyi((ptrfunc)t_up_to_line),	/* 27 */
Tinyi(nonum_message),		/* 28 */
Tinyi(hash),		/* 29 */
Tinyi(macroize),	/* 30 */
Tinyi(macpush),		/* 31 */
(ptrfunc)qfopen,	/* 32 */
fclose,			/* 33 */
(ptrfunc)fprintf,	/* 34 */
fseek,			/* 35 */
fflush,			/* 36 */
fgetc,			/* 37 */
waddch,			/* 38 */
waddstr,		/* 39 */
wmove,			/* 40 */
winsertln,		/* 41 */
wdeleteln,		/* 42 */
wattron,		/* 43 */
wattroff,		/* 44 */
delwin,			/* 45 */
box,			/* 46 */
wrefresh,		/* 47 */
(ptrfunc)newwin,	/* 48 */
wclear,			/* 49 */
wclrtoeol,		/* 50 */
(ptrfunc)subwin,	/* 51 */
wclrtobot,		/* 52 */
touchwin,		/* 53 */
#ifdef QNX
(ptrfunc)get_date,	/* 54 */
#else
(ptrfunc)time,		/* 54 */
#endif QNX
(ptrfunc)rand,		/* 55 */
(ptrfunc)ftell,		/* 56 */
(ptrfunc)fillchar,	/* 57 */
(ptrfunc)fread,		/* 58 */
(ptrfunc)fwrite,	/* 59 */

#ifdef TOS
(ptrfunc)xbios,		/* 60 */
(ptrfunc)bios,		/* 61 */
(ptrfunc)gemdos,	/* 62 */
(ptrfunc)al_linea0,	/* 63 */
(ptrfunc)linea1,	/* 64 */
(ptrfunc)linea2,	/* 65 */
(ptrfunc)linea3,	/* 66 */
(ptrfunc)linea4,	/* 67 */
(ptrfunc)linea5,	/* 68 */
(ptrfunc)linea6,	/* 69 */
(ptrfunc)linea7,	/* 70 */
(ptrfunc)linea8,	/* 71 */
(ptrfunc)linea9,	/* 72 */
(ptrfunc)lineaa,	/* 73 */
(ptrfunc)lineab,	/* 74 */
(ptrfunc)lineac,	/* 75 */
(ptrfunc)linead,	/* 76 */
(ptrfunc)lineae,	/* 77 */
(ptrfunc)lineaf,	/* 78 */
(ptrfunc)rawcall,	/* 79 */
(ptrfunc)comload,	/* 80 */
(ptrfunc)system,	/* 81 */
(ptrfunc)FindBitMap,	/* 82 */
(ptrfunc)CoMap,		/* 83 */
(ptrfunc)MapCo,		/* 84 */
(ptrfunc)WinMap,	/* 85 */
(ptrfunc)MapWin,	/* 86 */
(ptrfunc)DrawWin,	/* 87 */
(ptrfunc)draw_all,	/* 88 */
(ptrfunc)HndlSlot,	/* 89 */
(ptrfunc)WinSlot,	/* 90 */
(ptrfunc)CurWin,	/* 91 */
(ptrfunc)GemWin,	/* 92 */
(ptrfunc)GetWork,	/* 93 */
(ptrfunc)SizeWin,	/* 94 */
(ptrfunc)wresize,	/* 95 */
(ptrfunc)GWinFree,	/* 96 */
(ptrfunc)STMessage,	/* 97 */
(ptrfunc)win_align,	/* 98 */
(ptrfunc)FreeSlot,	/* 99 */
(ptrfunc)SetWork,	/* 100 */
(ptrfunc)copy_gwin,     /* 101 */
(ptrfunc)do_gfull,	/* 102 */
(ptrfunc)PgTitle,	/* 103 */
(ptrfunc)MkWindow,	/* 104 */
(ptrfunc)FindSlot,	/* 105 */
(ptrfunc)newAlloc,	/* 106 */
(ptrfunc)newFree,	/* 107 */

#endif

#if defined(Microsoft) && defined(msdos)
intdos,		/* 60 */
int86,		/* 61 */
int86x,		/* 62 */
segread,	/* 63 */
inp,		/* 64 */
outp,		/* 65 */
dos_blk,	/* 66 */
unlink,		/* 67 */
(ptrfunc)rawcall,	/* 68 */
(ptrfunc)callblock,	/* 69 */
Tinyi((ptrfunc)comload),/* 70 */
(ptrfunc)filelength,	/* 71 */
# ifdef HYBRID
(ptrfunc)ex_checkalloc, /* 72 */
(ptrfunc)segAlloc,	/* 73 */
(ptrfunc)segFree,	/* 74 */
# else
malloc,		/* 72 */
(ptrfunc)0,	/* 73 */
(ptrfunc)0,	/* 74 */
# endif HYBRID
changeDir,	/* 75 */
getCDir,	/* 76 */

#ifdef UDEBUG
(ptrfunc)do_undef, /* 77 */
#else
(ptrfunc)do_nop,   /* 77 */
#endif UDEBUG

(ptrfunc)fnfunc, /* 78 */
(ptrfunc)safeshell, /* 79 */
#else
# ifdef msdos
intcall,		/* 60 */
in,			/*  */
inb,			/*  */
out,			/* 2 */
outb,			/* 2 */
unlink,			/* 2 */
system,			/* 2 */
# endif
#endif
#ifdef QNX
shell,			/* 2 */
atoh,			/* 2 */
cd,			/* 2 */
attach,			/* 2 */
detach,			/* 2 */
create,			/* 2 */
get_date,		/* 2 */
io_in,			/* 2 */
io_out,			/* 2 */
send,			/* 2 */
receive,		/* 2 */
reply,			/* 2 */
get_ticks,		/* 2 */
#endif
/* room to patch the binary!!! */
(ptrfunc)0,
(ptrfunc)0,
(ptrfunc)0,
(ptrfunc)0,
(ptrfunc)0
};


#ifndef SAI
extern char keymacros, Attr_Map, tokentext;
extern char comargs;	/* command line args */
extern funcptr cmd_preproc;
#endif
extern char *step_delay, *CheckSpeed;

#ifdef msdos
static rint what_computer = 1;
# ifdef LARGE
static rint has_segments = TRUE;
# else
static rint has_segments = FALSE;
# endif
#endif msdos

#ifdef QNX
# ifdef ICON
   static rint what_computer = 2;
   static rint has_segments = FALSE;
# else
   static rint what_computer = 4;
   static rint has_segments = FALSE;
# endif ICON
#endif QNX

#ifdef unix
static rint what_computer = 3;
static rint has_segments = FALSE;
#endif

#ifdef ATARI520ST
static rint what_computer = 5;
static rint has_segments = TRUE;
#endif

extern WINDOW *pg_out;
extern funcptr builtins[];
extern int step_flag;
extern int errno;
extern int does_flicker;
extern int screen_mem_addr;
extern int mono_adapter;
extern pointer free_top;
extern struct modeInfo ModesInfo[];
extern long tc_result;		/* turbo call result */
extern char active_libs;
extern fileptr open_array[MAX_F_OPEN];
char safe_reserve[40];
#ifdef TOS
extern char *WinInfo;
extern long gl_menu;
extern int gl_xfull, gl_yfull, gl_hfull, gl_wfull;
extern int scr_area;
extern int charWidth, charHeight;
extern int BufferGraphics;
extern int global[];
extern int vdi_handle;
#endif

pointer p_array[] = {
Tinyp((pointer)&comargs),		/* 0 */
Tinyp((pointer)&pg_out),		/* 1 */
#ifndef GEM
Tinyp((pointer)&main_win),		/* 2 */
#else
Tinyp((pointer)0),
#endif
Tinyp((pointer)&cursor),		/* 3 */
Tinyp((pointer)&curr_workspace),	/* 4 */
Tinyp((pointer)&tokentext),	/* 5 */
Tinyp((pointer)&step_flag),	/* 6 */
Tinyp((pointer)&step_delay),	/* 7 */
(pointer)&what_computer,	/* 8 */
Tinyp((pointer)&CheckSpeed),	/* 9 */
#ifdef QNX
(pointer)&stdin,		/* 10 */
(pointer)&stdout, 		/* 11 */
#else
(pointer)stdin,			/* 10 */
(pointer)stdout,		/* 11 */
#endif
(pointer)ALICE_VERSION,		/* 12 */
(pointer)&stdscr,		/* 13 */
Tinyp((pointer)&keymacros),	/* 14 */
Tinyp((pointer)&Attr_Map),	/* 15 */
Tinyp((pointer)&st_display),	/* 16 */
Tinyp((pointer)curWindows),	/* 17 */
(pointer)&curr_workspace,	/* 18 */
#ifdef LOADABLE_TEMPLATES
(pointer)&Node_Names,		/* 19 */
(pointer)&node_table,		/* 20 */
#else
Tinyp((pointer)Node_Names),	/* 19 */
Tinyp((pointer)node_table),	/* 20 */
#endif
Tinyp((pointer)&sel_node),	/* 21 */
Tinyp((pointer)&sel_first),	/* 22 */
Tinyp((pointer)&sel_last),	/* 23 */
(pointer)safe_reserve,		/* 24 */
(pointer)&has_segments,		/* 25 */
Tinyp((pointer)&cmd_preproc),	/* 26 */
(pointer)&errno,	/* 27 */
(pointer)open_array,	/* 28 */
#ifdef TURBO
(pointer)&last_ioerr,	/* 29 */
#else
(pointer)0,
#endif
(pointer)&ex_stack,	/* 30 */
(pointer)&does_flicker,	/* 31 */
(pointer)&screen_mem_addr,/* 32 */
(pointer)&mono_adapter,	/* 34 */
(pointer)&free_top,	/* 35 */
#ifdef TURBO
(pointer)&active_libs,	/* 36 */
(pointer)cfunc_array,	/* 37 */
(pointer)ModesInfo,	/* 38 */
(pointer)&tc_result,	/* 39 */
(pointer)&io_res_on,	/* 40 */
#endif
#ifdef TOS
(pointer)&WinInfo,	/* 41 */
(pointer)&gl_menu,	/* 42 */
(pointer)&gl_xfull,	/* 43 */
(pointer)&gl_yfull,	/* 44 */
(pointer)&gl_wfull,	/* 45 */
(pointer)&gl_hfull,	/* 46 */
(pointer)&scr_area,	/* 47 */
(pointer)&charWidth,	/* 48 */
(pointer)&charHeight,	/* 49 */
(pointer)&BufferGraphics, /* 50 */
(pointer)global,	/* 51 */
(pointer)&vdi_handle,   /* 52 */
#endif

#ifdef HYBRID
(pointer)&tr_segment,	/* 41 */
(pointer)&st_segment,	/* 42 */
(pointer)Segs,		/* 43 */
#ifdef ES_TREE
(pointer)builtins,	/* 44 */
#else
0,
#endif
#endif
0
};

/* get a pointer from the magic array of pointers */

do_getptr(argc, argv )
int argc;
rint *argv;
{
#ifdef SAI
	int i;
	char buf[80];

	i = argv[-P_S];

	if( p_array[i] == NULL ) {
		sprintf( buf, "The Sytem builtin pointer SysPointer(%d) is not available in the StandAlone\n", i);
		sai_print( buf );
		sai_print("Interpreter, please run the program under full Alice\n");
		done();
	}
#endif

	*(pointer *)argv = p_array[ argv[-P_S] ];
}

/* c function with integer/ordinal return */

extern pint do_cfunc();

do_cintfunc( argc, argv )
int argc;
rint *argv;
{
	*argv = (rint) do_cfunc( argc, argv );
}

static pint longhold;

do_clongfunc( argc, argv )
int argc;
long * *argv;
{
	longhold = (long)do_cfunc(argc,argv);
	*argv = &longhold;
}

/* c function with pointer return */
do_cptrfunc( argc, argv )
int argc;
pointer *argv;
{
	*argv = (pointer) do_cfunc( argc, argv );
}

do_cproc( argc, argv )
int argc;
rint *argv;
{
	/* no return value */
	do_cfunc( argc, argv, 0 );
}

static
pint
do_cfunc( argc, argv  )
int argc;
rint *argv;
{
	int i;		/* index into args */
	int funcnum;
	int typesize;	/* size of type of given arg */
	pint ret;

	rint aa[MAXIARGS];
	int argarp;	/* index into aa */
	preg1 pointer getargs;
#ifdef SAI
	extern int sai_out;
#endif

	argarp = 0;

	funcnum = argv[-2*P_S];		/* which function */

#ifdef SAI
	if( sai_out == SAI_NORMAL ) {
		if( (funcnum >= 38) && (funcnum <= 53) ) {
			sai_print( "\nThis program uses the ALICE screen handling routines\n" );
			sai_print( "Run the program again, this time using the +w option\n" );
			done();
		}
	}
#endif SAI

	getargs = (pointer) (argv - 2*P_S);

	/* Construct the array of arguments */
	construct( getargs, argc - 1, aa, MAXIARGS );

	/* function for now, later we get fancy */
	/* the following code is HIGHLY NON-PORTABLE.  Aside from expecting
	 * all these integer type things to back together, it expects that a
	 * pint (a long in large model) is being returnd from the cfunc, when
	 * in fact it is usually an int or a pointer.  So this will get
	 * assigned into a pint where later it is cast back.  We hope this
	 * will work.
	 */

	printt2("funcnum = %d, calling %x\n", funcnum, cfunc_array[funcnum]);

	/* 
	 * If the function pointer is null, we had better not call it.
 	 * We are probably running as the standalone interpreter
	 * or something goopy like that. Perhaps, do_undef.
	 * We could give an error, like 'cfunc %d not supported in this
	 * version', or something like that
	 */
	if( cfunc_array[funcnum] == (ptrfunc)NULL ) {
		char buf[80];
#ifdef SAI
		sprintf( buf, "Sorry, that cfunc (%d) is not supported in this version\n" );
		sai_print(buf);
		done();
#else
		run_error( ER(9,"cfunc`Too many arguments to C function") );
#endif
	}

	return (*(cfunc_array[funcnum]))(aa[0], aa[1], aa[2], aa[3],
					 aa[4], aa[5], aa[6], aa[7],
					 aa[8], aa[9], aa[10],aa[11],
					 aa[12],aa[13],aa[14],aa[15],
					 aa[16],aa[17],aa[18],aa[19] );


}
#ifndef SAI
exec_ws( args )
char *args;
{
	workspace *toexec;
	extern int in_execws;

	toexec = get_nws( args, FALSE, 0 );
	in_execws = TRUE;
	do_run_prog( work_node( toexec ), 0 );
	in_execws = FALSE;
	display(TRUE);
}
#endif SAI
