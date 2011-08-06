/* Includes stuff from attempt to add windows, currently stopped*/
/*#include	"portab.h"*/
#define ROOT 0
#define NIL -1
#define	MAX_DEPTH	8	/* max depth of search or draw */
/*#define	NUM_WIN	12
#define	NUM_ACC	6*/

#define	R_BLOCK	20
#define	R_IBOX	25
#define	R_CHAR	27
#define	R_TEXT	28
#define	R_FTEXT	29
#define	R_ETEXT	30
#define	R_STEXT	31
#define	R_TITLE	32

#define	NONE	0x0000	/* Object flags	*/
#define	DEFAULT	0x0001
#define	EXIT	0x0002
#define	CHECKED	0x0004
#define	LASTOB	0x0008
#define	HIDETREE	0x0010
#define	SELECTED	0x0020
#define	DISABLED	0x0040
#define	FEINT	0x0040
#define	OUTLINED	0x0080

#define	WHITE	0	/* Object colors */
#define	BLACK	1
#define	RED	2
#define	GREEN	3
#define	BLUE	4
#define	CYAN	5
#define	YELLOW	6
#define	MAGENTA	7
#define	LWHITE	8
#define	LBLACK	9
#define	LRED	10
#define	LGREEN	11
#define	LBLUE	12
#define	LCYAN	13
#define	LYELLOW	14
#define	LMAGENTA	15

#define	R_TREE	0
#define	R_OBJECT	1
#define	R_TEDINFO	2

typedef	struct {
	unsigned short rsh_rssize;	/* total bytes in resource */
	unsigned short rsh_tedinfo;	/* offset to tedinfo[] */
	unsigned short rsh_object;	/* offset to object[] */
	unsigned short rsh_trindex;	/* offset to object tree index */
	short rsh_nteds;		/* number of tedinfos */
	short rsh_nobs;		/* number of objects */
	short rsh_ntrees;		/* number of trees */
} RSHDR;

typedef struct object {
	short	ob_next;
	short	ob_head;
	short	ob_tail;
	unsigned	char ob_type;
	unsigned	char ob_flags;
	unsigned	char ob_colbyte;
	unsigned	char ob_font;
	unsigned	char ob_border;
	unsigned	char ob_fill;
	char	*ob_spec;
	short	ob_x;
	short	ob_y;
	short	ob_width;
	short	ob_height;
} OBJECT;

typedef struct tedinfo {
	char *te_ptext;
	char *te_ptmplt;
	short	te_pvalid;
	short	te_ptxtlen;
} TEDINFO;

#define ORECT	struct orect

ORECT
{
	ORECT	*o_link;
	short	o_x;
	short	o_y;
	short	o_w;
	short	o_h;
} ;

typedef struct grect	{
	short g_x;
	short g_y;
	short g_w;
	short g_h;
} GRECT;

/*typedef struct window {
	short		w_flags;
	COMMENT:omitted PD
	short		w_kind;
	long		w_pname;
	long		w_pinfo;
	short		w_xfull;
	short		w_yfull;
	short		w_wfull;
	short		w_hfull;
	short		w_xwork;
	short		w_ywork;
	short		w_wwork;
	short		w_hwork;
	short		w_xprev;
	short		w_yprev;
	short		w_wprev;
	short		w_hprev;
	short		w_hslide;
	short		w_vslide;
	short		w_hslsiz;
	short		w_vslsiz;
	ORECT		*w_rlist;	owner rect. list
	ORECT		*w_rnext;	used for search first
					search next
} WINDOW;

typedef struct wsstr
{
	WORD		ws_xres;
	WORD		ws_yres;
	WORD		ws_noscale;
	WORD		ws_wpixel;
	WORD		ws_hpixel;
	WORD		ws_ncheights;
	WORD		ws_nlntypes;
	WORD		ws_nlnwidths;
	WORD		ws_nmktypes;
	WORD		ws_nmksizes;
	WORD		ws_nfaces;
	WORD		ws_npatts;
	WORD		ws_nhatchs;
	WORD		ws_ncolors;
	WORD		ws_ngdps;
	WORD		ws_supgdps[10];
	WORD		ws_attgdps[10];
	WORD		ws_color;
	WORD		ws_rotate;
	WORD		ws_fill;
	WORD		ws_cell;
	WORD		ws_npals;
	WORD		ws_nloc;
	WORD		ws_nval;
	WORD		ws_nchoice;
	WORD		ws_nstring;
	WORD		ws_type;
	WORD		ws_pts0;
	WORD		ws_chminh;
	WORD		ws_pts2;
	WORD		ws_chmaxh;
	WORD		ws_lnminw;
	WORD		ws_pts5;
	WORD		ws_lnmaxw;
	WORD		ws_pts7;
	WORD		ws_pts8;
	WORD		ws_mkminw;
	WORD		ws_pts10;
	WORD		ws_mkmaxw;
} WS;*/


