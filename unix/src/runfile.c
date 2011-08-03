/*
 *DESC:  Built in routines of Pascal - for doing I/O
 */
#define INTERPRETER 1
#define	FUNCTIONS 1

#include "alice.h"
#include <curses.h>

extern int last_ioerr;

#ifdef SAI
extern int sai_out;
#endif

#include "typecodes.h"
#ifdef PROC_INTERP
#include "process.h"
#endif
#include "bfuncs.h"
#include "interp.h"
#include "dbflags.h"

extern FILE *qfopen();
static write_it(), _write_it();

struct pas_file fil_input, fil_output, fil_kbd, fil_lst, fil_aux, fil_con,
		fil_trm, fil_inp, fil_out, fil_err;

fileptr predef_files[] = {
&fil_input, &fil_output, &fil_kbd, &fil_lst, &fil_aux, &fil_con,
		&fil_trm, &fil_inp, &fil_out, &fil_err };

#ifdef QNX
FILE ** predf_streams[] = {
	&stdin, &stdout, &stdin,
	&stdout, &stdout,
	&stdout, &stdout, &stdin, &stdout, &stderr };
#else
FILE * predf_streams[10];
#endif

void
init_predef()
{
	predf_streams[0] = stdin;
	predf_streams[1] = stdout;
	predf_streams[2] = stdin;
#ifdef msdos
	predf_streams[3] = stdprn;
	predf_streams[4] = stdaux;
#endif
	predf_streams[5] = stdout;
	predf_streams[6] = stdout;
	predf_streams[7] = stdin;
	predf_streams[8] = stdout;
	predf_streams[9] = stderr;
}

unsigned predf_flags[] = {
0,
0,
FIL_NREAD|FIL_OPEN|FIL_LAZY|FIL_TEXT|FIL_READ|FIL_KEYBOARD|FIL_RAWKEY,
FIL_TEXT|FIL_OPEN|FIL_WRITE|FIL_EOF|FIL_EOL,
FIL_TEXT|FIL_OPEN|FIL_READ|FIL_WRITE|FIL_DEVICE|FIL_LAZY|FIL_NREAD,
0,
0,
FIL_TEXT|FIL_OPEN|FIL_READ|FIL_LAZY|FIL_NREAD,
FIL_TEXT|FIL_OPEN|FIL_WRITE|FIL_EOF|FIL_EOL,
FIL_TEXT|FIL_OPEN|FIL_WRITE|FIL_EOF|FIL_EOL
};




int turbo_io = FALSE;		/* turbo style text files & reset */
int predef_number = sizeof(predef_files)/sizeof(fileptr);

extern fileptr file_setup();

fileptr open_array[MAX_F_OPEN];
int ofile_count = 0;

/* write out an argument, modifies stack */
do_write(args, type, enode, filedesc)
int	args;		/* number of args this call */
nodep type;		/* type of value to be written */
nodep enode;		/* expression that made the argument */
fileptr filedesc;	/* file to output to */
{
	if (args)
		write_it(type, enode, filedesc);
}

do_lnwrite(args, type, enode, filedesc)
int args;		/* number of args this call */
nodep type;		/* type of value to be written */
nodep enode;		/* expression that made the argument */
fileptr filedesc;
{
	if (args)
		write_it(type, enode, filedesc);
	else {
		fileout(filedesc, "\n" );
		if( filedesc )
			filedesc->f_flags |= FIL_EOL;
		flush_scr(filedesc);
		}
}

static
write_it(type, enode, filedesc)
nodep type;
nodep enode;			/* expression that made the argument */
fileptr filedesc;
{

	if( !filedesc )
		filedesc = &fil_output;
	/* BUG, should also detect file of char */
	if( !(filedesc->f_flags & FIL_TEXT ) ) {
		wrput( type, filedesc );
		return;
		}

	/* an eol has not been written */

/*	filedesc->f_flags &= ~FIL_HASEOL; */
	form_put( type, enode, filedesc, (char *)0 );
	flush_scr(filedesc);
}

