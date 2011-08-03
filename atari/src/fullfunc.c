#include "alice.h"
#include <curses.h>
#include "interp.h"
#include "extramem.h"
#include "flags.h"

do_hilo(argc,argv,symp)
int argc;
rint * argv;
symptr symp;
{
	register unsigned arg = int_stop;
	switch( sym_saveid(symp) ) {
		case -151:	/* lo */
			*argv = arg & 0xff;
			break;
		case -150:	/* hi */
			*argv = arg >> 8;
			break;
		case -152:	/* swap */
			*argv = (arg >> 8) | (arg << 8);
			break;
		}
}

do_where(argc,argv,symp)
int argc;
rint * argv;
symptr symp;
{
	extern WINDOW *pg_out;

	if( !pg_out )
		*argv = 0;
	else
		*argv = (sym_saveid(symp) == -154 /* wherex */ ? pg_out->_curx :
				pg_out->_cury ) + 1;
}

do_filefunc(argc,argv,symp)
int argc;
rint *argv;
symptr symp;
{
	char *fname;
	long loc;
	int code;
	extern long ftell();
	extern long filelength();
	fileptr thefile = (fileptr)p_spop();

	fname = thefile->f_name;

	code = sym_saveid(symp);

	if( code != -157 )
		check_undef( thefile, 1 );

	switch( code ) {
		case -157: /* erase */
			check_undef( &thefile->f_name, 1 );
			if( unlink(fname) ) {
				tp_error( 1, 0 );
				run_error( ER(321,"erase`Erase - can't delete file %s"), fname );
				}
			break;
		case -158: /* flush */
			fflush( thefile->desc.f_stream );
			break;
		case -171: /* filepos */
			/* if we have just read the record, we will be one
			   past */
			loc = ftell( thefile->desc.f_stream ) / seeksize(thefile);
			intlong(loc);
			*argv = (int)loc;
			break;
		case -181: /* longfilepos */
			loc = ftell( thefile->desc.f_stream ) / seeksize(thefile);
			*(rfloat *)argv = (rfloat)loc;
			break;
		case -182: /* filesize */
#ifdef MarkWilliams
			loc = filelength( thefile->desc.f_stream ) /
				seeksize( thefile );
#else
			loc = filelength( fileno( thefile->desc.f_stream ) ) /
						seeksize(thefile);
#endif
			intlong( loc );
			*argv = (int) loc;
			break;
		case -183: /* long file size */
#ifdef MarkWilliams
			loc = filelength( thefile->desc.f_stream ) /
				seeksize( thefile );
#else
			loc = filelength( fileno( thefile->desc.f_stream ) ) /
						seeksize(thefile);
#endif
			*(rfloat *)argv = (rfloat)loc;
			break;

		case -184: /* seekeoln */
			while( !eolcheck( thefile ) && strchr( " \t", thefile->f_buf ) )
					_a_get( thefile, FALSE );
			*argv = !!eolcheck( thefile );
			break;

		case -185: /* seekeof */
			while( TRUE ) {
				/* if lazy, do the fetch */
				if( thefile->f_flags & FIL_NREAD )
					_a_get( thefile, TRUE );
				/* if at end of file, quit */	
				if( thefile->f_flags & FIL_EOF )
					break;
				/* if not a whitespace, quit */
				if( !strchr( " \t\r\n", thefile->f_buf) )
					break;
				/* get the next character */
				inp_char( thefile, TRUE );
				}
			*argv = !!(thefile->f_flags & FIL_EOF);
			break;
		}
}

intlong(loc)
long loc;
{
	if( loc > 32767L ) {
		tp_error( 0xf2, 0 );
		run_error( ER(322,"rob`Can't reference record %ld (beyond 32767)"), loc );
		}
}

do_rndomize(argc,argv,symp)
int argc;
pointer argv;
symptr symp;
{
	long time();  /* no see! */

	srand( (int)time((long *)0) );
}

do_ptr(argc,argv,symp)
int argc;
rpointer argv;
symptr symp;
{
	FP_OFF( *argv ) = int_spop();
	FP_SEG( *argv ) = int_stop;
}

