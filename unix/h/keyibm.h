
/* special key definitions for an IBM PC */

#ifdef ICON_DISP
#define PFKEYNAMES	"F1:Run 2:Step 3:Del 4:Ins 5:Chg 6:Debug 7:Misc 8:File 9:Range 10:Select 11:Menu"
#else
#define PFKEYNAMES	"F1:Run 2:Step 3:Del 4:Ins 5:Chg 6:Debug 7:Menu 8:File 9:HELP 10:Select Sh9:Misc"
#endif

#define IBMF(x)		128+x
#define KEY_F(n)	(128+58+n)
#define KEY_SHF(n)	(128+83+n)
#define KEY_CF(n)	(128+93+n)
#define KEY_UP		IBMF(72)
#define KEY_DOWN	IBMF(80)
#define KEY_LEFT	IBMF(75)
#define KEY_RIGHT	IBMF(77)
#define KEY_BACKSPACE	8

#define AL_UP		KEY_UP
#define AL_DOWN		KEY_DOWN
#define AL_LEFT		KEY_LEFT
#define AL_RIGHT	KEY_RIGHT
#define AL_PARENT	CNTL('^')
#define AL_CHILD	IBMF(125)
#define AL_LSIB		IBMF(124)
#define AL_RSIB		IBMF(126)
#define AL_LTOKEN	IBMF(15)
#define AL_RTOKEN	0x09
/* these two have got to change */
#define AL_PGUP		IBMF(73)
#define AL_PGDOWN	IBMF(81)	
#define AL_HELP		KEY_F(9)
#define AL_BTAB		IBMF(115)   /* ctrl left arrow */
#define AL_TAB		IBMF(116)   /* ctrl right arrow */
#define AL_BACKSPACE	KEY_BACKSPACE
#define AL_EXPAND	CNTL('E')
#define AL_INSERT	IBMF(82)
#define AL_DELETE	IBMF(83)
#define AL_PRUNE	CNTL('P')
#define AL_COMMAND	CNTL('X')
#define	AL_2COMMAND	CNTL('X')
#define AL_BLOCK	IBMF(71)	/* home */
#define AL_CLEAR	IBMF(119)	/* ctrl home */
#define AL_REDRAW	IBMF(119)	/* ctrl home */
#define AL_SELECT	KEY_F(10)	
#define AL_UNSELECT	IBMF(128)	/* ctrl 9 */
#define AL_RUN		KEY_F(1)	/* f1 */
#define AL_MMENU	KEY_F(7)	/* master menu */
#define AL_QUIT		IBMF(16)	/* beta key */
#define AL_UNDO		CNTL('U')
#define AL_REDO		CNTL('R')
#define AL_EDIT		IBMF(18)	/* alt e */
#define AL_COPY		IBMF(46)	/* alt c */
#define AL_GET		IBMF(131)		/* ctrl + */
#define AL_MOVE		IBMF(130)		/* ctrl - */
#define AL_HIDE		IBMF(35)		/* ALT H */
#define AL_REVEAL	IBMF(19)		/* ALT R */
#define AL_PHELP	IBMF(25)		/* shift divide */
#define AL_POPBACK	IBMF(48)
#define AL_TOKLIST	IBMF(20)		/* alt t */
#define AL_TCHECK	CNTL('T')		/* function key 7 */
#define AL_SAVE		IBMF(31)
#define AL_LOAD		IBMF(38)
#define AL_STEP		KEY_F(2)
#define AL_SUPERSTEP	KEY_SHF(2)
#define AL_EXECUTE	KEY_SHF(1)
#define AL_ID		0x13			/* ^s */
#define AL_DMENU	KEY_F(3)
#define AL_IMENU	KEY_F(4)
#define AL_CMENU	KEY_F(5)
#define AL_RMENU	KEY_F(6)
#define AL_SMENU	KEY_SHF(10)
#define AL_GOMENU	IBMF(34)		/* alt g */
#define AL_MISC		KEY_SHF(9)
#define AL_FMENU	KEY_F(8)
#define AL_SEARCH	CNTL('F')
#define AL_TOKEN	CNTL('K')
#define AL_SHELL	KEY_SHF(3)

/* Special keys, but not top level commands */
#define AL_NOP		033
#define AL_TERM		CNTL('I')
#define AL_QUOTE	CNTL('V')