form_put( type, enode, filedesc, str )
nodep type;
nodep enode;			/* expression that made the argument */
fileptr filedesc;
char *str;			/* string or nothing */
{
	int	width;
	int	prec;

	if (ntype(enode) == N_OEXP_3)
		prec = (int)int_spop();
	else
		prec = OEXP_DEFAULT;
	if (ntype(enode) == N_OEXP_2 || ntype(enode) == N_OEXP_3)
		width = (int)int_spop();
	else
		width = OEXP_DEFAULT;

	_write_it(type, width, prec, filedesc, str);
}

/* number of digits in exponent */
#if defined(msdos) || defined(QNX)
#define EXPDIGITS 3
#else
#define EXPDIGITS 2
#endif

/* minimum width of e format number "-1.0e+00" */
#define MW (EXPDIGITS+6)

static
_write_it(_type, width, prec, filedesc, str)
nodep _type;
int	width;
int	prec;
fileptr filedesc;
char *str;
{
	register nodep type = comp_type(_type);
	register int	ntyp = ntype(type);
	char	buf[400];
	char	format[10];
	char	*s;
	char *bldbuf;
#ifndef SAI
	stackcheck();
#endif
	bldbuf = str ? str : buf;

	if (type == Basetype(BT_integer)) {
		if( width == 0 || (width == OEXP_DEFAULT && turbo_io) )
			strcpy( format, "%d" );
		else if (width == OEXP_DEFAULT)
			strcpy(format, " %d");
		else
			sprintf(format, "%%%dd", width);
		sprintf(bldbuf, format, int_spop());
	}
	else if (type == Basetype(BT_char)) {
		if (width == OEXP_DEFAULT)
			strcpy(format, "%c");
		else
			sprintf(format, "%%%dc", width);
		sprintf(bldbuf, format, int_spop());
	}
	else if( type == Basetype(BT_real)) {
#ifdef FLOATING
		if( width == OEXP_DEFAULT )
			strcpy(format, " %g" );
		else if( width == -1 )
			strcpy( format, "%g" );
		else if( prec == OEXP_DEFAULT ) {
			if( width < MW )
				width = MW;
/*			sprintf(format, "%%%d.%de", width, max(0, (width-5)));*/
			sprintf(format, "%% #%d.%de", width,width-(MW-1) );
			}
		else
			sprintf( format, "%%#%d.%df", width, prec );
		sprintf( bldbuf, format, f_spop() );
#else
		fileout( filedesc, "Sorry, no REALs" );
		return;
#endif
		}
	else if (is_stringtype(type)) {
		if (width == OEXP_DEFAULT)
			strcpy(format, "%s");
		else
			sprintf(format, "%%%d.%ds", width, width);
		s = str_spop()+STR_START;
		if (ntyp != N_CON_STRING) 
			do_undef(s, U_CHECK, strlen(s));
		sprintf(bldbuf, format, s);
	}
	else if (ntyp == N_TYP_ENUM) {
		if (width == OEXP_DEFAULT)
			strcpy(format, " %s");
		else if( width == 0 )
			strcpy( format, "%s" );
		else
			sprintf(format, "%%%d.%ds", width, width);
		sprintf(bldbuf, format, ordname(type, int_spop() ));
	}
	else if (is_set_type(type) ) {
		int	i;
		set	copy;
		int	any = FALSE;
		nodep elType;	/* type of element of set */
		extern nodep set_base_type();

		EDEBUG(5, "a set!\n", 0, 0);
		blk_move((pointer)copy, (pointer)set_spop(), SET_BYTES);
		fileout( filedesc, "[" );
		elType = set_base_type( type, type );
		for (i = 0; i < (SET_BYTES << 3); i++)
			if (copy[i >> 3] & (1 << (i&7))) {
				if (any)
					fileout(filedesc, ",");
				else
					any = TRUE;

				int_spush(i);
				_write_it(elType, width, prec, filedesc,(pointer)0);
			}
		fileout( filedesc, "]" );
		return;
	} else
		run_error(ER(97,"badwrite`Sorry, you may not 'write' a complex type'"));
	if( !str )
		fileout( filedesc, bldbuf );

}

