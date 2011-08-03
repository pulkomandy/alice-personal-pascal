 
#include <stdio.h>
#include <whoami.h>
#include <gemdefs.h>
#include <obdefs.h>
#include <osbind.h>

#define TRUE (1)
#define FALSE (0)
#define NIL -1

/*
 * Draw an object and whatever falls within that object
 */
DrawObj( obj, item )
OBJECT *obj;
int item;
{
	int fo_cx, fo_cy, fo_cw, fo_ch;

	objc_offset( obj, item, &fo_cx, &fo_cy );
	fo_cw = obj[item].ob_width;
	fo_ch = obj[item].ob_height;

	objc_draw( obj, item, MAX_DEPTH, fo_cx, fo_cy, fo_cw, fo_ch );

}

#ifdef DUMP_OBJ

char *ObTypes[] = {
	"G_BOX",
	"G_TEXT",
	"G_BOXTEXT",
	"G_IMAGE",
	"G_PROGDEF",
	"G_IBOX",
	"G_BUTTON",
	"G_BOXCHAR",
	"G_STRING",
	"G_FTEXT",
	"G_FBOXTEXT",
	"G_ICON",
	"G_TITLE",
	0 };

DumpTEDInfo( fp, ted )
FILE *fp;
TEDINFO *ted;
{

	pr(fp);
	fprintf( fp, "\nDump of the Tedinfo part:\n" );


	if( ted->te_ptext ) {
		pr(fp);
		fprintf( fp, "PText: '%s'\n", ted->te_ptext );
	}

	pr(fp);
	fprintf( fp, "PLength: %d\n", ted->te_txtlen );

	if( ted->te_ptmplt ) {
		pr(fp);
		fprintf( fp, "Template: '%s'\n", ted->te_ptmplt );
	}

	pr(fp);
	fprintf( fp, "TmpLen: %d\n", ted->te_tmplen );

	if( ted->te_pvalid ) {
		pr(fp);
		fprintf( fp, "PValid: '%s'\n", ted->te_pvalid );
	}

	pr(fp); fprintf( fp, "Font: %d\n", ted->te_font );
	pr(fp); fprintf( fp, "Justification: %d\n", ted->te_just );
	pr(fp); fprintf( fp, "Color: 0x%x\n", ted->te_color );
	pr(fp); fprintf( fp, "Thickness: %d\n\n", ted->te_thickness );
	
}

int indent = 0;

pr(fp)
FILE *fp;
{
	int i;
	for( i=0; i<indent; i++ ) fprintf( fp, "     " );
}

DumpTree( root, fname )
OBJECT *root;
char *fname;
{
	FILE *fp;
	FILE *fopen();

	fp = fopen( fname, "w" );
	if( fp == NULL ) return;

	fprintf( fp, "Dump of tree to %s\n\n", fname );
	indent = 0;

	DumpNode( 0, 0, root, 0, fp );

	fclose( fp );
}

DumpUser() {}
DumpIcon() {}
DumpImage() {}

DumpColor( fp, color )
FILE *fp;
long color;
{
	pr(fp); fprintf(fp, "Inside color: %d\n", (int)color & 0xf );
	color >>= 4;
	pr(fp); fprintf(fp, "Fill Pattern: %d\n", (int)color & 0x7 );
	color >>= 3;
	pr(fp); fprintf(fp, "Text Mode: %d\n", (int)color & 0x1 );
	color >>= 1;
	pr(fp); fprintf(fp, "Text Color: %d\n", (int)color & 0xf );
	color >>= 4;
	pr(fp); fprintf(fp, "Border Color: %d\n", (int)color & 0xf );
	color >>= 4;
	pr(fp); fprintf(fp, "Thickness: %d\n", (int)color & 0xff );
	color >>= 8;
	pr(fp); fprintf(fp, "Character: %d = '%c'\n", (int)color & 0xff, 
		(int)color & 0xff );
}

