
/*
 * Defines and structures for the ALICE ST's window management
 */


#define MAIN_SLOT	0
#define OUTPUT_SLOT	1

/*
 * The Kinds of windows on the desktop
 */

#define WIN_OUTPUT	1
#define WIN_EDIT	2
#define WIN_USER	3

#define TITLE_LEN	30	/* max # of chars in title */

typedef struct memform {
	long	fdaddr;
	int	fdw;
	int	fdh;
	int	fdwdwidth;
	int	fdstand;
	int	fdnplanes;
	int	fdr1, fdr2, fdr3;
	} MFDB;

typedef struct stwin {
	int handle;
	WINDOW *win;
	int used;
	int kind;
	int num_lines;
	int cur_line;
	GRECT work;
	char *alwin;
	char *title;
	char *info;		/* Pointer to info line */
	int tlen;
	MFDB *bit_image;
	int features;		/* What features each window has */
	char *the_file;		/* A pointer to the pas_file */

	} STWINDOW;



#define AW_KIND(X)	WinInfo[X].kind
#define AW_HANDLE(X)	WinInfo[X].handle
#define AW_WIN(X)	WinInfo[X].win
#define AW_WORK(X)	WinInfo[X].work
#define AW_INFO(X)	WinInfo[X].info
#define AW_TLEN(X)	WinInfo[X].tlen
#define AW_TITLE(X)	WinInfo[X].title
#define AW_USED(X)	WinInfo[X].used
#define AW_ALWIN(X)	WinInfo[X].alwin
#define AW_FEATURES(X)	WinInfo[X].features
#define AW_BITS(X)	WinInfo[X].bit_image
#define AW_NUMLINES(X)	WinInfo[X].num_lines
#define AW_CURLINE(X)	WinInfo[X].cur_line
#define AW_FILE(X)	WinInfo[X].the_file
extern STWINDOW WinInfo[];


extern MFDB *GemBits();
extern MFDB *MkImage();

#define CO_PIXEL	0
#define CO_NORMAL	1
#define CO_REVNORMAL	2
#define CO_STRETCH	3
#define CO_HIRES	4
#define CO_MEDRES	5