#ifdef TURBO
do_str( argc, argv )
int argc;
nodep *argv;
{
	char *dest;
	nodep str_type;
	int len;
	char dbuf[MAX_STR_LEN];

	dest = str_spop() + STR_START;
	str_type = NCAST p_spop();
	checksvar( str_type, "Str destination" );
	form_put( argv[-1], kid1(kid2(ex_loc)), (fileptr)0, dbuf );
	if( (len = strlen(dbuf)) > get_stsize( str_type ) )
		run_error( ER(41,"stringbig`String too long") );
	strcpy( dest, dbuf );
	do_undef( dest-1, U_SET, len+2 );

	dest[-1] = len;
}

#endif


/* only handles text files (input) for now */

do_read(args, type, enode, filedesc)
int	args;		/* number of args this call */
nodep type;		/* type of value to be written */
nodep enode;		/* node of value */
fileptr filedesc;
{
	register nodep rtype;
	register pointer	ptr;
	int i, slen;	
	char bstr[MAX_NUM_LEN];	
	extern rfloat atof();
	extern char validint(), validreal();

	if( args == 0 )
		return;
	if( !filedesc )
		filedesc = &fil_input;
	if( ntype(type) == N_CON_STRING )  {
		if( filedesc->f_flags & FIL_LAZY ) {
			do_write( args, type, enode, &fil_output );
			}
		return;
		}
			

	ptr = p_spop();

	if( !(filedesc->f_flags & FIL_TEXT ) ) {
		getread( filedesc, ptr );
		return;
		}
	rtype = comp_type(type);
	if (rtype == Basetype(BT_char)) {
		register char inchr;
		inchr = inp_char(filedesc, TRUE);
		sr_ccheck( enode, inchr );
		*(char *)ptr = inchr;
		do_undef(ptr, U_SET, sizeof(char));
		}
	else if (rtype == Basetype(BT_integer)) {
		register rint typedint;
		if( scanin( bstr, validint, filedesc ) ) {
			typedint = getint(bstr);
			sr_ccheck( enode, typedint );
			*(rint *) ptr = typedint;
			do_undef(ptr, U_SET, sizeof(rint));
			}
		}
	else if (rtype == Basetype(BT_real)) {
		if( scanin( bstr, validreal, filedesc ) ) {
			*(rfloat *)ptr = atof( bstr );
			do_undef(ptr, U_SET, sizeof(rfloat));
			}
		}
	else if( is_stringtype(rtype) ) {
		extern int turbo_flag;
		slen = STR_START + get_stsize( rtype );
		for( i = STR_START; i < slen && !eolcheck(filedesc); i++ ) {
			ptr[i] = inp_char(filedesc,TRUE);
			}
		ptr[i] = 0;	/* string end */
#ifdef TURBO
		ptr[0] = i-1;
#endif
		do_undef( ptr, U_SET, i+1 );
		/* mark the chars beyond the end undefined */
		if( i < slen - 1 && !turbo_flag )
			do_undef( ptr + i + 1, U_CLEAR, slen - i );
		}
}

