
#include "alice.h"
#include "alctype.h"
#include "extramem.h"
#include <curses.h>
#include <stdlib.h>
#include <unistd.h>
/*
 *DESC: Code to get help screens from compressed help files
 */


#ifndef HELPDIR
# if defined(QNX) || defined(msdos)
#  define HELPDIR "/alice/help"
# else
#  define HELPDIR "/u/alice/help/ibm"
# endif
#endif HELPDIR

extern FILE *qfopen();

#ifdef TESTHELP
static char verserr[] = "Bad version number.  Got %d, should be %d\n";

main( )
{
	char line[100];
	char class_ln[30];
	char sub_ln[30];
	int  status;

	FILE *helpf;
	FILE *hopen();
	int hfgets();

	status = init_help();	/* open the files, etc */

	if( status == 0 ) { 
		printf( "Open worked okay\n" );
	} else
		printf( "Open failed\n" );

	for(;;) {
		printf( "Class:\n" );
		gets( class_ln, 30 );
		printf( "Subject:\n" );
		gets( sub_ln, 30 );
		helpf = hopen( class_ln, sub_ln );
		if( helpf == NULL ) {
			printf( "Error: can not open %s%s\n",
				class_ln, sub_ln );
			exit( 1 );
		}
		while( hfgets( line, 80, helpf ) ) {
			printf( "%s", line );
		}
		hclose( helpf );
	}

}

char *malloc();

char *
checkalloc( size )
int size;
{
	char *p;
	p = malloc(size);

	if( !p )
		printf( "malloc fails\n" );
	return p;
}

#endif TESTHELP

#ifdef HUFFHELP

FILE *help_open();
int  help_gets();
int  init_help();

#endif

static char *help_dir;

FILE *
hopen( hdir, sub )
char *hdir;	/* directory, possibly nil */
char *sub;	/* subject */
{
	char hfname[MAX_STR_LEN];
	char nsub[MX_FM_WIDTH+1];
	register char *p1;

	strncpy( nsub, sub, MX_FM_WIDTH );

#define HELP_FLEN 12

	if( !strchr(nsub, '/' ) && strlen(nsub) > HELP_FLEN ) {
		nsub[HELP_FLEN] = 0;
		}

#ifdef HUFFHELP
	sprintf( hfname, "%s%s", hdir, nsub );
#else

	if (!help_dir ) {
		char *evar;
		char *places[5];
		int i;

		places[0] = getenv( "ALICEDIR" );
		places[2] = getenv("HOME");
		places[1] = ALICE_DIR;
		places[3] = ".";

		/* find the help directory */
		for( i = 0; i <= 3; i++ ) {
			if( places[i] ) {
				sprintf( hfname, "%s/help", places[i] );
				if( access( hfname, R_OK ) == 0 ) {
					help_dir = allocstring(hfname);
					break;
					}
				sprintf( hfname, "%s/.alice/help", places[i] );
				if( access( hfname, R_OK ) == 0 ) {
					help_dir = allocstring(hfname);
					break;
					}
				}
			}

		}
	if( help_dir )
		sprintf( hfname, "%s/%s%s", help_dir, hdir, nsub );
	 else
		 return (FILE *)0;
#endif HUFFHELP

	/* convert spaces to underscores, uppercase to lower */
	for( p1 = hfname; *p1; p1++ ) {
		if( !isalnum( *p1 ) && (*p1 != '/') )
			*p1 = '_';
		 else if( isupper( *p1 ) )
			*p1 += 'a' - 'A';
		}

#ifdef HUFFHELP
	return (FILE *) help_open( hfname );
#else
	return fopen( hfname, "r" );
#endif HUFFHELP
}


#ifdef HUFFHELP
hclose( fp )
FILE *fp;
{
}
#else
hclose( fp )
FILE *fp;
{
	fclose( fp );
}
#endif

