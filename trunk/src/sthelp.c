#include "alice.h"
#include <obdefs.h>
#include <gemdefs.h>
#include <osbind.h>

#include "alrsc.h"

extern long MkSpec();
extern FILE *hopen();
/*
 * Pop Help routines, pops a window (actually, just a box)
 * and display the help file
 */
extern int LoRes;

#define		M_HELP		0
#define		M_BORDER 	1
#define		M_BUTTON	2

#define		X_ITEMS 	3

extern char *allocstring();
extern char *checkalloc();

int
helpfile( dir, topic )
char *dir;
char *topic;
{
	char *ptr;
	int i;
	OBJECT *obj_array;
	OBJECT *p;
	int hlp;
	int fo_cx, fo_cy, fo_cw, fo_ch;
	extern int charHeight, charWidth;
	int hwidth, hheight;
	int curx, cury;
	int HadMenu;
	char **Files;
	char **Items;
	char *menutitle;
	int numfiles;
	int exitobj, picked;
	OBJECT *box;
	int BoxWidth;
	OBJECT *xref_array;
	TEDINFO *ted;
	long tree;
	char line[100];
	extern OBJECT *ObjAlloc();
	extern int vdi_handle;

	/*
	 * helpfile() jumps here after xrefing 
	 */
#ifndef LORESHELP
	if( LoRes ) {
		form_alert( 1, LDS( 372,
"[2][Sorry, Help is not|available in Low|Resolution - Use|Medium Resolution][ OK ]" ));
		return;
	}
#endif

	/* 30 - maximum number of xref lines */
	Files = (char **) checkalloc( 30 * sizeof( char * ) );
	Items = (char **) checkalloc( 30 * sizeof( char * ) );

more_help:

	/*
	 * Only have to allocate two objects: the box and the border
	 */

	if( hopen( dir, topic ) == NULL ) {
no_help:
		if( form_alert( 1, LDS( 373, 
"[1][Sorry, I can't find|help on that.|Make sure help files|are accessible.][ Cancel | Retry ]" ))
		 == 2 ) goto more_help;
		
		goto free_stuff;
	}

	/*
	 * If we are in LoRes, go into MedRes
	 */
#ifdef LORESHELP
	if( LoRes ) {
		Setscreen( -1L, -1L, 1 );
	}
#endif

	/*
	 * Now get the shape of the help
	 */
	hfgets( line, 80, (FILE *)NULL );

	/* see if the first char is a '|' */
	if( *line == '|' ) {

		hheight = atoi( line+1 );
		ptr = line;
		while( *ptr != ',' ) ptr++;
		hwidth = atoi( ptr+1 );

	} else {
		close_help();
		if( !init_help() )
			goto no_help;
		else
			goto more_help;
	}

	/*
	 * Get two objects for the box behind the help
	 */
	obj_array = ObjAlloc( 4 );

	/*
	 * Turn off the mouse
	 */
	MouseOn( FALSE );

	/* 
	 * Make the first object the root, and make it a box 
 	 */
	p = &obj_array[M_HELP];

	p->ob_type = G_BOX;
	p->ob_spec = MkSpec( 0, 1, BLACK, BLACK, MD_TRANS, IP_HOLLOW, WHITE );

	/* Initially, origin it at 0,0 then do a form_center */
	p->ob_x = 0;
	p->ob_y = 0;

	/* Make it a bit bigger than needed */
	p->ob_width = charWidth * (hwidth+2);
	p->ob_height = charHeight * (hheight+1) + 6;

	/* Make the border now */
	p = &obj_array[M_BORDER];
	p->ob_type = G_BOX;
	p->ob_x = 3;
	p->ob_y = 3;
	p->ob_width = obj_array[M_HELP].ob_width - 6;
	p->ob_height = obj_array[M_HELP].ob_height - 6;
	p->ob_spec = MkSpec( 0, 2, BLACK, BLACK, MD_TRANS, IP_HOLLOW, WHITE );
	objc_add( obj_array, M_HELP, M_BORDER );

	/* Now, center the box at the bottom of the screen */
	form_bottom( obj_array, &fo_cx, &fo_cy, &fo_cw, &fo_ch );

	form_dial( 0, 0, 0, 0, 0, fo_cx, fo_cy, fo_cw, fo_ch );

	/* Draw the dialog */
	DrawObj( obj_array, 0 );

	/* Get the physical position of the border */
	objc_offset( obj_array, M_BORDER, &curx, &cury );

	curx += 5;
	cury += charHeight + 2;

	/*
	 * Now we go and draw the text
	 */

	BoxWidth = 0;
	numfiles = 0;
	HadMenu = FALSE;

	while( hfgets( line, 80, (FILE *)NULL) ) {

		RemoveNL( line );
		if( *line == '|' ) {

			switch( line[1] ) {
				case 'm':
					menutitle = allocstring( line+2 );
					BoxWidth = max( strlen(menutitle) * 
						charWidth + 5, BoxWidth);
					break;
				case 'i':
					ptr = line;
					while( *ptr != ',' ) ptr++;
					*ptr = 0;
					Items[numfiles] = allocstring(line+2);
					BoxWidth = max( strlen(Items[numfiles])
						* charWidth + 5, BoxWidth );
					Files[numfiles] = allocstring(ptr+1);
					numfiles++;
					break;
				case 'p':
					HadMenu = TRUE;
					break;
			}
	
		} else {
			v_gtext( vdi_handle, curx, cury, line );
			cury += charHeight;
		}
	}
	MouseOn( TRUE );

	if( HadMenu ) {

		/* Now that the lovely help is all over the screen, give the
		 * user a chance to xref to another screen
	 	 */
		xref_array = ObjAlloc( numfiles + X_ITEMS + 2 );

		rsrc_gaddr( R_TREE, HELPXREF, &tree );
		blk_move( xref_array, tree, 3 * sizeof( OBJECT ) );

		for( i=0; i<3; i++ ) {
			box = &xref_array[i];
			box->ob_flags &= ~LASTOB;
		}

		/* Fix the surrounding box */
		box = xref_array;
		box->ob_width = BoxWidth + (2 * charWidth);
		box->ob_height = (charHeight) * (numfiles+2) + (charHeight/2);

		objc_offset( obj_array, M_BORDER, &curx, &cury );

		box->ob_x = curx + obj_array[M_BORDER].ob_width -
		    	    box->ob_width - 5;
		box->ob_y = cury + obj_array[M_BORDER].ob_height
		    	    - box->ob_height - 5;


		/* Fix the title */
		p = &xref_array[HXTEXT];
		p->ob_width = box->ob_width;
		p->ob_height = 2 * charHeight;

		/* Center the string */
		p->ob_x = 0;
		p->ob_y = 0;
		p->ob_type = G_TEXT;
		ted = p->ob_spec;
		ted->te_ptext = menutitle;

		/*
		 * Now fix the box containing the XRef's
		 */
		p = &xref_array[HXREFS];
		p->ob_x = charWidth;
		p->ob_y = 2 * charHeight;
		p->ob_width = BoxWidth;
		p->ob_height = charHeight * numfiles;

		/* Now make each of the xref buttons */
		for( i=0; i<numfiles; i++ ) {
			p = &xref_array[X_ITEMS+i];
			objc_add( xref_array, HXREFS, X_ITEMS+i );
			/* Center it left/right */
			p->ob_x = 0;
			p->ob_y = charHeight * i;
			p->ob_width = BoxWidth;
			p->ob_height = charHeight;
			p->ob_type = G_STRING;
			p->ob_spec = Items[i];
			p->ob_flags = SELECTABLE | EXIT;
			if( i == 0 )
				p->ob_flags |= DEFAULT;
			if( i == (numfiles - 1) )
				p->ob_flags |= LASTOB;
			p->ob_state = NORMAL;
		}

		/*
		 * Now interact with said dialog
		 */
		DrawObj( xref_array, 0 );

		exitobj = form_do( xref_array, -1 );
		exitobj &= 0x7f;

		/* User selected one of the items */
		picked = exitobj - X_ITEMS;
		if( picked < 0 || picked >= numfiles )
			picked = 0;

		strcpy( line, Files[picked] );

		/* Free up the memory */
		for( i=0; i<numfiles; i++ ) {
			mfree( Files[i] );
			mfree( Items[i] );
		}
		mfree( menutitle );
		mfree( xref_array );

/*
 * Finished with this help screen, perhaps go onto the next
 */
		/* If we were in lo-res, switch back to it */

#ifdef LORESHELP
		if( LoRes ) {
			Setscreen( -1L, -1L, 0 );
		}
#endif
		if( *line == 'H' ) {
			/* 
			 * In order to make things not recurse too much
			 * simply jump to yourself
			 */
			dir = line+1;
			topic = "";
			mfree( obj_array );

			/* Can get rid of old box */
			form_dial( 3, 0, 0, 0, 0, fo_cx, fo_cy, fo_cw, fo_ch );
			draw_all();

			goto more_help;

			/*
			 * helpfile( &Files[picked][1], "" );
			 */

		} else {
			/* Do the command */
			menuact( "", line );
		}
		/* End of code executed when there is a menu */

	} else {

		/* There is no menu, just return after user clicks on
		 * box
		 */
		char *ret_ed = LDS( 374, " OK " );

		/* Fix the surrounding box */
		box = &obj_array[M_BUTTON];
		box->ob_width = (strlen( ret_ed ) + 2) * charWidth;
		box->ob_height = charHeight + (charHeight/2);

		box->ob_x = obj_array[M_BORDER].ob_width -
		    	    box->ob_width - 8;
		box->ob_y = obj_array[M_BORDER].ob_height
		    	    - box->ob_height - 8;

		box->ob_spec = ret_ed;
		box->ob_type = G_BUTTON;
		box->ob_flags = DEFAULT | SELECTABLE | EXIT;
		box->ob_state = 0;
		objc_add( obj_array, M_BORDER, M_BUTTON );

		DrawObjPlus( obj_array, M_BUTTON, charWidth / 2 );
		form_do( obj_array, -1 );
	}

	/* Tell the AES to redraw what was clobbered */
	form_dial( 3, 0, 0, 0, 0, fo_cx, fo_cy, fo_cw, fo_ch );
	draw_all();
	mfree( obj_array );
free_stuff:
	mfree( Items );
	mfree( Files );
}

