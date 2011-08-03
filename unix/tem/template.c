
#include "template.h"

#ifndef BASIC
# ifdef QNX
#  include "tune.h"			/* QNX: BASIC, Pascal */
# else
#  include "/u/alice/h/tune.h"		/* XENIX: Pascal */
# endif
#else
#  include "/u/alice/bh/tune.h" 	/* XENIX: BASIC */
#endif

typedef short int16;			/* 16 bit integer */

extern int16 GetNum();
extern int16 KeyWord();
extern int16 TableLookup();
extern int16 GetToken();

static table FlagTable[] = {
	"none", 0,
	"line", F_LINE,
	"indent", F_INDENT,
	"symbol", F_SYMBOL,
	"multi", F_PMULTI,
	"nostop", F_NOSTOP,
	"type",   F_TYPE,
	"declare", F_DECLARE,
	"scope",   F_SCOPE,
	"e1kid",  F_E1KID,
	"e2kids", F_E2KIDS,
	(char *)-1, 0 };

#define TRUE	-1
#define FALSE 	0

int16	ttype;

int16	LargePointers = FALSE;
int16	Verbose = FALSE;
int16	SwapBytes = FALSE;
int16	Debugging = FALSE;

#define NODE_NAMES	0
#define NODE_INFO	1
#define ACTION_TABLE	2
#define CLASS_TABLE	3
#define TABLE_LENGTH	4
#define PRECEDENCE	5
#define NUM_NODES	6
#define MDATA_LENGTH	7
#define POINTER_SIZE    8
#define NODE_FLAGS	9
#define KID_COUNT	10
#define	TPL_VERSION	11
#define	TPL_FIRSTLD	12
#define	TPL_LASTLD	13
#define	NODE_COUNT	14

#define HEADER_SIZE	20



short	TablePointers[HEADER_SIZE];

#define SMALL_POINTERS	1
#define LARGE_POINTERS	2

extern char *GetString();

extern table TokenTable[];
extern table ActionTable[];
extern table ClassTable[];
extern table NodeTable[];

static table InfixTable[] = {
	"true", 1,
	"false", 0,
	(char *)-1, 0 };

#define NUMBER 1
#define STRING 2
#define IDENT  3

#define NODE		0
#define NAME		1
#define TEMPLATE 	2
#define OPRPREC 	3
#define KIDMATRIX 	4
#define KIDCLASSES 	5
#define NODECOUNT 	6
#define FLAGS 		7
#define INFIX 		8
#define ACTIONS 	9
#define CLASS 		10
#define DOEND 		11

int16 Reuse = FALSE;
int16 line_no = 1;

static table KeyWords[] = {
		"node", NODE,
		"name", NAME,
		"kidmatrix", KIDMATRIX,
		"kidclasses", KIDCLASSES,
		"template", TEMPLATE,
		"precedence", OPRPREC,
		"nodecount", NODECOUNT,
		"flags", FLAGS,
		"infix", INFIX,
		"actions", ACTIONS,
		"quit", DOEND,
		"class", CLASS,
		(char *)-1, 0 };

int16 CurNode = 0;
FILE *fp;

#define MAXNODES 128

/* Note well these defaults! */
int16 firstld = ALICE_VERSION;
int16 lastld = ALICE_VERSION;
int16 version = ALICE_VERSION;

char	tokentext[100];

int16	MNodeDefined = 0;

int16	NodesDefined[ MAXNODES ];
char	*NodeNames[ MAXNODES ];
int16	KidStuff[ MAXNODES * 4 ];
int16     KidPointers[ MAXNODES ];
int16	TmpPointers[ MAXNODES ];
char    *Templates[ MAXNODES * 4 ];
int16	OprPrec[ MAXNODES ];
unsigned NodeFlags[ MAXNODES ];
int16	Infix[ MAXNODES ];
int16	Node1Count[ MAXNODES ];
int16	Node2Count[ MAXNODES ];

int16	Actions[ 10 * MAXNODES ];
int16	ActionPointers[ MAXNODES ];
int16	ClassPointers[ MAXNODES ];

int16	Classes[ MAXNODES * 5 ];

