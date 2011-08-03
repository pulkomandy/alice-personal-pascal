/*
 * Yacc grammar for Alice PASCAL
 */

%{
#include "alice.h"
#include "input.h"
#include "class.h"
#include "typecodes.h"
#include "flags.h"
#ifndef RELEASE
# define PRINTT
#endif
#include "printt.h"
%}

/*
 * TERMINAL DECLARATIONS
 *
 */

%union {
	nodep	treeval;		/* maybe need tree and list types? */
	char	*strval;
	int	intval;
}

%token <intval>	YAND		YARRAY		YBEGIN		YCASE
		YSTTYPE		YSHL		YSHR		YXOR
		YCONST		YDIV		YDO		YDOTDOT
		YTO		YELSE		YEND		YFILE
		YFOR		YPROCEDURE	YGOTO		YIF
		YIN		YLABEL		YMOD		YNOT
		YOF		YOR		YPACKED		YNIL
		YFUNCTION	YPROG		YRECORD		YREPEAT
		YSET		YTHEN		YDOWNTO		YTYPE
		YUNTIL		YVAR		YWHILE		YWITH
		YCHAR		YCOLEQUALS	YPTR		YILLCH
		YOTHERWISE	YHIDE		YHIDEEND	YABSOLUTE
		YEXTERNAL	YINLINE		YOVERLAY	YCLASS_DECL
		YCLASS_CONST	YCLASS_TYPE	YCLASS_VAR	YCLASS_FIELD
		YCLASS_VARIANT	YCLASS_STAT	YCLASS_CASE

%token <strval>	YID		YINT		YNUMB		YSTRING

%token <treeval> YC_ROOT	YC_COMMENT	YC_LABEL	YC_PROC_ID
		YC_FUN_ID	YC_PROGRAM	YC_DECLARATIONS	YC_LABDECL
		YC_CONDECL	YC_TYPE_DECL	YC_VAR_DECL	YC_CONSTANT
		YC_TYPE		YC_TYPEID	YC_SIM_TYPE	YC_ST_TYPE
		YC_FIELD	YC_FORMAL	YC_STATEMENT	YC_CASE	
		YC_OUTPUT_EXPRESSION		YC_VAR		YC_EXP
		YC_DECL_ID	YC_HIDECL_ID	YC_VARIANT	YC_FLD_NAME
		YC_OCONSTANT	YC_PNAME	YC_BLCOMMENT	YREVEAL

%type <treeval>	goal		pname		file_list	sym_table
		decls		decl		labels		label_list
		label_decl	const_decls	const_decl	type_decls
		type_decl	var_decls	var_decl	proc_decl
		params		proc_header	func_header	func_type
		param		param_list	const		number
		const_list	type		simple_type	struct_type	
		simple_type_list fixed_part	field_list	field
		variant_part	variant_list	stat_list	cstat_list
		cstat		stat		wstat		expr
		element_list	element		variable	wexpr
		expr_list	wexpr_list	id_list		fid_list
		dec_id 		dec_id		symbol		ptr_symbol
		block_comment	block		variant		fdec_id
		caseconst	cc_list		con_char	hide
		var_decl_list	const_decl_list	type_decl_list	semi
		const_init	struct_const	array_const_list rec_const_list
		rec_const	set_const_list	set_const	rsym_table
		eid_list	edec_id		absdesc		confarray
		conflist	ptype		csemi		fsemi
		vsemi		opt_hidden_variant		com_list
		decl_list

%type <strval>	label		directive

%type <intval>	pop_table	backpatch	relop		addop
		divop		overkword

/*
 * PRECEDENCE DECLARATIONS
 *
 * Highest precedence is the unary logical NOT.
 * Next are the multiplying operators, signified by '*'.
 * Lower still are the binary adding operators, signified by '+'.
 * Finally, at lowest precedence and non-associative are the relationals.
 */

%binary	'<'	'='	'>'	YIN
%left	'+'	'-'	YOR
%left	UNARYSIGN
%left	'*'	'/'	YDIV	YMOD	YAND	YSHL	YSHR	YXOR
%left	YNOT

