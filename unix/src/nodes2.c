/* BEGIN_ARRAY */
#include "whoami.h"
#define CAST(atp) (atp)
#include "altypes.h"
#include "node.h"
#include "action.h"
#include "token.h"
#include "class.h"
#define UT_ANY		0
#define UT_EXACT	0x10
#define UT_UPKID	0x20
#define UT_PCODE	0x40
static ActionNum NULACT[] = {0};

static ClassNum cNULL[] = { C_PROGRAM };

static ClassNum cT_COMMENT[] = {NILCHL};
static ActionNum aPROGRAM[] = { TOK_BEGIN, UT_ANY, ACT_CHGOTO+4,
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
				0 };
static ClassNum cPROGRAM[] = /* hidden symbol child */
				{ C_PNAME,
					C_PNAME+64,
					C_BLCOMMENT+64, 
					C_DECLARATIONS+64,
					C_STATEMENT+64 } ;
static ActionNum aDECL_LABEL[] = { TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0 };
static ClassNum cDECL_LABEL[] = { C_LABEL_DECL+64 };
static ActionNum aDECL_CONST[] = { TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0};
static ClassNum cDECL_CONST[] = { C_CONST_DECL+64 };
static ActionNum aDECL_TYPE[] = { TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0};
static ClassNum cDECL_TYPE[] = { C_TYPE_DECL+64 };
static ActionNum aDECL_VAR[] = { TOK_CR, UT_PCODE+2, ACT_CR,
				TOK_CR, UT_ANY, ACT_BLCR,
				0};
static ClassNum cDECL_VAR[] = { C_VAR_DECL+64 };
static ActionNum aDECL_PROC[] = {
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
				0 };
static ClassNum cDECL_PROC[] = /* hidden symbol */
				{ C_HIDECL_ID, C_FORMAL+64,
					C_BLCOMMENT+64, 
					C_DECLARATIONS+64,
					C_STATEMENT+64 };
static ActionNum aDECL_FUNC[] = {
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
				0 };
static ClassNum cDECL_FUNC[] = /* hidden symbol */
				{ C_HIDECL_ID,
					C_FORMAL+64, C_TYPEID,
					C_BLCOMMENT+64, 
					C_DECLARATIONS+64,
					C_STATEMENT+64 };
static ActionNum aFORWARD[] = {
				TOK_CR, UT_ANY, ACT_H_EXPAND,
				0 };
static ClassNum cFORWARD[] = { C_ROUTNAME };
static ActionNum aCONST_INIT[] = { TOK_EQ, UT_ANY, ACT_CHGOTO + 2,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 3,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_LPAREN, UT_ANY, ACT_CHGOTO+2,
				0};
static ClassNum cCONST_INIT[] = { C_DECL_ID, C_TYPEID, C_INIT,
					C_COMMENT };
static ActionNum aCONST_DECL[] = { TOK_EQ, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 2,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_COLON, UT_ANY, ACT_TMINIT,
				0};
