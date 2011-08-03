
#include "alice.h"
#include <ctype.h>
#include <osbind.h>

#include "huff.h"

extern FILE *help_open();
extern int  help_gets();
extern int  init_help();

extern FILE *qfopen();
extern char *checkalloc();

/*
 * Concatenate the directory and the subject, convert the string as
 * required, and then call help_open()
 */

FILE *
hopen( hdir, sub )
char *hdir;	/* directory, possibly nil */
char *sub;	/* subject */
{
	char hfname[80];
	char nsub[80];
	register char *p1;

	strcpy( nsub, sub );

#define HELP_FLEN 12

	if( !strchr(nsub, '/' ) && strlen(nsub) > HELP_FLEN ) {
		nsub[HELP_FLEN] = 0;
		}

	sprintf( hfname, "%s%s", hdir, nsub );

	/* convert spaces to underscores, uppercase to lower */
	for( p1 = hfname; *p1; p1++ ) {
		if( !isalnum( *p1 ) && (*p1 != '/') )
			*p1 = '_';
		 else if( isupper( *p1 ) )
			*p1 += 'a' - 'A';
		}

	return (FILE *) help_open( hfname );
}


hclose( fp )
FILE *fp;
{
	/* This routine does nothing, since there is nothing to do to
	 * finish decoding a file
	 */
}

/*
 * Get a line of text from the compressed help file
 */

hfgets( buf, size, desc )
char *buf;
int size;
FILE *desc;
{
	return help_gets( buf, size, desc );
}

/*
 * These are the routines that actually do the decoding of the
 * information
 */

int strcount, tokcount;	/* Number of strings, tokens in the tree */
int num_nodes;		/* Number nodes in the tree */

FILE *huff_file = (FILE *)NULL;

unsigned int help_buf, help_mask;   /* 8 bit buffers for huff bits */

int help_eof;		/* Boolean that is true if eof reached */

int Multiplier;		/* Multiplier used in hash function */
int FileCount;		/* Number of entries in the file table */

int help_pos;		/* Current position in tree */

typedef int *htree;		/* pointer into the tree */
typedef file_entry *feptr;	/* Pointer to a file entry */
typedef char *stringptr;	/* string pointer */

feptr FileTable;	/* List of hash values and huffed file lengths */
htree HuffTree;		/* Pointer to array which represents Huffman tree */
stringptr HuffText;	/* pointer to token strings */
int HuffVers;		/* Version number of the help files */
long HuffOffset;	/* Offset from beginning of file to huffman codes */

/*
 * Close the help file routines, frees up the memory associated with
 * the tree, etc
 */

close_help()
{
	if( huff_file ) {
		fclose(huff_file);
		huff_file = (FILE *)NULL;
		free( HuffText );
		free( FileTable );
		free( HuffTree );
	}
}

/*
 * Take a filename, hash it, look it up in the file table, and then
 * seek to the appropriate spot, so that we can start decoding there
 */

FILE *
help_open( fname )
char *fname;
{
	int i, hval;
	long seek_adr;	/* offset to huffman codes themselves */

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
			help_eof = FALSE;
			return (FILE *)1;
		}
	}
	/* Hash value not found */
	return (FILE *)NULL;
}

int huff_disk = 0;
extern int qf_drive;

int
init_help()
{
	int status;
	char huffname[100];

	extern char *help_prefix;
	
	/* if already open, we're laughing */
	if( huff_file ) {
		/*
		 * If the media has changed, then close
		 * the help file and start again
		 */
		if( Mediach( huff_disk ) )
			close_help();
		else
			return FALSE;
	}

	sprintf( huffname, "%s.huf", help_prefix );

	/* Read in the index  (first part of the huffman file */
	status = ReadInTree( huffname );
	if( status ) {
		huff_file = (FILE *)NULL;
		return TRUE;
	}
	huff_disk = qf_drive;
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
				lineptr = &HuffText[token - 128];
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

/*
 * Get the next word from the currently open huffman file
 */

int
hf_getword( fp )
FILE *fp;
{
	int buf;

	buf = fgetc( fp ) << 8;
	buf |= fgetc( fp );
	return buf;
}

/*
 * Read in the huffman tree and associated information
 */

ReadInTree( fname )
char *fname;
{

	FILE *ht;
	int i;
	char *str;
	char line[80];
	extern long ftell();

	ht = qfopen( fname, "rb" );

	if( ht==NULL ) {
		return( 1 );
	}

	/* Get the version number, if wrong, return an error */
	HuffVers = hf_getword( ht );
	if( HuffVers != HELP_VERSION ) {
		fclose( ht );
		return( TRUE );
	}

	/* Get number of bytes in strings */
	strcount = hf_getword( ht );	

	HuffText = ( stringptr ) checkalloc( strcount );

	/* Read in the list of strings */
	fread( HuffText, sizeof( char ), strcount, ht );

	/* Now that we have read in the words, read in the table */
	num_nodes = hf_getword( ht );

	HuffTree = (htree) checkalloc( num_nodes * 2 * sizeof( int ) );

	fread( HuffTree, 2 * sizeof( int ), num_nodes, ht );

	FileCount = hf_getword( ht );
	Multiplier = hf_getword( ht );

	FileTable = (feptr) checkalloc( sizeof( file_entry )
					* FileCount );

	fread( FileTable, sizeof( file_entry ), FileCount, ht );

	/* If the version number is not the same as at the beginning of
         * the file, give an error 
         */

	if( HuffVers != hf_getword( ht ) ) {
		fclose( ht );
		return TRUE;
	}

	HuffOffset = ftell( ht );  /* huffman codes start now... */

	huff_file = ht;

	return FALSE;	/* everything ok */
}

/*
 * Take the filename and hash it to a wonderfully unique hash value
 */

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