do_lnread(args, type, enode, filedesc)
int	args;
nodep type;
nodep enode;		/* waste of time and cause of silly bug */
register fileptr filedesc;
{
	if (args)
		do_read(args, type, enode, filedesc);
	else {
		if( filedesc ) {
			if( !(filedesc->f_flags & FIL_TEXT ) ) 
				run_error( ER(98,"nontext`Readln - text file required"));
			}
		 else
			filedesc = &fil_input;
		while (!eolcheck(filedesc) && !last_ioerr )
			inp_char(filedesc, TRUE);
		/* we have read in the newline */

		/* if at end of file (turbo io), then return */
		if( turbo_io ) {
			if( filedesc->f_flags & FIL_EOF )
				return;
			if( !(filedesc->f_flags & (FIL_KEYBOARD|FIL_RAWKEY))) {
				/* pass the CR */
				inp_char( filedesc, TRUE );
				/* if the char after the CR was NOT LF, then
				   return, otherwise read another */
				if( inp_char( filedesc, FALSE ) != 10 )
					return;
				}
			}
		_a_get( filedesc,  FALSE );
		}
}


#ifdef HAS_IOFUNCS

do_eof( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;
	int flags;

	whfile = argc ? argv[-1] : &fil_input;
	check_undef( whfile, 1 );

	flags = whfile->f_flags;
	if( !(flags & FIL_EOF ) )
		if( (flags & FIL_NREAD) )
			_a_get( whfile,  TRUE );
	
	/* beware of nasty optimizers getting rid of !! */
	/* we are safe here because FIL_EOF is 1 */
	*(rint *)argv = !!(whfile->f_flags & FIL_EOF);
}

do_eoln( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;
	/* check for textfile ? */

	whfile = argc ? argv[-1] : &fil_input;
	check_undef( whfile, 1 );

	*(rint *)argv = eolcheck(whfile) ? 1 : 0;
}

do_page( argc, argv )
int argc;
fileptr *argv;
{
#ifndef SMALLIN
	register fileptr whfile;
	extern WINDOW *pg_out;
#ifndef SAI
	extern FILE *LogFile;
#endif
	/* check for textfile ? */

	whfile = argc ? argv[-1] : &fil_output;
	check_undef( whfile, 1 );

#ifndef PROC_INTERP
	if( whfile->f_flags & FIL_SCREEN ) {
# ifndef SAI
		if (LogFile)
			putc('\014', LogFile);
# else
		if( sai_out == SAI_SCREEN ) {
# endif
# ifndef ICON
		checkScreen();
# endif
		werase(pg_out);
		wrefresh(pg_out);
# ifdef TURTLE
		drawaturt();
		Fhome();		/* home turtle */
# endif
# ifdef SAI
		}
# endif
	}
	 else
#endif
		fileout( whfile, "\014" );
#endif SMALLIN
}


do_get( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;

	if( argc ) {
		whfile = argv[-2];
		check_undef(whfile, sizeof(struct pas_file)-sizeof(char));
		}
	 else {
		whfile = &fil_input;
		}
	_a_get( whfile,  FALSE );
}



do_put( argc, argv )
int argc;
fileptr *argv;
{
	register fileptr whfile;
	int datsiz;
	register pointer rdbuf;

	if( argc ) {
		whfile = argv[-2];
		check_undef(whfile, sizeof(struct pas_file)-sizeof(char));
		}
	 else {
		whfile = &fil_output;
		}

	datsiz = whfile->f_size;
	rdbuf = &(whfile->f_buf);
	check_undef( rdbuf, datsiz );
	putout( whfile, rdbuf, FALSE );
	do_undef( rdbuf, U_CLEAR, datsiz );
}


prp_pfile( argv, filedesc )
nodep * argv;
fileptr filedesc;
{
	nodep ftype;
	register nodep btype;
	register fileptr whfile = filedesc;

	btype = argv[-1];

	if( btype == Basetype(SP_file) )
		whfile->f_size = 0;
	 else {
		ftype = find_realtype( kid1( btype ) );
		whfile->f_size = calc_size( ftype, TRUE );
		}
	EDEBUG(7, "Set file size to %d for %x\n", whfile->f_size, whfile );
	do_undef( whfile, U_SET, sizeof( struct pas_file ) - 1 );
}

extern char *filevarname();

