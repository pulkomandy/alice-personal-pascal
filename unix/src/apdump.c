#include <stdio.h>

/*
 *DESC: A debugging file that allows dumps of the program tree to be printed
 *DESC: to the debug output.
 */

char fname[30];

int dump_indent = 0;
extern FILE *loadf;
FILE * dtrace;

main(argc,argv)
int argc;
char **argv;
{
	if( argc > 1 )  {

		LoadNodes("/u/alice/src/alice.tpl");
		sprintf( fname, "%s%s", argv[1], strchr( argv[1], '.' ) ? "" : ".ap" );

		if( loadf = fopen( fname, "r" ) )
			_load( );
		 else
			printf("Can't open %s\n", fname );
		
		}
	 else
		printf( "No file name given\n" );
		

}

Dumpload( form, ar1, ar2, ar3, ar4 )
char *form;
int ar1,ar2,ar3,ar4;
{
	int i;

	if( *form == 'I' ) {
		for( i = 0; i < dump_indent; i++ )
			putchar( ' ' );
		form++;
		}
	printf( form, ar1, ar2, ar3, ar4 );
}


char *
checkalloc( size )
int size;
{
	extern char *calloc();
	return calloc( size, 1 );
}


int curr_window;
clean_curws (){}
graft(){}
char *
allocstring(str)
char *str;
{
	char	*new = checkalloc(strlen(str) + 1);

	strcpy(new, str);
	return new;
}

max( a, b )
int a,b;
{
	return a > b ? a : b;
}
error( a, b, c, d, e, f, g )
char *a, *b, *c, *d, *e, *f, *g;
{
	extern long errloc;
	printf( "\nError at Seek %ld: ", errloc );
	printf( a,b,c,d,e,f,g );
	exit(1);
}

FILE *
qfopen( fname, args )
char *fname;
char *args;
{
	return fopen( fname, args );
}
message( a, b, c, d, e, f, g )
char *a, *b, *c, *d, *e, *f, *g;
{
	printf( a,b,c,d,e,f,g );
}

int *
sym_kid(){
	static int crap[20];
	return crap;
}
zap_history(){}
int *
tree(){
	static int crap[25];
	return crap;
}
char **
sized_list(){
	static char * bigboy[270];
	return bigboy;
}
linkup(){}
s_conval(){}
mfree(){}
int curr_workspace[30];
char *cursor;
s_node_kid(){}
treefree(){}
hash(){ return 0; }
set_wscur(){}
do_strscan(){}
do_copy(){}
do_plot(){}
do_memtovar(){}
do_round(){}
do_sound(){}
do_sysfunc(){}
do_lnwrite(){}
do_sysproc(){}
do_odd(){}
do_trunc(){}
do_ln(){}
do_strdelete(){}
do_arctan(){}
do_intr(){}
do_vartomem(){}
do_upcase(){}
do_clongfunc(){}
do_update(){}
do_sqrt(){}
do_vid(){}
do_eof(){}
do_iocheck(){}
do_cintfunc(){}
do_ord(){}
do_setattr(){}
do_grmode(){}
do_textmode(){}
do_val(){}
do_random(){}
do_setnext(){}
do_append(){}
do_lnread(){}
do_strinsert(){}
do_abs(){}
do_read(){}
do_pack(){}
do_substr(){}
do_chr(){}
do_memfunc(){}
do_sin(){}
do_clreol(){}
do_page(){}
do_get(){}
do_insdel(){}
do_colour(){}
do_peek(){}
do_cptrfunc(){}
do_draw(){}
do_delay(){}
do_charwait(){}
do_flpart(){}
do_address(){}
do_ioresult(){}
do_assign(){}
do_cos(){}
do_initrandom(){}
do_window(){}
do_getptr(){}
do_scrfunc(){}
do_new(){}
do_halt(){}
do_exp(){}
do_close(){}
do_pred(){}
do_succ(){}
do_getch(){}
do_strlen(){}
do_tplib(){}
do_sizeof(){}
do_sqr(){}
do_point(){}
do_write(){}
do_strcat(){}
do_stsize(){}
do_str(){}
do_dispose(){}
do_pause(){}
do_cproc(){}
do_put(){}
do_eoln(){}
do_cursorto(){}
do_poke(){}
do_makeptr(){}
do_reset(){}
do_rewrite(){}
do_blockio(){}
do_dirfunc(){}
do_filefunc(){}
do_fillch(){}
do_getdir(){}
do_hilo(){}
do_longseek(){}
do_markrel(){}
do_move(){}
do_nop(){}
do_parcnt(){}
do_parstr(){}
do_ptr(){}
do_rename(){}
do_rndomize(){}
do_segoff(){}
do_unpack(){}
do_where(){}
mem_cleanup(){}