/*#define NUM_ORECT (NUM_WIN * 10)*/	/* is this enough???	*/

/* Graphics library definitions */
#define	ARROW	0
#define	TEXT_CRSR	1
#define	HOURGLASS	2
#define	BUSYBEE	HOURGLASS
#define	POINT_HAND	3
#define	FLAT_HAND	4
#define	THIN_CROSS	5
#define	THICK_CROSS	6
#define	OUTLN_CROSS	7
#define	USER_DEF	255
#define	M_OFF	256
#define	M_ON	257

/* Event library definitions */
#define	MU_KEYBD	0x0001
#define	MU_BUTTON	0x0002
#define	MU_M1	0x0004
#define	MU_M2	0x0008
#define	MU_MESAG	0x0010
#define	MU_TIMER	0x0020

#define	MN_SELECTED	10

#define	WM_REDRAW	20
#define	WM_TOPPED	21
#define	WM_CLOSED	22
#define	WM_FULLED	23
#define	WM_ARROWED	24
#define	WM_HSLID	25
#define	WM_VSLID	26
#define	WM_SIZED	27
#define	WM_MOVED	28
#define	WM_NEWTOP	29

#define	AC_OPEN	40
#define	AC_CLOSE	41

#define	WA_UPPAGE	0
#define	WA_DNPAGE	1
#define	WA_UPLINE	2
#define	WA_DNLINE	3
#define	WA_LFPAGE	4
#define	WA_RTPAGE	5
#define	WA_LFLINE	6
#define	WA_RTLINE	7

/*	WINDLIB.H	05/05/84 - 01/26/85	Lee Lorenzen		*/

#define VF_INUSE 0x0001
#define VF_BROKEN 0x0002
#define VF_INTREE 0x0004
#define VF_SUBWIN 0x0008
#define VF_KEEPWIN 0x0010

#define NUM_ELEM 19

#define WC_BORDER 0
#define WC_WORK 1

#define DESKWH	0x0

#define NAME 0x0001
#define CLOSER 0x0002
#define FULLER 0x0004
#define MOVER 0x0008
#define INFO 0x0010
#define SIZER 0x0020
#define UPARROW 0x0040
#define DNARROW 0x0080
#define VSLIDE 0x0100
#define LFARROW 0x0200
#define RTARROW 0x0400
#define HSLIDE 0x0800

#define WS_FULL 0
#define WS_CURR 1
#define WS_PREV 2
#define WS_WORK 3
#define WS_TRUE 4
/*Goes in aes.h if ever done*/

int	appl_init(int); /*init.c*/
int	appl_exit(void); /*init.c*/
int	graf_handle(short *, short *, short *, short *); /*init.c*/
int	objc_draw(OBJECT *, int, int, int, int, int, int); /*resource.c*/
int	objc_find(OBJECT *, int, int, int, int); /*resource.c*/
int	objc_offset(OBJECT *, int, short *, short *); /*resource.c*/
int	form_do(OBJECT *,int); /*form.c*/
int	form_dial(int, int, int, int, int, int, int, int, int); /*form.c*/
int	form_alert(int, const char *); /*form.c*/
int	form_infobox(OBJECT *); /*form.c*/
int	form_error(int); /*form.c*/
int	form_error_text(char *);	/*form.c*/
int	form_input(int, char *, int, char *); /*form.c*/
void	form_input_text(int, char *, char *, char *);	/*form.c*/
int	form_center(OBJECT *,short *,short *,short *,short *); /*form.c*/
int	rsrc_load(const char *); /*resource.c*/
int	rsrc_include(void *); /*resource.c*/
int	rsrc_free(void); /*resource.c*/
int	rsrc_gaddr(int, int, void *); /*resource.c*/
int	graf_rubberbox(int, int, int, int, short *, short *); /*resource.c*/
int	graf_dragbox(int, int, int, int, int, int, int, int,
	short *, short *); /*resource.c*/
int	graf_slidebox(OBJECT *, int, int, int); /*resource.c*/
int	menu_bar(OBJECT *, int); /*menu.c*/
int	menu_tnormal(OBJECT *,int, int); /*menu.c*/
int	menu_icheck(OBJECT *,int, int); /*menu.c*/
int	menu_ienable(OBJECT *, int, int); /*menu.c*/
int	graf_mouse(int,void *); /*mouse.c*/
void	fsel_set_extn(char *); /*fsel.c*/
int	fsel_input(char *,char *,short *); /*fsel.c*/
int	fsel_exinput(char *,char *,short *,const char *); /*fsel.c*/
int	evnt_keybd(void);	/*event.c*/
int	evnt_button(int, int, int,short *,short *,short *,short *); /*event.c*/
int	evnt_mesag(short *); /*event.c*/
int	evnt_timer(int, int); /*event.c*/
int	evnt_multi(int, int, int, int, int, int, int, int, int, int, /*event.c*/
	int, int, int, int, short *, int, int, short *, short *, short *,
	short *, short *, short *); /*event.c*/
