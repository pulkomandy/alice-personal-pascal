/* workspace struct */

typedef struct _work {
	bits8 node_type;		/* always N_NULL - 0 */
	nodep n_parent;			/* always 0 */
	holdlist flags1;		/* workspace flags */
	holdlist flags2;		/* moe of them */
	nodep ws_node;			/* root of workspace */
	nodep ws_cursor;		/* cursor within workspace */
	nodep ws_scursor;		/* first line in display */
	nodep ws_im_parent;		/* parent scope in immediate ws */
	char *ws_name;			/* name for this workspace */
	char *ws_fname;			/* filename for this workspace */
	struct _work *ws_prev;		/* previous workspace */
	struct _work *ws_next;		/* next workspace */
	int ws_debug;			/* debug on ? */
#ifdef getyx
	WINDOW *ws_r_output;		/* window for program output */
	WINDOW *ws_db_output;		/* window for debug output */
#endif getyx
	} workspace;

typedef workspace *workp;
typedef workspace far *realworkp;

extern workp myWS();
extern workp get_nws();			/* get a workspace by name */
extern workp curr_workspace;	/* the current workspace */
#ifndef ES_TREE
extern workspace main_ws;
#endif

/* workspace flags for flags 2 */

#define WS_FRAGMENT	0x80
#define WS_IMMEDIATE	0x40
#define WS_DIRTY	0x20	/* should the file be written out? */

#ifdef ES_TREE
#define wsptr(ws)	((realworkp)(tr_far + (unsigned)(ws)))
#else
/* Is this right ??? */
#define wsptr(ws)	ws
#endif


/* workspace macros */ 
#define work_node(ws)		wsptr(ws) BPTS ws_node
#define cur_work_top		wsptr(curr_workspace) BPTS ws_node
#define work_cursor(ws)		wsptr(ws) BPTS ws_cursor
#define work_scursor(ws) 	wsptr(ws) BPTS ws_scursor
#define work_name(ws)		wsptr(ws) BPTS ws_name
#define work_fname(ws)		wsptr(ws) BPTS ws_fname
#define work_window(ws)		wsptr(ws) BPTS ws_window
#define work_prev(ws)		wsptr(ws) BPTS ws_prev
#define work_next(ws)		wsptr(ws) BPTS ws_next

#ifdef GEM
#define work_debug(ws)		(DbFlag)
extern int DbFlag;
#else
#define work_debug(ws)		wsptr(ws) BPTS ws_debug
#endif

#define work_owindow(ws)	wsptr(ws) BPTS ws_r_output
#define work_dbwindow(ws)	wsptr(ws) BPTS ws_db_output
#define work_dirty(ws)		(wsptr(ws) BPTS flags2 & WS_DIRTY)

#define	s_work_fname(ws, fname)	((wsptr(ws) BPTS ws_fname) = fname)

