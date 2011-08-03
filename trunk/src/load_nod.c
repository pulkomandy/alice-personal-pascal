
#include "alice.h"

#ifdef DB_LOADNODES
#  define PRINTT
#endif
#include "printt.h"
#include "extramem.h"
#ifdef PARSER
#include "input.h"
#endif

extern long ftell();

#define NODE_NAMES	0
#define NODE_INFO	1
#define ACTION_TABLE	2
#define CLASS_TABLE	3
#define TABLE_LENGTH	4
#define PRECEDENCE	5
#define NUM_NODES	6
#define MDATA_LENGTH	7
#define POINTER_SIZE	8
#define	NODE_FLAGS	9
#define KID_COUNT	10
#define	TPL_VERSION	11			/* what version this is */
#define	TPL_FIRSTLD	12			/* earliest version possible */
#define	TPL_LASTLD	13			/* latest version possible */
#define	NODE_COUNT	14

#define HEADER_SIZE	20

#ifdef DUMPNODES
#define	error(x)	printf( "%s\n", x )
#define checkalloc(x)	malloc(x)
extern char *malloc();
#define qfopen(x,y)	fopen(x,y)
#endif

#ifdef PARSER
#undef error
#define error(x)	fatal(x)
#endif

#define SMALL_POINTERS	1
#define LARGE_POINTERS	2

#if defined(msdos) || defined(ATARI520ST)
#define READ_IT	"rb"
#else
#define READ_IT "r"
#endif

/* If we are not hybrid, then just do a checkalloc */
#ifndef HYBRID
# ifndef DUMPNODES
#  define choosealloc	checkalloc
#  define bfread		fread
#  define ex_free		mfree
# else
#  define choosealloc	malloc
#  define bfread	fread
#  define mfree		free
# endif
#endif

/* Pointers only exist in the second data area */
#if LARGE || HYBRID
#define NEEDED_SIZE	LARGE_POINTERS
#else
#define NEEDED_SIZE	SMALL_POINTERS
#endif

/* Pointers for stuff in the main segment */

int	NodeCount;
unsigned int *Node_Names;
char *Node_Strings;
nf_type *lt_node_flags;
bits8 *lt_kid_count;

/* Pointers into the extra segment */
struct node_info far * far *node_table;
ClassNum far * far *K_Classes;
ActionNum far * far *N_Actions;
bits8 far *expr_prec;

#ifdef MarkWilliams
extern FILE *qfopen();
#else
extern FILE *qfopen(char *, char *);
#endif

#ifndef OLMSG
static char operr[] = "Cannot open template file";
static char rderr[] = "Error while reading template file";
static char wgerr[] = "Incompatible template file";
#endif

