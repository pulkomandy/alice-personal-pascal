
/* rcsfix.c */
#include <obdefs.h>
#include <gemdefs.h>

#ifndef BYTE                  /* Take care of 'portability' */ 
#define BYTE char             /* macros in the '.RSH' file. */
#define WORD int	      /* i absolutely refuse to use */
#define LONG long	      /* silly includes like portab.h */
#define NIL  (-1L)
#endif

#include "c:\alice\rcs\alice.h"         /* The header file from RSC   */
#include "c:\alice\rcs\alice.rsh"       /* The C source file from RSC */


#define adj(xywh, siz)	(siz * (xywh & 0x00FF) + (xywh >> 8))

rsrc_gaddr( thing, index, vr )
int thing;
int index;
long *vr;
{
	*vr = rs_trindex[index];
}

static void fix_trindex()
{
	int	test, ii;

	for (ii = 0; ii < NUM_TREE; ii++)
	{
		test = (int) rs_trindex[ii];
		rs_trindex[ii] = (long) &rs_object[test];
	}
}

static void fix_str(where)
long	*where;
{
	if (*where != NIL)
  	  *where = (long) rs_strings[(int) *where];
}

static void fix_objects()
{
	int	test, ii;
	int	wchar, hchar;

	graf_handle( &wchar, &hchar, &ii, &ii );

	for (ii = 0; ii < NUM_OBS; ii++)
	{
		rs_object[ii].ob_x = adj(rs_object[ii].ob_x, wchar);
		rs_object[ii].ob_y = adj(rs_object[ii].ob_y, hchar);
		rs_object[ii].ob_width = adj(rs_object[ii].ob_width, wchar);
		rs_object[ii].ob_height = adj(rs_object[ii].ob_height, hchar);
		test = (int) rs_object[ii].ob_spec;

		switch (rs_object[ii].ob_type)
		{
			case G_TITLE:
			case G_STRING:
			case G_BUTTON:
				fix_str(&rs_object[ii].ob_spec);
				break;
			case G_TEXT:
			case G_BOXTEXT:
			case G_FTEXT:
			case G_FBOXTEXT:
				if (test != NIL)
				   rs_object[ii].ob_spec =
					(char *) &rs_tedinfo[test];
				break;
			case G_ICON:
				if (test != NIL)
				   rs_object[ii].ob_spec =
					(char *) &rs_iconblk[test];
				break;
			case G_IMAGE:
				if (test != NIL)
				   rs_object[ii].ob_spec =
					(char *) &rs_bitblk[test];
				break;

			default:
				break;
		} /* switch */
	} /* for */
}

static void fix_tedinfo()
{
int	ii;

	for (ii = 0; ii < NUM_TI; ii++)
	{
		fix_str(&rs_tedinfo[ii].te_ptext);
		fix_str(&rs_tedinfo[ii].te_ptmplt);
		fix_str(&rs_tedinfo[ii].te_pvalid);
	}
}

static void fix_frstr()
{
	int	ii;

	for (ii = 0; ii < NUM_FRSTR; ii++)
		fix_str(&rs_frstr[ii]);
}

static void fix_img(where)
long	*where;
{
	if (*where != NIL)
	 *where = (long) (char *) rs_imdope[(int) *where].image;
}

static void fix_iconblk()
{
	int	ii;

	for (ii = 0; ii < NUM_IB; ii++)
	{
		fix_img(&rs_iconblk[ii].ib_pmask);
		fix_img(&rs_iconblk[ii].ib_pdata);
		fix_str(&rs_iconblk[ii].ib_ptext);
	}
}

static void fix_bitblk()
{
	int	ii;

	for (ii = 0; ii < NUM_BB; ii++)
		fix_img(&rs_bitblk[ii].bi_pdata);
}

static void fix_bb(where)
long	*where;
{
	if (*where != NIL)
  	  *where = (long) (char *) &rs_bitblk[(char) *where];
}

static void fix_frimg()
{
	int	ii;

	for (ii = 0; ii < NUM_FRIMG; ii++)
		fix_bb(&rs_frimg[ii]);
}

	
/* this is the only 'exported' function in
 * this module.
 */
void rsc_fix()
{
	fix_trindex();
	fix_objects();
	fix_tedinfo();
	fix_iconblk();
	fix_bitblk();
	fix_frstr();
	fix_frimg();
}

#undef adj
