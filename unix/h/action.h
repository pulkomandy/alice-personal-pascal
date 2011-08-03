/*
 * Actions used when a simple expansion isn't enough.
 * An 'action' is keyed when a user types a token on top of a stub
 * or node, and the template file says that the given action is
 * specified.  This number will be in the template file, and a big
 * case statement defines what goes on.
 */

#define EXPAND 128

#define ACT_SKIP	1	/* Skip to the next stub */
#define ACT_EXPLIST 	2	/* Expand the current list */
#define ACT_IGNORE	3	/* Ignore this token */
#define ACT_CDOTDOT	4	/* insert a .. node in case */

#define ACT_LDINT	11	/* label decl, integer */
#define ACT_CDID	12	/* const decl, id */
#define ACT_TDID	13	/* type decl, id */
#define ACT_VDID	14	/* var decl, id */
#define ACT_CONTOK	15	/* constant, token */
#define ACT_TYPID	16	/* type, id */
#define ACT_TYPSR	17	/* type, something indicating subrange */
#define ACT_STID	18	/* statement, id */
#define ACT_EXPID	19	/* expression, id */
#define ACT_COMMENT	20	/* Comment statement */
#define ACT_DPROC	21	/* declare procedure with sym tab */
#define ACT_DFUNC	22	/* declare function with sym tab */
#define ACT_DPGM	23	/* declare program with sym tab */
#define ACT_DECLARE	24	/* declare this symbol according to parent */
#define ACT_HDECLARE	25	/* declare this symbol one symbol table up */
#define ACT_CHGOTO	26	/* go to a specified child */
				/* leave a gap here for up to plus six */
#define ACT_C_EXPAND	33	/* expand below at the cursor */
#define ACT_CRIGHT	34	/* go to something on the right or down */
#define ACT_H_EXPAND	35	/* expand where node was found */
#define ACT_GDECL	36	/* make a new declaration anywhere */
#define ACT_CMMA_EXPAND 37	/* expand below at the cursor if necessary */
#define ACT_AREXP	38	/* array expanstion */
#define ACT_CR		39	/* ??? was deleted for some reason*/
#define ACT_MDOWNTO	40	/* change to a downto */
#define ACT_CEXPR	41	/* enter expression, token is first */
#define ACT_RECORD	42	/* make a record with symbol table */
#define ACT_CCONST	43	/* constant on a case */
#define ACT_FDID	44	/* id on a field */
#define ACT_PDID	45	/* id on a parameter */
#define ACT_INDEX	46	/* add array indexing */
#define ACT_INDIRECT	47	/* add pointer indirect */
#define ACT_FIELD	48	/* add field reference */
#define ACT_FDUSE	49	/* field use */
#define ACT_CALLMAKE	50	/* change undef into call */
#define ACT_DELSE	51	/* change if into if-else */
#define ACT_STUBCR	52	/* cr on a line stub */
#define ACT_IDCOMMENT	53	/* program name, mostly */
#define ACT_LBSTAT	54	/* labeled statment */
#define ACT_WRITELN	55	/* make a writeln */
#define ACT_SET_SUBRANGE 56	/* make a set subrange */
#define ACT_REEXPAND	57	/* re-expand zero len list */
#define ACT_BLCR	58	/* cr on a loop type header gap of three */
#define ACT_DPPROC	61	/* parameter procedure */
#define ACT_DPFUNC	62
#define ACT_MTO		63	/* downto into to loop */

#define ACT_BLCHG	64	/* block changes by token typed */
#define ACT_COMCHG	65	/* change block into commented out block */
#define ACT_PUTME	66	/* put me at foundnode */
#define ACT_TVAR	67	/* change to a var param */

#define ACT_ASGCOLON	68	/* Kludge to handle colons */
#define ACT_RECID	69	/* record initializer */
#define ACT_ABSCON	70	/* constant or ID on abs address */
#define ACT_TMINIT	71	/* transmog to init */
#define ACT_ABSMOG	72	/* transmog to absolute */