int
LoadNodes(fname)
char *fname;		/* template file */
{
	FILE *fp;

	unsigned r;
	char far *data;
	char far * far *adr;
	char far * far *newadr;

	int i, testnode, tlen;
	char line[10];
	char *ldata;
	int slen;
	int	numnodes;
	int	TablePointers[HEADER_SIZE];

	printt2( "About to open %s (%s)\n", fname, READ_IT );

	fp = qfopen( fname, READ_IT );

	if( fp == (FILE *)NULL ) {
		error( ER(19, operr) );
	}
	printt0( "Opened it successfully\n" );

	/* Read the header */
	if( fread( TablePointers, sizeof( int ), HEADER_SIZE, fp ) 
			!= HEADER_SIZE ) {
		fclose(fp);
		error( ER(22, rderr) );
	}

#ifdef DEBUG
	for( i=0; i <HEADER_SIZE; i++ ) {
		printt2( "Header entry %d: %d\n", i, TablePointers[i] );
	}
#endif

	printt2( "Pointer size = %d, version=%d\n", TablePointers[POINTER_SIZE],
		TablePointers[TPL_VERSION]);

	if( TablePointers[ POINTER_SIZE ] != NEEDED_SIZE
	  || ALICE_VERSION < TablePointers[TPL_FIRSTLD]  ||
	    ALICE_VERSION > TablePointers[TPL_LASTLD]
		) {
		error( ER(132, wgerr) );
	}

	NodeCount = TablePointers[NODE_COUNT];
	printt1("NodeCount=%d\n", NodeCount);

/* #ifdef SAI */

	/* If we are the Stand-Alone Interpreter, we only need the
	 * Kid Count, Node Count and the Node Flags, so just seek
	 * to the appropriate sections and fread them
	 */


/* #else */

	slen = TablePointers[MDATA_LENGTH];
	printt1( "Strings and node table = %d bytes\n", slen );

	/* Allocate the memory in the main segment */
	ldata = (char *) checkalloc( slen );

	printt1( "Memory allocated at %lx\n", (long)ldata );

	Node_Strings = ldata;
	if( ldata == (char *)NULL ) {
		error( ER(22,rderr) );
	}
	/* Read in the data */
	fread( ldata, sizeof(char), slen, fp );

	printt0( "have read in the data\n" );
	printt1( "fp now at %ld\n", ftell(fp) );

	Node_Names = (unsigned int *) 
			((long)ldata + (long)TablePointers[NODE_NAMES]);

	printt1( "Node_Names array at %lx\n", (long)Node_Names );

	printt0( "Extra segment data:\n" );

	tlen = TablePointers[TABLE_LENGTH];
	printt1( "Length of data = %d bytes\n", tlen );

	numnodes = TablePointers[NUM_NODES];

	printt1( "There are %d nodes\n", numnodes );

	data = (char far *) choosealloc( tlen );
	printt1( "Memory allocated at %lx\n", (long)data );

	if( data == (char far *)NULL ) {
		fclose(fp);
		error( ER(22, rderr) );
		return FALSE;
	}

	printt0( "About to read in data\n" );
	if( bfread( data, sizeof(char), tlen, fp ) != tlen ) {
		fclose(fp);
		/* ex_free(data); */
		error( ER(22, rderr) );
	}
	printt0( "Read in data successfully\n" );
	printt1( "fp now at %ld\n", ftell(fp) );

	printt0( "Now to do the relocation\n" );

	while( fread( &r, sizeof( unsigned ), 1, fp ) == 1 ) {
		printt2( "Relocating offset %x = %d dec.\n", r, r );
		/* Relocate the address */
		adr = (char far * far *) ((long)data + (long)r);
		printt1( "Address to relocate: %lx\n", (long)adr );
		printt1( "Old value = %lx\n", (long) *adr );
		newadr = (char far *)((long)*adr + (long)data);
		printt1( "New value = %lx\n", (long)newadr );
		*adr = newadr;
	}

	fclose( fp );
	printt0( "Finished relocation\n" );

	/* Located in the main data segment */
	lt_kid_count = (bits8 *) ((long)ldata + (long)TablePointers[KID_COUNT]);
	lt_node_flags = (nf_type *) ((long)ldata + 
			(long)TablePointers[NODE_FLAGS]);

	printt2( "kid_count at %lx, node_flags at %lx\n", 
		(long)lt_kid_count, (long)lt_node_flags );

	/* located in the extra segment */
	K_Classes =  (ClassNum far * far *) ((long)data + 
			(long)TablePointers[CLASS_TABLE]);
	expr_prec =  (bits8 far *) ((long)data + 
				(long)TablePointers[PRECEDENCE]);
	N_Actions =  (ActionNum far *) ((long)data + 
				(long)TablePointers[ACTION_TABLE]);
	node_table = (struct node_info far * far *) ((long)data + 
				(long)TablePointers[NODE_INFO]);

	printt2( "K_Classes at %lx, expr_prec at %lx\n",
		(long)K_Classes, (long)expr_prec );
	printt2( "N_Actions at %lx, node_table at %lx\n",
		(long)N_Actions, (long)node_table );

	printt0( "Everything went fine\n" );
	return TRUE;

/* #endif */
}