int16	KidPtr = 0;
int16	TmpPtr = 0;
int16	ActionPtr = 0;
int16	ClassPtr = 0;

int16	NodeInfo[ MAXNODES ];

int16	outc = FALSE;

main(argc, argv)
int16 argc;
char **argv;
{

	char line[40];
	int16 k;

	while( (argc > 1) ) {
		if( argv[1][0] == '-' ) {
		 switch( argv[1][1] ) {
			case 'l':
				printf( "Large Model Pointers\n" );
				LargePointers = TRUE;
				break;
			case 'v':
				printf( "Verbose\n");
				Verbose = TRUE;
				break;
			case 'c':
				printf( "Outputting a 'C' file\n" );
				outc = TRUE;
				break;
			case 's':
				printf( "Swapping bytes\n" );
				SwapBytes = TRUE;
				break;
			case 'd':
				printf( "Debugging\n" );
				Debugging = TRUE;
				break;
			default:
				printf( "Unknown option '%c'\n",
					argv[1][1] );
				exit(1);
			}
		 }
		 else if( argv[1][1] == '=' ) {
			switch( argv[1][0] ) {
				case 'F':
					firstld = atoi(argv[1]+2);
					break;
				case 'L':
					lastld = atoi( argv[1]+2 );
					break;
				case 'V':
					version = atoi( argv[1]+2 );
					break;
				default:
					printf( "Unknown option '%c='\n",
					argv[1][0] );
					exit(1);
				}
		     }
		  else
			break;
		argc--;
		argv++;
	}

	fp = fopen( argv[1], "r" );

	if( fp == (FILE *)NULL ) {
		printf( "Cannot open template source file %s\n", argv[1]);
		exit(1);
	}

	KidPtr = 0;

	for(;;) {
		k = KeyWord();	/* Get the next keyword */
		switch(k) {
			case NODE:
				(void)GetToken();
				k = TableLookup( NodeTable, tokentext );
				if( k == -1 ) {
					printf( "Unknown node %s\n",
						tokentext );
					terror();
				}

				CurNode = k;
				if( CurNode > MNodeDefined )
					MNodeDefined = CurNode;

				if( NodesDefined[CurNode] ) {
					printf( "Node %d already defined\n",
						CurNode );
					terror();
				}
				NodesDefined[CurNode] |= TN_DEFINED;
				break;
			case NAME:
				NodeNames[CurNode] = GetString();
				NodesDefined[CurNode] |= TN_NAME;
				break;
			case TEMPLATE:
				TmpPointers[CurNode] = TmpPtr;
				GetTemplate();
				NodesDefined[CurNode] |= TN_TEMPLATE;
				break;
			case OPRPREC:
				OprPrec[CurNode] = GetNum();
				NodesDefined[CurNode] |= TN_PRECEDENCE;
				break;
			case FLAGS:
				GetFlags();
				NodesDefined[CurNode] |= TN_FLAGS;
				break;
			case INFIX:
				GetInfix();
				NodesDefined[CurNode] |= TN_INFIX;
				break;
			case NODECOUNT:
				GetNodeCount();
				NodesDefined[CurNode] |= TN_NODECOUNT;
				KidPointers[CurNode] = KidPtr;
				if( GetKidMatrix() )
				    NodesDefined[CurNode] |= TN_KIDMATRIX;
				break;
			case ACTIONS:
				GetActions();
				NodesDefined[CurNode] |= TN_ACTIONS;
				break;
			case DOEND:
				CheckNodes();
				if( outc ) 
					WriteCFile( argv[2] );
				else
					WriteOutTable( argv[2] );
				exit(0);
			case CLASS:
				GetClass();
				NodesDefined[CurNode] |= TN_CLASS;
				break;
		}
	}
	exit(0);
}

