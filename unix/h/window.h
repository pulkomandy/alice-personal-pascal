/* alice window structure */

/* An alice window is more than just the curses window where output is
 * done.  We must keep track of what program element in on each line,
 * and how much updating the window needs, etc.
 */

struct al_window {
#ifdef getyx
	WINDOW *w_desc;			/* pointer to curses window struct */
#endif getyx
	int w_height;			/* how many lines in the window */
	int w_width;			/* how many columns */
	struct scr_line *w_lines;	/* line information tables */
	bits8 big_change;		/* redraw whole window ? */
	curspos start_cursor;		/* first line displayed */
	workp wspace;			/* workspace for this window */
#ifdef getyx
	WINDOW *out_split;		/* window with user output */
	WINDOW *db_split;		/* window for debug output within */
#endif
	bits8 start_pcode;		/* pcode in start cursor */
	bits8 start_sline;		/* sub line of start cursor */
	/* where the cursor is in it */
	/* any other mode information */
	};

typedef struct al_window alice_window;

/* various windowing defines */

#define MAXWINDOWS	10
#define STDWIN		0
#define MAINWIN 	1
#define CMDWIN		2
#define ERRWIN		3
#define	EXTRAWIN	4

extern alice_window *curr_window;	/* current window */
extern alice_window main_win;
extern WINDOW	*curWindows[];
extern WINDOW	*mainwin;
extern WINDOW	*cmdwin;
extern WINDOW	*errwin;

