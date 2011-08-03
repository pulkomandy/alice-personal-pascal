
#include "alice.h"
#include "interp.h"
#include "break.h"

#include <osbind.h>

int dev_map[] = { 0, 3, 2, 1 };

auxputs( fp, str, len )
FILE *fp;
char *str;
int len;
{
	int dev;
	int bios_dev;
	char c;
	long count;

	dev = (int)fp;

	/* If we do get a real file pointer, then write the chars to the
	 * file.  Just a precaution.
	 * This should also be modified to handle error conditions, and
	 * waiting for device ready before sending the character.
	 */
	if( (long)fp > 10L ) {
		while( len-- ) {
			c = *str++;
			if( (fputc( c, fp ) == EOF) && (c != EOF) ) {
				tp_error( 0xf0, 0 );
				run_error(ER(276, writErr));
				/* Handle the error */
			}
		}
	}

	bios_dev = dev_map[dev];
	while( len-- ) {
		c = *str++;
		count = 0L;
		/* While the device is busy, check the break key */
		while( Bcostat( bios_dev ) == 0 ) {
			count++;

			if( count == 1000L )
				PgTitle( " Waiting - Device Output " );

			if( got_break() ) {
				breakkey();
				return;
			}
		}
		/* Device is ready, output the char */
		Bconout( bios_dev, c );
	}
}

int
auxget( fp )
FILE *fp;
{
	/* Return a char from a 'special' device */
	int dev;
	int bios_dev;
	long count;
	int c;

	if( (long)fp > 10L )
		return fgetc( fp );

	dev = (int)fp;

	bios_dev = dev_map[dev];

	/* Get a single char from the device requested... */

	count = 0L;
	while( Bconstat( bios_dev ) == 0 ) {
		count++;

		if( count == 1000 )
			PgTitle( " Waiting - Device Input " );
	
		if( got_break() ) {
			breakkey();
			return ' ';
		}
	}
	return Bconin( bios_dev );
}

long
dev_open( fname )
char *fname;
{
	if( strcmp( fname, "midi:" ) == 0 )
		return (FILE *)T_DEV_MIDI;
		
	if( strcmp( fname, "aux:" ) == 0 )
		return (FILE *)T_DEV_AUX;
		
	return 0L;
}

int
dev_wait( fp )
long fp;
{
	int dev;

	if( fp > 10L )
		return FALSE;

	dev = (int)fp;
	dev = dev_map[ dev ];

	/* If the status is 0, there is NO char waiting */
	return Bconstat( dev ) != 0;
}
