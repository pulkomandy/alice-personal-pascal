#define	FNSIZE 128	/* maximum file node size */
#define	FMSIZE 128	/* maximum file name size */
#define	FESIZE 8	/* maximum file extension size */

#define	ALL_WHITE	0	/* bit blt rules */
#define	S_AND_D	1
#define	S_AND_NOTD	2
#define	S_ONLY	3
#define	NOTS_AND_D	4
#define	D_ONLY	5
#define	S_XOR_D	6
#define	S_OR_D	7
#define	NOT_SORD	8
#define	NOT_SXORD	9
#define	NOT_D	10
#define	S_OR_NOTD	11
#define	NOT_S	12
#define	NOTS_OR_D	13
#define	NOT_SANDD	14
#define	ALL_BLACK	15

typedef struct GFONT {
	short	font_id;	/* Font face identifier, 1 -> system font */
	short	size;		/* Font size in points */
	char	name[32];	/* Font name */
	short	first_ade;	/* Lowest ADE value in the face */
	short	last_ade;	/* Highest ADE value in the face */
	short	top;		/* Distance of top line	relative to baseline	*/
	short	ascent;		/* Distance of ascent line	relative to baseline	*/
	short	half;		/* Distance of half line	relative to baseline	*/
	short	descent;	/* Distance of decent line	relative to baseline	*/
	short	bottom;		/* Distance of bottom line	relative to baseline	*/
				/* All distances are measured in absolute values */
				/* rather than as offsets. They are always +ve */
	short	max_char_width;	/* Width of the widest character in font */
	
	short	max_cell_width;	/* Width of the widest character cell in face */
	short	loffset;	/* Left Offset see Vdi appendix G */
	short	roffset;	/* Right offset   "      "     " */
	short	thicken;	/* Number of pixels by which to thicken characters */
	short	ul_size;	/* Width in pixels of the underline	*/
	short	lighten;	/* The mask used to lighten characters */
	short	skew;		/* The mask used to determine	when to perform */
			/* additional rotation on the character to perform skewing */
	short	flags;		/* Flags */
				/*  $01 default system font */
				/*   02 horiz offset table should be used */
				/*   04 byte-swap flag (thanks to Motorola idiots) */
				/*   08 mono spaced font */
	short *	hoff_base;	/* Offset to horizontal offset table */
	short *	coff_base;	/* Offset to character offset table */
		/* font bitmap is byte[width][height], width must be even */
	short *	form_base;	/* Offset to font data */
	short	form_width;	/* Form width (#of bytes/scanline	in font data) */
	short	form_height;	/* Form height (#of scanlines in font data) */
	char *	next_font;	/* Pointer to next font in face */
} GFONT;

#define	THICKENED	0x0001	/*text effect definitions*/
#define	SHADED	0x0002
#define	SLANTED	0x0004
#define	UNDERLINED	0x0008
#define	DOUBLED	0x0010	/*Mine	BD*/

/* Raster library definitions */
typedef struct fdbstr {
	void	*fd_addr;
	short fd_w;
	short fd_h;
	short fd_wdwidth;
	short fd_stand;
	short fd_nplanes;
	short fd_r1;
	short fd_r2;
	short fd_r3;
} MFDB, FDB;

/*
 * MOUSE event flag bits
 */
#define	M_MOTION	0x001
#define	M_LEFT_DOWN	0x002
#define	M_LEFT_UP	0x004
#define	M_RIGHT_DOWN	0x008
#define	M_RIGHT_UP	0x010
#define	M_MIDDLE_DOWN	0x020
#define	M_MIDDLE_UP	0x040
#define	M_KEYPRESS	0x080
#define	M_EVENT	(M_MOTION | M_KEYPRESS | M_BUTTON_DOWN | M_BUTTON_UP)
#define	M_BUTTON_DOWN	(M_LEFT_DOWN | M_MIDDLE_DOWN | M_RIGHT_DOWN)
#define	M_BUTTON_UP	(M_LEFT_UP   | M_MIDDLE_UP   | M_RIGHT_UP)
#define	M_BUTTON_CHANGE (M_BUTTON_UP | M_BUTTON_DOWN )

/*
 * MOUSE button status bits
 */
#define	M_LEFT	4
#define	M_RIGHT	1
#define	M_MIDDLE	2


/*
 * KEYBOARD status word bits
 */

#define	_KB_RIGHT_ALT_	0x02	/* right shift key depressed */
#define	_KB_LEFT_ALT_	0x08	/* left shift key depressed */
#define	_KB_CTRL_	0x04	/* CTRL depressed */
#define	_KB_SHIFT_	0x01	/* ALT depressed */


typedef struct	_MOUSE  {
   unsigned char *shape;
   short hotx, hoty;
} _MOUSE;

typedef	struct	_MEVENT	{
    long	time;
    short	kinds;
    short	x,y;
    short	buttons;
    short	key;
    short	kbstat;
}	_MEVENT;

int vsl_color(int, int); /*draw.c*/

int vsf_color(int, int); /*draw.c*/

int vst_color(int, int); /*gtext.c*/
int vst_effects(int, int); /*gtext.c*/

int vst_font(int, int); /*gtext.c*/
int vst_user_font(int, GFONT *); /*gtext.c*/
int vst_load_font(int, const char *); /*gtext.c*/

void v_gtext(int, int, int, const char *); /*gtext.c*/

void v_pline(int, int, short *); /*draw.c*/
void v_bar(int, short *); /*draw.c*/
#define	vr_recfl v_bar

int v_opnvwk(int *); /*vmisc.c*/
void v_clrwk(int); /*vmisc.c*/
void v_clsvwk(int); /*vmisc.c*/
void vs_clip(int, int, short *); /*vmisc.c*/
int vex_retime(int, void *, void *, int); /*vmisc.c*/

void	v_show_c(int, int); /*mouse.c*/
void	v_hide_c(int); /*mouse.c*/
void	vq_key_s(int, short *); /*event.c*/
void	vq_mouse(int, short *, short *, short *); /*event.c*/
void	vq_keybd(int, short *); /*event.c*/
void	vro_cpyfm(int, int, short *, FDB *, FDB *); /*draw.c*/
