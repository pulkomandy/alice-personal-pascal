
#include "alice.h"
#include "interp.h"
#include <curses.h>
#include <obdefs.h>
#include <osbind.h>
#include <gemdefs.h>
#include <ctype.h>
#include <basepage.h>

extern int vdi_handle;

/*
 * This should be a BTPROC, so that we can see if the arrays, etc are big
 * enough.
 */
extern int MouseStat;

long Al_Ctrl[10];
int Al_NumVert;
int pts_buf[ 256 ];		/* space for maximum */

/* VDI( control, intin, ptsin, intout, ptsout ) */
do_vdicall( argc, argv )
int argc;
rint *argv;
{
	int *p_control;
	int *p_intin;
	int *p_intout;
	int *p_ptsin;
	int *p_ptsout;
	int flag;

	extern int ptsout[], intout[];

	p_ptsout = ptsout;
	p_intout = intout;

	if( argc >= 5 ) p_ptsout = p_spop();
	if( argc >= 4 ) p_intout = p_spop();
	p_ptsin = p_spop();
	p_intin = p_spop();
	p_control = p_spop();

	/* Now check that all the arrays are set up properly */
	if( p_control[1] )
		do_undef( p_ptsin, U_CHECK, p_control[1] * sizeof(int) * 2 );
	if( p_control[3] )
		do_undef( p_intin, U_CHECK, p_control[3] * sizeof(int) );

	p_control[6] = vdi_handle;
 
	Al_Ctrl[0] = p_control;
	Al_Ctrl[1] = p_intin;
	Al_Ctrl[2] = p_ptsin;
	Al_Ctrl[3] = p_intout;
	Al_Ctrl[4] = p_ptsout;

	flag = MouseStat;
	MouseOn( FALSE );

	/* Can't just call VDI, since that clobbers ad_c[1] */
	AlVdi();

	if( p_control[2] && argc >= 5 )
		do_undef( p_ptsout, U_SET, sizeof(int) * p_control[2] * 2 );
	if( p_control[4] && argc >= 4 )
		do_undef( p_intout, U_SET, sizeof(int) * p_control[4] );

	do_undef( p_control, U_SET, sizeof(int) * 6 );

	MouseOn( flag );

}

/*
 * Do a VDI Call, but make it relative to the current graphics window
 */

/* WinVDI( num_vert, control, intin, ptsin, intout, ptsout ); */

int Al_Scaled;

do_winvdi( argc, argv )
int argc;
rint *argv;
{
	int *p_control;
	int *p_intin;
	int *p_intout;
	int *p_ptsin;
	int *p_ptsout;
	int i;
	int flag;
	extern int _DoVDI();

	p_ptsout = ptsout;
	p_intout = intout;

	if( argc >= 6 ) p_ptsout = p_spop();
	if( argc >= 5 ) p_intout = p_spop();

	p_ptsin = p_spop();
	p_intin = p_spop();

	p_control = p_spop();

	/* Get the number of vertices in ptsin to scale */

	Al_NumVert = int_spop();

	/* Now check that all the arrays are set up properly */
	if( p_control[1] )
		do_undef( p_ptsin, U_CHECK, p_control[1] * sizeof(int) * 2 );
	if( p_control[3] )
		do_undef( p_intin, U_CHECK, p_control[3] * sizeof(int) );

	if( p_control[1] > 128 )
		run_error( ER( 361, "maximum of 128 points allowed" ) );

	/*
	 * Copy the points into a global array, so we can restore them later
	 */
	for( i=0; i < p_control[1] * 2; i++ ) {
		pts_buf[i] = p_ptsin[i];
	}

	/*
	 * Scale the co-ordinates according to the current coordinate
	 * system
	 */
	for( i=0; i<p_control[1] * 2; i += 2 ) {
		CoMap( &p_ptsin[i], &p_ptsin[i+1] );
	}

	/* Set up the control Block */

	p_control[6] = vdi_handle;
 
	Al_Ctrl[0] = p_control;
	Al_Ctrl[1] = p_intin;
	Al_Ctrl[2] = p_ptsin;
	Al_Ctrl[3] = p_intout;
	Al_Ctrl[4] = p_ptsout;

	Al_Scaled = FALSE;
	DrawWin( _DoVDI );

	if( p_control[2] && argc >= 6 )
		do_undef( p_ptsout, U_SET, sizeof(int) * p_control[2] * 2 );
	if( p_control[4] && argc >= 5 )
		do_undef( p_intout, U_SET, sizeof(int) * p_control[4] );

	do_undef( p_control, U_SET, sizeof(int) * 6 );

	/*
	 * Copy the points into a global array, so we can restore them later
	 */
	for( i=0; i < p_control[1] * 2; i++ ) {
		p_ptsin[i] = pts_buf[i];
	}
}

