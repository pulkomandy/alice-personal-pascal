/* special codes that add to tokens to say when they can be used  */


#define TOK_EOF		N_ST_COMMENT
#define TOK_AND		N_EXP_AND
#define TOK_ABSOLUTE	N_VAR_ABSOLUTE
#define TOK_ARRAY	N_TYP_ARRAY
#define TOK_BEGIN	N_ST_BLOCK
#define TOK_CASE	N_ST_CASE
#define TOK_CONST	N_DECL_CONST
#define TOK_DIV		N_EXP_DIV
#define TOK_DO		N_CONST_DECL  /* arbitrary */
#define TOK_DOTDOT	N_SET_SUBRANGE
#define TOK_DOWNTO	N_ST_DOWNTO
#define TOK_ELSE	N_ST_ELSE
#define TOK_END		N_VAR_DECL  /* arbitrary */
#define TOK_EXTERN	!!!!
#define TOK_FILE	N_TYP_FILE
#define TOK_FOR		N_ST_FOR
#define TOK_FORWARD	!!!!
#define TOK_FUNCTION	N_DECL_FUNC
#define TOK_GOTO	N_ST_GOTO
#define TOK_ID		N_ID
#define TOK_IF		N_ST_IF
#define TOK_ILLCH	N_TYPE_DECL /*arb*/
#define TOK_IN		N_EXP_IN
#define TOK_INT		N_CON_INT
#define TOK_LABEL	N_DECL_LABEL
#define TOK_MOD		N_EXP_MOD
#define TOK_NIL		N_EXP_NIL
#define TOK_NOT		N_EXP_NOT
#define TOK_NUMB	N_CON_REAL
#define TOK_OF		N_ST_LABEL	 /* arbitrary */
#define TOK_OR		N_EXP_OR
#define TOK_PACKED	N_TYP_PACKED
#define TOK_PROCEDURE	N_DECL_PROC
#define TOK_PROG	N_PROGRAM
#define TOK_RECORD	N_TYP_RECORD
#define TOK_REPEAT	N_ST_REPEAT
#define TOK_SET		N_TYP_SET
#define TOK_STRING	N_CON_STRING
#define TOK_STTYPE	N_TYP_STRING
#define TOK_THEN	N_FIELD /*arbitrary */
#define TOK_TO		N_VARIANT /*arbitrary */
#define TOK_TYPE	N_DECL_TYPE
#define TOK_UNTIL	N_CASE_ELSE /*arbitrary */
#define TOK_VAR		N_DECL_VAR
#define TOK_WHILE	N_ST_WHILE
#define TOK_WITH	N_ST_WITH
#define TOK_LPAREN	N_EXP_PAREN
#define TOK_SEMI	N_OEXP_2 /*arbitrary */
#define TOK_COMMA	N_LIST
#define TOK_COLON	N_OEXP_3
#define TOK_ASSIGN	N_ST_ASSIGN
#define TOK_STAR	N_EXP_TIMES
#define TOK_SHL		N_EXP_SHL
#define TOK_SHR		N_EXP_SHR
#define TOK_XOR		N_EXP_XOR
#define TOK_PLUS	N_EXP_PLUS
#define TOK_SLASH	N_EXP_SLASH
#define TOK_MINUS	N_EXP_MINUS
#define TOK_RPAREN	N_EXP_FUNC
#define TOK_LBRACKET	N_VAR_ARRAY
#define TOK_RBRACKET	N_CON_MINUS /* arbit */
#define TOK_EQ		N_EXP_EQ
#define TOK_LT 		N_EXP_LT
#define TOK_LE		N_EXP_LE
#define TOK_NE		N_EXP_NE
#define TOK_GT		N_EXP_GT
#define TOK_GE		N_EXP_GE
#define TOK_UPARROW	N_VAR_POINTER
#define TOK_CHAR	N_CON_CHAR
#define TOK_CMNT	N_T_COMMENT
#define TOK_DOT		N_VAR_FIELD
#define TOK_CR		N_CON_PLUS /*arbitarary */
#define TOK_QUESTION	N_ST_CALL /* arb */
/*
 * arbitrary defines for the special tokens.  Most tokens use the same
 * token number as their corresponding node number, but these must be
 * unique from any other token numbers, so we select nodes that don't
 * have corresponding tokens.
 */
#define TOK_CMD			N_HIDE
#define TOK_NOP			N_REVEAL