GetClass()
{
	int16 c, k;
	int16 n=0;

	ClassPointers[CurNode] = ClassPtr;
	
	(void)GetToken();

	c = 0;

	while( (k = TableLookup( ClassTable, tokentext )) != -1 ) {

		if( k >= C_LIST ) {
			c += k;
		} else {
			c += k;
			Classes[ClassPtr++] = c;
			n++;
			c = 0;
		}
		(void)GetToken();
	}
	PushBack();
	Classes[ClassPtr++] = 0;
	if( NodesDefined[CurNode] & TN_NODECOUNT ) {
		if( n != Node1Count[CurNode] ) {
			printf( "Hidden kids in node %s\n",
				NodeNames[CurNode] );
		}
	}

}


CheckNodes()
{
	int16 i;

	for(i=0; i<=MNodeDefined; i++ ) {

		if( NodesDefined[i] & TN_DEFINED ) {

			if( !(NodesDefined[i] & TN_NAME ) ) {
				printf( "Node %d has no name\n", i );
			}
			if( !(NodesDefined[i] & TN_TEMPLATE ) ) {
				printf( "Node %d has no templates\n", i );
			}
			if( !(NodesDefined[i] & TN_FLAGS ) ) {
				printf( "Node %d has no flags\n", i );
			}
			if( !(NodesDefined[i] & TN_NODECOUNT ) ) {
				printf( "Node %d has no nodecount\n" );
			}

		} else {
			printf( "Node %d is not defined\n", i );
		}
	}
}

FILE *output;
FILE *rfile;
int16 fpos = 0;

WriteCFile( f )
char *f;
{

	int16 i;
	FILE *fp;

	fp = fopen( f, "w" );

	fprintf( fp, "#include <alice.h>\n" );
	fprintf( fp, "\n" );

	fprintf( fp, "int16 NodeCount = %d;\n\n", MNodeDefined+1 );

	fprintf( fp, "bits8 lt_kid_count[] = {\n" );

	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_NODECOUNT ) {
			fprintf( fp, "%d, ", Node1Count[i] );
		} else {
			fprintf( fp, "0, " );
		}
		if( i % 10 == 9 ) fprintf( fp, "\n" );
	}
	fprintf( fp, "0 };\n\n" );

	/*
	 * Now write out the full count
	 */

	fprintf( fp, "bits8 lt_full_kids[] = {\n" );

	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_NODECOUNT ) {
			fprintf( fp, "%d, ", Node2Count[i] );
		} else {
			fprintf( fp, "0, " );
		}
		if( i % 10 == 9 ) fprintf( fp, "\n" );
	}
	fprintf( fp, "0 };\n\n" );

	/* Write out the node flags */

	fprintf( fp, "nf_type lt_node_flags[] = {\n" );
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_FLAGS ) {
			fprintf( fp, "%d, ", NodeFlags[i] );
		} else {
			fprintf( fp, "0, " );
		}
		if( i % 10 == 9 ) fprintf( fp, "\n" );
	}
	fprintf( fp, "0 };\n\n" );

	/* Now to write out the classes */
	fprintf( fp, "int16 lt_class_val[] = {\n" );

	for( i=0; i<ClassPtr; i++ ) {
		fprintf( fp, "%d, ", Classes[i] );
		if( i % 10 == 9 ) fprintf( fp, "\n" );
	}
	fprintf( fp, "0 };\n\n" );

	fprintf( fp, "int16 lt_K_Class[] = {\n" );
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_CLASS ) {
			fprintf( fp, "%d, ", ClassPointers[i] );
		} else {
			fprintf( fp, "0, " );
		}
		if( i % 10 == 9 ) fprintf( fp, "\n" );
	}
	fprintf( fp, "0 };\n\n" );

	fclose(fp);
}

