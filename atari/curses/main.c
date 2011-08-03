
#include <stdio.h>
#include <osbind.h>
#include <obdefs.h>
#include <gemdefs.h>
#include <gembind.h>
#include <curses.h>

/*
 * Specify the amount of space to allocate at load time for mallocs, etc
 */

#ifdef MarkWilliams
long	_stksize = 14000L;
#else
long	_RsvMem = 14000L;	/* How about 14K or so */
#endif

/*
 * Global Data Structures
 */
int	contrl[11];
int	intin[80];
int	ptsin[256];
int	intout[45];
int	ptsout[12];


/*
 * LOCAL Definitions
 */

int gl_wchar;		/* character height */
int gl_hchar;		/* character width */
int gl_hbox;		/* box (cell) width */
int gl_wbox;		/* box (cell) height */

int gl_hspace;		/* Height of space between lines */

long gl_menu;		/* The Menu Tree address */

int gem_handle;	/* GEM VDI Handle */
int vdi_handle;	/* application vdi handle */

/* The x,y,h,w of the full size window */
int gl_xfull;
int gl_yfull;
int gl_hfull;
int gl_wfull;

int work_out[57];

GRECT scr_area;		/* The Full Sized Screen */
GRECT work_area;	/* Drawing area for current window */

int gl_rmsg[8];	/* Message buffer */
long ad_rmsg;		/* A pointer to the message buffer */

int gl_apid;		/* The application ID */

int scr_planes;	/* Number of planes in video memory */

int m_out = 1;

int ev_which;

int mousex, mousey;
int bstate, bclicks;
int kstate, kreturn;

int alice_whndl;

int key_input;

char *wdw_title = " Alice Pascal ";
char *wdw_info = " Information about your work session ";

WINDOW *curWin = 0;

char buf[80];

main(argc, argv)
int argc;
char *argv[];
{
	int	work_in[11];
	int	i;

	gl_apid = appl_init();
	if( gl_apid == -1 ) 
		return(4);

	/*
	 * Open the Virtual Workstation
	 */
	for( i=0; i<10; i++ ) work_in[i] = 1;
	work_in[10] = 2;

	gem_handle = graf_handle( &gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox );
	vdi_handle = gem_handle;
	v_opnvwk( work_in, &vdi_handle, work_out );

	test();

	appl_exit();

}
