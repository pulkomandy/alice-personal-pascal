
/*
 * A 'ctype' file, with a few extensions that we need thrown in.
 */

#ifndef QNX
#include "ctype.h"
#endif

/*
 * On the QNX the ctype macros are functions and isalnum is missing.
 * Some versions of UNIX don't have isprint or isgraph.
 */

/* isalnum: true if char is alphanumeric */
#ifndef isalnum
#define isalnum(c) (isalpha(c)||isdigit(c))
#endif

/* isprint: true if char is a printing (noncontrol) char, including blank */
#ifndef isprint
#define isprint(c) ((c) >= ' ' && (c) <= '~')
#endif

/* isgraph: true if char is a graphic char, not including blank. */
#ifndef isgraph
#define isgraph(c) ((c) > ' ' && (c) <= '~')
#endif

#ifndef isascii
#define isascii(c) (((c) & 0377) < 0177)
#endif
