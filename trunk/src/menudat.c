#include <whoami.h>
/* the stuff of which menus are made. */


/*
 * currently menus will be stored as arrays of char *s, actually in
 * tuples.  The first tuple will be the name of the menu and the name
 * of the directory where help files can be found (or some link to it,
 * depending on the Alice version.)
 * The remaining tuples will consist of
 * 1) the string to be printed in the menu
 * 2) The associated action, if any.  n <= 10 indicates that the action should
 *    be some sort of default, like a command name made from the pritned
 *    string.  If n is less than 10, the nth word of the printed string
 *    is extracted and lowercased.  This string is also used in looking up
 *    help for this menu entry.
 * The last tuple starts with zero
 */

char menu_nop[] = "Do Nothing";
char nop_command[] = "nop";
char com_dir[] = "command/";
static char misc_dir[] = "misc/";
#ifdef DEMO
static char howget[] = "How to buy a working copy of ALICE";
static char hgefile[] = "Hmisc/dgetone";
#else
static char howget[] = "General Information";
static char hgefile[] = "Hmisc/getone";
#endif
static char tokl_str[] = "What can I type here?";
static char ip_str[] = "Information about Pascal";
static char clip_str[] = "Clip";
static char sym_str[] = "Possible Symbols";
char imm_str[] = "Immediate Mode Block";

static char	Word1[]	=	"\01";
static char	Word2[]	=	"\02";

#ifndef GEM
char *M_insert[] = {
"Insert Menu", com_dir,
menu_nop, nop_command,
"Extend list", 0,
"Insert", 0,
"Get workspace", 0,
tokl_str, "toklist",
sym_str, "symbol",
"Enclose", 0,
0 };


char *M_delete[] = {
"Delete Menu", com_dir,
menu_nop, nop_command,
"Delete", 0,
clip_str, 0,
"Clear this workspace", "new",
"Recover memory", 0,
"Move to workspace", 0,
"Raise", 0,
0 };

char *M_change[] = {
"Changes Menu", com_dir,
menu_nop, nop_command,
"Edit (expression)", 0,
"Undo", 0,
"Hide", 0,
"Reveal", 0,
"Copy to workspace", 0,
"Comment out", "comout",
"Redo", 0,
"Special changes", "specialties",
0};

#ifdef QNX
#define QNX_DISP 1
#endif
#ifdef ICON
#define ICON_DISP 1
#endif

char *M_io[] = {
"Files Menu", com_dir,
menu_nop, nop_command,
"Load New File", 0,
"Save", 0,
"Save to Text File", Word2,
"Merge (Load Library)", 0,
#ifdef QNX_DISP
"QNX command", "shell",
#else
# ifdef msdos
  "DOS command", "shell",
# endif
#endif
#ifdef unix
"Shell command", 0,
#endif
"Set Filename", Word1,
"Quit ALICE", "exit",
0 };

char *M_misc[] = {
"Miscellaneous Menu", com_dir,
menu_nop, nop_command,
imm_str, "immediate",
#ifdef HAS_MWINDOW
"Set windows*", "window",
#endif
"Set macro", "map",
"File i/o", "io",
"Autosave", 0,
"Special changes", "specialties",
"Redraw Screen", "clear",
"Set option", Word1,
"Change workspace", "ws",
0 };

char *M_move[] = {
"Move & Search Menu", com_dir,
menu_nop, nop_command,
"Go to mark", 0,
"Former cursor loc", "popback",
"Mark position", 0,
"Search forward", 0,
"Search backwards", "rsearch",
"Search again",	Word1,
#ifdef OLD_STUFF
"Find reserved word", "tsearch",
"Reserved word backwards", "rtsearch",
#endif
"Up to parent", Word2,
"Page up", "pageup",
"Page down", "pagedown",
"Go to declaration", "decl",
"Top of block", Word2,
0};

#endif GEM

char *M_helpkey[] = {
"What kind of help would you like", com_dir,
"Getting Started with ALICE", "Hmisc/intro",
#ifdef GEM
"Quick summary of Keys", "gemkeys",
#else
"Quick summary of Keys", "Hmisc/keys",
#endif
"Features of ALICE", "Hmisc/commands",
ip_str, "phelp",
tokl_str, "toklist",
"How to quit ALICE", "Hmisc/quit",
"What was the last error?", "lasterror",
"What does my error message mean?", "errhelp",
#ifndef GEM
menu_nop, nop_command,
#endif
0 };


char *M_master[] = {
"Master Menu", com_dir,
menu_nop, nop_command,
"Insertions", "insmenu",
"Help", 0,
"Deletions", "delmenu",
"Change Structures", 0,
"Move Cursor", Word1,
ip_str, "phelp",
tokl_str, "toklist",
"Miscellaneous", "misc",
"File Operations", "io",
"Running the Program", "debug",
"Special changes", "specialties",
sym_str, "symbol",
0 };

#ifndef GEM
char *M_range[] = {
"Select Commands", com_dir,
menu_nop, nop_command,
"Clear Selection", "unselect",
"Hide", 0,
"Copy to workspace", 0,
"Comment out", "comout",
"Edit", 0,
"Delete", 0,
clip_str, "clip",
"Move to workspace", 0,
"Special changes", "specialties",
"Raise", 0,
"Enclose", 0,
0 };
#endif

#ifndef GEM
char *M_init[] = {
"Welcome to ALICE Pascal", misc_dir,
"Edit a New Program", 0,
"Load in an existing Pascal Program", 0,
"Quit the ALICE system", 0,
"Experiment in Immediate Mode", Word2,
"What is ALICE all about?", Word2,
#ifndef ICON_DISP
howget, hgefile,
#endif
#ifdef DEMO
"Run a demonstration of ALICE", Word2,
#endif
0};

char *M_load[] = {
"Your workspace has not been saved", misc_dir,
"Go back to editing this workspace", "load",
"Load, forgetting changes to this workspace", 0,
#ifdef MaybeSomeDay
"Save the current workspace and load", "foo"
#endif
0};
#endif GEM

static char savename[] = "save";

#ifndef GEM
char *M_save[] = {
"That file already exists", misc_dir,
"Do not Save", savename,
"Overwrite the existing file", savename,
0};
#endif

static char backal[] = "Go back to ALICE";
static char qforg[] = "Quit, forgetting all the changes";
static char backom[] = "quit";
static char qd[] = "quit";

#ifndef GEM
char *M_quit[] = {
"Your file has not been saved", misc_dir,
backal, backom,
qforg, qd,
#ifdef MaybeSomeDay
"Save the file and quit", "savequit",
#endif
0};
#endif

/*
char *M_outmem[] = {
"You have run out of memory (The system may be corrupted!)", misc_dir,
backal, backom,
qforg, qd,
0};
*/

char *M_clearws[] = {
"Clearing Workspace", com_dir,
"Clear the workspace - You can't UNDO this", "new",
"Do NOT clear the workspace", "nop",
0};