WriteOutTable( f ) 
char *f;
{
	int16 i,j;
	int16 p;
	int16 nulact;
	unsigned u;
	int16 tlen;
	char reloc[50];

	TablePointers[TPL_VERSION] = version;
	TablePointers[TPL_FIRSTLD] = firstld;
	TablePointers[TPL_LASTLD] = lastld;	/* for now */

	TablePointers[NODE_COUNT] = MNodeDefined + 1;

	printf( "Template version %d\n", TablePointers[TPL_VERSION]);
	printf( "First Alice version %d\n", TablePointers[TPL_FIRSTLD]);
	printf( "Last Alice version %d\n", TablePointers[TPL_LASTLD]);
	printf( "NodeCount %d\n", TablePointers[NODE_COUNT]);

	printf( "Number of actions: %d\n", ActionPtr );
	printf( "Number of Classes: %d\n", ClassPtr );
	printf( "Number of template strings: %d\n", TmpPtr );
	printf( "Number of kids: %d\n", KidPtr );

	printf( "Highest node defined: %d\n", MNodeDefined );

	output = fopen( f, "w" );

	if( output == (FILE *)NULL ) {
		printf( "Cannot open output file %s\n", f );
		exit(1);
	}


	for( i=0; i<HEADER_SIZE; i++ ) {
		PutUnsigned( 0 );
	}

	fpos = 0;

	sprintf( reloc, "%s.rel", f );

	rfile = fopen( reloc, "w" );

	if( rfile == (FILE *)NULL ) {
		printf( "Cannot open relocation information file\n" );
		exit(1);
	}

	/* Write out the names of the nodes */
	PutString( "no name" );
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_NAME ) {
			/* get where we are in the file */
			p = FilePos();
			PutString( NodeNames[i] );
			free( NodeNames[i] );
			NodeNames[i] = (char *) p;
		}
 	}
	Even();

	/* Write out the offsets into the string table */
	TablePointers[ NODE_NAMES ] = FilePos();
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_NAME ) {
			PutUnsigned( NodeNames[i] );
		} else {
			PutUnsigned( 0 );
		}
	}

	TablePointers[KID_COUNT] = FilePos();
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_NODECOUNT ) {
			PutByte( Node1Count[i] );
		} else {
			PutByte( 0 );
		}
	}

	/* Write out the node flags */
	TablePointers[NODE_FLAGS] = FilePos();
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_FLAGS ) {
			PutUnsigned( NodeFlags[i] );
		} else {
			PutUnsigned( 0 );
		}
	}
	printf( "MDataLength = %d\n", fpos );

	TablePointers[ MDATA_LENGTH ] = fpos;

	fpos = 0;

	for( i=0; i < TmpPtr; i++ ) {
			p = FilePos();
			/* If there is text there, write it out */
			if( Templates[i] == (char *)-1 ) {
				/* No template here, do nothing */
			} else {
				PutString( Templates[i] );
				free( Templates[i] );
				Templates[i] = (char *)p;
			}
	}
	Even();

	for( i=0; i<= MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_TEMPLATE ) {
			p = FilePos();
			j = TmpPointers[i];
			while( Templates[j] != (char *)-1 ) {
				PutPointer( Templates[j], TRUE );
				j++;
			}
			PutPointer( 0, FALSE );
			TmpPointers[i] = p;
		}
	}

	Even();
	p = FilePos();
	for( i=0; i<KidPtr; i++ ) {
		PutByte( KidStuff[i] );
	}
	Even();

	
	/* Change class pointers to be relative to file */
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_KIDMATRIX ) {
			KidPointers[i] += p;
		}
	}

	/* Now write out the node_info nodes */


	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] ) {
			NodeInfo[i] = FilePos();

			if( NodesDefined[i] & TN_TEMPLATE ) {
			   PutPointer( TmpPointers[i], TRUE );
			} else {
			   PutPointer( 0, FALSE );
			}
			if( NodesDefined[i] & TN_KIDMATRIX ) {
			   PutPointer( KidPointers[i], TRUE );
			} else {
			   PutPointer( 0, FALSE );
			}
		
			/* Infix code */
			PutByte( Infix[i] );

			/* Full Count */
			PutByte( Node2Count[i] );

		}
	}

	TablePointers[ NODE_INFO ] = FilePos();
	/* Now the pointers into the node_info's */
	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] ) {
			PutPointer( NodeInfo[i], TRUE );
		} else {
			PutPointer( 0, FALSE );
		}
	}

	/* Now let's write out all the actions */

	nulact = FilePos();
	PutByte( 0 );
	Even();

	for( i=0; i < ActionPtr; i++ ) {
		PutByte( Actions[i] );
	}
	Even();

	TablePointers[ ACTION_TABLE ] = FilePos();

	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_ACTIONS ) {
			PutPointer( ActionPointers[i]+nulact+2, TRUE );
		} else {
			PutPointer( nulact, TRUE );
		}
	}

	/* Now to write out the classes */
	p = FilePos();
	for( i=0; i<ClassPtr; i++ ) {
		PutByte( Classes[i] );
	}
	Even();

	TablePointers[ CLASS_TABLE ] = FilePos();

	for( i=0; i<=MNodeDefined; i++ ) {
		if( NodesDefined[i] & TN_CLASS ) {
			PutPointer( ClassPointers[i] + p, TRUE );
		} else {
			PutPointer( 0, FALSE );
		}
	}

	/* Now write out the operator precedence */
	TablePointers[ PRECEDENCE ] = FilePos();

	for( i=FIRST_EXPRESSION; i<= LAST_EXPRESSION; i++ ) {
		if( NodesDefined[i] & TN_PRECEDENCE ) {
			PutByte( OprPrec[i] );
		} else {
			PutByte(0);
		}
	}

	tlen = FilePos();
	TablePointers[ TABLE_LENGTH ] = tlen;
	TablePointers[ NUM_NODES ] = MNodeDefined;

	if( LargePointers ) {
		TablePointers[POINTER_SIZE] = LARGE_POINTERS;
	} else {
		TablePointers[POINTER_SIZE] = SMALL_POINTERS;
	}
	printf( "Relocation information at %d\n", FilePos() );
	printf( "Node name table starts at %d\n", TablePointers[ NODE_NAMES ]);
	printf( "Node info table at %d\n", TablePointers[ NODE_INFO ] );
	printf( "Action table at %d\n", TablePointers[ ACTION_TABLE ] );
	printf( "Class table at %d\n", TablePointers[ CLASS_TABLE ] );
	printf( "Operator Precedence at %d\n", TablePointers[PRECEDENCE]);
	printf( "Kid Counts at %d\n", TablePointers[KID_COUNT] );
	fclose( rfile );

	fseek( output, 0L, 0 ); /* Seek to the beginning of the file */
	fpos = 0;

	/* Write out the raw offsets, loader will convert addresses */
	for( i=0; i<HEADER_SIZE; i++ ) {
		printf( "Header entry %d: %u\n", i, TablePointers[i] );
		PutUnsigned( TablePointers[i] );
	}

	fseek( output, 0L, 2);  /* Seek back to the end */
	
	rfile = fopen( reloc, "r" );
	while( fread( &u, sizeof(unsigned short), 1, rfile ) == 1 ) {
		PutUnsigned( u );
	}
	fclose( rfile );
	fclose( output );
}

