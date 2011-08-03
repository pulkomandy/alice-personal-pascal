
#include <stdio.h>
#include <obdefs.h>
#include <gemdefs.h>
#include "search.h"
#include "alrsc.h"

do_gsearch( mode )
int mode;
{
	OBJECT *tree;
	int	exit_obj;
	int	fo_cx, fo_cy, fo_ch, fo_cw;
	TEDINFO *ted;
	extern char SrchPattern[];

	/*
	 * First of all, get the address of the 'search' dialog
	 */
	rsrc_gaddr( R_TREE, SEARCH, &tree );

	switch( mode ) {
		case 0:
			/* Leave the last one selected */
			break;
		case SRCH_FWD:
			tree[SRCHFWD].ob_state |= SELECTED;
			tree[SRCHBACK].ob_state &= ~SELECTED;
			break;
		case SRCH_BACK:
			tree[SRCHFWD].ob_state &= ~SELECTED;
			tree[SRCHBACK].ob_state |= SELECTED;
			break;
	}

	/*
	 * Get a pointer into the TEDINFO struct for the prefix
	 */
	ted = (TEDINFO *)tree[SRCHTEXT].ob_spec;

	ted->te_ptext = SrchPattern;

	form_bottom( tree, &fo_cx, &fo_cy, &fo_cw, &fo_ch );
	scr_save( fo_cx, fo_cy, fo_cw, fo_ch );

	/*
	 * Draw the initial tree
	 */
	DrawObj( tree, 0 );

	exit_obj = form_do( tree, SRCHTEXT );
	exit_obj &= 0x7f;
	tree[exit_obj].ob_state = 0;

	scr_restore( fo_cx, fo_cy, fo_cw, fo_ch );

	if( exit_obj == SRCHCNCL )
		return;

	if( tree[SRCHFWD].ob_state & SELECTED ) 
		search( SRCH_FWD );
	else
		search( SRCH_BACK );

}

