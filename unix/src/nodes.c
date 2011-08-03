/*
 *	The node types for the first byte in any node
 */

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







#include "whoami.h"
#include "altypes.h"
#include "flags.h"
#include "class.h"
#include "tune.h"
#include "node.h"
#ifdef QNX
char	NodeStart;
#endif
static char *PNULL[] =  {"!c", 0} ;

static struct node_info sNULL = { PNULL, NILCHL,F_LINE, 0,1, 1};
static char *PT_COMMENT[] =  {"!c!ag!1E",0} ;

static struct node_info sT_COMMENT = { PT_COMMENT, NILCHL,NULLBYTE, 0,0, 1};
static char *PPROGRAM[] =  {
				"!c!kprogram !(Program-Name)!1p!2P,;\n",
				"!3l\n",
				"!4l\n",
				"!kbegin\n",
				"!5l\n",
				"!kend.\n",
				0} ;
static bits8 IPROGRAM[] =  {0,0,0,1,2,4,1} ;
static struct node_info sPROGRAM = { PPROGRAM, IPROGRAM,F_DECLARE | F_LINE | F_SYMBOL |F_SCOPE, 0,5, 7};
static char *PDECL_LABEL[] =  { "!c!klabel !1,;\n",0 } ;

static struct node_info sDECL_LABEL = { PDECL_LABEL, NILCHL,F_DECLARE | F_LINE, 0,1, 1};
static char *PDECL_CONST[] =  { 	"!c!kconst",
				"!1l\n",
				"",
				0 } ;
static bits8 IDECL_CONST[] =  { 0, 1 } ;
static struct node_info sDECL_CONST = { PDECL_CONST, IDECL_CONST,F_DECLARE | F_LINE | F_INDENT, 0,1, 1};
static char *PDECL_TYPE[] =  { 	"!c!ktype",
				"!1l\n",
				"",0 } ;
static bits8 IDECL_TYPE[] =  { 0, 1 } ;
static struct node_info sDECL_TYPE = { PDECL_TYPE, IDECL_TYPE,F_DECLARE | F_LINE | F_INDENT, 0,1, 1};
static char *PDECL_VAR[] =  { 	"!c!kvar",
				"!1l\n",
				"",0 } ;
static bits8 IDECL_VAR[] =  { 0, 1 } ;
static struct node_info sDECL_VAR = { PDECL_VAR, IDECL_VAR,F_DECLARE | F_LINE | F_INDENT, 0,1, 1};
static char *PDECL_PROC[] =  { 
				"!n!c!kprocedure !(proc-name)!1p!f{!2P;!f};\n",
				"!3l\n",
				"!4l\n",
				"!T!kbegin\n",
				"!5l\n",
				"!T!kend;\n", 0} ;
static bits8 IDECL_PROC[] =  {0, 0,0,1,2,4} ;
static struct node_info sDECL_PROC = { PDECL_PROC, IDECL_PROC,F_LINE|F_INDENT|F_SYMBOL |F_SCOPE,0,5, 7};
static char *PDECL_FUNC[] =  { 
				"!n!c!kfunction !(func-name)!1p!f{!2P; : !3p!f};\n",
				"!4l\n",
				"!5l\n",
				"!T!kbegin\n",
				"!6l\n",
				"!T!kend;\n", 0} ;
static bits8 IDECL_FUNC[] =  {0, 0,0,0,1,2,4} ;
static struct node_info sDECL_FUNC = { PDECL_FUNC, IDECL_FUNC,F_LINE|F_INDENT|F_SYMBOL |F_SCOPE,0,6, 8};
static char *PFORWARD[] =  { "!c!kForward <!1p>\n",0 } ;

static struct node_info sFORWARD = { PFORWARD, NILCHL,F_LINE , 0,1, 1};
static char *PCONST_INIT[] =  { "!1p !c!k: !2p = !3p;!4o!\t!4{!o\n", 0 } ;