%%
/*
 * PRODUCTIONS
 */

goal:
	block_comment YPROG pname sym_table file_list ';'
	  com_list decls backpatch block '.' pop_table
		{
		makeprog(tree(N_PROGRAM, $3, $5,
				conclist($1,$7), $8, $10, $4));
		}
		|
	YC_ROOT
		|
	YC_PROGRAM
		;

pname:
	YID
		{
			$$ = tree(N_T_COMMENT, $1);
		}
		|
	YC_PNAME
		;

file_list:
	'(' fid_list ')' 
		{
		$$ = $2;
		dump( $$ );
		}
		|
	/* empty */
		{ $$ = newlist(NIL); }
		;

block:
	YBEGIN stat_list YEND
		{ $$ = $2; }
		;

hide:
	YHIDE YREVEAL
		{
			$$ = tree(N_HIDE, $2, NIL);
			if ($1)
				or_2flag($$, NF2_KEEP_HIDDEN);
		}
		|
	YREVEAL
		{ $$ = tree(N_REVEAL, $1, NIL); }
		;


/*
 * DECLARATION PART
 */

pop_table:	/* nil */
		{
		/* now pop the symbol table stack */
		popSymtabStack();
		printt1("pop_table, new scope_level %d\n", scope_level);
		}
		;

backpatch:	/* nil */
		{ pop_backpatch(); }
		;

sym_table:	/* nil */
		{
			$$ = pushSymtabStack(tree(N_SYMBOL_TABLE, NIL), TRUE);
			if (!GlobalSymtab)
				GlobalSymtab = $$;
			push_backpatch();
		}
		;

rsym_table: 	/* nil */
		{
			$$ = pushSymtabStack(tree(N_SYMBOL_TABLE, NIL), FALSE);
			push_backpatch();
		}
		;

decls:
	/* nil */
		{ $$ = newlist(make_stub(C_DECLARATIONS)); }
		|
	decl_list
		;

decl_list:
	decl com_list
		{ $$ = conclist($1, $2); }
		|
	decl_list decl com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		|
	decl_list error
		{
			par_error("Malformed declaration");
			$$ = $1;
		}
		;

decl:
	YCLASS_DECL hide decls YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	labels
		{ $$ = tree(N_DECL_LABEL, $1); }
		|
	const_decls
		{ $$ = tree(N_DECL_CONST, $1); }
		|
	type_decls
		{ $$ = tree(N_DECL_TYPE, $1); }
		|
	var_decls
		{ $$ = tree(N_DECL_VAR, $1); }
		|
	proc_decl
		|
	YC_DECLARATIONS
		;


/*
 * LABEL PART
 */

labels:
	YLABEL label_list ';'
		{ $$ = $2; }
		;

label_list:
	label_decl
		{ $$ = newlist($1); }
		|
	label_list ',' label_decl
		{ $$ = conclist($1, $3); }
		;

label:
	YID
		|
	YINT
		;

label_decl:
	label
		{
			$$ = label_declare(trimzero($1));
			mfree($1);
		}
		|
	YC_LABEL
		;

/*
 * CONST PART
 */

const_decls:
	YCONST const_decl_list
		{ $$ = $2; }
		;

const_decl_list:
	const_decl com_list
		{ $$ = conclist($1, $2); }
		|
	const_decl_list const_decl com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		;

const_decl:
	YCLASS_CONST hide const_decl_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	dec_id '=' const ';'
		{ $$ = const_declare($1, $3); }
		|
	dec_id ':' type '=' const_init ';'
		{ $$ = const_init_declare($1, $3, $5); }
		|
	error ';'
		{ $$ = make_stub(C_CONST_DECL); }
		|
	YC_CONDECL
		;

const_init:
	const
		|
	struct_const
		;

struct_const:
	'(' array_const_list ')'
		{ $$ = tree(N_INIT_STRUCT, $2, NCAST yylineno); }
		|
	'(' rec_const_list ')'
		{ $$ = tree(N_INIT_STRUCT, $2, NCAST yylineno); }
		|
	'[' set_const_list ']'
		{ $$ = tree(N_EXP_SET, $2); }
		|
	'[' ']'
		{ $$ = tree(N_EXP_SET, sized_list(0)); }
		;

