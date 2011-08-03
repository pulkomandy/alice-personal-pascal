#define NUM_IN_BLANK	14		/* There are 14 objects in a blank
					 * menu treee
					 */
#define TITLEBAR	2

typedef struct _stmenu {
	int	allocated;
	int	used;
	int	num_titles;
	OBJECT	*tree;
	int	flag;
	int	active;
	} STMenu;

extern STMenu *NewMenu();