FilePos()
{
	return fpos;
}

PutString( s )
char *s;
{
	fpos += strlen( s ) + 1;

	while( *s ) {
		fputc( *s++, output);
	}
	fputc( 0, output );
}

PutByte( b )
int16 b;
{
	fputc( b, output );
	fpos++;
}

PutUnsigned( u )
unsigned short u;
{
	if( SwapBytes ) {
		fputc( u >> 8, output );
		fputc( u & 0xff, output );
	} else {
		fwrite( &u, sizeof(u), 1, output );
	}
	fpos += sizeof(u);
}

Even()
{
	if( fpos & 1 ) {
		fputc( 0, output );
		fpos++;
	}
}

PutPointer( p, reloc )
char *p;
int16 reloc;
{

	if( Debugging ) {
		printf( "PutPointer( %lx, %d ) offset = %u\n", (long)p, reloc,
			fpos );
	}

	if( reloc ) {
		if( SwapBytes ) {
			fputc( fpos & 0xff, rfile );
			fputc( fpos >> 8, rfile );
		} else
			fwrite( &fpos, sizeof(fpos), 1, rfile );
	}

	if( LargePointers ) {
		/* Large model pointers */
		if( SwapBytes ) {
			PutUnsigned( 0 );
			PutUnsigned( (unsigned)(((long)p) & 0xffff));
		} else {
			PutUnsigned( (unsigned)p );
			PutUnsigned( 0 );
		}
	} else {
		unsigned short smallp;

		smallp = (unsigned short)p;
		/* Small model pointers */
		fwrite( &smallp, sizeof(smallp), 1, output );
		fpos += sizeof(smallp);
	}
}

