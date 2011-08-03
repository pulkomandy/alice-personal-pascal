/*
 * memory
 */

#ifdef LARGE
#define EXTRAMEM	4096		/* extra user memory size */
#define EXTRASYSMEM	4096		/* extra system memory size */
#else

#define EXTRAMEM	1024		/* extra user memory size */
#define EXTRASYSMEM	512		/* extra system memory size */
#endif

extern char	*ExtraSysMem;		/* extra system memory */
