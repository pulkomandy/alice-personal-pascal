
#include "alice.h"
#include "interp.h"

#include <stdio.h>
#include <obdefs.h>
#include <gemdefs.h>

#include "stmenu.h"
#include "alrsc.h"


#define menu_alloc	newAlloc
#define menu_free	newFree

extern char *menu_alloc();

extern int gl_wchar, gl_hchar;

STMenu *UsrMenu;
int InGemApp = FALSE;

#ifndef SAI
extern long gl_menu;
#endif SAI

#define MAX_ITEMS	25
#define DASH_LEN	40

static char dashes[] = "----------------------------------------";

#ifndef OLMSG
char notmenu[] = "notmen`Error - pointer not allocated by NewMenuBar";
#endif

char *
newStrAlloc( str )
char *str;
{
	char *ptr;
	ptr = (char *) menu_alloc( strlen( str ) + 1 );
	/* If menu_alloc (newAlloc) fails, it will give a run_error */
	strcpy( ptr, str );
	return ptr;
}

/*
 * Create a new menu
 */

#define INIT_NUMBER	(NUM_IN_BLANK + 10)
#define MORE_OBJS	20

mact_err( m, active )
STMenu *m;
int active;
{
	if( m->active != active ) {
		if( m->active )
			run_error(
	ER(340,"menuact`Menu cannot be active while adding items") );

	}
}

STMenu *
NewMenu()
{
	OBJECT *ptr;
	OBJECT *tree;
	STMenu *menu;

	menu = (STMenu *) menu_alloc( sizeof( STMenu ) );

	menu->flag = 'M';
	menu->active = FALSE;

	/*
	 * allocate enough memory for some items
	 */
	ptr = (OBJECT *) menu_alloc( sizeof( OBJECT ) * INIT_NUMBER );

	menu->tree = ptr;

	menu->allocated = INIT_NUMBER;	/* set number allocated to INIT_NUMBER */
	menu->used = NUM_IN_BLANK;	/* set number used to that of a blank menu */
	menu->num_titles = 1;		/* One title, the desk menu */

	rsrc_gaddr( R_TREE, MENUBLNK, &tree );

	memcpy( tree, ptr, NUM_IN_BLANK * sizeof( OBJECT ) );

	return menu;
}

memcpy( src, dst, size )
register char *src;
register char *dst;
register unsigned int size;
{
	while( size-- ) *dst++ = *src++;
}

GetObj( mptr, tree, index )
STMenu *mptr;
OBJECT **tree;
int *index;
{
	STMenu *alloc;
	STMenu *ptr;
	OBJECT *obj;
	OBJECT *the_tree;
	unsigned int menu_size;
	int i;

	ptr = mptr;

	/*
	 * Are all the allocated objects used ?
	 */
	if( ptr->allocated == ptr->used ) {
		/*
		 * Time to allocate more memory
		 */
		menu_size = (sizeof(OBJECT) * ptr->allocated);

		alloc = (STMenu *)menu_alloc(menu_size + MORE_OBJS * sizeof( OBJECT ) );

		for( i=0; i<ptr->allocated + MORE_OBJS; i++ ) {
			InitObj( alloc, i );
		}
		memcpy( ptr->tree, alloc, menu_size );
		menu_free( ptr->tree );
		ptr->tree = alloc;
		ptr->allocated += MORE_OBJS;
	}

	the_tree = ptr->tree;
	obj = &the_tree[ptr->used];

	*tree = the_tree;
	*index = ptr->used;

	InitObj( *tree, *index );

	ptr->used++;

}

/*
 * Add a list of items to a specified menu
 */