array_const_list:
	const_init
		{ $$ = newlist($1); }
		|
	array_const_list ',' const_init
		{ $$ = conclist($1, $3); }
		;

rec_const_list:
	rec_const
		{ $$ = newlist($1); }
		|
	rec_const_list ';' rec_const
		{ $$ = conclist($1, $3); }
		;

rec_const:
	YID ':' const_init
		{ $$ = tree(N_FLD_INIT, tree(N_NAME, NCAST $1), $3); }
		;

set_const_list:
	set_const
		{ $$ = newlist($1); }
		|
	set_const_list ',' set_const
		{ $$ = conclist($1, $3); }
		;

set_const:
	const
		|
	const YDOTDOT const
		{ $$ = tree(N_SET_SUBRANGE, $1, $3); }
		;

/*
 * TYPE PART
 */

type_decls:
	YTYPE type_decl_list
		{ $$ = $2; }
		;

type_decl_list:
	type_decl com_list
		{ $$ = conclist($1, $2); }
		|
	type_decl_list type_decl com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		;

type_decl:
	YCLASS_TYPE hide type_decl_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	dec_id '=' type ';'
		{ $$ = type_declare($1, $3); }
		|
	error ';'
		{
			$$ = make_stub(C_TYPE_DECL);
			par_error("Malformed type declaration");
		}
		|
	YC_TYPE_DECL
		;

/*
 * VAR PART
 */

var_decls:
	YVAR var_decl_list
		{ $$ = $2; }
		;

var_decl_list:
	var_decl com_list
		{ $$ = conclist($1, $2); }
		|
	var_decl_list var_decl com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		;

var_decl:
	YCLASS_VAR hide var_decl_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	id_list ':' type ';'
		{ $$ = var_declare(N_VAR_DECL, T_VAR, $1, $3); }
		|
	id_list ':' type YABSOLUTE absdesc ';'
		{
			smallerror( "Absolute variables not allowed in small model ALICE" );
			if( listcount( $1 ) > 1 )
				nonfatal( "Only one ID allowed in absolute declaration" );
			$$ = abs_declare($1, $3, $5);
		}
		|
	error ';'
		{
			$$ = make_stub(C_VAR_DECL);
			par_error("Malformed var declaration");
		}
		|
	YC_VAR_DECL
		;

absdesc:
	symbol
		|
	expr ':' expr
		{
		$$ = tree( N_OEXP_2, $1, $3 );
		}
		;

/*
 * PROCEDURE AND FUNCTION DECLARATION PART
 */


proc_decl:
	proc_header backpatch pop_table directive ';'
		{
			$$ = $1;
			proc_declare($$);
		}
		|
	func_header backpatch pop_table directive ';'
		{
			$$ = $1;
			func_declare($$);
		}
		|
	proc_header com_list decls backpatch block ';' pop_table
		{
			$$ = $1;
			linkup($$, 2, conclist(kid3($$), $2));
			linkup($$, 3, $3);
			linkup($$, 4, $5);
			proc_declare($$);
		}
		|
	func_header com_list decls backpatch block ';' pop_table
		{
			$$ = $1;
			linkup($$, 3, conclist(kid4($$), $2));
			linkup($$, 4, $3);
			linkup($$, 5, $5);
			func_declare($$);
		}
		;

directive:
	YID
		|
	YEXTERNAL YSTRING
		{ fatal("external routines not supported"); }
		|
	YEXTERNAL YID '[' const ']'
		{ fatal("external routines not supported"); }
		;

params:
	/* lambda */
		{ $$ = newlist(NIL); }
		|
	'(' param_list ')'
		{ $$ = $2; }
		;
proc_header:
	block_comment overkword YPROCEDURE dec_id sym_table params ';'
		{
			nodep p;
			p = tparent($4);
			if (p != NIL) {
				changeSymtabStack(kid6(p), TRUE);
				linkup(tparent(p), get_kidnum(p),
					tree(N_FORWARD, tree(N_ID, $4)));
				$$ = p;
			}
			else
				$$ = tree(N_DECL_PROC,$4,$6,$1,NIL,NIL,$5);
		}
		;
