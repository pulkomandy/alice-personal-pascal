
/*
 *DESC: Control routines for the one line editor, used as the core of input
 */

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "ole.h"
#include "token.h"
#include "flags.h"
#include "alctype.h"
#include "class.h"

#ifdef DB_OLEDIT
# define PRINTT
#endif
#include "printt.h"

buf_el	OLE_TextBuf[ MAX_LEN_LINE + 3 ];
buf_el	*OLE_StartCursor;	/* The position at which the cursor starts */
buf_el	*OLE_EndCursor;		/* The position at which the cursor ends */

buf_el	*OLE_PrcText, *OLE_CurText, *OLE_FolText;

char	OLE_Result[ MAX_LEN_LINE ];
extern int domalloc;
extern	buf_el line_buffer[];
extern	alice_window	*curr_window;

#define OLE_IN_PREC	0
#define OLE_IN_CUR	1
#define OLE_IN_FOL	2

#ifdef DEBUG
static
PrintBufEl( ptr )
buf_el *ptr;
{
	while( *ptr ) printt1( "%c", *ptr++ & 0x7f );
}
#endif

OLE_ChopUpText( curs, in_text )
curspos curs;
int in_text;	/* Are we typing in for the first time ? */
{
	register buf_el *p, *dst;

	int texttype = OLE_IN_PREC;
	int addspace;

	addspace = (in_text && (ntype(curs) != N_STUB));

	/* Copy the text from treeprint, breaking it up into three parts.
	 * OLE_StartCur and EndCur point into the places in line_buffer
	 * where the cursor starts and ends.
	 */

	printt0( "Chopping up the text\n" );
	printt1( "OLE_StartCursor @ %lx\n", (long)OLE_StartCursor );
	printt1( "OLE_EndCursor @ %lx\n", (long)OLE_EndCursor );

	dst = OLE_TextBuf;

	/* Preceding text is first */
	OLE_PrcText = dst;
	OLE_FolText = (buf_el *)0;

	p = line_buffer;

	while( *p ) {

		switch( texttype ) {
			case OLE_IN_PREC:
				if( p == OLE_StartCursor ) {
			printt1( "Found start of cursor at %lx\n", (long)p);
					if( addspace ) *dst++ = ' ';
					*dst++ = 0;
					OLE_CurText = dst;
					*dst = 0;
					texttype = OLE_IN_CUR;
				} else {
					*dst++ = *p++;
				}
				break;
			case OLE_IN_CUR:
				if( p == OLE_EndCursor ) {
			printt1( "Found end of cursor at %lx\n", (long)p);
					*dst++ = 0;
					OLE_FolText = dst;
					*dst = 0;
					if( addspace ) *dst++ = ' ';
					texttype = OLE_IN_FOL;
				} else
				if( ntype(curs) == N_STUB ) {
					p++;
				} else {
					*dst++ = *p++;
				}
				break;
			case OLE_IN_FOL:
				*dst++ = *p++;
				break;	
		}
	}
	*dst = 0;

	/* If there is no following text, then the above loop will not
	 * set FolText properly, thus we must make it a special case
	 */
	if( OLE_FolText == (buf_el *)0 ) {
		OLE_FolText = dst;
	}

	/* If we are editing a Stub, set the edit text to be null */
	if( ntype( curs ) == N_STUB ) {
		printt0( "Edit text is a stub, set it to null\n" );
		*OLE_CurText = 0;
	}
	printt1( "OLE_PrcText @ %lx\n", (long) OLE_PrcText );
	printt1( "OLE_CurText @ %lx\n", (long) OLE_CurText );
	printt1( "OLE_FolText @ %lx\n", (long) OLE_FolText );
} 

/* This routine simply finds out whether the range selected (curs) can
 * be edited as text or not
 */

OLE_CanEdit( curs )
curspos	curs;
{

	int	cur_type;
	extern  char EditableClasses[];

	/* Get its class */
	cur_type = ex_class( curs );
	printt1( "OLE_CanEdit: type = %d\n", cur_type );

	/* If it is in the list of editable classes, then it's okay */
	if( !strchr( EditableClasses, cur_type ) ) {
		return FALSE;
	}
	/* The selected range is of a proper type to edit, return TRUE */
	printt0( "Range is valid for editing\n" );
	return TRUE;
}

/* This routine takes the line that the cursor is on and prints it into
 * a buffer.  While it is doing this, it stores pointers into the buffer
 * which point to the start and end of the text which represents the 
 * cursor node
 */
OLE_InitEdit( curs )
curspos curs;
{
	/* The call to find_phys_cursor calls out_line which then calls
	 * treeprint.  The net result is that the desired line is put
	 * into the line_buffer and the pointers are set up for ChopUpText
	 */
	find_phys_cursor( curr_window, curs, TRUE );
}

/* This routine actually calls the one line editor with the proper parameters
 *
 */

OLE_Edit( curs, flags, termch, row, column )
curspos curs;
int flags;
char *termch;
int row, column;
{
	char *rv;
	int x, y;
	char *term;
	extern int srch_row;
	int offset, OLE_Line;

	OLE_Line = srch_row;

	printt5( "OLE_Edit(%lx,%d,%s,%d,%d)\n", (long)curs, flags, 
		Safep(termch), row, column);
	printt1( "srch_row = %d\n", srch_row );

	offset = (row * curr_window->w_desc->_maxx) + column;

	rv = oledit( curr_window->w_desc, OLE_Line, flags,
			OLE_PrcText, OLE_CurText, OLE_FolText, termch,
			OLE_Result, offset );

}

OLE_Replace( curs )
curspos curs;
{
	extern nodep oletree; 
	int t;
	int cl;

	cl = ex_class( curs );
	printt1( "OLE_Replace(%lx)\n", (long)curs ); 

	/* parse the new string */
	if( cl == C_EXP || cl == C_VAR || cl == C_CONSTANT ) {

		strcat( OLE_Result, " " );
		domalloc = TRUE;
		t = s_expparse( OLE_Result, &oletree );
		/* Take the space off... */
		OLE_Result[ strlen(OLE_Result) - 1 ] = 0;

		/* If there were no tokens in OLE_Result, return */


	/* If we were editing an expression and the expression does not
 	 * parse properly, then set the node to be an expression syntax
	 * error, which is then grafted into the tree
	 */
		if( !t ) {
			/* Create an error node with the bad expression */
			oletree = tree(N_EXP_ERROR,allocESString(OLE_Result));
			s_node_flag(oletree, NF_ERROR);

			/* Give the message, but continue with the reuse */
			message(ER(121,"syntax`Expression syntax error"));
		}
	}
	printt1( "OLE_Replace: parse okay... oletree = %lx\n",
			(long) oletree );

	if( reuse( curs, ntype(curs), OLE_Result ) ) {
		printt0( "OLE_Replace, worked\n" );
	} else {
		/* Should give an error here: error( ER( */
		printt0( "OLE_Replace, failed\n" );
	}
}