usr_add_menu( menu, n, title, items )
STMenu *menu;
int n;
char *title;
char *items;
{
	char *start;
	OBJECT *tree;
	int box;
	int index;
	int i;
	char *ItemPtr[ MAX_ITEMS ];
	int  NumItems;

	NumItems = ParseItems( ItemPtr, items );

	tree = menu->tree;

	if( n == 0 ) {
		/*
		 * Set the 'about' message
		 */
		box = FindIndex( tree, 0 );
		index = tree[box].ob_head;
		tree[index].ob_spec = (long) ItemPtr[0];
		return;
	}

	if( n > menu->num_titles )
		run_error( ER(341,"menuorder`Attempt to add menus in wrong order !") );

	if( n == menu->num_titles ) {
		/*
		 * Otherwise, we are creating a new menu
		 */

		/*
		 * Get a new object to hold the title
		 */
		GetObj( menu, &tree, &index );

		objc_add( tree, TITLEBAR, index );
		tree[index].ob_type = G_TITLE;
		tree[index].ob_spec = (long) title;

		/*
		 * Now go and make a new box to hold the items
		 */
		GetObj( menu, &tree, &index );

		tree[index].ob_type = G_BOX;
		tree[index].ob_spec = (long) 0xFF1100L;

		/*
		 * Find the guy we are supposed to add this to
		 */

		objc_add( tree, tree[0].ob_tail, index );
		menu->num_titles++;
	} else {
		/* We are adding items to an existing menu, change
		 * the title of that menu
		 */

		index = tree[0].ob_head;
		index = tree[index].ob_head;

		index = tree[index].ob_head;

		for( i=0; i<n; i++ )
			index = tree[index].ob_next;

		tree[index].ob_spec = (long) title;

	}
	/*
	 * Add the items to the menu (either existing or just created)
	 */

	/*
	 * We are adding items to an existing menu
	 */
		
	/*
	 * Find the index of the box that holds the items for
	 * this menu
	 */
	box = FindIndex( menu->tree, n );

	for( i = 0; i < NumItems; i++ ) {

		GetObj( menu, &tree, &index );

		/*
		 * Add this item to the menu box
		 */

		objc_add( tree, box, index );
		tree[index].ob_type = G_STRING;
		tree[index].ob_spec = (long) ItemPtr[i];
	}

	/*
	 * Now that we have added the items, go and fix the box up
	 */
	FixBox( tree, box );
	FixBar( tree );
}

int
FindIndex( tree, n )
register OBJECT *tree;
int n;
{
	register int	index;
	register int	i;

	/*
	 * Find the first kid of the root
	 */
	index = tree[0].ob_tail;
	index = tree[index].ob_head;
	for( i=0; i<n; i++ ) 
		index = tree[index].ob_next;

	return index;
}

FixBar( tree )
OBJECT *tree;
{
	int index;
	int MBar;
	int t_head, t_tail, t_index;
	int m_head, m_tail, m_index;
	int cur_x;
	char *str;
	extern GRECT scr_area;

	/*
	 * Go and fix the menu bar for the specified menu
	 */

	index = tree[0].ob_head;
	index = tree[index].ob_head;

	MBar = index;

	t_head = tree[index].ob_head;
	t_tail = tree[index].ob_tail;

	/*
	 * Now go and find the head and tail pointers for the menu boxes
	 */
	index = tree[0].ob_tail;
	m_head = tree[index].ob_head;
	m_tail = tree[index].ob_tail;

	m_index = m_head;
	t_index = t_head;
	cur_x = 0;

	for( ;; ) {
		tree[t_index].ob_x = cur_x;
		tree[t_index].ob_y = 0;
		str = (char *) tree[t_index].ob_spec;
		tree[t_index].ob_width = strlen( str ) * gl_wchar;
		tree[t_index].ob_height = tree[MBar].ob_height;
		tree[m_index].ob_x = cur_x + tree[MBar].ob_x;

		/* If it extends past the end of the screen, shift it left */
		tree[m_index].ob_x = min( tree[m_index].ob_x,
			scr_area.g_w - tree[m_index].ob_width - gl_wchar );

		tree[m_index].ob_y = 0;
		cur_x += strlen( str ) * gl_wchar;

		if( t_index == t_tail ) break;

		m_index = tree[m_index].ob_next;
		t_index = tree[t_index].ob_next;

	}
	tree[MBar].ob_width = cur_x;

	if( cur_x > scr_area.g_w )
		run_error( ER(334,"fullmenu`Menu bar too wide") );

}

#ifndef OLMSG
char *SMenu[] = {
	"Dsk ", "File ", "Ed ", "Stc ", "Run ", "Dbg ", "Help ", "Go ", "Misc"
	};
#endif