hfgets( buf, size, desc )
char *buf;
int size;
FILE *desc;
{

#ifdef HUFFHELP
	return help_gets( buf, size, desc );
#else
	return (int) fgets( buf, size, desc );
#endif
}

RemoveNL( str )
char *str;
{
	register int slen;

	slen = strlen(str) - 1;

	if( slen >= 0 && str[slen] == '\n' )
		str[slen] = 0;
}


#ifdef HUFFHELP

#include "huff.h"

int strcount, tokcount;	/* Number of strings, tokens in the tree */
int num_nodes;		/* Number nodes in the tree */
FILE *huff_file = (FILE *)NULL;
unsigned int help_buf, help_mask;   /* 8 bit buffers for huff bits */
int count_left;

int help_eof;		/* Boolean that is true if eof reached */


int Multiplier;		/* Multiplier used in hash function */
int FileCount;		/* Number of entries in the file table */

int help_pos;		/* Current position in tree */

#ifdef HYBRID
typedef int far *htree;
typedef file_entry far *feptr;
typedef char far *stringptr;
#else
typedef int *htree;
typedef file_entry *feptr;
typedef char *stringptr;
#define choosefree mfree
#define bfread fread
#define choosealloc checkalloc
#endif

feptr FileTable;	/* List of hash values and huffed file lengths */
htree HuffTree;		/* Pointer to array which represents Huffman tree */
stringptr text;		/* pointer to token strings */
int HuffVers;		/* Version number of the help files */
long HuffOffset;	/* Offset from beginning of file to huffman codes */

close_help()
{
#ifdef HUFFHELP
	if( huff_file ) {
		fclose(huff_file);
		huff_file = (FILE *)NULL;
		choosefree( text, strcount );
		choosefree( FileTable, FileCount * sizeof(file_entry) );
		choosefree( HuffTree, num_nodes * 2 * sizeof(int) );
		}
#endif
}

FILE *
help_open( fname )
char *fname;
{
	int i, hval;
	long seek_adr;	/* offset to huffman codes themselves */

#ifdef TESTHELP
	printf( "help_open: opening %s\n", fname );
#endif

	/* init_help will not do anything if already in action */

	/*
	 * If we could not open the Huff Help file at init, say that we could
 	 * not open this file
 	 */

	if( init_help() || !huff_file ) return (FILE *)NULL;

	/* Search for the filename in the index */

	/* set seek start in middle of file */
	seek_adr = HuffOffset;

	hval = HashIndexName( fname );

	for( i=0; i<FileCount; i++ ) {

		/* Sum up lengths to get seek address */
		seek_adr += FileTable[i].h_offset;
		if( hval == FileTable[i].hash ) {
			/* Seek to the appropriate spot and init dehuffing */
			fseek( huff_file, seek_adr, BOF );
			help_pos = 0;
			help_mask = 0;
			count_left = 0;
			help_eof = FALSE;
			return (FILE *)1;
		}
	}
	/* Hash value not found */
	return (FILE *)NULL;
}

int
init_help()
{
	int status;
	char huffname[100];
#ifdef TESTHELP
#define help_prefix	"helpfile"
#else
	extern char *help_prefix;
#endif
	
	/* if already open, we're laughing */
	if( huff_file )
		return FALSE;

	sprintf( huffname, "%s.huf", help_prefix );

	/* Read in the index  (first part of the huffman file */
	status = ReadInTree( huffname );
	if( status ) {
		huff_file = (FILE *)NULL;
		return TRUE;
	}

	/* Now open the file (again), in preparation for the seeks/decoding */
#ifdef msdos
	huff_file = qfopen( huffname,"rb");
#else
	huff_file = qfopen( huffname,"r");
#endif
	if (huff_file == (FILE *)0)
		return TRUE;

	return FALSE;	/* everything went alright */
	
}