static struct node_info sCONST_INIT = { PCONST_INIT, NILCHL,F_LINE|F_DECLARE,1,4, 4};
static char *PCONST_DECL[] =  { "!1p !c!k= !2p;!\t!3{\n",0 } ;

static struct node_info sCONST_DECL = { PCONST_DECL, NILCHL,F_DECLARE | F_LINE, 1,3, 4};
static char *PTYPE_DECL[] =  { "!1p !c!k= !2p!R!\t!3{\n",0 } ;

static struct node_info sTYPE_DECL = { PTYPE_DECL, NILCHL,F_DECLARE | F_PMULTI | F_LINE, 1,3, 3};
static char *PVAR_DECL[] =  { "!1, !c!k: !2p!R!\t!3{\n",0 } ;

static struct node_info sVAR_DECL = { PVAR_DECL, NILCHL,F_DECLARE | F_PMULTI | F_LINE, 1,3, 3};
static char *PTYP_ENUM[] =  { "!c!k(!1,)",0 } ;

static struct node_info sTYP_ENUM = { PTYP_ENUM, NILCHL,F_DECLARE, 0,1, 1};
static char *PTYP_SUBRANGE[] =  { "!(lower-bound)!1p!c!k..!(upper-bound)!2p",0 } ;

static struct node_info sTYP_SUBRANGE = { PTYP_SUBRANGE, NILCHL,NULLBYTE, 1,2, 4};
static char *PTYP_PACKED[] =  { "!c!kpacked !1p",0 } ;

static struct node_info sTYP_PACKED = { PTYP_PACKED, NILCHL,F_DECLARE, 0,1, 1};
static char *PTYP_ARRAY[] =  { "!c!karray [!(range)!1p] of !2p",0 } ;

static struct node_info sTYP_ARRAY = { PTYP_ARRAY, NILCHL,F_DECLARE, 0,2, 5};
static char *PINIT_STRUCT[] =  { "!c!k(!1:)", 0 } ;

static struct node_info sINIT_STRUCT = { PINIT_STRUCT, NILCHL,0,0,1, 2};
static char *PTYP_STRING[] =  { "!k!cstring[!(Max Length)!1p]",0 } ;

static struct node_info sTYP_STRING = { PTYP_STRING, NILCHL,F_DECLARE, 0,1, 2};
static char *PFLD_INIT[] =  { "!1p!c:!2p", 0 } ;

static struct node_info sFLD_INIT = { PFLD_INIT, NILCHL,0, 1,2, 2};
static char *PTYP_SET[] =  { "!c!kset of !1p",0 } ;

static struct node_info sTYP_SET = { PTYP_SET, NILCHL,F_DECLARE, 0,1, 3};
static char *PTYP_FILE[] =  { "!c!kfile of !(file-element-type)!1p",0 } ;

static struct node_info sTYP_FILE = { PTYP_FILE, NILCHL,F_DECLARE, 0,1, 1};
static char *PTYP_POINTER[] =  { "!c!k^!1p",0 } ;

static struct node_info sTYP_POINTER = { PTYP_POINTER, NILCHL,NULLBYTE, 0,1, 1};
static char *PTYP_RECORD[] =  { "!c!krecord",
				"!1l\n",
				"!T!kend;\n",0 } ;
static bits8 ITYP_RECORD[] =  { 0, 1 } ;
static struct node_info sTYP_RECORD = { PTYP_RECORD, ITYP_RECORD,F_DECLARE | F_SYMBOL | F_INDENT, 0,1, 3};
static char *PFIELD[] =  { "!(field-name)!1, !c!k: !2p!R!\t!3{",0 } ;

static struct node_info sFIELD = { PFIELD, NILCHL,F_DECLARE | F_PMULTI | F_LINE, 1,3, 3};
static char *PVARIANT[] =  { "!c!kcase !(tag-name)!1p : !(tag-type)!2p of\n",
				"!3l\n",
				"{end variant part}\n",
				0 } ;