ShortenMenu( tree )
OBJECT *tree;
{
	int index;
	int t_head, t_tail, t_index;
	char *str;
	int i;

	/*
	 * Go and fix the menu bar for the specified menu
	 */

	index = tree[0].ob_head;
	index = tree[index].ob_head;

	t_head = tree[index].ob_head;
	t_tail = tree[index].ob_tail;

	/*
	 * Now go and find the head and tail pointers for the menu boxes
	 */

	t_index = t_head;

	i = 0;
	for( ;; ) {
#ifdef OLMSG
		tree[t_index].ob_spec = LDS( 484 + i, "foo" );
		i++;
#else
		tree[t_index].ob_spec = SMenu[i++];
#endif
		if( t_index == t_tail ) break;
		t_index = tree[t_index].ob_next;
	}
}

FixBox( tree, index )
OBJECT *tree;
int index;
{
	int tail, head;
	int i;
	int max_len;
	int cur_y;
	char *str;
	extern GRECT scr_area;
	long scr_size;
	long pix_size;

	/*
	 * This routine goes through all the kids of the box given, and
	 * makes sure that the x and y co-ordinates are proper, and that
	 * the x and y co-ordinates for the box are right
	 */
	head = tree[index].ob_head;
	tail = tree[index].ob_tail;

	i = head;
	max_len = 0;
	cur_y = 0;

	for( ;; ) {
		str = (char *) tree[i].ob_spec;

		if( *str != '-' ) {
			max_len = max( max_len, strlen( str ) );
		}

		tree[i].ob_x = 0;
		tree[i].ob_y = cur_y;
		tree[i].ob_height = gl_hchar;
		cur_y += gl_hchar;

		/*
		 * Last one, break
		 */
		if( i == tail ) break;
		i = tree[i].ob_next;
	}

	/*
	 * Okay, so the maximum width is 'max_len'
	 * So set the width of the containing box to 'max_len' * gl_wchar
	 */
	tree[index].ob_width = max_len * gl_wchar;
	tree[index].ob_height = cur_y;

	if( cur_y > scr_area.g_h )
		run_error( ER(333,"fullmenu`Cannot allocate any more items in menu" ));

	pix_size = (long)tree[index].ob_width * (long)tree[index].ob_height;
	scr_size = (long)scr_area.g_w * (long)scr_area.g_h;
	if( pix_size > (scr_size / 4L)) {
		run_error( ER( 359, "boxsize`Error - Menu Box too big") );
	}

	/*
	 * Now go and fix up the kids
	 */
	i = head;
	for(;;) {
		tree[i].ob_width = max_len * gl_wchar;
		str = (char *) tree[i].ob_spec;
		if( *str == '-' ) {
			tree[i].ob_spec = (long) &dashes[DASH_LEN-max_len];
			tree[i].ob_state = DISABLED;
		}
		if( i == tail ) break;
		i = tree[i].ob_next;
	}
}

/*
 * Activate the menu
 */

ShowMenu( m )
STMenu *m;
{
	extern STMenu *UsrMenu;

	/*
	 * If we are activating a new menu, or we are turning off a menu
	 * that wasn't displayed, turn off the one that was displayed
	 */

	NeedsGem();	/* ApplInit must be active */

	/*
	 * If there already is a menu bar, erase it
	 */
	if( UsrMenu ) {
		menu_bar( UsrMenu->tree, 0 );
		UsrMenu->active = FALSE;
	}

	if( m ) {
		/*
		 * Replacing it with a new one
		 */
		UsrMenu = m;
		SortTree( m->tree );
		menu_bar( m->tree, 1 );
		m->active = TRUE;
	}

}

/*
 * Sort the tree
 */
#define MAX_OBJS	150

int cur_index, max_index;
unsigned char ObjPosition[MAX_OBJS];

NumberNode( tree, index )
OBJECT *tree;
int index;
{
	int head, tail;

	ObjPosition[index] = cur_index++;

	if( index > max_index ) max_index = index;

	/*
	 * Now go and number the kids
	 */
	head = tree[index].ob_head;
	tail = tree[index].ob_tail;
	if( head != -1 ) {
		for( ;; ) {
			NumberNode( tree, head );
			if( head == tail ) break;
			head = tree[head].ob_next;
		}
	}

}

NumberTree( tree )
OBJECT *tree;
{
	cur_index = 0;
	max_index = 0;
	NumberNode( tree, 0 );
}

