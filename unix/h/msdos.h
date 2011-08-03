/*
 * Structures and defines for running Alice in an msdos environment
 */

/* file attributes */
#define	ATTR_FILE	0
#define	ATTR_DIRECTORY	0x10

/* relevant DOS function call error returns */
#define	EFILENOTFOUND	2		/* file not found */
#define	EPATHNOTFOUND	3		/* path not found */
#define EINVALIDDRIVE	15		/* invalid drive was specified */
#define	ENOMOREFILES	18		/* no more files */

/*
 * This structure is returned by msdos from "find first" and "find next"
 * function calls.
 */
typedef struct DiskTransferArea {
	bits8	reserved[21];		/* reserved for DOS for "find next" */
	bits8	attr;			/* attribute found */
	bits8	filesTime[2];		/* file's time */
	bits8	filesDate[2];		/* file's date */
	bits8	lowFileSize[2];		/* low word of file size */
	bits8	highFileSize[2];	/* high word of file size */
	bits8	filename[13];		/* null terminated name and extension */
} DTA;
