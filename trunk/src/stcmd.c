
#include "alice.h"
#include <obdefs.h>
#include <gemdefs.h>
#include <linea.h>

#include "alrsc.h"

extern int vdi_handle;
extern char *malloc();
extern int LoRes;

GemCommand( prompt, cmdbuf )
char *cmdbuf;
char *prompt;
{
	OBJECT *tree;
	int	exit_obj;
	int	fo_cx, fo_cy, fo_ch, fo_cw;
	TEDINFO *ted;
	int	i;
	char	*str;

	/*
	 * First of all, get the address of the 'command' dialog
	 */
	if( LoRes )
		rsrc_gaddr( R_TREE, LOCOMMAN, &tree );
	else
		rsrc_gaddr( R_TREE, COMMAND, &tree );

	/*
	 * Get a pointer into the TEDINFO struct for the prefix
	 */
	ted = (TEDINFO *)tree[CMDPRMPT].ob_spec;
	ted->te_ptext = prompt;
	ted->te_txtlen = strlen( prompt );

	ted = (TEDINFO *)tree[CMDEDIT].ob_spec;

	/*
	 * There might possibly be some type-ahead, yuck !, so go
	 * and get it if it exists...
	 */
	i = 0;
	/*
	 * Flush the GEM queue into the incom_buf
	 */
	while( ST_chr_waits() );

	str = ted->te_ptext;

#ifndef SAI
	while( get_lookahead() ) {
		str[i++] = readkey();
	}
#endif

	str[i] = 0;

	form_bottom( tree, &fo_cx, &fo_cy, &fo_cw, &fo_ch );
	scr_save( fo_cx, fo_cy, fo_cw, fo_ch );

	/*
	 * Draw the initial tree
	 */
	DrawObj( tree, 0 );

	exit_obj = form_do( tree, CMDEDIT );
	exit_obj &= 0x7f;
	tree[exit_obj].ob_state = 0;

	scr_restore( fo_cx, fo_cy, fo_cw, fo_ch );
	if( exit_obj == CMDCAN ) 
		strcpy( cmdbuf, "" );
	else
		strcpy( cmdbuf, ted->te_ptext );

}

typedef struct memform {
	long	fdaddr;
	int	fdw;
	int	fdh;
	int	fdwdwidth;
	int	fdstand;
	int	fdnplanes;
	int	fdr1, fdr2, fdr3;
	} MFDB;

MFDB scrMFDB, savMFDB;

char *SaveArea = (char *)0;
int SaveSize;

scr_save( x, y, w, h )
int x, y, w, h;
{
	int size;
	int pxy[10];

	if( SaveArea != (char *)0 )
		free( SaveArea );

	SaveArea = (char *)0;

	if( x < 0 || y < 0 ) {
		form_dial( 0, 0, 0, 0, 0, x, y, w, h );
		return;
	}

	/*
	 * Try to malloc enough memory to hold the bit image
	 */
	size = ((w + 15) / 16) * sizeof(int) * h * VPLANES;
	size += 2;

	SaveArea = (char *)malloc( size );

	SaveSize = size;

	if( SaveArea == (char *)0 ) 
		form_dial( 0, 0, 0, 0, 0, x, y, w, h );
	else {
		SaveArea[size-2] = 'Q';
		SaveArea[size-1] = 'R';

		/*
		 * Blit the area (x,y,w,h)
		 * into the SaveArea
		 */

		/*
		 * Set up screen MFDB
		 */
		scrMFDB.fdaddr = 0L;

		/*
		 * Save MFDB
		 */
		savMFDB.fdaddr = SaveArea;
		savMFDB.fdh = h;
		savMFDB.fdwdwidth = (w+15) / 16;
		savMFDB.fdw = savMFDB.fdwdwidth * 16;;
		savMFDB.fdnplanes = VPLANES;
		savMFDB.fdstand = 0;

		pxy[0] = x;
		pxy[1] = y;
		pxy[2] = x + w - 1;
		pxy[3] = y + h - 1;
		pxy[4] = 0;
		pxy[5] = 0;
		pxy[6] = w - 1;
		pxy[7] = h - 1;

		graf_mouse( M_OFF, 0L );
		vro_cpyfm( vdi_handle, 3, pxy, &scrMFDB, &savMFDB );
		graf_mouse( M_ON, 0L );

	}

}

scr_restore( x, y, w, h )
int x, y, w, h;
{
	int pxy[10];

	if( SaveArea == (char *)0 )
		form_dial( 3, 0, 0, 0, 0, x, y, w, h );
	else {
		if( SaveArea[SaveSize-2] != 'Q' ||
		    SaveArea[SaveSize-1] != 'R' ) {
			form_alert(1, LDS( 367,
		 "[2][ScrSave buffer got stomped !][ OK ]")
				);
			return;
		}

		/*
		 * Blit the area back to the screen
		 */
		/*
		 * Set up screen MFDB
		 */
		scrMFDB.fdaddr = 0L;

		/*
		 * Save MFDB
		 */
		savMFDB.fdaddr = SaveArea;
		savMFDB.fdh = h;
		savMFDB.fdwdwidth = (w+15) / 16;
		savMFDB.fdw = savMFDB.fdwdwidth * 16;;
		savMFDB.fdnplanes = VPLANES;
		savMFDB.fdstand = 0;

		pxy[4] = x;
		pxy[5] = y;
		pxy[6] = x + w - 1;
		pxy[7] = y + h - 1;

		pxy[0] = 0;
		pxy[1] = 0;
		pxy[2] = w - 1;
		pxy[3] = h - 1;

		graf_mouse( M_OFF, 0L );
		vro_cpyfm( vdi_handle, 3, pxy, &savMFDB, &scrMFDB );
		graf_mouse( M_ON, 0L );

		free( SaveArea );
		SaveArea = (char *)0;
	}
}