int 
help_gets( linebuf, size, fp )
char *linebuf;
int size;
FILE *fp;
{
	char lastchar = 1;
	stringptr lineptr;
	int  charcount = 0;
	register int fast_pos;
	register int token;
	char c;

	if( help_eof == TRUE ) { 
		return 0;
	}

	fast_pos = help_pos;

	while( (lastchar != '\n') && (charcount < size) ) {

		if( help_mask == 0 ) {
			help_buf = hf_getword( huff_file );
			help_mask = 0x8000;
		}

		if( help_buf & help_mask )
			fast_pos = HuffTree[ (fast_pos<<1) + 1 ];
		else
			fast_pos = HuffTree[ fast_pos<<1 ];

		help_mask >>= 1;

		if( HuffTree[ fast_pos<<1 ] == 0 ) {
			token = HuffTree[ (fast_pos<<1)+1 ];
			if( token == 0 ) { 
				help_eof = TRUE;
				return charcount;
			}

			if( token >= 128 ) {
				lineptr = &text[token - 128];
				while( *lineptr ) {
					lastchar = *linebuf++ = *lineptr++;
					charcount++;
				}
			}
			else {
				lastchar = *linebuf++ = token;
				charcount++;
				}
			fast_pos = 0;
		}
	}
  	*linebuf = 0;
	help_pos = fast_pos;
  	return charcount;
}

int
hf_getword( fp )
FILE *fp;
{
	int buf;

	buf = fgetc( fp ) << 8;
	buf |= fgetc( fp );
	return buf;
}

ReadInTree( fname )
char *fname;
{

	FILE *ht;
	int i;
	char *str;
	char line[80];
	extern long ftell();

#ifdef msdos
	ht = qfopen( fname, "rb" );
#else
	ht = qfopen( fname, "r" );
#endif

	if( ht==NULL ) {
		return( 1 );
	}

	/* Get the version number, if wrong, return an error */
	HuffVers = hf_getword( ht );
	if( HuffVers != HELP_VERSION ) {
#ifdef TESTHELP
		printf( verserr,
			HuffVers, HELP_VERSION );
#endif
		fclose( ht );
		return( TRUE );
	}

	/* Get number of bytes in strings */
	strcount = hf_getword( ht );	

	text = ( stringptr ) choosealloc( strcount );

	/* Read in the list of strings */
	bfread( text, sizeof( char ), strcount, ht );

	/* Now that we have read in the words, read in the table */
	num_nodes = hf_getword( ht );

	HuffTree = (htree) choosealloc( num_nodes * 2 * sizeof( int ) );

	bfread( HuffTree, 2 * sizeof( int ), num_nodes, ht );

	FileCount = hf_getword( ht );
	Multiplier = hf_getword( ht );

	FileTable = (feptr) choosealloc( sizeof( file_entry )
					* FileCount );

	bfread( FileTable, sizeof( file_entry ), FileCount, ht );

	/* If the version number is not the same as at the beginning of
         * the file, give an error 
         */

	if( HuffVers != hf_getword( ht ) ) {
#ifdef TESTHELP
		printf( "The two version numbers did not match\n" );
#endif
		fclose( ht );
		return TRUE;
	}

	HuffOffset = ftell( ht );  /* huffman codes start now... */

	fclose( ht );
	return FALSE;	/* everything ok */
}


#ifdef HYBRID
bfread( loc, itemsize, count, desc )
char far *loc;
unsigned int itemsize;
unsigned int count;
FILE *desc;
{
	register unsigned int counter;
	register unsigned total;

	total = itemsize * count;

	for( counter = 0; counter < total; counter++ )
		*loc++ = fgetc( desc );
	return count;
}
#endif

int
HashIndexName( fname )
char *fname;
{
	register int hashval = 0;

	while( *fname ) {
		hashval *= Multiplier;
		hashval += *fname++;
	}
	return hashval;
}

fgetbinstr( line, fp )
char *line;
FILE *fp;
{
char c;

	while( (c=fgetc(fp)) != 0 ) {
		*line++ = c;
	}
	*line = 0;
}

#endif HUFFHELP