DumpNode( x, y, root, ind, fp )
int x,y;	/* Node is based at (x,y) */
OBJECT *root;
int ind;
FILE *fp;
{
	OBJECT *tree;
	int obj;

	tree = &root[ind];

	pr(fp);
	fprintf( fp, "\nDump of object %d of tree @ %lx\n\n", ind, root );

	pr(fp);
	fprintf( fp, "Next: %d, Head: %d, Tail: %d\n", tree->ob_next,
		tree->ob_head, tree->ob_tail );

	pr(fp);
	fprintf( fp, "Type: %s\n", ObTypes[tree->ob_type - G_BOX] );

	pr(fp);
	fprintf( fp, "Flags: " );
	if( tree->ob_flags & SELECTABLE ) {
		fprintf( fp, "SELECTABLE " );
	}
	if( tree->ob_flags & DEFAULT ) {
		fprintf( fp, "DEFAULT " );
	}
	if( tree->ob_flags & EXIT ) {
		fprintf( fp, "EXIT " );
	}
	if( tree->ob_flags & EDITABLE ) {
		fprintf( fp, "EDITABLE " );
	}
	if( tree->ob_flags & RBUTTON ) {
		fprintf( fp, "RBUTTON " );
	}
	if( tree->ob_flags & LASTOB ) {
		fprintf( fp, "LASTOB " );
	}
	if( tree->ob_flags & TOUCHEXIT ) {
		fprintf( fp, "TOUCHEXIT " );
	}
	if( tree->ob_flags & HIDETREE ) {
		fprintf( fp, "HIDETREE " );
	}
	if( tree->ob_flags & INDIRECT ) {
		fprintf( fp, "INDIRECT " );
	}
	if( tree->ob_flags == 0 ) {
		fprintf( fp, "NONE" );
	}
	fprintf( fp, "\n" );

	pr(fp);
	fprintf( fp, "States: " );
	if( tree->ob_state & SELECTED ) {
		fprintf( fp, "SELECTED " );
	}
	if( tree->ob_state & CROSSED ) {
		fprintf( fp, "CROSSED " );
	}
	if( tree->ob_state & CHECKED ) {
		fprintf( fp, "CHECKED " );
	}
	if( tree->ob_state & DISABLED ) {
		fprintf( fp, "DISABLED " );
	}
	if( tree->ob_state & OUTLINED ) {
		fprintf( fp, "OUTLINED " );
	}
	if( tree->ob_state & SHADOWED ) {
		fprintf( fp, "SHADOWED " );
	}
	if( tree->ob_state == 0 ) {
		fprintf( fp, "NORMAL" );
	}
	fprintf( fp, "\n" );

	pr(fp);
	fprintf( fp, "ObSpec: 0x%lx\n", tree->ob_spec );

	pr(fp);
	fprintf( fp, "x: %d, y: %d, w: %d, h: %d\n",
		tree->ob_x, tree->ob_y,
		tree->ob_width, tree->ob_height );

	pr(fp); fprintf( fp, "Absolute @ (%d, %d), (%d, %d)\n", 
		x + tree->ob_x, y + tree->ob_y,
		x + tree->ob_x + tree->ob_width, 
		y + tree->ob_y + tree->ob_height );

	/* If we are dumping something with a TEDInfo, dump it */
	indent++;
	switch( tree->ob_type ) {
		case G_TEXT:
		case G_BOXTEXT:
		case G_FTEXT:
		case G_FBOXTEXT:
			DumpTEDInfo( fp, tree->ob_spec );
			break;

		case G_BOX:
		case G_IBOX:
		case G_BOXCHAR:
			DumpColor( fp, tree->ob_spec );
			break;

		case G_IMAGE:
			DumpImage( fp, tree->ob_spec );
			break;

		case G_USERDEF:
			DumpUser( fp, tree->ob_spec );
			break;

		case G_BUTTON:
		case G_STRING:
		case G_TITLE:
			pr(fp); fprintf( fp, "Text = %s\n", tree->ob_spec );
			break;

		case G_ICON:
			DumpIcon( fp, tree->ob_spec );
			break;
		
	}
	indent--;

	if( tree->ob_head != -1 ) {
		indent++;
		/* Go to the left kid, and dump until we hit the tail */
		obj = tree->ob_head;
		for(;;) {
			DumpNode( x + tree->ob_x, y + tree->ob_y,
				  root, obj, fp );
			if( obj == tree->ob_tail ) break;
			obj = root[obj].ob_next;
		}
		indent--;
	}

}

#endif

/*
 * This routine just takes a tree, and an index, and sets up the various
 * fields to default values
 */

InitObj( root, ind )
OBJECT *root;
int ind;
{
	OBJECT *tree;

	tree = &root[ind];
	tree->ob_next = -1;
	tree->ob_head = -1;
	tree->ob_tail = -1;
	tree->ob_type = 0;	/* Not a valid type */
	tree->ob_flags = NONE;	/* zero out the flags */
	tree->ob_state = NORMAL;	/* state is NORMAL */
	tree->ob_spec = 0L;	/* no ob_spec for now */
	tree->ob_x = 0;
	tree->ob_y = 0;
	tree->ob_width = 0;
	tree->ob_height = 0;
}

