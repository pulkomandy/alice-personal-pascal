
#include <stdio.h>
typedef unsigned int bits16;

#include "huff.h"

extern char *hgets();
extern long ftell();

char *huff_inp;
char *f_help;
int strsize, tokcount;	/* Number of strings, tokens in the tree */
int num_nodes;		/* Number nodes in the tree */
FILE *huff_file = (FILE *)NULL;

static unsigned int help_buf, help_mask;   /* 8 bit buffers for huff bits */

static int help_eof;	/* Boolean that is true if eof reached */

static char *text = NULL;	/* pointer to token strings */
static file_entry *FileTable;	/*List of hash values and huffed file lengths */
static int *HuffTree;	/* Pointer to array which represents Huffman tree */

static int Multiplier;	/* Multiplier used in hash function */
static int FileCount;	/* Number of entries in the file table */
static int help_pos;	/* Current position in tree */
static int HuffVers;	/* Version number of the help files */
static long HuffOffset;	/* Offset from beginning of file to huffman codes */



#define TRUE	1
#define FALSE	0

extern char *malloc();

char *
checkalloc( size )
int size;
{
	/* You can add debugging code here if desired */
	char *p;
	p = malloc(size);
	return p;
}

/* Dummy routine */
hclose(){}

FILE *
hopen( fname )
char *fname;
{
	int i, hval;
	long seek_adr;	/* offset to huffman codes themselves */

	/*
	 * If we could not open the Huff Help file at init, say that we could
 	 * not open this file
 	 */

	if( !huff_file ) return (FILE *)NULL;

	/* Search for the filename in the index */

	/* set seek start where huffman data starts */
	seek_adr = HuffOffset;

	printf( "Data starts at %ld\n", seek_adr );

	huff_inp = NULL;

	hval = HashIndexName( fname );
	printf( "Hashed string '%s' to value %4x\n", fname, hval );

	for( i=0; i<FileCount; i++ ) {

		/* Sum up lengths to get seek address */
		seek_adr += FileTable[i].h_offset;
		if( hval == FileTable[i].hash ) {
			printf( "Found hash value at file: %d\n", i );
			printf( "Seek address: %ld\n", seek_adr );

			/* Seek to the appropriate spot and init dehuffing */
			fseek( huff_file, seek_adr, BOF );

			printf( "File pointer now at %ld\n", ftell(huff_file));

			help_pos = 0;
			help_mask = 0;
			help_eof = FALSE;
			/* Return non-zero, signifying everything okay */
			return (FILE *)1;
		}
	}
	printf( "Hash value was not found\n" );

	/* Hash value not found */
	/* So return a code saying the open didn't work */
	return (FILE *)NULL;
}

/* This is the procedure that reads in the index of the compressed help
 * file as well as the huffman tree, and the tokens 
 * returns:   0  - everything went okay
 *	      1  - wrong version number
 * 	      2  - couldn't open the file
 * 	      3  - error allocating memory for the buffers 
 */

int
init_help( huffname )
char *huffname;
{
	int status;

	/* Read in the index  (first part of the huffman file */
	status = ReadInTree( huffname );
	if( status ) {
		huff_file = (FILE *)NULL;
		return status;
	}

	/* Now open the file (again), in preparation for the seeks/decoding 
	 * and leave it open
	 */

	huff_file = fopen( huffname,"rb");

	if (huff_file == (FILE *)0)
		return 2;

	return 0;	/* everything went alright */
	
}

close_help()
{
    	if( huff_file ) {
		fclose(huff_file);
		huff_file = (FILE *)NULL;
		free( HuffTree );
		free( FileTable );
		/* only free text if allocated */
		if( text ) free( text );
	}
}

char *
hgets( linebuf, size )
char *linebuf;
int size;
{
	char lastchar = 1;
	char *lineptr;
	char *slinebuf = linebuf;

	int  charcount = 0;
	register int fast_pos;
	register int token;

	if( help_eof == TRUE ) { 
		return NULL;
	}

	/* If we are holding a string back, return it */
	if( huff_inp ) {
		lastchar = 0;
		while( (*huff_inp) && (lastchar != '\n') &&
			(charcount < size) ) {
			lastchar = *linebuf++ = *huff_inp++;
			charcount++;
		}
		/* If we have finished the string, set huff_inp to 0 */
		if( *huff_inp == 0 ) huff_inp = 0;
	}
	
	fast_pos = help_pos;

	while( (lastchar != '\n') && (charcount < size) ) {

		if( help_mask == 0 ) {
			help_buf = hf_getword( huff_file );
			help_mask = 0x8000;
		}

		if( help_buf & help_mask ) {
			fast_pos = HuffTree[ (fast_pos<<1) + 1 ];
		}
		else {
			fast_pos = HuffTree[ fast_pos<<1 ];
		}
		help_mask >>= 1;

		if( HuffTree[ fast_pos<<1 ] == 0 ) {
			token = HuffTree[ (fast_pos<<1)+1 ];
			if( token == 0 ) { 
				help_eof = TRUE;
				if( charcount > 0 ) {
					*linebuf = 0;
					return slinebuf;
				} else
					return NULL;
			}

			if( token >= 128 ) {
				lineptr = &text[token - 128];
				while( *lineptr ) {
					lastchar = *linebuf++ = *lineptr++;
					charcount++;

					/* if this is a string with an embedded
					 * newline in it, return but keep a ptr
					 * to where we were in the string
					 */
					if( charcount >= size ) {
						huff_inp = lineptr;
						break;
					}

					if( lastchar == '\n' ) {
						huff_inp = lineptr;
						break;
					}
				}
			}
			else {
				lastchar = *linebuf++ = token;
				charcount++;
				}
			fast_pos = 0;
			/* Start back at the beginning of the tree */
		}
	}
  	*linebuf = 0;
	help_pos = fast_pos;
  	return slinebuf;
}

