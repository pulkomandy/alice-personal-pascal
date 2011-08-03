
#include <stdio.h>
#include <osbind.h>
#include <basepage.h>

extern FILE *fopen();

#define SCR_WIDTH	640
#define SCR_HEIGHT	400
#define X_MAG		1
#define Y_MAG		1
#define LINE_HEIGHT	8
#define BYTES_WIDE	80
#define HORIZ_OFFSET	0


int snap_number = 0;

take_snap()
{
	FILE *fp;
	long ptr;
	int amount;
	char snapname[20];

	int i;

	sprintf( snapname, "snap%d.pic", snap_number++ );

	fp = fopen( snapname, "wb" );

	/* Write the standard header */
	WriteHdr( fp );

	/* Get a pointer to screen memory */
	ptr = Physbase();


	amount = LINE_HEIGHT * BYTES_WIDE;

	for( i=0; i<(SCR_HEIGHT / LINE_HEIGHT); i++ ) {

		PutWord( fp, i * LINE_HEIGHT );
		PutWord( fp, LINE_HEIGHT );
		PutWord( fp, HORIZ_OFFSET );
		PutWord( fp, BYTES_WIDE );

		fwrite( ptr, amount, 1, fp );
		ptr += amount;
	}
	fclose( fp );
}

static
PutWord( fp, val )
FILE *fp;
unsigned int val;
{
	unsigned int low;
	unsigned int high;

	low = val & 0xff;
	high = val >> 8;

	fputc( low, fp );
	fputc( high, fp );
}

static
WriteHdr( fp )
FILE *fp;
{
	PutWord( fp, SCR_WIDTH );
	PutWord( fp, SCR_HEIGHT );
	PutWord( fp, X_MAG );
	PutWord( fp, Y_MAG );
}

main()
{

	long save_ssp;
	long *scrvec;
	extern int take_snap();
	long size;

	printf( "Installing snapshot routine...\n" );

	scrvec = (long *)(0x502);

	save_ssp = Super( 0L );

	*scrvec = take_snap;

	Super( save_ssp );

	size = BP->p_tlen + BP->p_dlen + BP->p_blen + 0x100L;
	Ptermres( size, 0 );

}