func_header:
	block_comment overkword YFUNCTION dec_id sym_table params func_type ';'
		{
			nodep p;
			p = tparent($4);
			if (p != NIL) {
				changeSymtabStack(kid7(p), TRUE);
				linkup(tparent(p), get_kidnum(p),
					tree(N_FORWARD, tree(N_ID, $4)));
				$$ = p;
			}
			else
				$$ = tree(N_DECL_FUNC,$4,$6,$7,$1,NIL,NIL,$5);
		}
		;

overkword:
	/* lambda */
		{ /* nothing */ }
		|
	YOVERLAY
		{ unsupported("Warning: Turbo Pascal OVERLAY directive ignored" ); }
		;

func_type:
	':' type
		{ $$ = $2; }
		|
	/* lambda */
		{ $$ = NIL; }
		;

/*
 * PARAMETERS
 */

param:
	id_list ':' ptype
		{ $$ = param_declare(N_FORM_VALUE, $1, $3); }
		|
	YVAR id_list ':' ptype
		{ $$ = param_declare(N_FORM_REF, $2, $4); }
		|
	YVAR id_list
		{
			$$ = param_declare(N_FORM_REF, $2, make_stub(C_TYPEID));
		}
		|
	YFUNCTION dec_id sym_table params backpatch pop_table ':' symbol
		{
			int i, n;

			s_sym_dtype((symptr) $2, T_FORM_FUNCTION);
			$$ = tree(N_FORM_FUNCTION, $2, $4, $8, $3);
		}
		|
	YPROCEDURE dec_id sym_table params backpatch pop_table
		{
			int i, n;

			s_sym_dtype((symptr) $2, T_FORM_PROCEDURE);
			$$ = tree(N_FORM_PROCEDURE, $2, $4, $3);
		}
		|
	YC_FORMAL
		;

ptype:
	symbol
		|
	YFILE
		{
			$$ = symref("AnyFile");
			unsupported("Note: untyped file mapped to AnyFile");
		}
		|
	YARRAY '[' conflist ']' YOF ptype
		{ fatal("Conformant arrays not supported"); }
		|
	YPACKED '[' conflist ']' YOF ptype
		{ fatal("Conformant arrays not supported"); }
		;

conflist:
	confarray
		|
	conflist ';' confarray
		;

confarray:
	YID YDOTDOT YID ':' YID 
		{ /* nothing */ }
		;

param_list:
	param
		{ $$ = newlist($1); }
		|
	param_list ';' param
		{ $$ = conclist($1, $3); }
		;

/*
 * CONSTANTS
 */

const:
	YSTRING
		{ $$ = tree(N_CON_STRING, (nodep)$1, (nodep)strlen($1)); }
		|
	con_char
		|
	number
		|
	'+' number
		{ $$ = tree(N_CON_PLUS, $2); }
		|
	'-' number
		{ $$ = tree(N_CON_MINUS, $2); }
		|
	'+' YC_CONSTANT
		{ $$ = tree(N_CON_PLUS, $2); }
		|
	'-' YC_CONSTANT
		{ $$ = tree(N_CON_MINUS, $2); }
		|
	YC_CONSTANT
		;

number:
	symbol
		|
	YINT
		{ $$ = tree(N_CON_INT, (nodep)$1, (nodep)atoi($1)); }
		|
	YNUMB
		{ $$ = tree(N_CON_REAL, $1 /* , value not needed */); }
		;

caseconst:
	const
		|
	const YDOTDOT const
		{ $$ = tree( N_TYP_SUBRANGE, $1, $3 ); }
		;

cc_list:
	caseconst
		{ $$ = newlist($1); }
		|
	cc_list ',' caseconst
		{ $$ = conclist($1, $3); }

const_list:
	const
		{ $$ = newlist($1); }
		|
	const_list ',' const
		{ $$ = conclist($1, $3); }
		;

/*
 * TYPES
 */

