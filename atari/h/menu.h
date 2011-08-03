/*
 * Structure used by pop-up menu routine.
 */

#define MAX_ITEMS 160 

struct menu_data {
	int num_items;
	int max_width;		/* width of largest item */
	int m_options;		/* not used */
	char *m_title;		/*menu title */
	char *item[MAX_ITEMS];
};

#define HIGH_REV	4	/* 1-> mark with rev vid, 0-> outline	*/
#define OUTLINE		2	/* 1-> outline each item		*/
#define DBL_SP		1	/* double space item strings		*/

/* highlight codes */
#define NO_HIGHLIGHT 0
#define SELECT_ITEM 1
#define TITLE_HIGHLIGHT 2
#define MORE_HIGHLIGHT 3

extern char *M_insert[];
extern char *M_delete[];
extern char *M_change[];
extern char *M_io[];
extern char *M_misc[];
extern char *M_run[];
extern char *M_move[];
extern char *M_helpkey[];
extern char *M_master[];
extern char *M_range[];
extern char *M_init[];
extern char *M_quit[];
extern char *M_load[];
extern char *M_save[];