do_reset( argc, argv )
int argc;
fileptr *argv;
{
#ifndef ICRIPPLE
	register fileptr whfile;

	
	whfile = file_setup( argc, argv,
			FIL_READ| (turbo_io ? FIL_LAZY|FIL_RANDOM|FIL_WRITE:0),
		turbo_io ? "rb+" : "r", "Reset" );
	/* now do the manditory get on the file */
	if( !last_ioerr )
		if( turbo_io )
			whfile->f_flags |= FIL_NREAD;
		 else
			_a_get( whfile, FALSE );
#else
#endif ICRIPPLE
}

do_assign( argc, argv )
int argc;
fileptr *argv;
{
	givename( argv[-1], str_spop()+STR_START );
}
		

do_rewrite( argc, argv )
int argc;
fileptr *argv;
{
#ifndef ICRIPPLE
	register fileptr whfile;

	whfile = file_setup( argc, argv, FIL_EOF|FIL_WRITE, "w",
				"Rewrite" );
	/* mark the buffer as undefined */
	do_undef( &(whfile->f_buf), U_CLEAR, whfile->f_size );
#else
#endif ICRIPPLE
}

do_charwait(argc, argv)
int argc;
rint *argv;
{
#ifdef SAI
	*argv = tst_charwait();
#else
	extern int	inbuf_count;

	*argv = inbuf_count > 0 || tst_charwait();
#endif
}

tst_charwait()
{
#ifdef QNX
	return getbc(TRUE) > 0;
#else
#ifdef msdos
	return dos_chr_waits();
#else
	return 1;	/* always waiting */
#endif
#endif
}

do_getch(argc, argv)
int argc;
rint *argv;
{
	extern WINDOW *pg_out;

	checkScreen();
#ifdef SAI
	if( sai_out == SAI_SCREEN ) {
		if( !tst_charwait() ) {
			curOn( pg_out );
			wrefresh( pg_out );
		}
		*argv = wgetch();
	}
	 else {
		*argv = _rchar();
	}
#else

#ifdef QNX
	*argv = getbc(FALSE);
#else
	if( !tst_charwait() ) {
		curOn( pg_out );
		wrefresh( pg_out );
		}
	*argv = keyget();
	curOff( pg_out );
#endif
#endif SAI
}

/* close all files to the given stack level */
/* also disble tracing in this area */

closeall(allofthem, lowbound, upbound)
int allofthem;		/* every file, no matter where on the stack */
pointer lowbound, upbound;	/* bounds to close within */
{
	int i;
	register fileptr thefile;

#ifdef QNX
	extern FILE *dirfile;

	if( allofthem && dirfile ) {
		fclose( dirfile );
		dirfile = (FILE *)0;
		}
#endif

#ifdef Trace
	/* disable tracing for removed memory */
	traceingoff( allofthem, lowbound, upbound ); 
#endif

	for( i = 0; i < ofile_count; i++ )
		if( (thefile=open_array[i]) && (allofthem ||
				(((pointer)thefile<upbound&&(pointer)thefile>=lowbound)
				&& thefile != &fil_kbd
				&& thefile != &fil_input
				&& thefile != &fil_output ))) {
			/* ADD CHECK FOR WINDOW, CLOSE IT */
			if( thefile->desc.f_stream )
				stclose( thefile );
			if( thefile->f_name ) {
				dmfree( thefile->f_name );
				thefile->f_name = 0;
				}
			open_array[i] = 0;
			}
}
/* close off file stream */

stclose( fil )
register fileptr fil;
{
	/* if a text file open for write without a newline last,
		write one */
	if( fil->desc.f_stream ) {
		if( !turbo_io && (fil->f_flags&(FIL_WRITE|FIL_TEXT|FIL_EOL))==
					(FIL_WRITE|FIL_TEXT)){
			fileout( fil, "\n" );
			}
		fclose( fil->desc.f_stream );
		fil->desc.f_stream = (FILE *)0;
		}
}

