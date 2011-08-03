
#include "alice.h"
#include <curses.h>
#include "menu.h"
#include "alctype.h"

#include <obdefs.h>
#include <gemdefs.h>

#include "alrsc.h"

int ScLines[] = { SCSTR1, SCSTR2, SCSTR3, SCSTR4, SCSTR5, SCSTR6, SCSTR7,
		  SCSTR8, SCSTR9, SCSTR10, -1 };

extern int LoRes;

#ifdef DB_POP
# define PRINTT
#endif
#include "printt.h"

#define CASELESS(X)  ((X) | 0x20)
strcompare( xstr1, xstr2 )
char **xstr1;
char **xstr2;
{
	register char *s1 = *xstr1;
	register char *s2 = *xstr2;

	while( *s1 && *s2 ) {
		if( CASELESS(*s1) != CASELESS( *s2 ) )
			if( CASELESS(*s1) < CASELESS(*s2) )
				return -1;
			else
				return 1;

		s1++, s2++;
	}
	if( CASELESS(*s1) == CASELESS(*s2) ) return 0;

	return  CASELESS(*s1) < CASELESS(*s2);
}

/*
 */

char *
pop_menu(menu, helpfunc, sort, str, offset )
struct menu_data *menu;        /* info to control style of menu  */
int (*helpfunc)();      /* help function for this menu */
int sort;
char **str;
int *offset;
{
	extern int MenuState;
	OBJECT *tree;
	int  n;		/* The number of items displayed */
	int  i;
	int  nlines;
	int  exit_obj;
	int  pos;
	int  first;
	int  fo_cx, fo_cy, fo_cw, fo_ch;
	int  cur_x, cur_y;
	int  slid_x, slid_y;
	int  nargs;
	OBJECT *p;
	int  above;
	TEDINFO *ted, *title_ted;
	int	selected;
	extern  int charWidth;
	char *the_str;
	int mflag = MenuState;

#ifdef DEBUG
	char	msgbuf[80];
#endif

	MenuCtrl( TRUE );

	/*
	 * Count the number of possible symbols
	 */
	n = nargs = menu->num_items;

	if( sort ) {
		shellsort( menu->item, n, sizeof( char * ), strcompare );
	}

	printt5( "PopMenu(%lx,%lx,%d,%lx,%lx)\n",
		menu, helpfunc, sort, str, offset );

	printt1( "There are %d items in the menu\n", n );

#ifdef DB_POP
	for( i=0; i<n; i++ ) {
		printt2( "Item %d: %s\n", i, menu->item[i] );
	}
#endif

	/*
	 * First of all, get the address of the 'complete' dialog
	 */
	if( LoRes )
		rsrc_gaddr( R_TREE, LOCOMPLE, &tree );
	else
		rsrc_gaddr( R_TREE, COMPLETE, &tree );

	title_ted = (TEDINFO *) tree[SCTITLE].ob_spec;
	title_ted->te_ptext = menu->m_title;
	title_ted->te_txtlen = strlen(menu->m_title);

	form_center( tree, &fo_cx, &fo_cy, &fo_cw, &fo_ch );

	scr_save( fo_cx, fo_cy, fo_cw, fo_ch );

	nlines = 0;
	for( i=0; ScLines[i] != -1; i++ ) {
		p = &tree[ScLines[i]];
		p->ob_spec = (long)"";
		p->ob_state = 0;
		nlines++;
	}

	printt2( "There are %d selections and %d lines\n", nargs, nlines );

	/*
	 * Draw the initial tree
	 */
	if( helpfunc ) {
		tree[SCOK].ob_flags = SELECTABLE;
		tree[SCOK].ob_state = 0;
	} else {
		tree[SCOK].ob_flags = SELECTABLE;
		tree[SCOK].ob_state = DISABLED;
	}

	DrawObj( tree, 0 );


	/*
	 * Now we have the list of symbols available, set up the
	 * object tree
	 */

	printt2( "There are %d current selections and %d lines\n",
		n, nlines );

	/* Calculate the size of the slider box */
	if( n <= nlines ) 
		tree[SCSLIDER].ob_height = tree[SCSLIDBX].ob_height;
	else
		tree[SCSLIDER].ob_height = tree[SCSLIDBX].ob_height
					 * nlines / n;

	tree[SCSLIDER].ob_height = min( tree[SCSLIDBX].ob_height,
			tree[SCSLIDER].ob_height );

	printt1( "Height of slider = %d\n", tree[SCSLIDER].ob_height );

	/* Calculate the position of the box */
	tree[SCSLIDER].ob_y = 0;
	tree[SCSLIDER].ob_x = 0;

	first = 0;
	for(;;) {
		/*
		 * For each visible line, pick a symbol
		 */
		for( i=0; ScLines[i] != -1; i++ ) {
			p = &tree[ScLines[i]];
			if( (first+i) < n ) {
				p->ob_spec = (long)menu->item[first+i];
				p->ob_flags |= SELECTABLE | EXIT;
			} else {
				p->ob_spec = (long)"";
				p->ob_flags = 0;
			}
			p->ob_state = 0;
		}
		/*
		 * Draw the selections
		 */
		DrawObj( tree, SCITEMS );

		/*
		 * Draw the right slider and box
		 */
		DrawObj( tree, SCSLIDBX );
		DrawObj( tree, SCSLIDER );

		exit_obj = form_do( tree, -1 );
		exit_obj &= 0x7f;

		printt1( "form_do exits with obj %d\n", exit_obj );

		switch( exit_obj ) {
		  case SCUP:
			printt0( "Scroll Up\n" );
			first--;
			break;

		  case SCDOWN:
			printt0( "Scroll Down\n" );
			first++;
			break;

		  case SCOK:
			/* What does OKAY mean ? */
			/* Can click on okay when only 1 selected */
			printt0( "Okay\n" );
			break;

		  case SCCANCEL:
			printt0( "Cancel\n" );
			selected = -1;
			objc_dsel( tree, SCCANCEL );
			goto pop_finish;

		  case SCSLIDER:
			printt0( "Slider\n" );
			/* Clicked in slider box, watch it */
			pos = graf_slidebox( tree, SCSLIDBX,
				SCSLIDER, 1 );
			first = (int)( (long)(n - nlines) * 
			   	(long)(pos) / 1000L);
			break;

		  case SCSLIDBX:
			/* Clicked in the surrounding box,
			 * figure out if it was above or below
			 * the slider
			 */
			printt0( "Page Up/Down\n" );
			graf_mkstate( &cur_x, &cur_y, &pos, &pos );
			objc_offset( tree, SCSLIDER, &slid_x, &slid_y);
			above = cur_y < slid_y;
			if( above ) 
				first -= nlines;
			else
				first += nlines;
			break;
		  default:
			printt0( "Selection of an object\n" );
			/* Must be a SCSTR1-9 */
			for( i=0; ScLines[i] != -1; i++ ) {
				if( ScLines[i] == exit_obj ) break;
			}
			selected = first + i;
			if( tree[SCOK].ob_state & SELECTED ) {
				if( helpfunc )
					(* helpfunc)( selected, menu );
				DrawObj( tree, 0 );
			} else
				goto pop_finish;
		}
		first = min( first, n - nlines );
		first = max( first, 0 );

		/* We have changed the selections
		 */
		if( n > nlines ) 
			tree[SCSLIDER].ob_y = 
				(tree[SCSLIDBX].ob_height -
				 tree[SCSLIDER].ob_height )
					 * first / (n - nlines);
		else
			tree[SCSLIDER].ob_y = 0;

	}

pop_finish:
	/*
	 * If there was an item selected, then return it
	 */
	printt1( "Item %d selected\n", selected );

	if( selected >= 0 ) {
		the_str = menu->item[selected];
		printt1( "The selection = <%s>\n", the_str );
	} else {
		printt0( "selected < 0, the_str == NULL\n" );
		the_str = (char *)0;
	}

	if( offset )
		*offset = selected;

	if( str )
		*str = the_str;

	scr_restore( fo_cx, fo_cy, fo_cw, fo_ch );

	MenuCtrl( mflag );

	return the_str;		
}

