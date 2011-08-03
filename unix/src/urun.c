
/*
 *
 * Main for runtime process.  Uses message communication with editor process
 * to save space.
 *
 */
#define INTERPRETER 1

#include "alice.h"
#include <curses.h>

int is_cooked = FALSE;
extern WINDOW *pg_out;

go_cooked() {
}

go_raw() {
}

get_inp_char() {
#ifdef SAI	
	return getchar();
#else
	return keyget();
#endif
}

in_char_waiting() {
	return TRUE;
}