/*
 * Draw an object tree, plus a little bit
 */

DrawObjPlus( obj, item, offset )
OBJECT *obj;
int item;
int offset;
{
	int fo_cx, fo_cy, fo_cw, fo_ch;

	objc_offset( obj, item, &fo_cx, &fo_cy );
	fo_cw = obj[item].ob_width;
	fo_ch = obj[item].ob_height;

	fo_cx -= offset;
	fo_cy -= offset;
	fo_cw += 2 * offset;
	fo_ch += 2 * offset;

	objc_draw( obj, item, MAX_DEPTH, fo_cx, fo_cy, fo_cw, fo_ch );
}

RemoveNL( line )
char *line;
{
	while( *line ) line++ ;
	line--;
	if( *line == '\n' ) *line = 0;
}

#ifndef RELEASE
checkhelp()
{

	FILE *fp;
	char line[100];
	char msg[100];

	fp = fopen( "alice.lst", "r" );

	if( fp == (FILE *)0 ) return;

	while( fgets( line, 80, fp ) ) {
		RemoveNL( line );
		sprintf( msg, "[2][Help file -|%s][ OK | Abort ]", line );
		if( form_alert( 1, msg ) == 2 ) break;
		helpfile( line, "" );
	}
	fclose( fp );
}
#endif