type:
	simple_type
		|
	YPTR ptr_symbol
		{ $$ = tree(N_TYP_POINTER, $2 ); }
		|
	struct_type
		|
	YPACKED struct_type
		{ $$ = pack($2); }
		|
	YC_TYPE
		;

con_char:
	YCHAR
		{ $$ = tree(N_CON_CHAR, (nodep)$1); }
		|
	YPTR YID
		{
			if (strlen($2) != 1)
				nonfatal("Bad control character \'%s\'\n", $2);
			$$ = tree(N_CON_CHAR, (nodep)($2[0] & 0x1f));
			free($2);
		}
		|
	YPTR '['
		{ $$ = tree(N_CON_CHAR, (nodep) ('[' & 0x1f)); }
		|
	YPTR ']'
		{ $$ = tree(N_CON_CHAR, (nodep) (']' & 0x1f)); }
		|
	YPTR YPTR
		{ $$ = tree(N_CON_CHAR, (nodep) ($2 & 0x1f)); }
		;

simple_type:
	symbol
		|
	symbol '[' const ']'
		{
			nonfatal( "Turbo Pascal style buffer size ignored" );
			$$ = $1;
		}
		|
	'(' eid_list ')'
		{ $$ = enum_declare($2); }
		|
	const YDOTDOT const
		{ $$ = tree(N_TYP_SUBRANGE, $1, $3 ); }
		|
	YC_SIM_TYPE
		;

struct_type:
	YARRAY '[' simple_type_list ']' YOF type
		{
			/* multi dim. array - subscripts are kids of list */
			listp	l	= $3;
			int	i;
			int	nk	= listcount(l);

			$$ = $6;
			for (i = nk-1; i >= 0; i--)
				$$ = tree(N_TYP_ARRAY, node_kid(l, i), $$);
		}
		|
	YSTTYPE '[' const ']'
		{ $$ = tree( N_TYP_STRING, $3 ); }
		|
	YFILE YOF type
		{ $$ = tree(N_TYP_FILE, $3); }
		|
	YFILE
		{ $$ = symref("AnyFile"); }
		|
	YSET YOF simple_type
		{ $$ = tree(N_TYP_SET, $3); }
		|
	YRECORD rsym_table field_list backpatch pop_table YEND
		{ $$ = tree(N_TYP_RECORD, $3, $2 ); }
		|
	YC_ST_TYPE
		;

simple_type_list:
	simple_type
		{ $$ = newlist($1); }
		|
	simple_type_list ',' simple_type
		{ $$ = addlist($1, $3); }
		;

/*
 * RECORD TYPE
 */

field_list:
	fixed_part variant_part
		{
			if ($1 == NIL && $2 == NIL)
				$$ = newlist(make_stub(C_FIELD));
			else
				$$ = conclist($1, $2);
		}
		;

fixed_part:
	field com_list
		{ $$ = conclist($1, $2); }
		|
	fixed_part fsemi field com_list
		{ $$ = conclist(conclist(conclist($1, $2), $3), $4); }
		|
	fixed_part error
		{
			par_error("Malformed record declaration");
			$$ = NIL;
		}
		;

fsemi:
	';'
		 { $$ = NIL; }
		|
	YC_FIELD
		|
	YCLASS_FIELD hide field_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		;

field:
	/* lambda */
		{ $$ = NIL; }
		|
	id_list ':' type
		{ $$ = var_declare(N_FIELD, T_FIELD, $1, $3 ); }
		;


variant_part:
	/* lambda */
		{ $$ = NIL; }
		|
	opt_hidden_variant
		;

opt_hidden_variant:
	YCLASS_VARIANT hide opt_hidden_variant YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	YCASE symbol YOF variant_list
		{ $$ = tree(N_VARIANT, make_stub(C_DECL_ID), $2, $4); }
		|
	YCASE dec_id ':' symbol YOF variant_list
		{
			$$ = tree(N_VARIANT, $2, $4, $6);
			s_sym_dtype( (symptr)$2, T_FIELD );
		}
		;

variant_list:
	variant com_list
		{ $$ = conclist($1, $2); }
		|
	variant_list vsemi variant com_list
		{ $$ = conclist(conclist(conclist($1, $2), $3), $4); }
		|
	variant_list error
		{
			par_error("Malformed record declaration");
			$$ = NIL;
		}
		;

