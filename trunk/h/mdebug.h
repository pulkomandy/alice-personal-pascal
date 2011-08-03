/* debugging for map program */

#ifdef DMFREE
# define dmfree(p)		mfree(p, __FILE__, __LINE__)
#else
# define dmfree(p)		mfree(p)
#endif DMFREE

#ifdef DEBUG
# define tracing	(trace != NULL)
# define DB_LOAD	1
# define DB_SAVE	1
#else
# define tracing	0
#endif

#define HAS_SAVE	1