do_close(argc, argv)
{
	register fileptr toclose;
	register int i;

	toclose = (fileptr)p_spop();
	check_undef( toclose, 1 );
	if( close_remove( toclose ) ) {
		tp_error( 4, 0 );
		run_error( ER(303,"notopen`File not open") );
		}
}

/* removes a file from open array if it is open. */
/* returns non-zero if the file wasn't there */

close_remove( toclose )
register fileptr toclose;
{
	int i;
	if( toclose->desc.f_stream ) {
		for( i = 0; i < ofile_count; i++ )
			if( toclose == open_array[i] ) {
				stclose( toclose );
				open_array[i] = 0;
				return FALSE;
				}
		/* it wasn't in the list.  Close it anyway */
		stclose( toclose );
		}
	/* was not in the file list */
	return TRUE;
}

set_descrip( fil, sfname, sfargs, mflags )
register fileptr fil;			/* file being opened */
char *sfname;			/* name for fopen */
char *sfargs;			/* args for fopen */
int mflags;			/* flags to give after open */
{	
	register int i;
	int fzero;
	FILE *stdfile;		/* stdio file to open */
	char opstr[5];

	fzero = -1;
	for( i = 0; i < ofile_count; i++ ) {
		if( open_array[i] == 0 )
			fzero = i;
		else if( open_array[i] == fil && fil->desc.f_stream )  {
			EDEBUG( 5, "Closing file #%d is %x\n", i, fil );
			stclose( fil );
			/* no need to set to zero */
			break;
			}
		}

	/* if we did not get the file, try for an empty spot or make one */
	if( i == ofile_count ) {
		if( fzero < 0 )
			i = ofile_count++;
		 else
			i = fzero;
		}

	if( ofile_count >= MAX_F_OPEN ) {
		tp_error( 0xf3, TRUE );
		run_error( ER(99,"opf`Too many active files") );
		}
#ifdef msdos
	/* binary file mode if not a text file */
	sprintf( opstr, "%s%s", sfargs, mflags & FIL_TEXT ? "" : "b" );
	stdfile = qfopen( sfname, opstr );
#else
	stdfile = qfopen( sfname, sfargs );
#endif
	open_array[i] = fil;
	fil->desc.f_stream = stdfile;
	fil->f_flags = FIL_OPEN | mflags;

	/* return true if open failed */
	return stdfile == (FILE *)0;
}


do_append( argc, argv )
int argc;
fileptr *argv;
{
#ifndef ICRIPPLE
	register fileptr whfile;

	whfile = file_setup( argc, argv, FIL_EOF | FIL_WRITE, "a", "Append" );

#ifdef No_append_open
	/* seek to end of file */
	fseek( whfile->desc.f_stream, 0l, 2 );
#endif
	/* mark the buffer as undefined */
	do_undef( &(whfile->f_buf), U_CLEAR, whfile->f_size );
#else
#endif ICRIPPLE
}

do_update( argc, argv )
int argc;
fileptr *argv;
{
#ifndef ICRIPPLE
	register fileptr whfile;

	whfile = file_setup( argc, argv, FIL_RANDOM|FIL_READ|FIL_WRITE,
#ifdef QNX
			"rw", "Update" );
#else
			"r+", "Update" );
#endif
	/* now do the manditory get on the file */
	if( !last_ioerr )
		if( turbo_io )
			whfile->f_flags |= FIL_NREAD;
		 else
			_a_get( whfile, FALSE );
#else
#endif ICRIPPLE
}

do_setnext( argc, argv, sym )
int argc;
fileptr *argv;
symptr sym;
{
	register fileptr whfile;
	long howfar;

	whfile = argv[-1];
	if( sym_saveid( sym ) != -100 &&!(whfile->f_flags & FIL_RANDOM ) ) {
		tp_error( 5, 0 );
		run_error( ER(100,"rfile`This file was not initialized with 'update'"));
		}

	howfar = seeksize(whfile) * (long)(int_spop());
	EDEBUG( 8, "seek location is %ld\n", howfar , 0 );
	/* assume no lazy i/o, or we would have to do the get now */
	if( fseek( whfile->desc.f_stream, howfar, 0 ) ) {
		tp_error( 0x91, 0 );
		run_error( ER(305,"badseek`Seek/Setnext out of bounds") );
		}
	whfile->f_flags &= ~(FIL_EOF|FIL_EOL);
	if( sym_saveid(sym) == -100 ) /* was turbo seek */
		/* mark data to be read by readget */
		whfile->f_flags |= FIL_NREAD;
}