vsemi:
	';'
		{ $$ = NIL; }
		|
	YCLASS_VARIANT hide variant_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	YC_VARIANT
		;

variant:
	/* lambda */
		{ $$ = NIL; }
		|
	const_list ':' '(' field_list ')'
		{ $$ = tree(N_ST_VCASE, $1, $4); }
		;

/*
 * STATEMENT LIST
 */

stat_list:
	com_list stat com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		|
	stat_list semi stat com_list
		{ $$ = conclist(conclist(conclist($1, $2), $3), $4); }
		;

semi:
	';'
		{ $$ = NIL; }
		|
	YCLASS_STAT hide stat_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	YC_STATEMENT
		|
	YC_COMMENT
		;


/*
 * CASE STATEMENT LIST
 */

cstat_list:
	com_list cstat com_list
		{ $$ = conclist(conclist($1, $2), $3); }
		|
	cstat_list csemi cstat com_list
		{ $$ = conclist(conclist(conclist($1, $2), $3), $4); }
		|
	error
		{
			$$ = newlist(make_stub(C_CASE));
Kerror:
			par_error("Malformed statement in case");
		}
		|
	cstat_list error
		{ goto Kerror; }
		;

csemi:
	';'
		{ $$ = NIL; }
		|
	YCLASS_CASE hide cstat_list YHIDEEND
		{ $$ = hide_block($2, $3); }
		|
	YC_CASE
		;


cstat:
	/* lambda */
		{ $$ = make_stub(C_CASE); }
		|
	cc_list ':' stat
		{ $$ = tree(N_CASE, $1, newlist($3)); }
		|
	YELSE stat_list
		{ $$ = tree(N_CASE_ELSE, newlist($2)); }
		|
	YOTHERWISE stat
		{ $$ = tree(N_CASE_ELSE, newlist($2)); }
		;

/*
 * STATEMENT
 */

stat:
	/* lambda */
		{ $$ = make_stub(C_STATEMENT); }
		|
	label ':' stat
		{
			$$ = conclist(tree(N_ST_LABEL, symref(trimzero($1))), $3);
			mfree($1);
		}
		|
	symbol
		{ $$ = tree(N_ST_CALL, $1, newlist(NIL)); }
		|
	YC_PROC_ID
		{ $$ = tree(N_ST_CALL, $1, newlist(NIL)); }
		|
	symbol '(' wexpr_list ')'
		{ $$ = tree(N_ST_CALL, $1, $3); }
		|
	YC_PROC_ID '(' wexpr_list ')'
		{ $$ = tree(N_ST_CALL, $1, $3); }
		|
	YID error
		{ goto NSerror; }
		|
	variable YCOLEQUALS expr
		{ $$ = tree(N_ST_ASSIGN, $1, $3 ); }
		|
	YBEGIN stat_list YEND
		{ $$ = $2; }
		|
	YCASE expr YOF cstat_list YEND
		{ $$ = tree(N_ST_CASE, $2, $4); }
		|
	YWHILE expr YDO stat
		{ $$ = tree(N_ST_WHILE, $2, newlist($4)); }
		|
	YREPEAT stat_list YUNTIL expr
		{ $$ = tree(N_ST_REPEAT, $2, $4); }
		|
	YFOR variable YCOLEQUALS expr YTO expr YDO stat
		{ $$ = tree(N_ST_FOR, $2, $4, $6, newlist($8)); }
		|
	YFOR variable YCOLEQUALS expr YDOWNTO expr YDO stat
		{ $$ = tree(N_ST_DOWNTO, $2, $4, $6, newlist($8)); }
		|
	YGOTO label
		{
			$$ = tree(N_ST_GOTO, symref(trimzero($2)));
			mfree($2);
		}
		|
	YIF expr YTHEN stat
		{ $$ = tree(N_ST_IF, $2, newlist($4)); }
		|
	YIF expr YTHEN stat YELSE stat
		{ $$ = tree(N_ST_ELSE, $2, newlist($4), newlist($6)); }
		|
	YDO YIF variable YSET stat ';' '}'
		{ $$ = tree(N_ST_TRACE, $3, newlist($5)); }
		|
	YWITH variable			/* should be var_list */
		{ 
			$$ = tree(N_ST_WITH, $2, NIL );
			pushSymtabStack($$, FALSE);
		}
	wstat
		{
			linkup($<treeval>3, 1, newlist($4));
			popSymtabStack();
			$$ = $<treeval>3;
		}
		|
	YINLINE '(' error ')'
		{
			$$ = makecom("Inline assembler");
			unsupported( "Inline assembler not supported" );
		}
		|
	error
		{
NSerror:
			$$ = makecom("Error Statement");
Serror:
			par_error("Malformed statement");
		}
		;