do_segoff(argc,argv,symp)
int argc;
rint * argv;
symptr symp;
{
	*argv = sym_saveid(symp) == -166 /* off */ ? FP_OFF(int_stop) :
						FP_SEG(int_stop);
}

do_move(argc,argv)
int argc;
pointer argv;
{
	rint nbytes;
	rpointer to;

	nbytes = int_spop();
	to = p_spop();
	blk_move( to, p_spop(), nbytes );
	do_undef( to, U_SET, nbytes );
}

do_fillch(argc,argv)
int argc;
pointer argv;

{
	rint filler;
	rint nbytes;

	filler = int_spop();
	nbytes = int_spop();

	fillchar( p_stop, filler, nbytes );
	do_undef( p_stop, U_SET, nbytes );

}

do_rename(argc,argv,symp)
int argc;
pointer argv;
symptr symp;
{
	unsigned char *newname;
	fileptr oldname;

	newname = str_spop() + STR_START;
	do_undef( newname, U_CHECK, newname[-1] );

	oldname = (fileptr) p_stop;

	check_undef( &oldname->f_name, 1 );
	if( rename( oldname->f_name, newname ) ) {
		tp_error( 1, 0 );
		run_error( ER(322,"rename`Rename of %s to %s failed"),
				oldname->f_name, newname );
		}
	givename( oldname, newname );
}

do_dirfunc(argc,argv,symp)
int argc;
pointer argv;
symptr symp;
{
	char *dname = str_spop() + STR_START;
	do_undef( dname, U_CHECK, dname[-1] );
	switch( sym_saveid(symp) ) {
		case -172:	/* chdir */
			if( strlen(dname) > 1 && dname[1] == ':' ) {
				selDisk( (dname[0] & 0x1f) - 1 );
				dname += 2;
				}
			if( chdir( dname ) ) {
				tp_error( 1, 0 );
				run_error( ER(207,"baddir`Can't chdir to %s"),
								dname );
				}
			break;
		case -173:
			if( mkdir( dname ) ) {
				tp_error( 1, 0 );
				run_error( ER(323,"dir`Can't create directory %s"), dname );
				}
			break;
		case -174:
			if( rmdir( dname ) ) {
				tp_error( 1, 0 );
				run_error( ER(323,"dir`Can't remove directory %s"), dname );
				}
			break;
		}
}

do_getdir(argc,argv)
int argc;
pointer argv;
{
	unsigned char *destr;
	int seldnum;
	char dlet;
	char dirbuf[255];
	nodep strtype;

	destr = (char *)p_spop() + STR_START;
	strtype = NCAST p_spop();
	checksvar( strtype, LDS( 466, "Directory string" ));
	seldnum = int_stop;
	if( seldnum < 0 || seldnum > selDisk(getDisk()) ) {
		tp_error( 1, 0 );
		run_error( ER(325,"baddir`GetDir: Invalid drive number") );
		}
		
	getCDir( dirbuf, seldnum );
	if( strlen(dirbuf) + 3 > get_stsize( strtype, destr ) )
		run_error( ER(41,"stringbig`String too long") );
	dlet = seldnum ? '@' + seldnum : 'A' + getDisk() ;
#ifdef TOS
	sprintf( destr, "%c:%s", dlet, dirbuf );
#else
	sprintf( destr, "%c:\\%s", dlet, dirbuf );
#endif
	destr[-1] = strlen(destr);
	do_undef( destr-1, U_SET, destr[-1] + 2 );
}

do_markrel(argc,argv,symp)
int argc;
pointer argv;
symptr symp;
{
	rpointer * thevar;
	nodep vartyp;
	rpointer freetop;
	extern rpointer newAlloc();
	/* we use the regular allocator to simulate mark and release */
	/* any improper use will cause a 'dispose of disposed memory' error */

	thevar = (rpointer *)p_spop();
	vartyp = NCAST p_stop;

	freetop = newAlloc( 1 );
	newFree( freetop, 1 );

	if( sym_saveid(symp) == -176 /*mark*/ ) {
		*thevar = freetop;
		do_undef( thevar, U_SET, sizeof(rpointer) );
		}
	 else { /* release */
		check_undef( thevar, 1 );
		newFree( *thevar, freetop - *thevar );
		}
		
}

