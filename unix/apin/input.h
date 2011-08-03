extern int	yylineno;
extern int	turbo_flag;
extern int	BlockFileMade;
extern Boolean	LibRefd;
extern nodep	LibTree;
extern nodep	LibSymtab;

#define error message
#define bug message

/* These functions are not necessarily declared in functions.h. */
extern listp	addlist();
extern listp	newlist();
extern listp	conclist();
extern listp	commentize();
extern nodep	ptr_symref();
extern nodep	makecom();
extern nodep	comstub();
extern char	*nextcomment();
extern nodep	fix0ArgFuncs();
extern char	*trimzero();
extern nodep	pack();

#define FNCAST			/* fake NCAST, for list/sym -> node */
#define FLCAST			/* fake LCAST, for node -> list */

typedef struct symtabStackEntry {
	nodep	symtab;
	Boolean	isBlock;
} SymtabStackEntry;

#define MAX_BLOCK_DEPTH 20
extern SymtabStackEntry SymtabStack[];
extern nodep	GlobalSymtab;	/* global symtab */
extern nodep	CurSymtab;     	/* current symtab */
extern nodep	CurBlkSymtab;	/* current block symtab */
extern int	scope_level;	/* current block number */

#define	SV_CONCAT	-126		/* Concat sym_saveid (for Parser) */

#define	YYMAXDEPTH	250
