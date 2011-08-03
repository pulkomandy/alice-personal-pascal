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

char *M_helpkey[] = {
"What kind of help would you like", "command/",
"Getting Started with ALICE", "Hmisc/intro",
"Quick summary of Keys", "gemkeys",
"Features of ALICE", "Hmisc/commands",
"Information about Pascal", "phelp",
"What can I type here ?", "toklist",
"How to quit ALICE", "Hmisc/quit",
"What was the last error?", "lasterror",
"What does my error message mean?", "errhelp",
0 };