table WhereTable[] = {
	"any", UT_ANY,
	"exact", UT_EXACT,
	"pcode", UT_PCODE,
	"upkid", UT_UPKID,
	(char *)-1, 0 };

GetActions()
{
	int16 k, v;

	ActionPointers[CurNode] = ActionPtr;

	for(;;) {

		(void)GetToken();
		k = TableLookup( TokenTable, tokentext );
		if( k == -1 ) {
			PushBack();
			break;
		}
		/* Store the token */
		Actions[ActionPtr++] = k;

		v = 0;
	
		/* Get where the action should occur */	
		if( GetToken() == NUMBER ) {
			PushBack();
			v = GetNum();
			goto gotit;
		}

		k = TableLookup( WhereTable, tokentext );
		/* If it was the EXACT token, set the value */
		if( k == UT_EXACT ) {
			v = UT_EXACT;
			(void)GetToken();
			k = TableLookup( WhereTable, tokentext );
		}
		if( k == UT_ANY ) {
			if( v == UT_EXACT ) {
				printf( "Exact and Any ???\n" );
				terror();
			}
			v = UT_ANY;
		} else if( k == UT_UPKID ) {
			v += k;
			v += GetNum();
		} else if( k == UT_PCODE ) {
			v += k;
			v += GetNum();
		} else {
			printf( "Unknown 'where' in GetActions\n" );
			terror();
		}

gotit:
		Actions[ActionPtr++] = v;

		/* Now get the actions */
		k = GetToken();
		if( k == NUMBER ) {
			PushBack();
			k = GetNum();
		} else	{
			k = TableLookup( ActionTable, tokentext );
	
			if( k == -1 ) {
				printf( "Unknown Action in GetActions() - %s\n",
				tokentext );
				terror();
			}
		}

		if( k == ACT_CHGOTO ) {
			k += GetNum();
		} else if( k == ACT_BLCR ) {
			k = GetToken();
			PushBack();
			if( k == NUMBER ) {
				k = GetNum() + ACT_BLCR;
			} 
			  else k = ACT_BLCR;
		}
		Actions[ActionPtr++] = k;

	}
	/* Terminate the actions */
	Actions[ActionPtr++] = 0;

}

GetNodeCount()
{
	Node1Count[CurNode] = GetNum();
	Node2Count[CurNode] = GetNum();
}

GetInfix()
{
	int16 k;

	(void)GetToken();
	k = TableLookup( InfixTable, tokentext );
	if( k == -1 ) {
		printf( "Error: expecting true value\n" );
		terror();
	}
	Infix[CurNode] = k;
}
	
GetFlags()
{
	int16 k, f;

	f = 0;
	for(;;) {
		(void)GetToken();
		k = TableLookup( FlagTable, tokentext );
		if( k == -1 ) {
			PushBack();
			break;
		}
		if( k == 0 ) {
			f = 0;
			break;
		}
		f |= k;
	}
	NodeFlags[CurNode] = f;
}

GetTemplate()
{
	char *p;

	while( GetToken() == STRING ) {
		p = (char *) malloc( strlen( tokentext ) + 1 );
		strcpy( p, tokentext );
		Templates[TmpPtr++] = p;
	}
	PushBack();
	Templates[TmpPtr++] = (char *)-1;
}

char * 
GetString()
{
	char *p;

	if( GetToken() != STRING ) {
		printf( "Error: expecting string\n" );
		terror();
	}

	p = (char *) malloc( strlen( tokentext ) + 1 );
	strcpy( p, tokentext );
	return p;
}

#ifdef Old
GetKidMatrix()
{
	int16 n;

	/* If it is the token nil, do nothing */
	(void)GetToken();
	if( strcmp( tokentext, "nil" ) == 0 ) {
	return FALSE;
	}
	PushBack();
	while( GetToken() == NUMBER ) {
		n = atoi( tokentext );
		KidStuff[ KidPtr++ ] = n;
	}
	PushBack();
	return TRUE;
}