#ifndef SAI
long
MkSpec( ch, thick, border, text, md, fillp, inside )
char ch;
int thick, border, text, md, fillp, inside;
{
	register long val;

	val = ch & 0xff;
	val <<= 8;
	val |= thick & 0xff;
	val <<= 4;
	val |= border & 0xf;

	val <<= 4;
	val |= text & 0xf;
	val <<= 1;
	if( md == MD_TRANS ) {
		val |= 0;
	} else  {
		val |= 1;
	}
	val <<= 3;
	val |= fillp & 0x7;
	val <<= 4;
	val |= inside;

	return val;
}

/*
 * Allocate an object tree of 'num' nodes
 */

OBJECT *
ObjAlloc( num )
int num;
{
	int i;
	OBJECT *ptr;
	extern char *checkalloc();

	ptr = checkalloc( sizeof( OBJECT ) * num );
	if( !ptr ) return ptr;

	for( i=0; i<num; i++ ) InitObj( ptr, i );
	ptr[num-1].ob_flags |= LASTOB;
	return ptr;
}


int
hndl_dial( tree, def, x, y, w, h )
OBJECT *tree;
int def;
int x, y, w, h;
{
	int	xdial, ydial, wdial, hdial;
	int	exit_obj;

	form_center( tree, &xdial, &ydial, &wdial, &hdial );
	scr_save( xdial, ydial, wdial, hdial );
	form_dial( 1, x, y, w, h, xdial, ydial, wdial, hdial );
	objc_draw( tree, ROOT, MAX_DEPTH, xdial, ydial, wdial, hdial );
	exit_obj = form_do( tree, def );
	form_dial( 2, x, y, w, h, xdial, ydial, wdial, hdial );
	scr_restore( xdial, ydial, wdial, hdial );
	return exit_obj;
}

#endif SAI

int
inside(x, y, pt)		/* determine if x,y is in rectangle	*/
	int		x, y;
	GRECT		*pt;
	{
	if ( (x >= pt->g_x) && (y >= pt->g_y) &&
	    (x < pt->g_x + pt->g_w) && (y < pt->g_y + pt->g_h) )
		return(TRUE);
	else
		return(FALSE);
	} 

					/* If the object is SELECTED,	*/
objc_dsel(tree, obj)			/* deselect it.			*/
	OBJECT  *tree;
	int	obj;
	{
	if (tree[obj].ob_state & SELECTED)
		objc_toggle(tree, obj);
	}

objc_toggle(tree, obj)			/* Reverse the SELECT state */
	OBJECT  *tree;			/* of an object, and redraw */
	int	obj;			/* it immediately.	    */
	{
	int	state, newstate;
	GRECT	root, ob_rect;

	objc_xywh(tree, ROOT, &root);
	state = tree[obj].ob_state;
	newstate = state ^ SELECTED;
	objc_change(tree, obj, 0, root.g_x, root.g_y, 
		root.g_w, root.g_h, newstate, 1);
	}

objc_xywh(tree, obj, p)			/* through 'p'			*/
	OBJECT	*tree;
	int	obj;
	GRECT	*p;
	{
	objc_offset(tree, obj, &p->g_x, &p->g_y);
	p->g_w = tree[obj].ob_width;
	p->g_h = tree[obj].ob_height;
	}

rc_copy( src, dst )		/* copy source to destination rectangle	*/
register long *src;
register long *dst;
{
	*dst++ = *src++;
	*dst++ = *src++;
}

/*
 * Test whether two rectangles are equal
 */
int
rc_equal( p1, p2 )
register long *p1, *p2;
{
	if( *p1++ != *p2++ ) return FALSE;
	return *p1 == *p2;
}

int
rc_intersect(p1, p2)		/* compute intersection of two GRECTs	*/
register GRECT		*p1, *p2;
{
	register int tx, ty, tw, th;

	tw = min(p2->g_x + p2->g_w, p1->g_x + p1->g_w);
	th = min(p2->g_y + p2->g_h, p1->g_y + p1->g_h);
	tx = max(p2->g_x, p1->g_x);
	ty = max(p2->g_y, p1->g_y);
	p2->g_x = tx;
	p2->g_y = ty;
	p2->g_w = tw - tx;
	p2->g_h = th - ty;
	return( (tw > tx) && (th > ty) );
	}

form_bottom( tree, x, y, w, h )
OBJECT *tree;
int *x, *y, *w, *h;
{
	extern GRECT scr_area;

	/* We want to center the dialog left to right */
	tree->ob_x = *x = scr_area.g_x + (scr_area.g_w / 2) -
		     (tree->ob_width / 2);
	tree->ob_y = *y = scr_area.g_y + scr_area.g_h - tree->ob_height - 5;
	*w = tree->ob_width;
	*h = tree->ob_height;
}

 