static bits8 IVARIANT[] = {0, 0, 0, 1 };
static struct node_info sVARIANT = { PVARIANT, IVARIANT,F_DECLARE | F_LINE, 0,3, 4};
static char *PST_VCASE[] =  { "!1, !c!k: (\n",
				"!2l\n",
				"!T!k)\n",
				0 } ;
static bits8 IST_VCASE[] = {0, 0, 1 };
static struct node_info sST_VCASE = { PST_VCASE, IST_VCASE,F_INDENT | F_DECLARE | F_LINE, 1,2, 2};
static char *PFORM_VALUE[] =  { "!(parm-name)!1,!c!k: !2p",0 } ;

static struct node_info sFORM_VALUE = { PFORM_VALUE, NILCHL,F_DECLARE, 1,2, 2};
static char *PFORM_REF[] =  { "!c!kvar !(parm-name)!1,: !2p",0 } ;

static struct node_info sFORM_REF = { PFORM_REF, NILCHL,F_DECLARE, 0,2, 2};
static char *PFORM_FUNCTION[] =  { "!c!kfunction !(func-name)!1p!2P; : !3p",0 } ;

static struct node_info sFORM_FUNCTION = { PFORM_FUNCTION, NILCHL,F_SYMBOL | F_SCOPE, 0,3, 4};
static char *PFORM_PROCEDURE[] =  { "!c!kprocedure !(proc-name)!1p!2P;",0 } ;

static struct node_info sFORM_PROCEDURE = { PFORM_PROCEDURE, NILCHL,F_SYMBOL | F_SCOPE, 0,2, 3};
static char *PNOTDONE[] =  { "!1p !c{non-ALICE}\n",0 } ;

static struct node_info sNOTDONE = { PNOTDONE, NILCHL,F_LINE, 1,1, 1};
static char *PST_COMMENT[] =  { "!c!1{\n",0 } ;

static struct node_info sST_COMMENT = { PST_COMMENT, NILCHL,F_LINE, 0,1, 1};
static char *PST_LABEL[] =  { "!1p!c:\n",0 } ;

static struct node_info sST_LABEL = { PST_LABEL, NILCHL,F_LINE, 1,1, 1};
static char *PST_GOTO[] =  { "!c!kgoto !(label)!1p;\n",0 } ;

static struct node_info sST_GOTO = { PST_GOTO, NILCHL,F_LINE, 0,1, 1};
static char *PST_CALL[] =  { "!1p!c!2P,;\n",0 } ;

static struct node_info sST_CALL = { PST_CALL, NILCHL,F_LINE, 1,2, 2};
static char *PST_ASSIGN[] =  { "!1p !c:= !2p;\n",0 } ;

static struct node_info sST_ASSIGN = { PST_ASSIGN, NILCHL,F_E2KIDS | F_LINE, 1,2, 3};
static char *PIMM_BLOCK[] =  { "!c{ -----Immediate Statement Block-----\n",
				"!1l\n",
				"begin\n",
				"!2l\n",
				"  -----End Immediate Statements-----}\n",
				0 } ;
static bits8 IIMM_BLOCK[] = {0, 1, 3 };
static struct node_info sIMM_BLOCK = { PIMM_BLOCK, IIMM_BLOCK,F_SCOPE | F_LINE |F_SYMBOL, 0,2, 4};
static char *PST_SPECIAL[] =  { "!k!1p !cbegin\n",
				"!2l\n",
				"!T!kend;\n",
				0 } ;