SwapTree( tree, old, new )
OBJECT *tree;
int old, new;
{
	OBJECT *p;
	OBJECT temp;
	int i;

	/*
	 * Swap the objects 'old' and 'new' in this tree
	 */
	for( i=0; i<=max_index; i++ ) {
		/*
		 * This is an object that is not in the tree
		 * but is in the array
		 */
		if( ObjPosition[i] == 255 ) continue;

		p = &tree[i];

		if( p->ob_head == old )
			p->ob_head = new;
		else if( p->ob_head == new )
			p->ob_head = old;

		if( p->ob_tail == old )
			p->ob_tail = new;
		else if( p->ob_tail == new )
			p->ob_tail = old;

		if( p->ob_next == old )
			p->ob_next = new;
		else if( p->ob_next == new )
			p->ob_next = old;

	}

	memcpy( &tree[old], &temp, sizeof( OBJECT ) );
	memcpy( &tree[new], &tree[old], sizeof( OBJECT ) );
	memcpy( &temp, &tree[new], sizeof( OBJECT ) );
}

SortTree( tree )
OBJECT *tree;
{
	int i, t;
	int swap;
	int a, b;

	for( i=0; i<MAX_OBJS; i++ ) {
		ObjPosition[i] = 255;
	}

	NumberTree( tree );

	swap = TRUE;
	while( swap ) {

		swap = FALSE;
		for( i=0; i <= max_index; i++ ) {
			if( ObjPosition[i] == 255 ) continue;

			if( ObjPosition[i] != i ) {
				/*
				 * Swap 'em
				 */
				a = i;
				b = ObjPosition[i];

				t = ObjPosition[a];
				ObjPosition[a] = ObjPosition[b];
				ObjPosition[b] = t;

				SwapTree( tree, a, b );

				swap = TRUE;
			}
		}

	}

	for( i=0; i<=max_index; i++ ) {
		tree[i].ob_flags &= ~LASTOB;
	}

	tree[max_index].ob_flags |= LASTOB;
}

/*
 ************************** Interpreter Routines *********************
 */

do_gnewmenu( argc, argv )
int argc;
pointer *argv;
{
	STMenu *menu;

	menu = NewMenu();

	*argv = menu;
}

do_gaddmenu( argc, argv )
int argc;
rint *argv;
{
	STMenu *menu;
	int title_num;
	char *title;
	char *items;

	items = str_spop() + STR_START;
	title = str_spop() + STR_START;
	title_num = int_spop();
	menu = p_spop();

	if( menu->flag != 'M' )
		run_error( ER( 355, notmenu ));

	/* Menu must not be active ! */
	mact_err( menu, FALSE );

	usr_add_menu( menu, title_num, newStrAlloc(title), items );
}

do_gshowmenu( argc, argv )
int argc;
rint *argv;
{
	STMenu *menu;

	menu = p_spop();
	if( menu && menu->flag != 'M' )
		run_error( ER(355, notmenu) );

	ShowMenu( menu );
}

/*
 * Convert a mnemonic into the index of a tree
 */

int
GetTreeIndex( mnem )
unsigned int mnem;
{
	int i;
	register OBJECT *tree;
	unsigned int code;

	/*
	 * Calculate the address of the tree, and the index of the item
	 * in question
	 */
	if( !UsrMenu ) {
		run_error( ER(349,"nouser`No User Menu") );
	}

	tree = UsrMenu->tree;

	for( i=0; i<UsrMenu->used; i++ ) {
		code = tree[i].ob_spec >> 24;
		if( code == mnem ) return i;
	}

	run_error( ER(330,"badcode`Error - can't find mnemonic in menu") );

}

do_gmenucheck()
{
	OBJECT *tree;
	int index;

	int flag = int_spop();
	int mnem = int_spop();

	index = GetTreeIndex( mnem );
	tree = UsrMenu->tree;

	menu_icheck( tree, index, flag );
}

do_gmenugettext( argc, argv )
int argc;
pointer *argv;
{
	OBJECT *tree;
	int index;
	char buf[80];

	int mnem = int_spop();

	index = GetTreeIndex( mnem );
	tree = UsrMenu->tree;
	strcpy( buf, tree[index].ob_spec );
	space_strip( buf );

	str_ret_push( argv, buf, strlen( buf ) );
}

/*
 * Clear the highlighting of the menu title
 */

