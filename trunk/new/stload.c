
#include "alice.h"
#include "interp.h"

#include <obdefs.h>

extern char *checkalloc();
extern FILE *qfopen();
extern char *allocstring();

typedef int (* fun_ptr)();

#define FROM_CURRENT		1	/* Seek from current file pos */

/*
 * The header of an executable
 */

typedef struct hdr {
	int	magic;		/* Magic word, contains 601Ah */

	long	text_size;	/* size of text segment */
	long	data_size;	/* size of data segment */

	long	bss_size;	/* Block storage segment size */
	long	stab_size;	/* Symbol Table size */

	long	zero1;		/* reserved, zero */
	long	zero2;		/* reserved, zero */

	int	reloc;		/* Flag - 0 - relocation info present */

	} HEADER;


/*
 * Maximum number of libraries
 */

#define MAX_TLIB 10

int num_libs = 0;
long st_libadr;
long st_version = ALICE_VERSION;
long st_parms;

extern long cfunc_array[];
extern long p_array[];

long STLibPBlock[] = {
	(long)cfunc_array,
	(long)p_array,
	};

struct tlib {
	char *libname;
	char *libadr;		/* address of library */
	} active_libs[MAX_TLIB];

#define SAV_RSIZE 100

do_tplib( argc, argv, sym )
int argc;
pointer *argv;
symptr sym;
{
	char *nam;
	register int libnum;
	fun_ptr the_func;
	int aa[30];
	int offset;
	int tc_result;

	nam = argv[-2] + STR_START;

	offset = argv[-4];

	construct( argv - 4, argc - 2, aa, sizeof( aa ) / sizeof(int) );

	for( libnum = 0; libnum < num_libs; libnum++ )
		if( !strcmp( nam, active_libs[libnum].libname ) )  {
			goto gotlib;
			}

	if( num_libs < MAX_TLIB-1 ) {
		char *libload;
		extern char *comload();

		if( libload = comload( nam ) ) {
			active_libs[num_libs].libadr = libload;
			active_libs[num_libs++].libname = allocstring(nam);
			}
		 else 
			run_error(ER(308,"llib`Could not load library %s"),nam);
		}
	 else
		run_error( ER(309,"llib`Too many loaded libraries : %s"), nam );

gotlib:

	/* Now call the routine */
	the_func = active_libs[libnum].libadr + (long)offset;
	st_libadr = (long) the_func;
	st_parms = STLibPBlock;

	tc_result = call_lib( aa[0], aa[1], aa[2], aa[3], aa[4], aa[5],
			aa[6], aa[7], aa[8], aa[9], aa[10], aa[11], aa[12],
			aa[13], aa[14], aa[15], aa[16], aa[17], aa[18],
			aa[19], aa[20] );

	/* If STLibFunc, store the return value */
	if( sym_saveid(sym) == -130 )
		*(rint *)argv = (rint)tc_result;
}

/*
 * Load a library from a file 'fp' that has been opened for binary read
 */

char *
lbfload( fp )
FILE *fp;
{

	HEADER hdr;
	long offset;
	int value;
	long total;
	int i, c;
	long image_size;
	char *mem;

	fread( &hdr, 1, sizeof( HEADER ), fp );

	if( hdr.magic != 0x601a ) {
		return 0L;
	}

	total = hdr.text_size + hdr.data_size + hdr.bss_size;

	mem = (char *) checkalloc( (int)total );

	if( mem == (char *)0 ) {
		return 0L;
	}

	image_size = hdr.text_size + hdr.data_size;

	/*
	 * Now read in the text and data
	 */
	if( fread( mem, 1, (int)image_size, fp ) != (int)image_size ) {
		free( mem );
		return 0L;
	}

	/*
	 * Seek past the symbol table
	 */
	if( hdr.stab_size )
		fseek( fp, hdr.stab_size, FROM_CURRENT );

	/*
	 * Now there is the relocation info
	 * (if present)
	 */
	if( hdr.reloc == 0) {

		fread( &offset, 1, sizeof( long ), fp );

		/* Offset is 0L if no reloc info */
		if( offset ) {
			Reloc( mem, offset );
			while( (c = fgetc( fp )) != 0 ) {
	
				if( c == EOF ) {
					break;
				}
				if( c == 1 )
					offset += 254;
				else {
					offset += c;
					Reloc( mem, offset );
				}
			}
		}
	}

	/*
	 * Return the image
	 */
	return mem;
}

Reloc( mem, offset )
char *mem;
long offset;
{
	long *lptr;
	long value;

	lptr = (long)mem + (long)offset;
	value = *lptr;
	*lptr = value + (long)mem;
}

char *
comload( fname )
char *fname;
{
	FILE *fp;
	char *ret;

	/*
	 * Read the header of a .TOS or .PRG file
	 */

	fp = qfopen( fname, "rb" );

	/* If we can't open the file, then return 'nil' */
	if( fp == (FILE *)0 ) {
		return 0L;
	}

	/* Otherwise, load the library */
	ret = lbfload( fp );

	/* Whatever happened, close the file */
	fclose( fp );

	return ret;

}