wstat:
	YDO stat
		{ $$ = $2; }
		|
	',' variable
		{ 
			$$ = tree( N_ST_WITH, $2, NIL );
			pushSymtabStack($$, FALSE);
		}
	wstat
		{
			linkup( $<treeval>3, 1, newlist( $4 ) );
			popSymtabStack();
			$$ = $<treeval>3;
		}
		;

/*
 * EXPRESSION
 */

expr:
	expr relop expr			%prec '<'
		{ $$ = tree($2, (nodep)$1, $3 ); }
		|
	'+' expr			%prec UNARYSIGN
		{ $$ = tree(N_EXP_UPLUS, $2 ); }
		|
	'-' expr			%prec UNARYSIGN
		{ $$ = tree(N_EXP_UMINUS, $2 ); }
		|
	expr addop expr			%prec '+'
		{ $$ = tree($2, (nodep)$1, $3); }
		|
	expr divop expr			%prec '*'
		{ $$ = tree($2, (nodep)$1, $3); }
		|
	YNIL
		{ $$ = tree(N_EXP_NIL); }
		|
	YSTRING
		{ $$ = tree(N_CON_STRING, (nodep)$1, (nodep)strlen($1)); }
		|
	con_char
		|
	YINT
		{ $$ = tree(N_CON_INT, (nodep)$1, (nodep)atoi($1)); }
		|
	YNUMB
		{ $$ = tree(N_CON_REAL, (nodep)$1); }
		|
	variable
		{ $$ = fix0ArgFuncs($1); }
		|
	symbol '(' expr_list ')'
		{
			if (turbo_flag && sym_saveid(kid_id($1)) == SV_CONCAT) {
				int	i;
				node	addflag;

				if (listcount($3) == 0)
					$$ = makecom("concat()");
				else {
					$$ = kid1($3);
					for (i = 1; i < listcount($3); i++)
						$$ = tree(N_EXP_PLUS, $$,
							  node_kid($3, i));
					}
				}
			else
				$$ = tree(N_EXP_FUNC, $1, $3 );
		}
		|
	'(' expr ')'
		{ $$ = tree(N_EXP_PAREN, $2 ); }
		|
	YNOT expr			%prec YNOT
		{ $$ = tree(N_EXP_NOT, $2 ); }
		|
	'[' element_list ']'
		{ $$ = tree(N_EXP_SET, $2); }
		|
	'[' ']'
		{ $$ = tree(N_EXP_SET, newlist(NIL) ); }
		|
	YC_EXP
		;

element_list:
	element
		{ $$ = newlist($1); }
		|
	element_list ',' element
		{ $$ = conclist($1, $3); }
		;
element:
	expr
		{ $$ = $1; }
		|
	expr YDOTDOT expr
		{ $$ = tree(N_SET_SUBRANGE, $1, $3 ); }
		;

/*
 * QUALIFIED VARIABLES
 */

variable:
	symbol
		|
	variable '[' expr ':' expr ']'
		{
			smallerror("colons in array index not permitted in small model Alice");
			$$ = tree(N_VAR_ARRAY, $1, tree(N_OEXP_2, $3, $5));
		}
		|
	variable '[' expr_list ']'
		{
			/* multi dim. array - subscripts are kids of list */
			listp	l = $3;
			int	i;
			int	nk = listcount(l);

			$$ = $1;
			for (i = 0; i < nk; i++)
				$$ = tree(N_VAR_ARRAY, $$, node_kid(l, i));
		}
		|
	variable '.' YID
		{
			extern nodep fld_symref();
			$$ = fld_symref($3, $1);
			mfree($3);
		}
		|
	variable YPTR
		{ $$ = tree(N_VAR_POINTER, $1 ); }
		|
	YC_VAR
		;