static ClassNum cCONST_DECL[] = { C_DECL_ID, C_CONSTANT, C_COMMENT };
static ActionNum aTYPE_DECL[] = { TOK_EQ, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO +2,
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cTYPE_DECL[] = { C_DECL_ID, C_TYPE, C_COMMENT };
static ActionNum aVAR_DECL[] = { TOK_COLON, UT_ANY, ACT_CHGOTO + 1,
				TOK_SEMI, UT_ANY, ACT_CHGOTO + 2,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cVAR_DECL[] = { C_DECL_ID+64, C_TYPE, C_COMMENT };
static ActionNum aTYP_ENUM[] = { TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				0};
static ClassNum cTYP_ENUM[] = { C_DECL_ID+64 };
static ActionNum aTYP_SUBRANGE[] = { TOK_DOTDOT, UT_ANY, ACT_CHGOTO+1,
				TOK_DOT, UT_ANY, ACT_CHGOTO+1,
				0 };
static ClassNum cTYP_SUBRANGE[] = /* upper and lower */
				{ C_OCONSTANT, C_OCONSTANT };

static ClassNum cTYP_PACKED[] = { C_ST_TYPE };
static ActionNum aTYP_ARRAY[] = { TOK_COMMA, UT_UPKID+0, ACT_AREXP, /* special */
				TOK_LBRACKET, UT_ANY, ACT_CHGOTO + 0,
				TOK_RBRACKET, UT_ANY, ACT_CHGOTO + 1,
				TOK_OF, UT_ANY,	ACT_CHGOTO + 1,
				0};
static ClassNum cTYP_ARRAY[] = /* array had bounds and size kids */
				{ C_SIM_TYPE, C_TYPE } ;
static ActionNum aINIT_STRUCT[] = {
				TOK_COMMA, UT_ANY, ACT_CMMA_EXPAND,
				TOK_SEMI, UT_ANY, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				0 };
static ClassNum cINIT_STRUCT[] = { C_INIT+64 };
static ActionNum aTYP_STRING[] = { 
				TOK_LBRACKET, UT_ANY, ACT_CHGOTO + 0,
				TOK_RBRACKET, UT_ANY, ACT_CRIGHT,
				0};
static ClassNum cTYP_STRING[] = /* array had bounds and size kids */
				{ C_OCONSTANT } ;
static ActionNum aFLD_INIT[] = {
				TOK_COLON, UT_ANY, ACT_CHGOTO + 1,
				0};
static ClassNum cFLD_INIT[] = { C_FLD_NAME, C_INIT } ;
static ActionNum aTYP_SET[] = { 
				TOK_OF, UT_ANY,	ACT_CHGOTO + 0,
				0};
static ClassNum cTYP_SET[] = /* upper and lower bound */
				{ C_SIM_TYPE } ;
static ActionNum aTYP_FILE[] = { 
				TOK_OF, UT_ANY,	ACT_CHGOTO + 0,
				0};
static ClassNum cTYP_FILE[] = { C_TYPE } ;

static ClassNum cTYP_POINTER[] = { C_TYPEID } ;
static ActionNum aTYP_RECORD[] = {
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_END, UT_ANY, ACT_CRIGHT,
				0};
static ClassNum cTYP_RECORD[] = /* a symtab and the size of it */
				{ C_FIELD+64 } ;
static ActionNum aFIELD[] = {
				TOK_CR, UT_ANY, ACT_CR,
				TOK_SEMI, UT_ANY, ACT_CHGOTO+2,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				0 };
static ClassNum cFIELD[] = { C_DECL_ID+64, C_TYPE, C_COMMENT } ;
static ActionNum aVARIANT[] = {
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_COLON, UT_UPKID+0, ACT_CHGOTO+1,
				0 };
static ClassNum cVARIANT[] = { C_DECL_ID, C_TYPEID, C_VARIANT+64 };
static ActionNum aST_VCASE[] = {
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR,
				TOK_RPAREN, UT_ANY, ACT_CRIGHT,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_COLON, UT_UPKID+0, ACT_CHGOTO+1,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				0};
static ClassNum cST_VCASE[] = { C_CONSTANT+64, C_FIELD+64 };
static ActionNum aFORM_VALUE[] = {
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				TOK_VAR, UT_ANY, ACT_TVAR,
				0 };
static ClassNum cFORM_VALUE[] = { C_DECL_ID+64, C_TYPEID } ;
static ActionNum aFORM_REF[] = {
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_ANY, ACT_CHGOTO+1,
				0 };
static ClassNum cFORM_REF[] = { C_DECL_ID+64, C_TYPEID } ;
static ActionNum aFORM_FUNCTION[] = {
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_C_EXPAND,
				0 };
static ClassNum cFORM_FUNCTION[] = { C_DECL_ID, C_FORMAL+64, C_TYPEID };
static ActionNum aFORM_PROCEDURE[] = {
				TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_REEXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_C_EXPAND,
				0 };
static ClassNum cFORM_PROCEDURE[] = { C_DECL_ID, C_FORMAL+64 };
static ActionNum aNOTDONE[] = {
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cNOTDONE[] = { C_COMMENT } ;
static ActionNum aST_COMMENT[] = {
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cST_COMMENT[] = { C_COMMENT } ;
static ActionNum aST_LABEL[] = {
				TOK_CR, UT_ANY, ACT_CR,
				TOK_COLON, UT_ANY, ACT_CR,
				0};
static ClassNum cST_LABEL[] = { C_LABEL } ;
static ActionNum aST_GOTO[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cST_GOTO[] = { C_LABEL } ;
static ActionNum aST_CALL[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+1, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_IGNORE,
				TOK_LPAREN, UT_ANY, ACT_REEXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				0};
static ClassNum cST_CALL[] = { C_PROC_ID, C_EXP+64 } ;
static ActionNum aST_ASSIGN[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_CR,
				TOK_ASSIGN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
#ifdef notdef
				TOK_COLON, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
#else				
				TOK_COLON, UT_ANY, ACT_ASGCOLON,
#endif
				TOK_EQ, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_LPAREN, UT_EXACT+UT_UPKID+0, ACT_CALLMAKE,
				0};
static ClassNum cST_ASSIGN[] = /* kid for size */
				{ C_VAR, C_EXP } ;
static ActionNum aIMM_BLOCK[] = {
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
				0 };
static ClassNum cIMM_BLOCK[] = { C_DECLARATIONS+64,
					C_STATEMENT+64 };
static ActionNum aST_SPECIAL[] = { TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0};
static ClassNum cST_SPECIAL[] = { C_SPECIAL, C_STATEMENT+64 };
static ActionNum aST_IF[] = { TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_THEN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				TOK_ELSE, UT_ANY, ACT_DELSE,
				0};
static ClassNum cST_IF[] = { C_EXP, C_STATEMENT+64 } ;
static ActionNum aST_ELSE[] = { TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
				TOK_ELSE, UT_ANY, ACT_CHGOTO+2,
				TOK_THEN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_BEGIN, UT_EXACT+UT_UPKID+0, ACT_CHGOTO+1,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_BLCR+1,
				TOK_CR, UT_PCODE+3, ACT_BLCR+1,
				TOK_CR, UT_PCODE+5, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0};
static ClassNum cST_ELSE[] = { C_EXP, C_STATEMENT+64, C_STATEMENT+64 } ;
static ActionNum aST_FOR[] = { TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
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
				0 };
static ClassNum cST_FOR[] = { C_VAR, C_EXP, C_EXP, C_STATEMENT+64 } ;
static ActionNum aST_DOWNTO[] = { TOK_SEMI, UT_PCODE+2, ACT_H_EXPAND,
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
				0 };
static ClassNum cST_DOWNTO[] = { C_VAR, C_EXP, C_EXP, C_STATEMENT+64 } ;
static ActionNum aST_WHILE[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+1,
				TOK_DO, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 };
static ClassNum cST_WHILE[] = { C_EXP, C_STATEMENT+64 } ;
static ActionNum aST_REPEAT[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_UNTIL, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_UPKID+1, ACT_H_EXPAND,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_PCODE+0, ACT_BLCR+0,
				0 };
static ClassNum cST_REPEAT[] = { C_STATEMENT+64, C_EXP } ;
static ActionNum aST_WITH[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 };
static ClassNum cST_WITH[] = { C_VAR, C_STATEMENT+64 } ;
static ActionNum aST_CASE[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_OF, UT_ANY, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 };
static ClassNum cST_CASE[] = { C_EXP, C_CASE+64 } ;
static ActionNum aST_BLOCK[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_END, UT_ANY, ACT_CRIGHT,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 };
static ClassNum cST_BLOCK[] = { C_STATEMENT+64 } ;

static ClassNum cST_COUT[] = { C_STATEMENT+64 } ;
static ActionNum aCASE[] = { TOK_SEMI, UT_ANY, ACT_H_EXPAND,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				TOK_COLON, UT_UPKID+0+UT_EXACT, ACT_CHGOTO+1,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				TOK_DOTDOT, UT_UPKID+0, ACT_CDOTDOT,
				TOK_DOT, UT_UPKID+0, ACT_CDOTDOT,
				0 };
static ClassNum cCASE[] = { C_CASECONST+64, C_STATEMENT+64 } ;
static ActionNum aCASE_ELSE[] = { TOK_ELSE, UT_ANY, ACT_CHGOTO+0,
				TOK_BEGIN, UT_ANY, ACT_CHGOTO+0,
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_EXACT, ACT_BLCR,
				0};
static ClassNum cCASE_ELSE[] = { C_STATEMENT+64 } ;

static ClassNum cEXP_SHL[] = { C_EXP, C_EXP } ;

static ClassNum cOEXP_2[] = { C_EXP, C_EXP } ;

static ClassNum cOEXP_3[] = { C_EXP, C_EXP, C_EXP } ;

static ClassNum cCON_PLUS[] = { C_CONSTANT };

static ClassNum cCON_MINUS[] = { C_CONSTANT };

static ClassNum cCON_INT[] = {NILCHL};

static ClassNum cCON_REAL[] = {NILCHL};

static ClassNum cCON_CHAR[] = {NILCHL};

static ClassNum cCON_STRING[] = {NILCHL};
static ActionNum aVAR_ARRAY[] = { TOK_COMMA, UT_UPKID+1, ACT_INDEX, /* special */
				TOK_RBRACKET, UT_ANY, ACT_PUTME,
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cVAR_ARRAY[] = { C_VAR, C_EXP } ;
static ActionNum aVAR_POINTER[] = {
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cVAR_POINTER[] = { C_VAR } ;
static ActionNum aVAR_FIELD[] = {
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cVAR_FIELD[] = { C_VAR, C_FLD_NAME } ;
static ActionNum aVF_WITH[] = {
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cVF_WITH[] = {NILCHL};

static ClassNum cEXP_NIL[] = /* not sure */
				{ 0 };
static ActionNum aEXP_PAREN[] = { TOK_RPAREN, UT_ANY, ACT_IGNORE,
				0 };
static ClassNum cEXP_PAREN[] = { C_EXP } ;

static ClassNum cEXP_SHR[] = { C_EXP, C_EXP } ;
static ActionNum aEXP_SET[] = { TOK_LBRACKET, UT_ANY, ACT_REEXPAND,
				TOK_RBRACKET, UT_UPKID+0, ACT_IGNORE,
				TOK_COMMA, UT_UPKID+0, ACT_C_EXPAND,
				 0};
static ClassNum cEXP_SET[] = { C_EXP+64 } ;

static ClassNum cEXP_NOT[] = { C_EXP } ;
static ActionNum aEXP_FUNC[] = { TOK_COMMA, UT_UPKID+1, ACT_CMMA_EXPAND,
				TOK_RPAREN, UT_ANY, ACT_IGNORE,
				TOK_LPAREN, UT_ANY, ACT_REEXPAND,
				0 };
static ClassNum cEXP_FUNC[] = { C_FUN_ID, C_EXP+64 } ;

static ClassNum cEXP_UPLUS[] = { C_EXP } ;

static ClassNum cEXP_UMINUS[] = { C_EXP } ;

static ClassNum cEXP_PLUS[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_MINUS[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_OR[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_TIMES[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_SLASH[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_DIV[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_MOD[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_AND[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_EQ[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_NE[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_LT[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_LE[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_GT[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_GE[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_IN[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_XOR[] = { C_EXP, C_EXP } ;
static ActionNum aSET_SUBRANGE[] = { TOK_DOTDOT, UT_ANY, ACT_CHGOTO+1,
				TOK_DOT, UT_ANY, ACT_CHGOTO+1,
				0 };
static ClassNum cSET_SUBRANGE[] = { C_EXP, C_EXP } ;

static ClassNum cEXP_ERROR[] = { 0 };
static ActionNum aREVEAL[] = {
				TOK_CR, UT_PCODE+2, ACT_H_EXPAND,
				TOK_CR, UT_ANY, ACT_BLCR+0,
				0 };
static ClassNum cREVEAL[] = { C_COMMENT, C_PASSUP+64 };
static ActionNum aHIDE[] = {
				TOK_CR, UT_ANY, ACT_CR,
				TOK_DOT, UT_ANY, ACT_CHGOTO+0,
				0 };
static ClassNum cHIDE[] = { C_COMMENT, C_PASSUP+64 };

static ClassNum cSTUB[] = {NILCHL};
static ActionNum aID[] = {
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cID[] = {NILCHL};
static ActionNum aNAME[] = {
				TOK_LBRACKET, UT_ANY, ACT_INDEX,
				TOK_UPARROW, UT_ANY, ACT_INDIRECT,
				TOK_DOT, UT_ANY, ACT_FIELD,
				0 };
static ClassNum cNAME[] = {NILCHL};

static ClassNum cLIST[] = /* special case */
				 { 0 } ;

static ClassNum cSYMBOL_TABLE[] = { 0 };

static ClassNum cDECL_ID[] = { 0 };
 char *Node_Names[] = {
"root"
				,
"comment"
				,
"pascal program"
				,
"declarations"
				,
"declarations"
				,
"declarations"
				,
"declarations"
				,
"procedure"
				,
"function"
				,
"Forward Declaration"
				,
"initializer"
				,
0,
"constant declaration"
				,
"type declaration"
				,
"variable declaration"
				,
"enumerated type"
				,
"subrange"
				,
"packed type"
				,
"array"
				,
"complex initializer"
				,
"string type"
				,
"field initializer"
				,
"set type"
				,
"file type"
				,
"pointer type"
				,
"record type"
				,
"record field"
				,
"variant record"
				,
"variant case"
				,
"parameter"
				,
"var parameter"
				,
"function parameter"
				,
"procedure parameter"
				,
"pass through comment"
				,
"statement comment"
				,
0,
"labeled statement"
				,
"goto"
				,
"procedure call"
				,
0,
0,
"assignment statement"
				,
"Immediate Statements"
				,
"generic block"
				,
"if-then"
				,
"if-then-else"
				,
"for loop"
				,
"reverse for loop"
				,
"while loop"
				,
"repeat loop"
				,
"with block"
				,
"case"
				,
"compound statement"
				,
"commented out code"
				,
"case instance"
				,
"case else"
				,
0,
" left",
"field width",
"precision",
"signed constant",
"negative constant",
"integer constant",
"real constant",
"character constant",
"string constant",
"array indexing",
"indirection",
"field use",
"implied field use",
"nil pointer",
"parentheses",
" right",
"set",
"not",
"function call",
"unary plus",
"unary minus",
"addition",
"subtraction",
"or",
"multiplication",
"division",
"integer divide",
"modulus",
"and",
"equality",
"not equal",
"less than",
"less or equal",
"greater than",
"greater or equal",
"set membership",
"exclusive or",
"range set",
"bad expression",
"revealed code"
				,
"hidden code"
				,
"placeholder"
				,
"Symbol"
				,
"Unref Symbol"
				,
"List"
				,
"Symbol Table"
				,
"name"
				,

0 };
bits8 expr_prec[] = {




















































5
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
0
				,
5
				,
0
				,
1
				,
0
				,
3
				,
3
				,
10
				,
10
				,
10
				,
5
				,
5
				,
5
				,
5
				,
5
				,
15
				,
15
				,
15
				,
15
				,
15
				,
15
				,
15
				,
10
				,
0
				,
0
				,









0 };
ActionNum *N_Actions[] = {
NULACT,
NULACT,
aPROGRAM,
aDECL_LABEL,
aDECL_CONST,
aDECL_TYPE,
aDECL_VAR,
aDECL_PROC,
aDECL_FUNC,
aFORWARD,
aCONST_INIT,
NULACT,
aCONST_DECL,
aTYPE_DECL,
aVAR_DECL,
aTYP_ENUM,
aTYP_SUBRANGE,
NULACT,
aTYP_ARRAY,
aINIT_STRUCT,
aTYP_STRING,
aFLD_INIT,
aTYP_SET,
aTYP_FILE,
NULACT,
aTYP_RECORD,
aFIELD,
aVARIANT,
aST_VCASE,
aFORM_VALUE,
aFORM_REF,
aFORM_FUNCTION,
aFORM_PROCEDURE,
aNOTDONE,
aST_COMMENT,
NULACT,
aST_LABEL,
aST_GOTO,
aST_CALL,
NULACT,
NULACT,
aST_ASSIGN,
aIMM_BLOCK,
aST_SPECIAL,
aST_IF,
aST_ELSE,
aST_FOR,
aST_DOWNTO,
aST_WHILE,
aST_REPEAT,
aST_WITH,
aST_CASE,
aST_BLOCK,
NULACT,
aCASE,
aCASE_ELSE,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
aVAR_ARRAY,
aVAR_POINTER,
aVAR_FIELD,
aVF_WITH,
NULACT,
aEXP_PAREN,
NULACT,
aEXP_SET,
NULACT,
aEXP_FUNC,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
NULACT,
aSET_SUBRANGE,
NULACT,
aREVEAL,
aHIDE,
NULACT,
aID,
aNAME,
NULACT,
NULACT,
NULACT,

0 };
ClassNum *K_Classes[] = {
cNULL,
cT_COMMENT,
cPROGRAM,
cDECL_LABEL,
cDECL_CONST,
cDECL_TYPE,
cDECL_VAR,
cDECL_PROC,
cDECL_FUNC,
cFORWARD,
cCONST_INIT,
CAST(ClassNum *)0,
cCONST_DECL,
cTYPE_DECL,
cVAR_DECL,
cTYP_ENUM,
cTYP_SUBRANGE,
cTYP_PACKED,
cTYP_ARRAY,
cINIT_STRUCT,
cTYP_STRING,
cFLD_INIT,
cTYP_SET,
cTYP_FILE,
cTYP_POINTER,
cTYP_RECORD,
cFIELD,
cVARIANT,
cST_VCASE,
cFORM_VALUE,
cFORM_REF,
cFORM_FUNCTION,
cFORM_PROCEDURE,
cNOTDONE,
cST_COMMENT,
CAST(ClassNum *)0,
cST_LABEL,
cST_GOTO,
cST_CALL,
CAST(ClassNum *)0,
CAST(ClassNum *)0,
cST_ASSIGN,
cIMM_BLOCK,
cST_SPECIAL,
cST_IF,
cST_ELSE,
cST_FOR,
cST_DOWNTO,
cST_WHILE,
cST_REPEAT,
cST_WITH,
cST_CASE,
cST_BLOCK,
cST_COUT,
cCASE,
cCASE_ELSE,
CAST(ClassNum *)0,
cEXP_SHL,
cOEXP_2,
cOEXP_3,
cCON_PLUS,
cCON_MINUS,
cCON_INT,
cCON_REAL,
cCON_CHAR,
cCON_STRING,
cVAR_ARRAY,
cVAR_POINTER,
cVAR_FIELD,
cVF_WITH,
cEXP_NIL,
cEXP_PAREN,
cEXP_SHR,
cEXP_SET,
cEXP_NOT,
cEXP_FUNC,
cEXP_UPLUS,
cEXP_UMINUS,
cEXP_PLUS,
cEXP_MINUS,
cEXP_OR,
cEXP_TIMES,
cEXP_SLASH,
cEXP_DIV,
cEXP_MOD,
cEXP_AND,
cEXP_EQ,
cEXP_NE,
cEXP_LT,
cEXP_LE,
cEXP_GT,
cEXP_GE,
cEXP_IN,
cEXP_XOR,
cSET_SUBRANGE,
cEXP_ERROR,
cREVEAL,
cHIDE,
cSTUB,
cID,
cNAME,
cLIST,
cSYMBOL_TABLE,
cDECL_ID,

0 };
#ifdef QNX
char	NodeEnd;
#endif

