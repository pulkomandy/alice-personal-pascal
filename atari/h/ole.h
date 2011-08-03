/* Flags for the one line editor */

#define OLE_NONE	0	/* no options */
#define OLE_NOHL	2	/* don't highlight */
#define OLE_STR		4	/* we're part of a string */
#define OLE_EXP		8	/* we're typing in an expression */
#define OLE_TOP		0x40	/* called from top level */
#define OLE_CMD		0x10	/* editing a command */
#define	OLE_EDITING	0x80	/* editing via AL_IEDIT  (ug) */

#define ISID(fl) ((fl&(OLE_TOP|OLE_EXP|OLE_STR)) == OLE_TOP)
extern char *oledit();
