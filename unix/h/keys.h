
/*
 * Command key internal representation
 */

#define AL_ICOMMAND		0200	/* Internal command key */
#define AL_FNAME		0001	/* internal filename code */
#define AL_IEDIT		0601	/* Internal edit key */
#define AL_IDOTDOT		0602	/* internal dotdot */
#define KEY_COORD		0603	/* absolute coords */

#ifdef msdos
#include "keyibm.h"

#else msdos

#define PFKEYNAMES	"F1:Run 2:Step 3:Del 4:Ins 5:Chg 6:Debug 7:Menu 8:File 9:HELP 10:Sel 11:Misc"

#include "keycurs.h"

/*
 * Key mappings for ANSI terminals into Alice functions.
 * Upper case letters are temporary and useful for debugging,
 * but we have to allow users to type them.
 */
#define AL_HOME		0
#define AL_CLEAR	KEY_CLEAR



#define AL_PGUP		0
#define AL_PGDOWN	0
#define AL_HELP		KEY_F(2)
#define AL_TAB		CNTL('F')
#define AL_ID		KEY_F(3)

/* Special keys but not top level commands */
#define AL_NOP		CNTL('N')
#define AL_TERM		CNTL('I')
#define AL_QUOTE	CNTL('V')
#define AL_COMPLETE	KEY_F(3)

#endif msdos
