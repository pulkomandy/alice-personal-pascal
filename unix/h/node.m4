/*
 *	The node types for the first byte in any node
 */
define( `nodenum', -1 )
/* macro arguments
 * $1 - name of node
 * $2 - array of print codes
 * $3 - array of maps from child numbers to print codes.  first entry
 *      is for the nodes as a whole
 * $4 - flags for the node
 * $5 - number of children node has and number to get memory for
 * $6 - array of classes of children.
 * $7 - map of actions upon tokens 
 * $8 - english node name for help
 * $9 - expression precedence if present
 */
define(LISTER, 1)
define(makenode, `divert(9)define(`nodenum',incr(nodenum))
`#define' N_$1 nodenum 
divert(1)static char *P$1[] = $2;
ifelse(`$3',NILCHL,,`static bits8 I$1[] = $3;')
static struct node_info s$1 = { P$1, ifelse(`$3',NILCHL,NILCHL,I$1),$4,$5};
divert(2)&s$1,
divert(4)ifelse(`$8',,0,`$8'),
divert(5)ifelse(`$9',,,`$9,')
divert(3)ifelse(`$7',NULACT,,`static ActionNum a$1[] = $7;')
static ClassNum c$1[] = $6;
divert(6)ifelse(`$7',NULACT,NULACT,a$1),
divert(7)c$1,
divert(-1)' )

define(mfakenode, `divert(9)define(`nodenum',incr(nodenum))
`#define' N_$1 nodenum
divert(2)0,
divert(4)0,
divert(6)NULACT,
divert(7)CAST(ClassNum *)0,
divert(-1)')

define( setequal, `divert(9)
`#define' $1 nodenum
divert(10)' )
divert(1)
`#include' "whoami.h"
`#include' "altypes.h"
`#include' "flags.h"
`#include' "class.h"
`#include' "tune.h"
`#include' "node.h"
`#ifdef' QNX
`char	NodeStart;'
`#endif'
divert(2)struct node_info *node_table[] = {
divert(3)
/* BEGIN_ARRAY */
`#include' "whoami.h"
`#define' CAST(atp) (atp)
`#include' "altypes.h"
`#include' "node.h"
`#include' "action.h"
`#include' "token.h"
`#include' "class.h"
`#define' UT_ANY		0
`#define' UT_EXACT	0x10
`#define' UT_UPKID	0x20
`#define' UT_PCODE	0x40
static ActionNum NULACT[] = {0};
divert(4) char *Node_Names[] = {
divert(5)bits8 expr_prec[] = {
divert(6)ActionNum *N_Actions[] = {
divert(7)ClassNum *K_Classes[] = {
divert(9)
/* BEGIN_DEFINE */
`#define' NILCHL 0
divert(10)
define(CLIST,`$1+64')


makenode( NULL, ` {"!c", 0} ',
				NILCHL,
				`F_LINE, 0',
				`1, 1',
				{ C_PROGRAM },
				`NULACT',
				"root"
				)
makenode( T_COMMENT, ` {"!c!ag!1E",0} ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1',
				{NILCHL},
				`NULACT',
				"comment"
				)
/* first node in list of standard type nodes */
setequal( FIRST_STANDARD )
	
makenode( PROGRAM, ` {
				"!c!kprogram !(Program-Name)!1p!2P,;\n",
				"!3l\n",
				"!4l\n",
				"!kbegin\n",
				"!5l\n",
				"!kend.\n",
				0} ',
				` {0,0,0,1,2,4,1} ',
				`F_DECLARE | F_LINE | F_SYMBOL |F_SCOPE, 0',
				`5, 7', /* hidden symbol child */
				`{ C_PNAME,
					CLIST(C_PNAME),
					CLIST(C_BLCOMMENT), 
					CLIST(C_DECLARATIONS),
					CLIST(C_STATEMENT) } ',
				`{ TOK_BEGIN, UT_ANY, ACT_CHGOTO+4,
				TOK_VAR,  UT_ANY, ACT_GDECL,
				TOK_CONST, UT_ANY, ACT_GDECL,
				TOK_TYPE, UT_ANY, ACT_GDECL,
				TOK_LABEL, UT_ANY, ACT_GDECL,
				TOK_PROCEDURE, UT_ANY, ACT_GDECL,
				TOK_FUNCTION, UT_ANY, ACT_GDECL,
				TOK_LPAREN, UT_UPKID+0, ACT_REEXPAND,
				TOK_LPAREN, UT_UPKID+1, ACT_REEXPAND,
				TOK_LPAREN, UT_PCODE+0, ACT_REEXPAND,
				TOK_RPAREN, UT_UPKID+1, ACT_CHGOTO+2,
				TOK_SEMI, UT_PCODE+0, ACT_BLCR,
				TOK_CR,UT_PCODE+3,ACT_BLCR+2,
				TOK_CR,UT_PCODE+5,0,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_COMMA, UT_UPKID+1, ACT_C_EXPAND,
				TOK_PROG, UT_ANY, ACT_CHGOTO+0,
				0 }',
				"pascal program"
				)
	/* pointers to program name, the global symbol table,
	   the declarations lists and the statement list */


makenode( DECL_LABEL, ` { "!c!klabel !1,;\n",0 } ',
				NILCHL,
				`F_DECLARE | F_LINE, 0',
				`1, 1',
				`{ CLIST(C_LABEL_DECL) }',
				`{ TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0 }',
				"declarations"
				)
	/* pointer to a name table entry with the integer */

makenode( DECL_CONST, ` { 	"!c!kconst",
				"!1l\n",
				"",
				0 } ',
				` { 0, 1 } ',
				`F_DECLARE | F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_CONST_DECL) }',
				`{ TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0}',
				"declarations"
				)
	/* pointer into the name table and the constant tree */

makenode( DECL_TYPE, ` { 	"!c!ktype",
				"!1l\n",
				"",0 } ',
				` { 0, 1 } ',
				`F_DECLARE | F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_TYPE_DECL) }',
				`{ TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0}',
				"declarations"
				)
	/* pointer to symbol table entry and to the type tree */

makenode( DECL_VAR, ` { 	"!c!kvar",
				"!1l\n",
				"",0 } ',
				` { 0, 1 } ',
				`F_DECLARE | F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_VAR_DECL) }',
				`{ TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0}',
				"declarations"
				)
	/* pointer to symbol table entry and type tree */

makenode( DECL_PROC, ` { 
				"!n!c!kprocedure !(proc-name)!1p!f{!2P;!f};\n",
				"!3l\n",
				"!4l\n",
				"!T!kbegin\n",
				"!5l\n",
				"!T!kend;\n", 0} ',
				` {0, 0,0,1,2,4} ',
				`F_LINE|F_INDENT|F_SYMBOL |F_SCOPE,0',
				`5, 7',/* hidden symbol */
				`{ C_HIDECL_ID, CLIST(C_FORMAL),
					CLIST(C_BLCOMMENT), 
					CLIST(C_DECLARATIONS),
					CLIST(C_STATEMENT) }',
				`{
				TOK_VAR, UT_ANY, ACT_GDECL,
				TOK_CONST, UT_ANY, ACT_GDECL,
				TOK_TYPE, UT_ANY, ACT_GDECL,
				TOK_LABEL, UT_ANY, ACT_GDECL,
				TOK_PROCEDURE, UT_ANY, ACT_GDECL,
				TOK_FUNCTION, UT_ANY, ACT_GDECL,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+4,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+3, ACT_BLCR+2,
				TOK_CR, UT_PCODE+5, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_SEMI, UT_PCODE+0, ACT_BLCR,
				0 }',
				"procedure"
				)
	/* pointer to comment, procedure name, parameters, block symbol
	   table, and either FORWARD or the declarations and stat list */

makenode( DECL_FUNC, ` { 
				"!n!c!kfunction !(func-name)!1p!f{!2P; : !3p!f};\n",
				"!4l\n",
				"!5l\n",
				"!T!kbegin\n",
				"!6l\n",
				"!T!kend;\n", 0} ',
				` {0, 0,0,0,1,2,4} ',
				`F_LINE|F_INDENT|F_SYMBOL |F_SCOPE,0',
				`6, 8',/* hidden symbol */
				`{ C_HIDECL_ID,
					CLIST(C_FORMAL), C_TYPEID,
					CLIST(C_BLCOMMENT), 
					CLIST(C_DECLARATIONS),
					CLIST(C_STATEMENT) }',
				`{
				TOK_VAR, UT_ANY, ACT_GDECL,
				TOK_CONST, UT_ANY, ACT_GDECL,
				TOK_TYPE, UT_ANY, ACT_GDECL,
				TOK_LABEL, UT_ANY, ACT_GDECL,
				TOK_PROCEDURE, UT_ANY, ACT_GDECL,
				TOK_FUNCTION, UT_ANY, ACT_GDECL,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+5,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+3, ACT_BLCR+2,
				TOK_CR, UT_PCODE+5, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_SEMI, UT_PCODE+0, ACT_BLCR,
				TOK_COLON, UT_PCODE+0, ACT_CHGOTO + 2,
				TOK_RPAREN, UT_UPKID+1, ACT_CHGOTO + 2,
				0 }',
				"function"
				)
	/* like PROC but also a type tree with the return value */

makenode( FORWARD, ` { "!c!kForward <!1p>\n",0 } ',
				NILCHL,
				`F_LINE , 0',
				`1, 1',
				`{ C_ROUTNAME }',
				`{
				TOK_CR, UT_ANY, ACT_H_EXPAND,
				0 }',
				"Forward Declaration"
				)
makenode( CONST_INIT, ` { "!1p !c!k: !2p = !3p;!4o!\t!4{!o\n", 0 } ',
				NILCHL,
				`F_LINE|F_DECLARE,1',
				`4, 4',
				`{ C_DECL_ID, C_TYPEID, C_INIT,
					C_COMMENT }',
				`{ TOK_EQ, UT_ANY, ACT_CHGOTO + 2,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 3,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_LPAREN, UT_ANY, ACT_CHGOTO+2,
				0}',
				"initializer"
				)
mfakenode( INIT_RECORD, ` { "!c!k(!1;)", 0 } ',
				NILCHL,
				`0,0',
				`1, 1',
				`{ CLIST(C_RECINIT) }',
				`NULACT',
				"record initializer"
				)
makenode( CONST_DECL, ` { "!1p !c!k= !2p;!\t!3{\n",0 } ',
				NILCHL,
				`F_DECLARE | F_LINE, 1',
				`3, 4',
				`{ C_DECL_ID, C_CONSTANT, C_COMMENT }',
				`{ TOK_EQ, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 2,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_COLON, UT_ANY, ACT_TMINIT,
				0}',
				"constant declaration"
				)
makenode( TYPE_DECL, ` { "!1p !c!k= !2p!R!\t!3{\n",0 } ',	
				NILCHL,
				`F_DECLARE | F_PMULTI | F_LINE, 1',
				`3, 3',
				`{ C_DECL_ID, C_TYPE, C_COMMENT }',
				`{ TOK_EQ, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO +2,
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"type declaration"
				)
makenode( VAR_DECL, ` { "!1, !c!k: !2p!R!\t!3{\n",0 } ',	
				NILCHL,
				`F_DECLARE | F_PMULTI | F_LINE, 1',
				`3, 3',
				`{ CLIST(C_DECL_ID), C_TYPE, C_COMMENT }',
				`{ TOK_COLON, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 2,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"variable declaration"
				)



makenode( TYP_ENUM, ` { "!c!k(!1,)",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`1, 1',
				`{ CLIST(C_DECL_ID) }',
				`{ TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				0}',
				"enumerated type"
				)
	/* a pointer to a list of enumeration elements */

makenode( TYP_SUBRANGE, ` { "!(lower-bound)!1p!c!k..!(upper-bound)!2p",0 } ',
				NILCHL,
				`NULLBYTE, 1',
				`2, 4',		/* upper and lower */
				`{ C_OCONSTANT, C_OCONSTANT }',
				`{ TOK_DOTDOT, UT_ANY, ACT_CHGOTO+1,
				TOK_DOT, UT_ANY, ACT_CHGOTO+1,
				0 }',
				"subrange"
				)
	/* pointers to two scalars */

makenode( TYP_PACKED, ` { "!c!kpacked !1p",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`1, 1',
				`{ C_ST_TYPE }',
				`NULACT',
				"packed type"
				)
	/* a pointer to a type tree */

makenode( TYP_ARRAY, ` { "!c!karray [!(range)!1p] of !2p",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`2, 5', /* array had bounds and size kids */
				`{ C_SIM_TYPE, C_TYPE } ',
				`{ TOK_COMMA, UT_UPKID+0, ACT_AREXP, /* special */
				TOK_LBRACKET, UT_ANY, ACT_CHGOTO + 0,
				TOK_RBRACKET, UT_ANY, ACT_CHGOTO + 1,
				TOK_OF, UT_ANY,	ACT_CHGOTO + 1,
				0}',
				"array"
				)
makenode( INIT_STRUCT, ` { "!c!k(!1:)", 0 } ',
				NILCHL,
				`0,0',
				`1, 2',
				`{ CLIST(C_INIT) }',
				`{
				TOK_COMMA, UT_ANY, ACT_CMMA_EXPAND,
				TOK_SEMI, UT_ANY, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				0 }',
				"complex initializer"
				)
makenode( TYP_STRING, ` { "!k!cstring[!(Max Length)!1p]",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`1, 2', /* array had bounds and size kids */
				`{ C_OCONSTANT } ',
				`{ 
				TOK_LBRACKET, UT_ANY, ACT_CHGOTO + 0,
				TOK_RBRACKET, UT_ANY, ACT_CRIGHT,
				0}',
				"string type"
				)
makenode( FLD_INIT, ` { "!1p!c:!2p", 0 } ',
				NILCHL,
				`0, 1',
				`2, 2', 
				`{ C_FLD_NAME, C_INIT } ',
				`{
				TOK_COLON, UT_ANY, ACT_CHGOTO + 1,
				0}',
				"field initializer"
				)
	/* a pointer to a list of type trees for the indices and to
	   the type tree for the constituent type */

makenode( TYP_SET, ` { "!c!kset of !1p",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`1, 3',	/* upper and lower bound */
				`{ C_SIM_TYPE } ',
				`{ 
				TOK_OF, UT_ANY,	ACT_CHGOTO + 0,
				0}',
				"set type"
				)
	/* a pointer to a scalar type tree */

makenode( TYP_FILE, ` { "!c!kfile of !(file-element-type)!1p",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`1, 1',
				`{ C_TYPE } ',
				`{ 
				TOK_OF, UT_ANY,	ACT_CHGOTO + 0,
				0}',
				"file type"
				)
	/* a pointer to a type tree */

makenode( TYP_POINTER, ` { "!c!k^!1p",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`1, 1',
				`{ C_TYPEID } ',
				`NULACT',
				"pointer type"
				)
	/* a pointer to a type tree */

makenode( TYP_RECORD, ` { "!c!krecord",
				"!1l\n",
				"!T!kend;\n",0 } ',
				` { 0, 1 } ',
				`F_DECLARE | F_SYMBOL | F_INDENT, 0',
				`1, 3', /* a symtab and the size of it */
				`{ CLIST(C_FIELD) } ',
				`{
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_END, UT_ANY, ACT_CRIGHT,
				0}',
				"record type"
				)
	/* actually a list of record entries, with pointers to 
	   names on one side and type trees on the other.
	   There should be a bit indicating the presence of a variant
	   part.
	 */
makenode( FIELD, ` { "!(field-name)!1, !c!k: !2p!R!\t!3{",0 } ',
				NILCHL,
				`F_DECLARE | F_PMULTI | F_LINE, 1',
				`3, 3',
				`{ CLIST(C_DECL_ID), C_TYPE, C_COMMENT } ',
				`{
				TOK_CR, UT_ANY, ACT_CR,
				TOK_SEMI, UT_ANY, ACT_CHGOTO+2,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				0 }',
				"record field"
				)
	/* may be not needed any more */
makenode( VARIANT, ` { "!c!kcase !(tag-name)!1p : !(tag-type)!2p of\n",
				"!3l\n",
				"{end variant part}\n",
				0 } ',
				`{0, 0, 0, 1 }',
				`F_DECLARE | F_LINE, 0',
				`3, 4',
				`{ C_DECL_ID, C_TYPEID, CLIST(C_VARIANT) }',
				`{
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_COLON, UT_UPKID+0, ACT_CHGOTO+1,
				0 }',
				"variant record"
				)

makenode( ST_VCASE, ` { "!1, !c!k: (\n",
				"!2l\n",
				"!T!k)\n",
				0 } ',
				`{0, 0, 1 }',
				`F_INDENT | F_DECLARE | F_LINE, 1',
				`2, 2',
				`{ CLIST(C_CONSTANT), CLIST(C_FIELD) }',
				`{
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_COLON, UT_UPKID+0, ACT_CHGOTO+1,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				0}',
				"variant case"
				)

makenode( FORM_VALUE, ` { "!(parm-name)!1,!c!k: !2p",0 } ',
				NILCHL,
				`F_DECLARE, 1',
				`2, 2',
				`{ CLIST(C_DECL_ID), C_TYPEID } ',
				`{
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_VAR, UT_ANY, ACT_TVAR,
				0 }',
				"parameter"
				)
	/* a list node, with pointers to names and type trees. */
makenode( FORM_REF, ` { "!c!kvar !(parm-name)!1,: !2p",0 } ',
				NILCHL,
				`F_DECLARE, 0',
				`2, 2',
				`{ CLIST(C_DECL_ID), C_TYPEID } ',
				`{
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				0 }',
				"var parameter"
				)
	/* same as formal, just a var param */
makenode( FORM_FUNCTION, ` { "!c!kfunction !(func-name)!1p!2P; : !3p",0 } ',
				NILCHL,
				`F_SYMBOL | F_SCOPE, 0',
				`3, 4',
				`{ C_DECL_ID, CLIST(C_FORMAL), C_TYPEID }',
				`{
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_C_EXPAND,
				0 }',
				"function parameter"
				)
	/* needs a name, a list of formals and a type */
makenode( FORM_PROCEDURE, ` { "!c!kprocedure !(proc-name)!1p!2P;",0 } ',
				NILCHL,
				`F_SYMBOL | F_SCOPE, 0',
				`2, 3',
				`{ C_DECL_ID, CLIST(C_FORMAL) }',
				`{
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_C_EXPAND,
				0 }',
				"procedure parameter"
				)
	/* needs a name and a list of formals */
makenode( NOTDONE, ` { "!1p !c{non-ALICE}\n",0 } ',
				NILCHL,
				`F_LINE, 1',
				`1, 1',
				`{ C_COMMENT } ',
				`{
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"pass through comment"
				)

makenode( ST_COMMENT, ` { "!c!1{\n",0 } ',
				NILCHL,
				`F_LINE, 0',
				`1, 1',
				`{ C_COMMENT } ',
				`{
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"statement comment"
				)
	/* a pointer to text */
mfakenode( ST_TRACE, ` { "!k!cTRACE !1p begin\n",
				"!2l\n",
				"	!kend;\n",
				0 } ',
				` {0, 0, 1 } ',
				`F_LINE | F_INDENT, 1',
				`2, 2',
				`{ C_EXP, CLIST(C_STATEMENT) }',
				`{ /*TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,*/
				0}',
				"debug block"
				)

makenode( ST_LABEL, ` { "!1p!c:\n",0 } ',
				NILCHL,
				`F_LINE, 1',
				`1, 1',
				`{ C_LABEL } ',
				`{
				TOK_CR, UT_ANY, ACT_CR,
				TOK_COLON, UT_ANY, ACT_CR,
				0}',
				"labeled statement"
				)
	/* standard node with integer in it */

makenode( ST_GOTO, ` { "!c!kgoto !(label)!1p;\n",0 } ',
				NILCHL,
				`F_LINE, 0',
				`1, 1',
				`{ C_LABEL } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"goto"
				)
	/* standard node with integer in it */

makenode( ST_CALL, ` { "!1p!c!2P,;\n",0 } ',
				NILCHL,
				`F_LINE, 1',
				`2, 2',
				`{ C_PROC_ID, CLIST(C_EXP) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_IGNORE,
				TOK_LPAREN, UT_ANY, ACT_REEXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0}',
				"procedure call"
				)
setequal( FIRST_RUN )
mfakenode( ST_EX1, ` { 0 } ',
				NILCHL,
				`F_LINE, 0',
				`0, 0', /* array had bounds and size kids */
				`{ 0 } ',
				`NULACT',
				"extra stat 1"
				)
mfakenode( ST_EX2, ` { 0 } ',
				NILCHL,
				`F_LINE, 0',
				`0, 0', /* array had bounds and size kids */
				`{ 0 } ',
				`NULACT',
				"extra stat 1"
				)
	/* pointers to the symbol table entry for the procedure
	   and the list of actuals. */

makenode( ST_ASSIGN, ` { "!1p !c:= !2p;\n",0 } ',
				NILCHL,
				`F_E2KIDS | F_LINE, 1',
				`2, 3',	/* kid for size */
				`{ C_VAR, C_EXP } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_ASSIGN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
#ifdef notdef
				TOK_COLON, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
#else				
				TOK_COLON, UT_ANY, ACT_ASGCOLON,
#endif
				TOK_EQ, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_CALLMAKE,
				0}',
				"assignment statement"
				)
	/* a variable tree on one side and an expression tree on
	   the other */

makenode( IMM_BLOCK, ` { "!c{ -----Immediate Statement Block-----\n",
				"!1l\n",
				"begin\n",
				"!2l\n",
				"  -----End Immediate Statements-----}\n",
				0 } ',
				`{0, 1, 3 }',
				`F_SCOPE | F_LINE |F_SYMBOL, 0',
				`2, 4',
				`{ CLIST(C_DECLARATIONS),
					CLIST(C_STATEMENT) }',
				`{
				TOK_VAR, UT_ANY, ACT_GDECL,
				TOK_CONST, UT_ANY, ACT_GDECL,
				TOK_TYPE, UT_ANY, ACT_GDECL,
				TOK_LABEL, UT_ANY, ACT_GDECL,
				TOK_PROCEDURE, UT_ANY, ACT_GDECL,
				TOK_FUNCTION, UT_ANY, ACT_GDECL,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_BLCR+1,
				TOK_CR, UT_PCODE+4, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				0 }',
				"Immediate Statements"
				)
	/* pointers to a list of actuals with optional formatting
	   expressions.  Perhaps a flag to say if we are talking
	   write or writeln here.
	   We also need a pointer to a file variable if necessary
	   */

	   /* now an extra node so we can insert more easily.  others later */
makenode( ST_SPECIAL, ` { "!k!1p !cbegin\n",
				"!2l\n",
				"!T!kend;\n",
				0 } ',
				` {0, 0, 1 } ',
				`F_LINE | F_INDENT, 1',
				`2, 2',
				`{ C_SPECIAL, CLIST(C_STATEMENT) }',
				`{ TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0}',
				"generic block"
				)

makenode( ST_IF, ` { "!c!kif !(Condition)!1p then begin\n",
				"!2l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,1} ',
				`F_E1KID | F_LINE | F_INDENT, 0',
				`2, 2', 
				`{ C_EXP, CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_THEN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				TOK_ELSE, UT_ANY, ACT_DELSE,
				0}',
				"if-then"
				)
	/* pointer to the conditional expression tree, pointer to the
	   IF part statement and a pointer to the else part statement
	   Null pointer if no else part
	   */

makenode( ST_ELSE, ` { "!c!kif !(Condition)!1p then begin\n",
				"!2l\n",
				"!T!kend\n",
				" !kelse begin\n",
				"!3l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,1,4} ',
				`F_E1KID | F_LINE | F_INDENT, 0',
				`3, 3',
				`{ C_EXP, CLIST(C_STATEMENT), CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_ELSE, UT_ANY, ACT_CHGOTO+2,
				TOK_THEN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_BLCR+1,
				TOK_CR, UT_PCODE+3, ACT_BLCR+1,
				TOK_CR, UT_PCODE+5, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0}',
				"if-then-else"
				)

makenode( ST_FOR, ` { "!c!kfor !(variable)!1p := !(start)!2p to !(finish)!3p do begin\n",
				"!4l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,0,0,1} ',
				`F_E2KIDS | F_LINE | F_INDENT, 0',
				`4, 4',
				`{ C_VAR, C_EXP, C_EXP, CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_DOWNTO, UT_ANY, ACT_MDOWNTO,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+3,
				TOK_ASSIGN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_TO, UT_EXACT+UT_UPKID+1, ACT_CHGOTO+2,
				TOK_TO, UT_UPKID+0, ACT_CHGOTO+2,
				TOK_COLON, UT_EXACT+UT_UPKID+0, ACT_ASGCOLON,
				TOK_COLON, UT_EXACT+UT_UPKID+1, ACT_ASGCOLON,
				TOK_EQ, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_DO, UT_ANY, ACT_CHGOTO+3,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"for loop"
				)
	/* pointer to the expression trees for start and finish, plus
	the variable tree for the control variable.  A flag for downto
	and a pointer to the statement */

makenode( ST_DOWNTO, ` { "!c!kfor !(variable)!1p := !(start)!2p downto !(final)!3p do begin\n",
				"!4l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,0,0,1} ',
				`F_E2KIDS | F_LINE | F_INDENT, 0',
				`4, 4',
				`{ C_VAR, C_EXP, C_EXP, CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_TO, UT_ANY, ACT_MTO,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+3,
				TOK_ASSIGN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_DOWNTO, UT_EXACT+UT_UPKID+1, ACT_CHGOTO+2,
				TOK_DOWNTO, UT_UPKID+0, ACT_CHGOTO+2,
				TOK_COLON, UT_EXACT+UT_UPKID+0, ACT_ASGCOLON,
				TOK_COLON, UT_EXACT+UT_UPKID+1, ACT_ASGCOLON,
				TOK_DO, UT_ANY, ACT_CHGOTO+3,
				TOK_EQ, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"reverse for loop"
				)

makenode( ST_WHILE, ` { "!c!kwhile !(Condition)!1p do begin\n",
				"!2l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,1} ',
				`F_LINE | F_INDENT, 0',
				`2, 2',
				`{ C_EXP, CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+1,
				TOK_DO, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"while loop"
				)
	/* a pointer to the conditional expression tree and to the
	   loop body statement */

makenode( ST_REPEAT, ` { "!c!krepeat\n",
				"!1l\n",
				"!kuntil !(Condition)!2p;\n",0 } ',
				` {0, 1,2} ',
				`F_LINE | F_INDENT, 0',
				`2, 2',
				`{ CLIST(C_STATEMENT), C_EXP } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_UNTIL, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_UPKID+1, ACT_H_EXPAND,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_PCODE+0, ACT_BLCR+0,
				0 }',
				"repeat loop"
				)
	/* a pointer to the conditional expression tree and to the loop
	   body statement again */

makenode( ST_WITH, `{ "!c!kwith !(record-variable)!1p do begin\n",
				"!2l\n",
				"!T!kend;\n",
				0 } ',
				`{0,0,1}',
				`F_E1KID | F_LINE | F_INDENT, 0',
				`2, 3',
				`{ C_VAR, CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"with block"
				)
	/* a pointer to a list of variables and to the list of statements */

makenode( ST_CASE, `{ "!c!kcase !1p of\n",
				"!2l\n",
				"!T!kend;\n",0 } ',
				` {0, 0,1} ',
				`F_E1KID | F_LINE | F_INDENT, 0',
				`2, 2',
				`{ C_EXP, CLIST(C_CASE) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_OF, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"case"
				)
	/* a pointer to the expression and the list of cases
	   Also a pointer to the else clause, unless we want to make
	   this a single case? */

makenode( ST_BLOCK, ` { "!c!kbegin\n",
				"!1l\n",
				"!T!kend;\n",0 } ',
				` { 0, 1 } ',
				`F_E1KID | F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"compound statement"
				)

makenode( ST_COUT, ` { "!c{\n",
				"!1l\n",
				"}\n",0 } ',
				`{ 0, 1 }',
				`F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_STATEMENT) } ',
				`NULACT',
				"commented out code"
				)



makenode( CASE, ` { "!1,!c: !kbegin\n",
				"!2l\n",
				"!T!kend;\n",
				0 }',
				` {0, 0,1} ',
				`F_LINE | F_INDENT, 1',
				`2, 2',
				`{ CLIST(C_CASECONST), CLIST(C_STATEMENT) } ',
				`{ TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_UPKID+0+UT_EXACT, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				TOK_DOTDOT, UT_UPKID+0, ACT_CDOTDOT,
				TOK_DOT, UT_UPKID+0, ACT_CDOTDOT,
				0 }',
				"case instance"
				)
	/* a pointer to a list of case labels and to the statement */
makenode( CASE_ELSE, ` { "!c!kelse begin\n",
				"!1l\n",
				"!T!kend;\n",0 } ',
				` { 0, 1 } ',
				`F_LINE | F_INDENT, 0',
				`1, 1',
				`{ CLIST(C_STATEMENT) } ',
				`{ TOK_ELSE, UT_ANY, ACT_CHGOTO+0,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_EXACT, ACT_BLCR,
				0}',
				"case else"
				)
				

setequal( LAST_RUN )
/* the following are expression nodes */
/* all expression nodes have an extra word to put their type in */

mfakenode( EXP_EX1, ` { 0 } ',
				NILCHL,
				`F_LINE, 0',
				`0, 0', /* array had bounds and size kids */
				`{ 0 } ',
				`NULACT',
				0
				)
makenode( EXP_SHL, ` { "!1p !cshl !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"shift left",
				5
				)
setequal( FIRST_EXPRESSION )
setequal( FIRST_VALUE )

makenode( OEXP_2, ` { "!1p!c:!2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"field width",
				0
				)
makenode( OEXP_3, ` { "!1p!c:!2p:!3p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`3, 4',
				`{ C_EXP, C_EXP, C_EXP } ',
				`NULACT',
				"precision",
				0
				)
makenode( CON_PLUS, ` { "!c+!1p",0 } ',
				NILCHL,
				`F_E1KID, 0',
				`1, 1',
				`{ C_CONSTANT }',
				`NULACT',
				"signed constant",
				0
				)
makenode( CON_MINUS, ` { "!c-!1p",0 } ',
				NILCHL,
				`F_E1KID, 0',
				`1, 1',
				`{ C_CONSTANT }',
				`NULACT',
				"negative constant",
				0
				)
makenode( CON_INT, ` { "!c!ai!1E",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 2',
				`{NILCHL}',
				`NULACT',
				"integer constant",
				0
				)
makenode( CON_REAL, ` { "!c!aj!1E",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1+sizeof(rfloat)/sizeof(pointer)',
				`{NILCHL}',
				`NULACT',
				"real constant",
				0
				)
/* octal 47 is the single quote which is the m4 character */
makenode( CON_CHAR, ` { "!c!ah!1C",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 2',
				`{NILCHL}',
				`NULACT',
				"character constant",
				0
				)
/* this node exists as a special type node "packed array[1..kid2(n)] of char"*/
makenode( CON_STRING, ` { "!c!ak!1S",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 2',
				`{NILCHL}',
				`NULACT',
				"string constant",
				0
				)

makenode( VAR_ARRAY, ` { "!1p!c[!(Subscript)!2p]",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_VAR, C_EXP } ',
				`{ TOK_COMMA, UT_UPKID+1, ACT_INDEX, /* special */
				TOK_RBRACKET, UT_ANY, ACT_PUTME,
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"array indexing",
				0
				)
	/* pointer to array entry in symbol table and list of
	   index expression trees */

makenode( VAR_POINTER, ` { "!1p!c^",0 } ',
				NILCHL,
				`F_TYPE | F_E1KID, 1',
				`1, 2',
				`{ C_VAR } ',
				`{
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"indirection",
				0
				)
	/* pointer to pointer variable in sym tab */

makenode( VAR_FIELD, ` { "!1p!c.!2p",0 } ',
				NILCHL,
				`F_TYPE | F_E1KID, 1',
				`2, 3',
				`{ C_VAR, C_FLD_NAME } ',
				`{
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"field use",
				0
				)
makenode( VF_WITH, ` { "!c!1I",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 2',
				{NILCHL},
				`{
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"implied field use",
				0
				)
	/* pointer to record variable in sym tab and pointer to field
	   name in symbol table */
makenode( EXP_NIL, ` { "!c!knil",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 0', /* not sure */
				{ 0 },
				`NULACT',
				"nil pointer",
				0
				)
makenode( EXP_PAREN, ` { "!c(!1p)",0 } ',
				NILCHL,
				`F_E1KID, 0',
				`1, 2',
				`{ C_EXP } ',
				`{ TOK_RPAREN, UT_ANY, ACT_IGNORE,
				0 }',
				"parentheses",
				0
				)
makenode( EXP_SHR, ` { "!1p !cshr !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"shift right",
				5
				)
makenode( EXP_SET, ` { "!c[!1,]",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`1, 1',
				`{ CLIST(C_EXP) } ',
				`{ TOK_LBRACKET, UT_ANY, ACT_REEXPAND,
				TOK_RBRACKET, UT_UPKID+0, ACT_IGNORE,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				 0}',
				"set",
				0
				)
	/* pointer to list of set nodes */
makenode( EXP_NOT, ` { "!c!knot !1p",0 } ',
				NILCHL,
				`F_TYPE| F_E1KID, 0',
				`1, 2',
				`{ C_EXP } ',
				`NULACT',
				"not",
				1
				)
makenode( EXP_FUNC, ` { "!1p!c!2P,",0 } ',
				NILCHL,
				`NULLBYTE, 1',
				`2, 2',
				`{ C_FUN_ID, CLIST(C_EXP) } ',
				`{ TOK_COMMA, UT_UPKID+1, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_IGNORE,
				TOK_LPAREN, UT_ANY, ACT_REEXPAND,
				0 }',
				"function call",
				0
				)
makenode( EXP_UPLUS, ` { "!c+!1p",0 } ',
				NILCHL,
				`F_E1KID, 0',
				`1, 1',
				`{ C_EXP } ',
				`NULACT',
				"unary plus",
				3
				)
makenode( EXP_UMINUS, ` { "!c-!1p",0 } ',
				NILCHL,
				`F_E1KID, 0',
				`1, 1',
				`{ C_EXP } ',
				`NULACT',
				"unary minus",
				3
				)
makenode( EXP_PLUS, ` { "!1p !c+ !2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"addition",
				10
				)
makenode( EXP_MINUS, ` { "!1p !c- !2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"subtraction",
				10
				)
makenode( EXP_OR, ` { "!1p !c!kor !2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"or",
				10
				)
makenode( EXP_TIMES, ` { "!1p!c*!2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"multiplication",
				5
				)
makenode( EXP_SLASH, ` { "!1p!c/!2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"division",
				5
				)
makenode( EXP_DIV, ` { "!1p !c!kdiv !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"integer divide",
				5
				)
makenode( EXP_MOD, ` { "!1p !c!kmod !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"modulus",
				5
				)
makenode( EXP_AND, ` { "!1p !c!kand !2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"and",
				5
				)
makenode( EXP_EQ, ` { "!1p !c= !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"equality",
				15
				)
makenode( EXP_NE, ` { "!1p !c<> !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"not equal",
				15
				)
makenode( EXP_LT, ` { "!1p !c< !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"less than",
				15
				)
makenode( EXP_LE, ` { "!1p !c<= !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"less or equal",
				15
				)
makenode( EXP_GT, ` { "!1p !c> !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"greater than",
				15
				)
makenode( EXP_GE, ` { "!1p !c>= !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"greater or equal",
				15
				)
makenode( EXP_IN, ` { "!1p !c!kin !2p",0 } ',
				NILCHL,
				`F_E2KIDS, 1',
				`2, 2',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"set membership",
				15
				)
makenode( EXP_XOR, ` { "!1p !cxor !2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`NULACT',
				"exclusive or",
				10
				)
makenode( SET_SUBRANGE, ` { "!1p!c..!2p",0 } ',
				NILCHL,
				`F_TYPE | F_E2KIDS, 1',
				`2, 3',
				`{ C_EXP, C_EXP } ',
				`{ TOK_DOTDOT, UT_ANY, ACT_CHGOTO+1,
				TOK_DOT, UT_ANY, ACT_CHGOTO+1,
				0 }',
				"range set",
				0
				)
	/* two expression trees giving the subrange */

	/* Expression syntax error, stored as a string */
makenode( EXP_ERROR, ` { "!c!ac!1E", 0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1',
				`{ 0 }',
				`NULACT',
				"bad expression",
				0
				)

	/* Extra node so we don't have to reinsert it */
setequal( LAST_EXPRESSION )
makenode( REVEAL, ` { 		"!c!k{+Revealed !1p }\n",
				"!2l\n",
				"!H\n",
				0 } ',
				`{0, 0,1}',
				`F_DECLARE | F_LINE, 0',
				`2, 2',
				`{ C_COMMENT, CLIST(C_PASSUP) }',
				`{
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 }',
				"revealed code"
				)
makenode( HIDE, ` { "!c!k... {!1p} ...", 0 } ',
				NILCHL,
				`F_DECLARE | F_LINE, 0',
				`1, 2',
				`{ C_COMMENT, CLIST(C_PASSUP) }',
				`{
				TOK_CR, UT_ANY, ACT_CR,
				TOK_DOT, UT_ANY, ACT_CHGOTO+0,
				0 }',
				"hidden code"
				)

makenode( STUB,			`{ "!c!s", 0 }',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1',
				`{NILCHL}',
				`NULACT',
				"placeholder"
				)
setequal( LAST_STANDARD )

/* special nodes */
makenode( ID, ` { "!c!1I",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1',
				`{NILCHL}',
				`{
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"Symbol"
				)
makenode( NAME, ` { "!c!1N",0 } ',
				NILCHL,
				`NULLBYTE, 0',
				`0, 1',
				`{NILCHL}',
				`{
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 }',
				"Unref Symbol"
				)
	/* symbol table entry */
makenode( LIST, ` { "!c!L",0 } ',
				NILCHL,
				`F_DECLARE | F_NOSTOP, 0',
				`0, 0', /* special case */
				` { 0 } ',
				`NULACT',
				"List"
				)

makenode( SYMBOL_TABLE, ` { 0 } ',
				NILCHL,
				`F_NOSTOP, 0',
				`0,1',
				`{ 0 }',
				`NULACT',
				"Symbol Table"
				)
makenode( DECL_ID, `{ "!c!D", 0 }',
				NILCHL,
				`NULLBYTE, 0',
				`0, 0',
				`{ 0 }',
				`NULACT',
				"name"
				)

divert(9)
/* the following nodes are not real nodes, just codes for do_exp_prod */
`#define' N_HIDECL_ID N_DECL_ID+1
`#define' N_ID_FIELD N_DECL_ID+2


divert(2)
0 };
divert(4)
0 };
divert(5)
0 };
divert(6)
0 };
divert(7)
0 };
`#ifdef QNX'
`char	NodeEnd;'
`#endif'
