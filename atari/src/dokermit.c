
#define KERMIT	"c:\bin\kermit.ttp"

#include <stdio.h>
#include <osbind.h>

char env[] = { 0, 0, 0, 0 };

main( argc, argv )
int argc;
char *argv[];
{
	FILE *fp;
	extern FILE *fopen();
	char line[80];
	char sysline[80];

	if( argc < 2 ) {
		printf( "usage: dokermit filename\n" );
		exit(1);
	}

	fp = fopen( argv[1], "r" );

	if( fp == (FILE *)0 ) {
		printf( "Can't open %s\n", argv[1] );
		exit(1);
	}

	while( fgets( line, 80, fp ) ) {

		if( line[ strlen(line)-1 ] == '\n' )
			line[strlen(line)-1] = 0;
		if( strcmp( line, "backup" ) == 0 ) break;

		printf( "Sending %s\n", line );
		sprintf( sysline, "kermit s %s", line );
		system( sysline );
	}
	fclose( fp );
}