char
hgetc()
{
	char line[2];

	if( hgets( line, 1 ) ) {
		return line[0];
	} else {
		return EOF;
	}
}

int
hf_getword( fp )
FILE *fp;
{
	int buf;

	buf = getc( fp ) << 8;
	buf |= getc( fp );
	return buf;
}

ReadInTree( fname )
char *fname;
{

	FILE *ht;
	register int i, j, h;
	extern long ftell();
	int num;

	ht = fopen( fname, "rb" );

	if( ht==NULL ) {
		printf( "Error - could not open %s\n", fname );
		return( 2 );	/* Error opening file */
	}

	/* Get the version number, if wrong, return an error */
	HuffVers = hf_getword( ht );

	printf( "Version number: %d\n", HuffVers );

	/* Get memory requirements of strings */

	strsize = hf_getword( ht );	

	printf( "Size of strings: %d\n", strsize );

	if( strsize ) {
		text = (char *) checkalloc( strsize );

		if( text == NULL ) {
			fclose( ht );	
			return 3;
		}

		/* Read in the list of strings */
		fread( text, sizeof( char ), strsize, ht );
	}

	/* Now that we have read in the words, read in the table */
	num_nodes = hf_getword( ht );

	printf( "Number of nodes: %d\n", num_nodes );

	HuffTree = (int *) checkalloc( num_nodes * 2 * sizeof( int ) );

	if( HuffTree == NULL ) {
		fclose( ht );
		if( text ) free( text );
		return 3;
	}

	fread( HuffTree, 2 * sizeof( int ), num_nodes, ht );

	FileCount = hf_getword( ht );
	Multiplier = hf_getword( ht );

	printf( "File count: %d,  Multiplier: %d\n", FileCount, Multiplier );

	FileTable = (file_entry *) checkalloc( sizeof( file_entry )
					* FileCount );

	if( FileTable == NULL ) {
		free( HuffTree );
		if( text ) free( text );
		fclose( ht );
		return 3;
	}

	fread( FileTable, sizeof( file_entry ), FileCount, ht );

	for( i=0; i<FileCount; i++ ) {
		printf( "%4d: %4x %8d\n", i, FileTable[i].hash,
			FileTable[i].h_offset );
	}

	for( i=0; i<FileCount; i++ ) {
		h = FileTable[i].hash;
		num = 0;
		for( j=0; j<FileCount; j++ ) {
			if( FileTable[j].hash == h ) num++;
		}
		if( num != 1 ) {
			printf( "Error hash code occurs %d times for file %d\n",
				num, i );
		}
	}

	/* If the version number is not the same as at the beginning of
         * the file, give an error 
         */

	i = hf_getword(ht);

	printf( "Second version number: %d\n", i );

	HuffOffset = ftell( ht );  /* huffman codes start now... */

	printf( "Data starts at: %ld\n", HuffOffset );

	fclose( ht );
	return 0;	/* everything ok */
}

int
HashIndexName( fname )
char *fname;
{
	int hashval = 0;

	while( *fname ) {
		hashval *= Multiplier;
		hashval += *fname++;
	}
	return hashval;
}

main(argc, argv )
int argc;
char *argv[];
{

	int status;
	char line[100];
	char *str;

	str = "c:\\helpfile.huf";

	if( argc > 1 )
		str = argv[1];

	printf( "Testhelp(%s)\n", str );

	status = init_help( str );

	printf( "Status of init: %d\n", status );

	for( ;; ) {

		printf( "Filename:\n" );

		if( !gets( line, 80 ) ) break;

		RemoveNL( line );

		if( hopen( line ) ) {
			printf( "Open was successfull\n" );
			while( hgets( line, 90 ) ) {
				printf( "%s\n", line );
			}
		}
	}
	printf( "Testhelp finished\n" );
}

RemoveNL( str )
char *str;
{
	int len;

	len = strlen(str);
	if( str[len-1] == '\n' )
		str[len-1] = 0;
}