/* This can only be done on untyped "anyfile" files */

do_blockio(argc,argv,symp)
int argc;
pointer argv;
symptr symp;
{
	rint *result;
	unsigned char *dest, *desptr;
	unsigned nrecs;
	fileptr blockfile;
	rint numxfer;
	long maxb, bcount;	/* for simulated fread */
	int readch;
	FILE *strm;		/* input stream */

	if( argc > 3 )
		result = (rint *)p_spop();
	nrecs = (unsigned)int_spop();
	dest = p_spop();
	blockfile = (fileptr)p_stop;

	check_undef( blockfile, 1 );
	if( blockfile->f_size )
run_error( ER(323,"blockio`Block I/O routine must be performed on untyped file") );
	if( sym_saveid(symp) == -178 ) {

		if( !(blockfile->f_flags & FIL_READ ) ) {
			tp_error( 2, 0 );
			run_error( ER(95,"notread`READ/GET - file not reset for reading") );
			}
		/* do the transfer */
		/* fread does not allow partial transfer, but blockread does */
		strm = blockfile->desc.f_stream;
		desptr = dest;
		maxb = 128L * nrecs;
		for( bcount = 0; bcount < maxb; bcount++ )
			if( (readch = getc( strm ) ) == EOF )
				break;
			 else
				*desptr++ = readch;
		numxfer = (int)((bcount + 127) / 128 );
		do_undef( dest, U_SET, (unsigned)bcount );
		}
	 else {
		/* block write */
		if( !(blockfile->f_flags & FIL_WRITE ) ) {
			tp_error( 3, 0 );
			run_error( ER(102,"notwrite`BlockWrite - file not prepared for writing") );
			}
		do_undef( dest, U_CHECK, 128 * nrecs );
		numxfer = fwrite( dest, 128, nrecs, blockfile->desc.f_stream );
		}
	if( argc > 3 ) {
		*result = numxfer;
		do_undef( result, U_SET, sizeof(rint) );
		}
}

do_longseek(argc,argv)
int argc;
rfloat * argv;
{
	rfloat seekloc;
	fileptr seekfile;
	long howfar;

	seekloc = f_spop();
	seekfile = (fileptr)p_stop;

	check_undef( seekfile, 1 );
	
	howfar = (long)seekloc * (long)seeksize(seekfile);
	if( fseek( seekfile->desc.f_stream, howfar, 0 ) ) {
		tp_error( 0x91, 0 );
		run_error( ER(305,"badseek`Seek/Setnext out of bounds") );
		}
	/* mark data to be read by readget */
	seekfile->f_flags |= FIL_NREAD;
}

#define MAXARGS 20

static int got_comline = FALSE;
static char cmdline_buffer[127];

#ifdef SAI
/* These are set by the mainline sai_main */
int num_args = 0;
char **coml_args;
#else SAI
static int num_args = 0;
static char *coml_args[MAXARGS];
#endif SAI

#ifdef GEM

#ifndef SAI
char *strtokptr;

char *
strtok( buf, delims )
char *buf;
char *delims;
{
	char *s;

	if( buf )
		strtokptr = buf;

	if( strtokptr == (char *)0 ) return strtokptr;

	s = strtokptr;

	while( strchr( delims, *strtokptr ) == (char *)0 && *strtokptr )
			strtokptr++;

	/*
	 * If we have reached the end of the string, null the pointer
	 */
	if( *strtokptr == 0 )
		strtokptr = (char *)0;
	else
		*strtokptr++ = 0;

	return s;
}
#endif SAI

#endif GEM

parse_comline()
{
#ifndef SAI

	extern char *strtok();

	if( got_comline )
		return;
	got_comline = TRUE;
	num_args = 0;
	getprline( LDS( 418, "Command Line? " ),
		cmdline_buffer, sizeof(cmdline_buffer) );

	coml_args[0] = strtok( cmdline_buffer, " \t" );
	while( coml_args[num_args] && num_args < MAXARGS-1 )
		coml_args[++num_args] = strtok( (char *)0, " \t" );

#endif SAI
}

	