#endif

GetKidMatrix()
{
	int16 kmarray[20];
	int16 i;
	register char *scp;		/* scan pointer in template */
	char **loopp;		/* template loop */
	char **temps;		/* pointer to array of templates */

	temps = &Templates[TmpPointers[CurNode]];

	if( temps[1] == (char *)-1 )
		return FALSE;

	for( i = 0; i < sizeof(kmarray)/sizeof(int16); i++ )
		kmarray[i] = -1;

	for( loopp = temps; *loopp != (char *)-1; loopp++ )
		for( scp = *loopp; *scp; scp++ )
			if( *scp == '!' ) {
				++scp;
				if( *scp >= '1' && *scp <= '9' ) {
					int16 theel;
					/* store only the first one */
					theel = *scp - '0';
					if( kmarray[theel] == -1 )
						kmarray[theel] = loopp - temps;
					}
				else if( *scp == 'c' )
					kmarray[0] = loopp - temps;
				}
	for( i = 0; i <= Node1Count[CurNode]; i++ ) {
		if( kmarray[i] == -1 )
			printf( "Kid matrix entry %d missing for node %s\n",
					i -1, NodeNames[CurNode] );
		KidStuff[KidPtr++] = kmarray[i];
		}
	return TRUE;
		
}

PushBack()
{
	Reuse = TRUE;
}

int16
GetNum()
{
	int16 n;

	if( GetToken() != NUMBER ) {
		printf( "Error: expecting number\n" );
		terror();
	}
	n = atoi( tokentext );
	return n;
}

int16
KeyWord()
{
	int16 k;

	(void)GetToken();
	k = TableLookup( KeyWords, tokentext );
	if( k == -1 ) {
		printf( "Error: expecting keyword\n" );
		terror();
	}
	return k;
}

int16
TableLookup( t, str )
table *t;
char *str;
{
	char st[50];
	int16 i = 0;

	while( *str ) {
		st[i++] = tolower(*str);
		str++;
	}
	st[i] = 0;
	while( t->s != (char *)-1 ) {
		if( strcmp( t->s, st ) == 0 ) {
			return t->n;
		}
		t++;
	}
	return -1;
}

int16	
GetToken()
{
	char c;
	int16 i = 0;
	int16 t;

	if( Reuse ) {
		Reuse = FALSE;
		return ttype;
	}

	c = getc(fp);

	while( (c == ' ') || (c == '\n') || (c == '\t') || (c == '#') || ( c == ',' )){
		if( c == '\n' ) line_no++;
		if( c == '#' ) {
			while( (c = getc(fp)) != '\n' );
			line_no++;
		} else {
			c = getc(fp);
		}
	}
	if( c == EOF ) {
		strcpy( tokentext, "quit" );
		return;
	}

	if( isdigit( c ) ) {
		while( isdigit(c) ) {
			tokentext[i++] = c;
			c = getc(fp);
		}
		t = NUMBER;
	} else if( isalpha( c ) ){ 
		while( isalpha(c) || c == '_' || isdigit(c) ) {
			tokentext[i++] = c;
			c = getc(fp);
		}
		t = IDENT;
	} else if( c == '\"' ) {
		while( (c = getc(fp)) != '\"' ) {
			if( c == '\n' ) {
				printf( "Newline in string\n" );
				terror();
			}
			if( c == '\\' ) {
				c = getc(fp);
				switch( c ) {
					case 'n':
#ifdef QNX
						/* XENIX newline expected */
						tokentext[i++] = '\012';
#else
						tokentext[i++] = '\n';
#endif
						break;
					case 't':
						tokentext[i++] = '\t';
						break;
					default:
						tokentext[i++] = c;
						break;
				}
			} else
				tokentext[i++] = c;
		}
		t = STRING;
	} else {
		printf( "Unknown character '%c' = %x\n", c, c );
		terror();
	}
	if( c == '\n' ) line_no++;

	tokentext[i] = 0;
	ttype = t;
	return t;
}

terror()
{
	printf( "Error: %s on line %d\n", tokentext, line_no );
	exit(1);
}