/* get the record size for seek */

seeksize(whfile)
register fileptr whfile;
{
	return whfile->f_size ? whfile->f_size : 128;
}

fileptr
file_setup( argc, argv, flags, opargs, procname )
int argc;
fileptr *argv;
int flags;		/* what flags to put in on open */
char *opargs;		/* open args */
char *procname;		/* function name in error */
{
	register fileptr whfile;
	pointer fname;

	whfile = argv[-2];
	if( argc > 1 ) 
		givename( whfile, fname = str_spop() + STR_START);
	 else {
		fname = filevarname(whfile);
		/* if was no name, assign this one */
		if( !whfile->f_name )
			givename( whfile, fname );
		}

	if( (nodep)argv[-1] == Basetype(BT_text) )
		flags |= FIL_TEXT|FIL_EOL;
	if( set_descrip( whfile, fname, opargs, flags) ) {
		tp_error( 1, whfile );
		run_error(ER(101,"wrfil`%s - can't open file %s"), procname, fname );
		}
	prp_pfile( (nodep *)argv, whfile );
	return whfile;
}

givename( whfile, name )
register fileptr whfile;
char *name;
{
	if( whfile->f_name )
		dmfree( whfile->f_name );
	whfile->f_name = allocstring(name);
	do_undef( &whfile->f_name, U_SET, sizeof(char *) );
}
#else
do_eof( argc, argv ) {}
do_eoln( argc, argv ) {}
do_page( argc, argv ) {}
do_get( argc, argv ) {}
do_put( argc, argv ) {}
do_reset( argc, argv ) {}
do_rewrite( argc, argv ) {}
do_charwait(argc, argv) {}
do_getch(argc, argv) {}
do_append( argc, argv ) {}
do_update( argc, argv ) {}
do_setnext( argc, argv ) {}
closeall(){}
#endif
putout( whfile, where, style )
fileptr whfile;
pointer where;
int style;		/* true for text file, false otherwise */
{
	register int i;
#ifndef OLMSG
	static char	writErr[]	= "badwrite3`Error writing file (disk full?)";
#endif

	/* if it is a random file, we can put anywhere */
	/* in fact eof should not be true but for now we let it */
	if( !(whfile->f_flags & FIL_WRITE ) ) {
		tp_error( 3, 0 );
		run_error( ER(102,"notwrite`Put/Write - File not open for writing") );
		}
	if( !(whfile->f_flags & (FIL_EOF|FIL_RANDOM|FIL_DEVICE)) ) {
		tp_error( 0xf0, 0 );
		run_error( ER(103,"noteof`Put/Write - Not at end of file") );
		}
	/* If a text file */
	if( style ) {
		if( whfile->f_flags & FIL_SCREEN )
			prg_output( TRUE, where );
	 	else
#ifdef ATARI520ST
			fputs( where, whfile->desc.f_stream );
#else
			if (fputs( where, whfile->desc.f_stream ) == EOF) {
				tp_error( 0xF0, 0 );
				run_error(ER(276, writErr));
				}
#endif
		}
		
	 else
#ifdef QNX
		if( fput( where, whfile->f_size, whfile->desc.f_stream )
					!= whfile->f_size ) {
			tp_error( 0xf0, 0 );
			run_error(ER(276, writErr));
			}
#else
		for( i = 0; i < whfile->f_size; i++ )
			if (fputc( where[i], whfile->desc.f_stream ) == EOF) {
				tp_error( 0xf0, 0 );
				run_error(ER(276, writErr));
				}
#endif QNX
}
fileout( filedesc, string )
fileptr filedesc;
char *string;
{
	register fileptr realdesc = filedesc ? filedesc : &fil_output;
	putout( realdesc, string, TRUE );
	/* turn off has EOL */
	realdesc->f_flags &= ~FIL_EOL;
}