static bits8 IST_SPECIAL[] =  {0, 0, 1 } ;
static struct node_info sST_SPECIAL = { PST_SPECIAL, IST_SPECIAL,F_LINE | F_INDENT, 1,2, 2};
static char *PST_IF[] =  { "!c!kif !(Condition)!1p then begin\n",
				"!2l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_IF[] =  {0, 0,1} ;
static struct node_info sST_IF = { PST_IF, IST_IF,F_E1KID | F_LINE | F_INDENT, 0,2, 2};
static char *PST_ELSE[] =  { "!c!kif !(Condition)!1p then begin\n",
				"!2l\n",
				"!T!kend\n",
				" !kelse begin\n",
				"!3l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_ELSE[] =  {0, 0,1,4} ;
static struct node_info sST_ELSE = { PST_ELSE, IST_ELSE,F_E1KID | F_LINE | F_INDENT, 0,3, 3};
static char *PST_FOR[] =  { "!c!kfor !(variable)!1p := !(start)!2p to !(finish)!3p do begin\n",
				"!4l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_FOR[] =  {0, 0,0,0,1} ;
static struct node_info sST_FOR = { PST_FOR, IST_FOR,F_E2KIDS | F_LINE | F_INDENT, 0,4, 4};
static char *PST_DOWNTO[] =  { "!c!kfor !(variable)!1p := !(start)!2p downto !(final)!3p do begin\n",
				"!4l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_DOWNTO[] =  {0, 0,0,0,1} ;
static struct node_info sST_DOWNTO = { PST_DOWNTO, IST_DOWNTO,F_E2KIDS | F_LINE | F_INDENT, 0,4, 4};
static char *PST_WHILE[] =  { "!c!kwhile !(Condition)!1p do begin\n",
				"!2l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_WHILE[] =  {0, 0,1} ;
static struct node_info sST_WHILE = { PST_WHILE, IST_WHILE,F_LINE | F_INDENT, 0,2, 2};
static char *PST_REPEAT[] =  { "!c!krepeat\n",
				"!1l\n",
				"!kuntil !(Condition)!2p;\n",0 } ;
static bits8 IST_REPEAT[] =  {0, 1,2} ;
static struct node_info sST_REPEAT = { PST_REPEAT, IST_REPEAT,F_LINE | F_INDENT, 0,2, 2};
static char *PST_WITH[] = { "!c!kwith !(record-variable)!1p do begin\n",
				"!2l\n",
				"!T!kend;\n",
				0 } ;
static bits8 IST_WITH[] = {0,0,1};
static struct node_info sST_WITH = { PST_WITH, IST_WITH,F_E1KID | F_LINE | F_INDENT, 0,2, 3};
static char *PST_CASE[] = { "!c!kcase !1p of\n",
				"!2l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_CASE[] =  {0, 0,1} ;
static struct node_info sST_CASE = { PST_CASE, IST_CASE,F_E1KID | F_LINE | F_INDENT, 0,2, 2};
static char *PST_BLOCK[] =  { "!c!kbegin\n",
				"!1l\n",
				"!T!kend;\n",0 } ;
static bits8 IST_BLOCK[] =  { 0, 1 } ;
static struct node_info sST_BLOCK = { PST_BLOCK, IST_BLOCK,F_E1KID | F_LINE | F_INDENT, 0,1, 1};
static char *PST_COUT[] =  { "!c{\n",
				"!1l\n",
				"}\n",0 } ;
static bits8 IST_COUT[] = { 0, 1 };
static struct node_info sST_COUT = { PST_COUT, IST_COUT,F_LINE | F_INDENT, 0,1, 1};
static char *PCASE[] =  { "!1,!c: !kbegin\n",
				"!2l\n",
				"!T!kend;\n",
				0 };
static bits8 ICASE[] =  {0, 0,1} ;
static struct node_info sCASE = { PCASE, ICASE,F_LINE | F_INDENT, 1,2, 2};
static char *PCASE_ELSE[] =  { "!c!kelse begin\n",
				"!1l\n",
				"!T!kend;\n",0 } ;
static bits8 ICASE_ELSE[] =  { 0, 1 } ;
static struct node_info sCASE_ELSE = { PCASE_ELSE, ICASE_ELSE,F_LINE | F_INDENT, 0,1, 1};
static char *PEXP_SHL[] =  { "!1p !cshl !2p",0 } ;

static struct node_info sEXP_SHL = { PEXP_SHL, NILCHL,F_E2KIDS, 1,2, 2};
static char *POEXP_2[] =  { "!1p!c:!2p",0 } ;

static struct node_info sOEXP_2 = { POEXP_2, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *POEXP_3[] =  { "!1p!c:!2p:!3p",0 } ;

static struct node_info sOEXP_3 = { POEXP_3, NILCHL,F_TYPE | F_E2KIDS, 1,3, 4};
static char *PCON_PLUS[] =  { "!c+!1p",0 } ;

static struct node_info sCON_PLUS = { PCON_PLUS, NILCHL,F_E1KID, 0,1, 1};
static char *PCON_MINUS[] =  { "!c-!1p",0 } ;

static struct node_info sCON_MINUS = { PCON_MINUS, NILCHL,F_E1KID, 0,1, 1};
static char *PCON_INT[] =  { "!c!ai!1E",0 } ;

static struct node_info sCON_INT = { PCON_INT, NILCHL,NULLBYTE, 0,0, 2};
static char *PCON_REAL[] =  { "!c!aj!1E",0 } ;

static struct node_info sCON_REAL = { PCON_REAL, NILCHL,NULLBYTE, 0,0, 1+sizeof(rfloat)/sizeof(pointer)};
static char *PCON_CHAR[] =  { "!c!ah!1C",0 } ;

static struct node_info sCON_CHAR = { PCON_CHAR, NILCHL,NULLBYTE, 0,0, 2};
static char *PCON_STRING[] =  { "!c!ak!1S",0 } ;

static struct node_info sCON_STRING = { PCON_STRING, NILCHL,NULLBYTE, 0,0, 2};
static char *PVAR_ARRAY[] =  { "!1p!c[!(Subscript)!2p]",0 } ;

static struct node_info sVAR_ARRAY = { PVAR_ARRAY, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PVAR_POINTER[] =  { "!1p!c^",0 } ;

static struct node_info sVAR_POINTER = { PVAR_POINTER, NILCHL,F_TYPE | F_E1KID, 1,1, 2};
static char *PVAR_FIELD[] =  { "!1p!c.!2p",0 } ;

static struct node_info sVAR_FIELD = { PVAR_FIELD, NILCHL,F_TYPE | F_E1KID, 1,2, 3};
static char *PVF_WITH[] =  { "!c!1I",0 } ;

static struct node_info sVF_WITH = { PVF_WITH, NILCHL,NULLBYTE, 0,0, 2};
static char *PEXP_NIL[] =  { "!c!knil",0 } ;

static struct node_info sEXP_NIL = { PEXP_NIL, NILCHL,NULLBYTE, 0,0, 0};
static char *PEXP_PAREN[] =  { "!c(!1p)",0 } ;

static struct node_info sEXP_PAREN = { PEXP_PAREN, NILCHL,F_E1KID, 0,1, 2};
static char *PEXP_SHR[] =  { "!1p !cshr !2p",0 } ;

static struct node_info sEXP_SHR = { PEXP_SHR, NILCHL,F_E2KIDS, 1,2, 2};
static char *PEXP_SET[] =  { "!c[!1,]",0 } ;

static struct node_info sEXP_SET = { PEXP_SET, NILCHL,NULLBYTE, 0,1, 1};
static char *PEXP_NOT[] =  { "!c!knot !1p",0 } ;

static struct node_info sEXP_NOT = { PEXP_NOT, NILCHL,F_TYPE| F_E1KID, 0,1, 2};
static char *PEXP_FUNC[] =  { "!1p!c!2P,",0 } ;

static struct node_info sEXP_FUNC = { PEXP_FUNC, NILCHL,NULLBYTE, 1,2, 2};
static char *PEXP_UPLUS[] =  { "!c+!1p",0 } ;

static struct node_info sEXP_UPLUS = { PEXP_UPLUS, NILCHL,F_E1KID, 0,1, 1};
static char *PEXP_UMINUS[] =  { "!c-!1p",0 } ;

static struct node_info sEXP_UMINUS = { PEXP_UMINUS, NILCHL,F_E1KID, 0,1, 1};
static char *PEXP_PLUS[] =  { "!1p !c+ !2p",0 } ;

static struct node_info sEXP_PLUS = { PEXP_PLUS, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_MINUS[] =  { "!1p !c- !2p",0 } ;

static struct node_info sEXP_MINUS = { PEXP_MINUS, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_OR[] =  { "!1p !c!kor !2p",0 } ;

static struct node_info sEXP_OR = { PEXP_OR, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_TIMES[] =  { "!1p!c*!2p",0 } ;

static struct node_info sEXP_TIMES = { PEXP_TIMES, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_SLASH[] =  { "!1p!c/!2p",0 } ;

static struct node_info sEXP_SLASH = { PEXP_SLASH, NILCHL,F_E2KIDS, 1,2, 2};
static char *PEXP_DIV[] =  { "!1p !c!kdiv !2p",0 } ;

static struct node_info sEXP_DIV = { PEXP_DIV, NILCHL,F_E2KIDS, 1,2, 2};
static char *PEXP_MOD[] =  { "!1p !c!kmod !2p",0 } ;

static struct node_info sEXP_MOD = { PEXP_MOD, NILCHL,F_E2KIDS, 1,2, 2};
static char *PEXP_AND[] =  { "!1p !c!kand !2p",0 } ;

static struct node_info sEXP_AND = { PEXP_AND, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_EQ[] =  { "!1p !c= !2p",0 } ;

static struct node_info sEXP_EQ = { PEXP_EQ, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_NE[] =  { "!1p !c<> !2p",0 } ;

static struct node_info sEXP_NE = { PEXP_NE, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_LT[] =  { "!1p !c< !2p",0 } ;

static struct node_info sEXP_LT = { PEXP_LT, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_LE[] =  { "!1p !c<= !2p",0 } ;

static struct node_info sEXP_LE = { PEXP_LE, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_GT[] =  { "!1p !c> !2p",0 } ;

static struct node_info sEXP_GT = { PEXP_GT, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_GE[] =  { "!1p !c>= !2p",0 } ;

static struct node_info sEXP_GE = { PEXP_GE, NILCHL,F_E2KIDS, 1,2, 3};
static char *PEXP_IN[] =  { "!1p !c!kin !2p",0 } ;

static struct node_info sEXP_IN = { PEXP_IN, NILCHL,F_E2KIDS, 1,2, 2};
static char *PEXP_XOR[] =  { "!1p !cxor !2p",0 } ;

static struct node_info sEXP_XOR = { PEXP_XOR, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PSET_SUBRANGE[] =  { "!1p!c..!2p",0 } ;

static struct node_info sSET_SUBRANGE = { PSET_SUBRANGE, NILCHL,F_TYPE | F_E2KIDS, 1,2, 3};
static char *PEXP_ERROR[] =  { "!c!ac!1E", 0 } ;

static struct node_info sEXP_ERROR = { PEXP_ERROR, NILCHL,NULLBYTE, 0,0, 1};
static char *PREVEAL[] =  { 		"!c!k{+Revealed !1p }\n",
				"!2l\n",
				"!H\n",
				0 } ;
static bits8 IREVEAL[] = {0, 0,1};
static struct node_info sREVEAL = { PREVEAL, IREVEAL,F_DECLARE | F_LINE, 0,2, 2};
static char *PHIDE[] =  { "!c!k... {!1p} ...", 0 } ;

static struct node_info sHIDE = { PHIDE, NILCHL,F_DECLARE | F_LINE, 0,1, 2};
static char *PSTUB[] = { "!c!s", 0 };

static struct node_info sSTUB = { PSTUB, NILCHL,NULLBYTE, 0,0, 1};
static char *PID[] =  { "!c!1I",0 } ;

static struct node_info sID = { PID, NILCHL,NULLBYTE, 0,0, 1};
static char *PNAME[] =  { "!c!1N",0 } ;

static struct node_info sNAME = { PNAME, NILCHL,NULLBYTE, 0,0, 1};
static char *PLIST[] =  { "!c!L",0 } ;

static struct node_info sLIST = { PLIST, NILCHL,F_DECLARE | F_NOSTOP, 0,0, 0};
static char *PSYMBOL_TABLE[] =  { 0 } ;

static struct node_info sSYMBOL_TABLE = { PSYMBOL_TABLE, NILCHL,F_NOSTOP, 0,0,1};
static char *PDECL_ID[] = { "!c!D", 0 };

static struct node_info sDECL_ID = { PDECL_ID, NILCHL,NULLBYTE, 0,0, 0};
struct node_info *node_table[] = {
&sNULL,
&sT_COMMENT,
&sPROGRAM,
&sDECL_LABEL,
&sDECL_CONST,
&sDECL_TYPE,
&sDECL_VAR,
&sDECL_PROC,
&sDECL_FUNC,
&sFORWARD,
&sCONST_INIT,
0,
&sCONST_DECL,
&sTYPE_DECL,
&sVAR_DECL,
&sTYP_ENUM,
&sTYP_SUBRANGE,
&sTYP_PACKED,
&sTYP_ARRAY,
&sINIT_STRUCT,
&sTYP_STRING,
&sFLD_INIT,
&sTYP_SET,
&sTYP_FILE,
&sTYP_POINTER,
&sTYP_RECORD,
&sFIELD,
&sVARIANT,
&sST_VCASE,
&sFORM_VALUE,
&sFORM_REF,
&sFORM_FUNCTION,
&sFORM_PROCEDURE,
&sNOTDONE,
&sST_COMMENT,
0,
&sST_LABEL,
&sST_GOTO,
&sST_CALL,
0,
0,
&sST_ASSIGN,
&sIMM_BLOCK,
&sST_SPECIAL,
&sST_IF,
&sST_ELSE,
&sST_FOR,
&sST_DOWNTO,
&sST_WHILE,
&sST_REPEAT,
&sST_WITH,
&sST_CASE,
&sST_BLOCK,
&sST_COUT,
&sCASE,
&sCASE_ELSE,
0,
&sEXP_SHL,
&sOEXP_2,
&sOEXP_3,
&sCON_PLUS,
&sCON_MINUS,
&sCON_INT,
&sCON_REAL,
&sCON_CHAR,
&sCON_STRING,
&sVAR_ARRAY,
&sVAR_POINTER,
&sVAR_FIELD,
&sVF_WITH,
&sEXP_NIL,
&sEXP_PAREN,
&sEXP_SHR,
&sEXP_SET,
&sEXP_NOT,
&sEXP_FUNC,
&sEXP_UPLUS,
&sEXP_UMINUS,
&sEXP_PLUS,
&sEXP_MINUS,
&sEXP_OR,
&sEXP_TIMES,
&sEXP_SLASH,
&sEXP_DIV,
&sEXP_MOD,
&sEXP_AND,
&sEXP_EQ,
&sEXP_NE,
&sEXP_LT,
&sEXP_LE,
&sEXP_GT,
&sEXP_GE,
&sEXP_IN,
&sEXP_XOR,
&sSET_SUBRANGE,
&sEXP_ERROR,
&sREVEAL,
&sHIDE,
&sSTUB,
&sID,
&sNAME,
&sLIST,
&sSYMBOL_TABLE,
&sDECL_ID,

0 };