clr_menu( index )
int index;
{
	if( UsrMenu )
		menu_tnormal( UsrMenu->tree, index, 1 );
}

do_gmenuenable()
{
	OBJECT *tree;
	int index;

	int flag = int_spop();
	int mnem = int_spop();

	index = GetTreeIndex( mnem );
	tree = UsrMenu->tree;

	menu_ienable( tree, index, flag );

}

do_gmenutext()
{
	OBJECT *tree;
	int index;
	int box;

	char *str = str_spop() + STR_START;
	int mnem = int_spop();

	/*
	 * Find out which object this refers to
	 */
	index = GetTreeIndex( mnem );
	tree = UsrMenu->tree;

	/*
	 * Now go and set the text of an item
	 */
	tree[index].ob_spec = (long) newStrAlloc( str );

	for( box = 0; box < UsrMenu->num_titles; box++) 
		FixBox( tree, box );

}

do_gterm()
{
	if( !InGemApp ) return;

	/*
	 * We are terminating the USER GEM application
	 * If there is a menu, erase it
	 */

	if( UsrMenu ) {
		menu_bar( UsrMenu->tree, 0 );
		UsrMenu = 0L;
	}

#ifndef SAI
	/*
	 * Go back and redraw our menu
	 */
	menu_bar( gl_menu, 1 );
#endif SAI

	MenuCtrl( FALSE );

	InGemApp = FALSE;

}

NeedsGem()
{
	if( !InGemApp ) {
		/*
		 * Tell the user he should have done a GemStart
		 */
		do_ginit();
	}

}

do_ginit()
{
	if( InGemApp ) return;

	UsrMenu = 0L;

	MenuCtrl( TRUE );		/* Turn on the mouse */

#ifndef SAI
	/*
	 * Erase our menu
	 */
	menu_bar( gl_menu, 0 );
#endif SAI

	InGemApp = TRUE;

}

GetMNumber( index, title, item, mnemonic )
int index;
int *title;
int *item;
int *mnemonic;
{
	register int i, j;
	int box;
	register OBJECT *tree;
	int head, tail;

	/*
	 * Convert an object tree index into the menu number and
	 * the item number
	 */

	/*
	 * Nothing to do but to try each menu sequentially
	 */

	if( !UsrMenu ) {
		run_error( ER( 349, "nouser`No User Menu") );
	}

	tree = UsrMenu->tree;

	for( i=0; i<UsrMenu->num_titles; i++ ) {
		box = FindIndex( tree, i );
		head = tree[box].ob_head;
		tail = tree[box].ob_tail;
		j = 0;
		for( ;; ) {
			if( head == index ) {
				*title = i;
				*item = j;
				*mnemonic = tree[index].ob_spec >> 24;
				return;
			}
			if( head == tail ) break;
			j++;
			head = tree[head].ob_next;
		}
	}
	/*
	 * We have looked through all the menus, but haven't found the 
	 * index, something is VERY WRONG
	 */
	run_error( ER(331, "bug`Can't find index in menu tree") );

}

/*
 * Allocate storage for the string, break the line up into separate items
 * and check for the mnemonic codes
 */

int
ParseItems( ItemPtr, items )
char *ItemPtr[];
char *items;
{
	char *str;
	int num_items = 0;
	int i;
	unsigned char code;
	char *colon;
	extern char *strchr();

	str = newStrAlloc( items );

	while( *str ) {

		ItemPtr[ num_items++ ] = str;
		while( *str && *str != '|' ) str++;
		if( *str == '|' ) {
			*str = 0;
			str++;
		}
		if( num_items == MAX_ITEMS ) break;
	}

	/*
	 * Handle the mnemonic code
 	 */
	for( i=0; i<num_items; i++ ) {
		str = ItemPtr[i];
		code = 0;
		if( str[0] == '-' ) {
			/* Dashes, do nothing */
		} else if( str[1] == ':' && strlen(str) > 2 ) {
			code = str[0];
			str += 2;
		} else if( str[0] == '#' ) {
			/*
			 * Get a pointer to the ':'
			 */
			colon = strchr( str, ':' );
			if( colon ) {
				*colon = 0;
				code = atoi( str+1 );
				str = colon + 1;
			}
		}
		ItemPtr[i] = (char *)((long)str |(((long)code) << 24));
	}

	return num_items;

}