/*
 * Expression with write widths
 */
wexpr:
	expr
		|
	expr ':' expr
		{ $$ = tree(N_OEXP_2, $1, $3 ); }
		|
	expr ':' expr ':' expr
		{ $$ = tree(N_OEXP_3, $1, $3, $5 ); }
		;

expr_list:
	expr
		{ $$ = newlist($1); }
		|
	expr_list ',' expr
		{ $$ = conclist($1,$3); }
		;

wexpr_list:
	wexpr
		{ $$ = newlist($1); }
		|
	wexpr_list ',' wexpr
		{ $$ = conclist($1, $3); }
		;

/*
 * OPERATORS
 */

relop:
	'='	= $$ = N_EXP_EQ;
		|
	'<'	= $$ = N_EXP_LT;
		|
	'>'	= $$ = N_EXP_GT;
		|
	'<' '>'	= $$ = N_EXP_NE;
		|
	'<' '='	= $$ = N_EXP_LE;
		|
	'>' '='	= $$ = N_EXP_GE;
		|
	YIN	= $$ = N_EXP_IN;
		;
addop:
	'+'	= $$ = N_EXP_PLUS;
		|
	'-'	= $$ = N_EXP_MINUS;
		|
	YOR	= $$ = N_EXP_OR;
		|
	YXOR	= $$ = N_EXP_XOR;
		;
divop:
	'*'	= $$ = N_EXP_TIMES;
		|
	'/'	= $$ = N_EXP_SLASH;
		|
	YDIV	= $$ = N_EXP_DIV;
		|
	YMOD	= $$ = N_EXP_MOD;
		|
	YAND	= $$ = N_EXP_AND;
		|
	YSHL	= $$ = N_EXP_SHL;
		|
	YSHR	= $$ = N_EXP_SHR;
		;


/*
 * LISTS
 */

id_list:
	dec_id
		{ $$ = newlist($1); }
		|
	id_list ',' dec_id
		{ $$ = conclist($1, $3); }
		;

eid_list:
	edec_id
		{ $$ = newlist($1); }
		|
	eid_list ',' edec_id
		{ $$ = conclist($1, $3); }
		;


fid_list:
	fdec_id
		{ $$ = newlist($1); }
		|
	fid_list ',' fdec_id
		{ $$ = conclist($1, $3); }
		;

dec_id:
	YID
		{
			$$ = NCAST s_declare($1, 0 /* no type */, CurSymtab);
			check_backpatch($1);
			mfree($1);
		}
		|
	YC_PROC_ID
		|
	YC_FUN_ID
		|
	YC_FLD_NAME
		|
	YC_DECL_ID
		|
	YC_HIDECL_ID
		;

fdec_id:
	pname
		;

edec_id:
	YID
		{
			$$ = NCAST s_declare($1, T_ENUM_ELEMENT, CurBlkSymtab);
			mfree($1);
		}
		|
	YC_DECL_ID
		;

symbol:
	YID
		{
			$$ = NCAST symref( $1 );
			mfree($1);
		}
		|
	YC_TYPEID
		;

/* Same as symbol, but used in ^type references, so needn't be declared yet */
ptr_symbol:
	YID
		{ $$ = ptr_symref( $1 ); }
		|
	YC_TYPEID
		;

block_comment:
	com_list
		{
			if ($1)
				$$ = $1;
			else
				$$ = newlist(make_stub(C_BLCOMMENT));
		}
		;

com_list:
	/* lambda */
		{
			scoop_comments();
			if (iscombuf()) {
				$$ = newlist(makecom(nextcomment()));
				while (iscombuf())
					$$ = addlist($$, makecom(nextcomment()));
				}
			else
				$$ = NIL;
		}
		;
%%