int
_DoVDI( x, y )
int x, y;
{
	int i;
	int *ptsin;

	ptsin = (int *) Al_Ctrl[2];

	/* If we haven't scaled the points already, and it is time, go
	 * do it
	 */
	if( (x != 0 || y != 0) && !Al_Scaled ) {
		for( i=0; i<Al_NumVert * 2; i += 2 ) {
			ptsin[i] += x;
			ptsin[i+1] += y;
		}
		Al_Scaled = TRUE;
	}
	AlVdi();
}

/* AES( control, int_in, addr_in, int_out, addr_out ) */

long Al_AES[6];

do_aescall( argc, argv )
int argc;
rint *argv;
{
	int *p_control;
	int *p_intin;
	int *p_intout;
	int *p_addrin;
	int *p_addrout;
	extern int addr_out[], int_out[];
	extern int global[];
	extern char ctrl_cn[][3];
	int func;

	p_addrout = addr_out;
	p_intout = int_out;

	if( argc >= 5 ) p_addrout = p_spop();
	if( argc >= 4 ) p_intout = p_spop();

	p_addrin = p_spop();
	p_intin = p_spop();
	p_control = p_spop();

	/*
	 * Set up control[1-3]
	 */
	if( p_control[1] == -1 ) {
		func = p_control[0];
		func -= 10;
		p_control[1] = ctrl_cn[func][0];
		p_control[2] = ctrl_cn[func][1];
		p_control[3] = ctrl_cn[func][2];
	}

	/* Now check that all the arrays are set up properly */
	if( p_control[1] )
		do_undef( p_intin, U_CHECK, p_control[1] * sizeof(int) );
	if( p_control[3] )
		do_undef( p_addrin, U_CHECK, p_control[3] * sizeof(long) );

	Al_AES[0] = p_control;
	Al_AES[1] = global;
	Al_AES[2] = p_intin;
	Al_AES[3] = p_intout;
	Al_AES[4] = p_addrin;
	Al_AES[5] = p_addrout;

	crystal( Al_AES );

	if( p_control[2] && argc >= 4 )
		do_undef( p_intout, U_SET, sizeof(int) * p_control[2] );
	if( p_control[4] && argc >= 5 )
		do_undef( p_addrout, U_SET, sizeof(long) * p_control[4] );

}


#ifndef SAI

extern int exc_buserr(), exc_addrerr(), exc_illerr(), exc_priverr();
extern int exc_divide();

extern int ExceptNumber;

struct {
	int	exception;
	int	(* address)();
	} ExcTable[] = {

	2, &exc_buserr,
	3, &exc_addrerr,
	4, &exc_illerr,
	8, &exc_priverr,
	5, &exc_divide,
	-1, &exc_buserr
	};

long OldVectors[6];

static int IsInstalled = FALSE;

installExceptions()
{
	int i;

	i = 0;
	while( ExcTable[i].exception != -1 ) {
		OldVectors[i] = Setexc( ExcTable[i].exception, 
				ExcTable[i].address );
		i++;
	}
	IsInstalled = TRUE;
}

restoreExceptions()
{
	int i;

	if( !IsInstalled )
		return;

	i = 0;
	while( ExcTable[i].exception != -1 ) {
		Setexc( ExcTable[i].exception, OldVectors[i] );
		i++;
	}
}

#ifdef OLMSG
char *
TheExcepts( num )
int num;
{
	return LDS( 383 + num, "foo" );
}
#else OLMSG

#define TheExcepts(num)	ExceptStrs[num]

char *ExceptStrs[] = {
	"Bus Error",
	"Address Error",
	"Illegal Instruction",
	"Privilege Violation",
	"Divide By Zero"
	};

#endif OLMSG

