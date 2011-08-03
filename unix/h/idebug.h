/*
 * Control of debugging defines.
 * DEBUG must be defined or all debugging is turned off.
 * Others control individual debugging outputs.
 */
#define DEBUG 0

#define UDEBUG 1
#define SMALLIN 1

#ifdef DEBUG

#define XDEBUG 1
# define tracing	(dtrace != NULL)

/* #define DB_RUNTIME 1 */
/* #define DB_TYPECHECK 1 */
/* #define DB_TYPES 1 */
/*#define DB_DUMP 1 */

# define Stomp_Check 1	/* enable edit tree child number checking */

#else

# define tracing 0

#endif

/* Defines to turn off features to make alice fit in 64K */


/* See also DB_DUMP above */

#  define HAS_INTERP	1
#define HAS_TYPECHECK 1

