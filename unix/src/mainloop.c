#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "window.h"
#include "flags.h"
#include "keys.h"
/*
 *DESC: Main token/command input loop
 */


#ifdef DB_MAINLOOP
# define PRINTT
#endif
#include "printt.h"

curspos ocursor = NIL;		/* where arrow keys left cursor */

int	phys_row;
int	phys_column;

nodep	CheckAt;		/* typecheck here if nonnil, else at cursor */
nodep	WhereDeclsChanged = NIL;/* recompile decls here if nonnil */
nodep	OldDeclsChanged = NIL;	/* last place that decls changed */
Boolean	CodeChanged;
Boolean anywflag;		/* find cursor any print code this time */

int autsave = 0;		/* how often to do automatic save */
int ascount = 0;		/* counter in auto saving */

int	CheckSpeed = 4;		/* decls and line-by-line checking */
/* checking speeds:
	0 - no automatic checking, user must typecheck manually
	2 - check decls only when code changes
	4 - check all decls when one changes, check code line by line
	6 - check all code when a decl changes 
 */

mainloop()
{
	unsigned cmd;
	nodep myLine;
	extern int phys_pcode;
	extern int row, scol, ecol;
	int curserr;

	for (;;) {
		here(1);
		checkfar(1);
		printt5("mainloop, cursor %x, ocursor %x, srch_row %d, srch_real %d, look.=%x\n",
			cursor, ocursor, srch_row, srch_real, lookaheadchar);
		hist_mark();
		printt0("back from hist_mark()\n");
		if (lookaheadchar == AL_NOP || lookaheadchar == ' ')
			lookaheadchar = 0;
		/*
		 * Redisplay unconditionall, not just when there's no
		 * typeahead, because often there is typeahead of the
		 * terminating character, like comma or right paren.
		 * Ideally the redisplay should be done only if dirty
		 * from the keyboard driver when we are ready for real
		 * input.
		 */
		newrange(FALSE);
		display(FALSE);
		/*
		 * ocursor is set by arrow keys to indicate that that's
		 * where the arrow keys left the cursor.  Only if it
		 * was set (possibly in them middle of a token) and
		 * hasn't moved since do we want to leave it alone.
		 */
		here(2);
		checkfar(2);
		if (cursor != ocursor) {
			/* don't do if moves are in type-ahead */
			find_phys_cursor( curr_window, cursor,anywflag);
			anywflag = FALSE;
			phys_pcode = kcget( -1, cursor );
			flusht();
			}
		here(3);
		ocursor = NIL;
		CheckAt = NIL;
		CodeChanged = FALSE;
		if( autsave &&  ascount++ > autsave ) {
			ascount = 0;
			if( work_fname( curr_workspace ) )  {
				save(0);
				statusLine();
				}
			 else
				warning( ER(220,"autosave`Automatic Save is engaged but there is no file name.") ); 

			}
		here(4);
		cmd = topcmd( srch_row, srch_real, srch_real+srch_width, cursnode );
		printt2("mainloop: topcmd returns %s (%d)\n", tokname(cmd), cmd);

		mark_line( cursor, SMALL_CHANGE );

		do_command(cmd);

		here(5);
		checkfar(3);
		/* never leave the cursor on a list or below hide */
		cursor = hidebound(realcp( cursor ));

		mark_line( cursor, SMALL_CHANGE );
		curserr = node_flag(cursor) & NF_ERROR;
		if( curserr && !WhereDeclsChanged ) {
			NodeNum cltype = ntype( t_up_to_line(cursor));
			if( ntype_info(cltype) & F_DECLARE||highType( cltype ) ) {
				WhereDeclsChanged = my_block(cursor);
				printt1("mainloop: WhereDeclsChanged=%x\n",
					WhereDeclsChanged);
				}
			}
		/*
		 * Whenever a graft, etc takes place we check to see
		 * if changes are being made to a declaration.  If they
		 * are then we set WhereDeclsChanged.  If it has been
		 * set when we get back here we recompile its declaration
		 * block.
		 */
		if (CheckSpeed && WhereDeclsChanged &&
				( CheckSpeed >= 4 || CodeChanged ||
				WhereDeclsChanged != OldDeclsChanged) ) {
			if( CheckSpeed < 4 && 
					WhereDeclsChanged != OldDeclsChanged ) {
				nodep decswap;
				decswap = OldDeclsChanged;
				OldDeclsChanged = WhereDeclsChanged;
				WhereDeclsChanged = decswap;
				printt1("mainloop: WhereDeclsChanged=%x (2)\n",
					WhereDeclsChanged);
				}
			
			if( WhereDeclsChanged ) {
				clr_node_flag(WhereDeclsChanged, NF_TCHECKED);
#ifndef ATARI520ST
				printt3("mainloop: c_comp_decls(%x, %x, %x)\n",
						my_scope(WhereDeclsChanged),
						WhereDeclsChanged,
						CheckSpeed < 6 ? TC_NOSTUBS :
						 (TC_DESCEND | TC_FULL) );
#endif
				here(4);
				checkfar(4);
				c_comp_decls(my_scope(WhereDeclsChanged),
			     		WhereDeclsChanged,
					CheckSpeed < 6 ? TC_NOSTUBS :
					 (TC_DESCEND | TC_FULL) );
				here(5);
				checkfar(5);
				WhereDeclsChanged = NIL;
				}
			}
		/*
		 * Typecheck the current line.  This was added late in the
		 * game, and isn't really the best way of doing things.
		 * Every time we return from a command, we typecheck
		 * the current line (or if CheckAt has already been set
		 * by a skip down routine, we typecheck there).  We
		 * don't typecheck declarations.
		 */
		printt2("CheckAt=%x, cursor=%x\n", CheckAt, cursor);
		if (!CheckAt && (curserr||is_undid(cursor)))
			CheckAt = cursor;
		if( CheckAt && CheckSpeed ) {
			myLine = t_up_to_line(CheckAt);
			if (!( highType(ntype(myLine)) ||
				(ntype_info(ntype(myLine)) & F_DECLARE)) ) {
				printt2("mainloop: c_typecheck(%x, 0, %x)\n",
					(int)myLine,  TC_NOSTUBS|TC_ONERROR);
				c_typecheck(myLine, 0, TC_NOSTUBS|TC_ONERROR );
				here(6);
				checkfar(6);
				}
			}

		/* if code was changed, clear chance at resuming */
		if( CodeChanged )
			clear_resume();
#ifdef CHECKSUM
		cscheck();
#endif
	}
}