printExceptions()
{
	FILE *fp;
	int i;
	char msg[100];
	long tbase, tlen;
	int rel = FALSE;

#ifdef ERROR_FILE
	extern long ExcRegs[];
	extern int ExcExtra[];
	extern int ExcTrace[];
#endif

	extern long ExcPC;
	extern long ExcStack;
	extern int IsRunning;

	/*
	 * Only write out an error file if it is not a release version
	 */
#ifdef ERROR_FILE
	fp = fopen( "alice.err", "w" );

	if( fp != (FILE *)0 ) {

		fprintf( fp, "\nAlice Error Report File\n\n" );
		fprintf( fp, "Exception %d occured (%s)\n\n", ExceptNumber,
				TheExcepts(ExceptNumber-1) );

		fprintf( fp, "PC =    %8lx (rel: %lx)\n",
				ExcPC, ExcPC - BP->p_tbase );
		fprintf( fp, "Stack = %8lx\n\n",
				ExcStack );

		for( i=0; i<16; i++ ) {
			fprintf( fp, "%c%d: %8lx (%ld)\n", 
				(i<8) ? 'D' : 'A', i % 8,ExcRegs[i],ExcRegs[i]);
		}
		if( ExceptNumber == 1 || ExceptNumber == 2 ) {
			fprintf( fp, "\nSupplemental information:\n" );
			for( i=0; i<4; i++ ) {
				fprintf( fp, "%2d %4x\n", i+1, ExcExtra[i] );
			}
		}
		fprintf( fp, "\n" );
		fprintf( fp, "Text Base: %8lx  (%ld)\n", 
			BP->p_tbase, BP->p_tbase );
		fprintf( fp, "Text Len : %8lx  (%ld)\n",
			BP->p_tlen, BP->p_tlen );

		fprintf( fp, "Data Base: %8lx  (%ld)\n", 
			BP->p_dbase, BP->p_dbase );
		fprintf( fp, "Data Len : %8lx  (%ld)\n",
			BP->p_dlen, BP->p_dlen );

		fprintf( fp, "BSS  Base: %8lx  (%ld)\n", 
			BP->p_bbase, BP->p_bbase );
		fprintf( fp, "BSS  Len : %8lx  (%ld)\n",
			BP->p_blen, BP->p_blen );

		fprintf( fp, "\nStack Traceback:\n" );
		for( i=0; i<100; i++ ) {
			fprintf( fp, "%4x ", ExcTrace[i] );
			if( (i % 10) == 9 ) fprintf( fp, "\n" );
		}
		fprintf( fp, "\n" );

		fclose( fp );
	}
#endif ERROR_FILE

	tbase = BP->p_tbase;
	tlen = BP->p_tlen;

	if( ExcPC >= tbase && ExcPC <= (tbase + tlen ) ) {
		ExcPC -= tbase;
		rel = TRUE;
	}
	if( !IsRunning ) {
		sprintf( msg, LDS( 388, 
"[2][Fatal Error %d !|%s at %c%lx|Save Workspaces|Immediately !][ OK | Abort ]"),
		ExceptNumber, TheExcepts(ExceptNumber-1),
			rel ? '+' : '$', ExcPC );

	} else {
		run_error( ER(360, 
		"runexc`%s trap while running program"), 
			TheExcepts(ExceptNumber - 1) );
	}

	MenuCtrl( TRUE );

	IsRunning = FALSE;

	if( form_alert( 1, msg ) == 2 )
		done( "now" );
}

#endif SAI

/*
 * Construct an array from the args
 */
do_construct( argc, argv  )
int argc;
pointer *argv;
{
	construct( argv-2, argc-1, argv[-2], 999 );
}

construct( getargs, argc, aa, maxsize )
pointer getargs;
int argc;
rint *aa;
int maxsize;
{
	int i;		/* index into args */
	int typesize;	/* size of type of given arg */
	int argarp;	/* index into aa */

	argarp = 0;

	for( i = 0; i < argc; i++ ) {
		int wordcount;
		int dex;
		preg1 nodep ourtype;
		getargs -= sizeof(pointer);
		ourtype = * ((nodep *) getargs);
		/* most things are just pushed as pointers */
		if( is_real(ourtype) )
#ifdef FLOATING
			typesize = sizeof(rfloat);
#else
			typesize = sizeof(double);
#endif
		 else if( ntype(ourtype) == N_TYP_SET )
			typesize = SET_BYTES;
		 else if( is_ordinal(ourtype) )
			typesize = sizeof(rint);
		  else
			typesize = sizeof(pointer);
		getargs -= max( typesize, sizeof(pointer) ); 
		if( is_stringtype(ourtype) && ourtype != Basetype(BT_char) )
			((pointer *)getargs)[0]++;	/* increment string */
		wordcount = typesize / sizeof(rint);
		/* loop through words of this argument, put in array */
		for( dex = 0; dex < wordcount; dex++ ) {
			aa[argarp++] = ((rint *)getargs)[dex];
			if( argarp >= maxsize )
				run_error( ER( 9, "construct`Too many arguments to C function") );
			}
		}

	do_undef( aa, U_SET, sizeof(int) * argarp );
}