do_parstr(argc,argv)
int argc;
pointer argv;
{
	char *retstr;
	rint argnum;

	argnum = int_stop;

	parse_comline();
	if( argnum < 1 || argnum > num_args )
		retstr = "";
	 else
		retstr = coml_args[argnum-1];
	str_ret_push( argv, retstr, strlen(retstr) );
}

do_parcnt(argc,argv)
int argc;
rint * argv;
{
	parse_comline();
	*argv = num_args;
}

fullinit()
{
	got_comline = FALSE;
}

do_pack(argc, argv)
int argc;
pointer argv;
{
	rint offset;
	rpointer source;
	rpointer dest;
	unsigned int compsize;
	unsigned int destsize;
	int setslen = FALSE;
	nodep desttype, sourcetype;

	dest = p_spop();
	desttype = NCAST p_spop();
	offset = int_spop();
	skip_type();
	source = p_spop();
	sourcetype = NCAST p_stop;

	compsize = int_kid( 2, sourcetype );
	destsize = int_kid( 4, desttype );

	offset -= int_kid( 3, sourcetype );
	/* account for extra zero byte possible in string */
	if( setslen = is_stringtype( desttype ) ) {
		dest += STR_START;
		destsize -= STR_START;
		}
	blk_move( dest, source + offset * compsize, destsize * compsize );
	do_undef( dest, U_SET, destsize * compsize );
	/* If it is a string, set the Turbo Pascal style length */
	if( setslen ) {
		/* set zero byte at end */
		dest[destsize] = 0;
		dest[-1] = strlen( dest );
		do_undef( dest-1, U_SET, 1 );
		}
}

do_unpack(argc, argv)
int argc;
pointer argv;
{
	rint offset;
	rpointer source;
	rpointer dest;
	unsigned int compsize;
	unsigned int sourcesize;
	nodep desttype, sourcetype;

	offset = int_spop();
	skip_type();
	dest = p_spop();
	desttype = NCAST p_spop();
	source = p_spop();
	sourcetype = NCAST p_stop;

	compsize = int_kid( 2, desttype );
	sourcesize = int_kid( 4, sourcetype );
	/* account for extra zero byte possible in string */
	if( is_stringtype( sourcetype ) ) {
		source += STR_START;
		sourcesize -= STR_START;
		}
	offset -= int_kid( 3, desttype );
	dest += offset * compsize;
	blk_move( dest , source, sourcesize * compsize );
	do_undef( dest, U_SET, sourcesize * compsize );

}
do_opendir( argc, argv )
int argc;
pointer *argv;
{
	int attr = int_spop();
	dofirst( str_spop() + STR_START, attr );
}

do_readdir( argc, argv )
int argc;
pointer *argv;
{
	extern int dir_current;
	extern char *curfilename();
	char *fname;
	rint *attarg;

	if( dir_current ) {
		attarg = (rint *)p_spop();
		fname = curfilename(attarg);
		str_ret_push( argv, fname, strlen(fname) );
		dir_current = !findNext();
		do_undef( attarg, U_SET, sizeof(rint) );
		}
	 else
		str_ret_push( argv, "", 0 );
}

do_artostr(argc, argv)
int argc;
pointer *argv;
{
	pointer aradr;
	nodep artype;
	int size;
	int len;
	char buf[257];

	aradr = p_spop();
	artype = NCAST p_spop();
	if( ntype(artype) != N_TYP_ARRAY
			|| int_kid(2,artype) != 1
			|| node_flag(artype) & NF_STRING
			|| (size = calc_size(artype,TRUE)) > 255 )
		run_error( ER(326,"Argument of ArToStr must be array of char less than 255 in size"));
	strncpy( buf, aradr, 255 );
	buf[256] = 0;
	len = strlen(buf);
	do_undef( aradr, U_CHECK, len );
	str_ret_push( argv, buf, len );
}

do_strtoar(argc,argv)
int argc;
pointer *argv;
{
	unsigned char *thestr;
	pointer thear;
	int arsize;


	thestr = str_spop() + STR_START;

	skip_type();
	thear = p_spop();
	arsize = calc_size( NCAST p_spop(), TRUE );
	strncpy( thear, thestr, arsize );
	do_undef( thear, U_SET, min( arsize, 1+strlen(thestr) ) );

}