eolcheck( whfile )
fileptr whfile;
{
	if( whfile->f_flags & FIL_NREAD )
		_a_get( whfile, TRUE );
	return whfile->f_flags & FIL_EOL;
}

do_cursorto( argc, argv, bsym )
int argc;
rint *argv;
symptr bsym;
{
	extern WINDOW *pg_out;
	extern int step_flag;
	extern WINDOW *pg_out;
	int	row, col;
#ifdef SAI
	if( sai_out == SAI_NORMAL ) 
		sai_err("CursorTo");
#endif
	checkScreen();

	/* gotoxy is -83 */
	if( sym_saveid(bsym) == -83 ) {
		row = int_spop() - 1;
		col = int_stop - 1;
		}
	else {
		col = int_spop();
		row = int_stop;
		}
	if( turbo_io && row >= pg_out->_maxy && row < 25 )
		row = pg_out->_maxy - (25 - row);

	if( wmove(pg_out, row, col) != OK && !step_flag )
		run_error( ER(304,"cto`Cursor coordinates out of bounds") );
}

do_scrfunc( argc, argv )
int argc;
rint *argv;
{
	extern WINDOW *pg_out;
	register int rv;
#ifdef msdos
	extern int mono_adapter;
#endif
	int y,x;
	extern struct modeInfo	*RunModeInfo;	/* current runtime mode info */

#ifdef SAI
	if( sai_out == SAI_SCREEN )
#endif
		getyx( pg_out, y, x );

	switch( int_stop ) {
		case 1:
#ifdef SAI
			if( sai_out == SAI_NORMAL ) sai_err("ScrXY(1)");
#endif
			rv = pg_out->_maxy;
			break;
		case 2:
#ifdef SAI
			if( sai_out == SAI_NORMAL ) sai_err("ScrXY(2)");
#endif
			rv = pg_out->_maxx;
			break;
		case 3:
			rv = RunModeInfo->pixHeight;
			break;
		case 4:
			rv = RunModeInfo->pixWidth;
			break;
		case 5:
			rv = RunModeInfo->maxColour;
			break;
		case 7:
#ifdef msdos
			rv = mono_adapter;
#else
			rv = TRUE;
#endif
			break;
		case 8:
#if defined(msdos) || defined(QNX)
			{
			extern int mon_is_colour;
			rv = mon_is_colour;
			}
#else
			rv = FALSE;
#endif
			break;
		case -1:
#ifdef SAI
			if( sai_out == SAI_NORMAL ) sai_err("ScrXY(-1)");
#endif
			rv = y;
			break;
		case -2:
#ifdef SAI
			if( sai_out == SAI_NORMAL ) sai_err("ScrXY(-2)");
#endif
			rv = x;
			break;
		default:
			scr_err(int_stop);
		}
	*argv = rv;
}

#ifdef SAI
sai_err( str )
char *str;
{
	char buf[80];
	sprintf( buf, "%s cannot be called by the StandAlone Interpreter\n",str);
	sai_print(buf);
	sai_print( "when the +w option is not used\n" );
	done();
}
#endif

scr_err( i )
int i;
{
	run_error( ER(104,"scrxy`ScrXY: Illegal screen query %d\n"), i );
}

int io_res_on = FALSE;		/* is i/o result checking on */
int last_ioerr = 0;		/* code from last operation */

do_iocheck( )
{
	io_res_on = int_spop();
}

do_ioresult( argc, argv )
int argc;
rint *argv;
{
	*argv = last_ioerr;
	last_ioerr = 0;
}
