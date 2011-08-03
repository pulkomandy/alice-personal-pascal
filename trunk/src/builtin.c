/* stop the readin of the function externals */
#define NOFUNCS 1

#ifdef PARSER
# define NOBFUNC
#endif

#include "alice.h"
#include <curses.h>
#include "workspace.h"
#include "typecodes.h"
#include "bfuncs.h"
#include "flags.h"

#ifdef PARSER
# include "input.h"
#endif

#ifdef GEM
static char win_str[] = "\200Window";
#endif

#ifdef ESBUILT
# define NNC(arg) (char*)0
#else
int built_count = BUILT_COUNT;
# define NNC(arg) arg
#endif
/* -127 is free */
#define RD(x) NCAST &x

#define IBC (bargs)
#define symnode struct symbol_node
#define ssymnode static symnode
#define ss static symnode
#define Mflgs SF_DEFYET
#define Lmf SF_DEFYET|SF_LARGE
struct list2node {			/* special list with two elements */
	bits8 node_type;		/* always N_LIST */
	nodep n_parent;
	holdlist l_count;		/* number of children in list */
	holdlist l_acount;		/* allocation count */
	nodep n2_kids[2];		/* 2 children of various kinds */
	};
struct node2 {
	bits8 node_type;
	nodep n_parent;
	holdlist flags1;
	holdlist flags2;
	nodep n2_kids[2];		/* 2 children in this */
	};
struct node3 {
	bits8 node_type;
	nodep n_parent;
	holdlist flags1;
	holdlist flags2;
	nodep n2_kids[3];		/* 2 children in this */
	};
	/* special fake node for parameter names */
typedef struct fake_node {
	bits8 node_type;
	char *fake_name;
	} fake;
/* generic stub */

/* flags are two bytes now */
#define NLF   0,0

node G_Stub = { N_STUB, NIL, NLF, NIL };
/* node G_LStub = { N_LIST, NIL, 1,1, &G_Stub }; */

/* special set and pointer base types */
node BT_set =	{ N_TYP_SET, NIL, NLF, NIL };


/* a special function return type which means, "my type is the same as
   that of my argument (need not take room, just an address) */

#define	HashByte	0	/* place holder, filled in later */

static struct node2 intzero = { N_CON_INT, NIL, NLF, NCAST 0, NCAST 0 };
static struct node2 int255 = { N_CON_INT, NIL, NLF, NCAST 0, NCAST 255 };


static struct node3 BT_byte = { N_TYP_SUBRANGE, NIL, NLF, &intzero, &int255,
	NCAST sizeof(char)};
char SP_passtype = 0;
symnode SP_proc = { N_DECL_ID, NIL, NLF, NNC("Procedure"), T_BTYPE, HashByte, (symptr)0,
	Basetype(SP_proc), 0, 8, 0, Mflgs, 0 };

extern struct node3 BT_anytype;

ssymnode fldobject = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("Object"),
		T_FIELD, HashByte, CAST(symptr)0, Basetype(BT_pointer),
		0, sizeof(pointer), 0, Mflgs, -117 };
ssymnode fldint = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("Intval"),
		T_FIELD, HashByte, &fldobject, Basetype(BT_integer),
		0, sizeof(rint), 0, Mflgs, -136 };
ssymnode fldreal = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("Realval"),
		T_FIELD, HashByte, &fldint, Basetype(BT_real),
		0, sizeof(double), 0, Mflgs, -137 };
ssymnode fldcomp = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("Component_Size"),
		T_FIELD, HashByte, &fldreal, Basetype(BT_integer),
		sizeof(pointer), sizeof(rint), 0, Mflgs, -138 };
ssymnode fldsize = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("Size"),
		T_FIELD, HashByte, &fldcomp, Basetype(BT_integer),
		sizeof(double), sizeof(rint), 0, Mflgs, -118 };
ssymnode fldtypecode = {N_DECL_ID, Basetype(BT_anytype), NLF, NNC("TypeCode"),
		T_FIELD, HashByte, &fldsize, Basetype(BT_integer),
		sizeof(double)+sizeof(rint), sizeof(rint), 0, Mflgs, -119 };

extern node recsymt;
struct node3 BT_anytype = { N_TYP_RECORD, NIL, NLF, NIL, &recsymt,
			NCAST sizeof(struct anyvar) };
static node recsymt = { N_SYMBOL_TABLE, &BT_anytype, NLF, (nodep)&fldtypecode };
symnode SP_string = {N_DECL_ID, NIL, NLF, NNC("String"), T_BTYPE, HashByte,
	&fldtypecode, Basetype(SP_string), 0, sizeof(pointer), 0, Mflgs, 0 };
symnode SP_variable = { N_DECL_ID, NIL, NLF, NNC("Variable"), T_BTYPE, HashByte,
	&SP_string, Basetype(SP_variable), 0, sizeof(pointer), 0, 0 , Mflgs};

symnode SP_ordinal = {N_DECL_ID, NIL, NLF, NNC("Ordinal"), T_BTYPE, HashByte,
	&SP_variable, Basetype(SP_ordinal), 0, sizeof(int), 0, Mflgs, 0 };
symnode SP_number = { N_DECL_ID, NIL, NLF, NNC("Number"), T_BTYPE, HashByte,
	&SP_ordinal, Basetype(SP_number), 0, sizeof(rfloat), 0, Mflgs, 0 };

symnode SP_byte = {  N_DECL_ID, NIL, NLF, NNC("Byte"), T_BBTYPE, HashByte,
	CAST(symptr)0, Basetype(BT_byte), 255, sizeof(char), 0, Mflgs, -99 };
symnode BT_integer = {  N_DECL_ID, NIL, NLF, NNC("integer"), T_BTYPE, HashByte,
	&SP_byte, CAST(nodep )&BT_integer, MAXINT, sizeof(rint), 0, Mflgs, -1};
symnode BT_pointer = { N_DECL_ID, NIL, NLF, NNC("Pointer"), T_BTYPE, HashByte,
	&BT_integer, Basetype(BT_pointer), 0, sizeof(pointer), 0, Mflgs, -71 };
symnode BT_real = {  N_DECL_ID, NIL, NLF, NNC("real"), T_BTYPE, HashByte,
	&BT_pointer, CAST(nodep )&BT_real, 0, sizeof(rfloat), 0, Mflgs, -2 };
ssymnode boolparm = {  N_DECL_ID, NIL, NLF, NNC("Boolean"), T_BBTYPE, HashByte,
	&BT_real, &BT_boolean, 1, sizeof(char), 0, Mflgs, -3 };
symnode BT_char = {  N_DECL_ID, NIL, NLF, NNC("char"), T_BTYPE, HashByte,
	&boolparm, CAST(nodep )&BT_char, 255, sizeof(char), 0, Mflgs, -4 };
symnode ndat = { N_DECL_ID, NIL, NLF, NNC("Generic"), T_BBTYPE, HashByte,
	&BT_char, Basetype(BT_anytype), 0, sizeof(struct anyvar), 0,Mflgs,-116};
symnode SP_file = { N_DECL_ID, NIL, NLF, NNC("AnyFile"), T_BTYPE, HashByte,
	&ndat, Basetype(SP_file), 0, sizeof(struct pas_file), 0, Mflgs, -188 };

static node refchar = { N_ID, NIL, NLF, Basetype(BT_char) };
node BT_text = { N_TYP_FILE, NIL, NLF, &refchar };
ssymnode nd4 = {  N_DECL_ID, NIL, NLF, NNC("text"), T_BBTYPE, HashByte,
		&SP_file, CAST(nodep )&BT_text, 0, sizeof(int *), 0, Mflgs, -5 };

		/* Declarations for builtin routines */

/* parameter list for arbitrary and no parameter routines */

static bargs empty_list[] = { NULL, NULL };

/* built in parameter declarations can cheat.
 * on the left, we don't even need a list.  A pure stub is ok.
 * and if it isn't a stub it doesn't even have to be a symbol.
 * it can be a quick kludge with the name where the parent goes
 */

	/* parameter list, one real parameter */
static struct node2 un_real_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_real) };
static bargs l_real_param[] = { NULL, IBC &un_real_param, NULL, NULL };
static struct node2 un_str_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_string) };
/* static struct node2 un_vstr_param = { N_FORM_REF, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_string) }; */

static struct node2 un_ptr_param = { N_FORM_REF, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_pointer) };
static struct node2 un_valptr_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_pointer) };
static struct node2 un_var_param = { N_FORM_REF, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_variable) };

static struct node2 un_fil_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_file) };

	/* parameter list, one integer parameter */

static struct node2 un_int_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_integer) };
static struct node2 un_chr_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_char) };
static struct node2 un_vint_param = { N_FORM_REF, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(BT_integer) };

static struct node2 un_bool_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(boolparm) };

static struct node2 un_text_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(nd4) };

static bargs l_int_param[] = { NULL, IBC &un_int_param, NULL, NULL };

	/* parameter list for peek */

static bargs l_peek_param[] = { NULL, IBC &un_int_param, "\200address", NULL };


/* parameter list, one ordinal parameter */
static struct node2 un_ord_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_ordinal) };
static bargs l_ord_param[] = { NULL, IBC &un_ord_param, NULL, NULL };

/* parameter list, one number parameter passed on */
static struct node2 un_num_param = { N_FORM_VALUE, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_number) };
static struct node2 un_vnum_param = { N_FORM_REF, NIL, NLF,
		CAST(nodep ) &G_Stub, Basetype(SP_number) };
static bargs l_num_param[] = { NULL, IBC &un_num_param, NULL, NULL };

/* declarations for the i/o guys */

static rout_node r_write = { 0, do_write, 1, 255, empty_list };
static rout_node r_lnwrite = { 0, do_lnwrite, 0, 255, empty_list };
static rout_node r_read = { 0, do_read, 1, 255, empty_list };
static rout_node r_readln = { 0, do_lnread, 0, 255, empty_list };

static rout_node r_pause = { 0, do_pause, 0, 1, empty_list };
static char offsname[] = "\200Offset";
static char destname[] = "\200Dest";

#ifdef GEM
static bargs l_inttext_param[] = {
	NULL, IBC &un_int_param, NULL,
	      IBC &un_text_param, win_str,
	NULL };
#endif

#ifdef FULL
static char sourcename[] = "\200Source";
static bargs l_pack[] = { NULL, IBC &un_var_param, sourcename,
				IBC &un_ord_param, offsname,
				IBC &un_var_param, destname, NULL };
static bargs l_unpack[] = { NULL, IBC &un_var_param, sourcename,
				IBC &un_var_param, destname,
				IBC &un_ord_param, offsname, NULL };
static rout_node r_pack = {0, do_pack, 3, 3, l_pack };
static rout_node r_unpack = {0, do_unpack, 3, 3, l_unpack };
#else

static rout_node r_pack = { 0, do_pack, 3, 3, empty_list };
#define r_unpack r_pack
#endif

/* the real to int guys */
static rout_node r_trunc = { 0, do_trunc, 1, 1, l_real_param };
static rout_node r_round = { 0, do_round, 1, 1, l_real_param };
/* the math guys */
static rout_node r_sin = { 0, do_sin, 1, 1, l_real_param };
static rout_node r_cos = { 0, do_cos, 1, 1, l_real_param };
static rout_node r_arctan = { 0, do_arctan, 1, 1, l_real_param };
static rout_node r_ln = { 0, do_ln, 1, 1, l_real_param };
static rout_node r_exp = { 0, do_exp, 1, 1, l_real_param };
static rout_node r_sqrt = { 0, do_sqrt, 1, 1, l_real_param };

#ifdef QNX
static bargs l_sysproc[] = {NULL, IBC &un_int_param, "\200Routine", NULL };
#else
static bargs l_sysproc[] = {NULL, IBC &un_valptr_param, "\200Routine", NULL };
#endif

static rout_node r_sysproc = {0, do_sysproc, 1, 10, l_sysproc};
static rout_node r_sysfunc = {0, do_sysfunc, 1, 10, l_sysproc};
static rout_node r_peek = { 0, do_peek, 1, 1, l_peek_param };
static rout_node r_chr = { 0, do_chr, 1, 1, l_int_param };
static rout_node r_odd = {0, do_odd, 1, 1, l_int_param };

static rout_node r_abs = {0, do_abs, 1, 1, l_num_param };
static rout_node r_sqr = {0, do_sqr, 1, 1, l_num_param };

static rout_node r_ord = {0, do_ord, 1, 1, l_ord_param };
static rout_node r_succ = {0, do_succ, 1, 1, l_ord_param };
static rout_node r_pred = {0, do_pred, 1, 1, l_ord_param };
static bargs l_poke_param[] = {NULL,  IBC &un_int_param, "\200Address",
			IBC &un_int_param, "\200Value", NULL};
static rout_node r_poke = {0, do_poke, 2, 2, l_poke_param };
static bargs l_strcat[] = { NULL, IBC &un_str_param, NULL, IBC &un_str_param, NULL, NULL};
static rout_node r_strcat = {0, do_strcat, 2, 2, l_strcat };

static char longstr[] = "\200Length";

static bargs l_strdelete[] = {NULL, IBC &un_str_param, NULL, IBC &un_int_param,
		longstr, IBC &un_int_param, offsname, NULL };
static bargs l_delete[] = {NULL, IBC &un_str_param, NULL, IBC &un_int_param,
		offsname, IBC &un_int_param, longstr, NULL };
static rout_node r_strdelete = { 0, do_strdelete, 3, 3, l_strdelete };
static rout_node r_delete = { 0, do_strdelete, 3, 3, l_delete };
static rout_node r_copy = { 0, do_copy, 3, 3, l_delete };

static bargs l_strinsert[] = {NULL, IBC &un_str_param, NULL, IBC &un_str_param,
		NULL, IBC &un_int_param, offsname, NULL };
static rout_node r_strinsert = { 0, do_strinsert, 3, 3, l_strinsert };

static bargs l_substr[] = {NULL, IBC &un_str_param, NULL, IBC &un_int_param,
		longstr, IBC &un_int_param, offsname, 
		IBC &un_str_param, destname, NULL };
static rout_node r_substr = { 0, do_substr, 4, 4, l_substr };

static bargs l_onestr[] = {NULL, IBC &un_str_param, NULL, NULL };
static bargs l_twostr[] = {NULL, IBC &un_str_param, NULL, IBC &un_str_param, NULL,
					NULL };

static rout_node r_strlen = {0, do_strlen, 1, 1, l_onestr };
static rout_node r_strscan = {0, do_strscan, 2, 2, l_twostr };
static rout_node r_strsize = {0, do_stsize, 1, 1, l_onestr };

static bargs l_inrand[] = {NULL, IBC &un_int_param, "\200seed", IBC &un_int_param,
			"\200modulus", NULL };

static rout_node r_initrandom = { 0, do_initrandom, 2, 2, l_inrand };
static rout_node r_random = { 0, do_random, 0, 1, l_int_param };

static bargs l_pointer[] = {NULL, IBC &un_ptr_param, NULL, NULL };
static bargs l_valpointer[] = {NULL, IBC &un_valptr_param, NULL, NULL };
static bargs l_ponandsize[] = {NULL, IBC &un_ptr_param, NULL, IBC &un_int_param,
			"\200Size", NULL };
static bargs l_variable[] = {NULL, IBC &un_var_param, NULL, NULL };
static bargs l_addrrot[] = {NULL, IBC &un_var_param, NULL, IBC &un_int_param,
		"\200GetSegment", NULL };

/* Is this correct ??? */
static bargs l_blkmove[] = {NULL, IBC &un_var_param, NULL, IBC &un_int_param,
		offsname, IBC &un_int_param, "\200Segment", NULL };


static bargs l_onetext[] = {NULL, IBC &un_text_param, NULL, NULL };

static bargs l_fnname[] = {NULL, IBC &un_fil_param, NULL, IBC &un_str_param,
		"\200Filename", NULL };
static bargs l_onefile[] = {NULL, IBC &un_fil_param, NULL, NULL };
static bargs l_setnext[] = {NULL, IBC &un_fil_param, NULL, IBC &un_int_param,
		"\200Record #", NULL };

static bargs l_cursorto[] = {NULL, IBC &un_int_param, "\200Row", IBC &un_int_param,
		"\200Column", 
#ifdef GEM
		IBC &un_text_param, win_str,
#endif
		NULL };

#ifdef GEM
static bargs l_gotoxy[] = {NULL, IBC &un_int_param, "\200Column",
		IBC &un_int_param, "\200Row", 
		IBC &un_text_param, win_str,
		NULL };
#endif

static bargs l_twoint[] = {NULL, IBC &un_int_param, NULL, IBC &un_int_param,
		NULL, NULL };
static bargs l_tplib[] = {NULL, IBC &un_str_param, "\200Library", IBC &un_int_param,
		offsname, NULL };
static bargs l_onechar[] ={NULL, IBC &un_chr_param, NULL, NULL };

static rout_node r_makeptr = { 0, do_makeptr, 1, 1, l_variable };
#ifdef GEM
static rout_node r_cursorto = { 0, do_cursorto, 2, 3, l_cursorto };
static rout_node r_gotoxy = { 0, do_cursorto, 2, 3, l_gotoxy };
#else
static rout_node r_cursorto = { 0, do_cursorto, 2, 2, l_cursorto };
static rout_node r_gotoxy = { 0, do_cursorto, 2, 2, l_twoint };
#endif
static rout_node r_eof = {0, do_eof, 0, 1, l_onefile };
static rout_node r_eoln = {0, do_eoln, 0, 1, l_onetext };
static rout_node r_get = {0, do_get, 0, 1, l_onefile };
static rout_node r_put = {0, do_put, 0, 1, l_onefile };
static rout_node r_reset = {0, do_reset, 1, 2, l_fnname };
static rout_node r_rewrite = {0, do_rewrite, 1, 2, l_fnname };
static rout_node r_append = {0, do_append, 1, 2, l_fnname };
static rout_node r_update = {0, do_update, 1, 2, l_fnname };
static rout_node r_setnext = {0, do_setnext, 2, 2, l_setnext };

static rout_node r_dispose = { 0, do_dispose, 1, 99, l_valpointer };
static rout_node r_new = { 0, do_new, 1, 99, l_pointer };
static rout_node r_getmem = { 0, do_new, 2, 2, l_ponandsize };
static rout_node r_page = {0, do_page, 0, 1, l_onetext };
static rout_node r_close = {0, do_close, 1, 1, l_onefile };
static rout_node r_freemem = { 0, do_dispose, 2, 2, l_ponandsize };
static rout_node r_memf = { 0, do_memfunc, 0, 0, empty_list };
static rout_node r_tplib = { 0, do_tplib, 2, 255, l_tplib };
#ifdef GEM
static rout_node r_vid = { 0, do_vid, 0, 1, l_onetext };
#else
static rout_node r_vid = { 0, do_vid, 0, 0, empty_list };
#endif
static rout_node r_upcase ={ 0, do_upcase, 1, 1, l_onechar };

#ifdef FULL

static struct node3 SP_specbarray = { N_TYP_ARRAY, NIL, NLF,
	Basetype(BT_integer), Basetype(SP_byte), NCAST 1 };
static struct node3 SP_specwarray = { N_TYP_ARRAY, NIL, NLF,
		Basetype(BT_integer), Basetype(BT_integer), NCAST 2 };

#include "newbuilt.c"
ssymnode ndmem = {  N_DECL_ID, NIL, NLF, NNC("Mem"), T_MEMVAR, HashByte,
	&ndParamC, &SP_specbarray, 0, 0, 0, Lmf, -147  };
ssymnode ndmemw = {  N_DECL_ID, NIL, NLF, NNC("MemW"), T_MEMVAR, HashByte,
	&ndmem, &SP_specwarray, 0, 0, 0, Lmf, -126  };

#ifndef GEM

ssymnode ndport = {  N_DECL_ID, NIL, NLF, NNC("Port"), T_PORTVAR, HashByte,
	&ndmemw, &SP_specbarray, 0, 0, 0, Lmf, -149  };
ssymnode ndportw = {  N_DECL_ID, NIL, NLF, NNC("PortW"), T_PORTVAR, HashByte,
	&ndport, &SP_specwarray, 0, 0, 0, Lmf, -148  };
symnode ndlst = { N_DECL_ID, NIL, NLF, NNC("Lst"), T_FILENAME, HashByte,
		&ndportw, Basetype(BT_text), 3, sizeof(char), 0, Lmf, -140 };
symnode ndaux = { N_DECL_ID, NIL, NLF, NNC("Aux"), T_FILENAME, HashByte,
		&ndlst, Basetype(BT_text), 4, sizeof(char), 0, Lmf, -141 };
#else
#define ndaux ndmemw
#endif

symnode ndcon = { N_DECL_ID, NIL, NLF, NNC("Con"), T_FILENAME, HashByte,
		&ndaux, Basetype(BT_text), 5, sizeof(char), 0, Lmf, -142 };

#ifndef GEM
symnode ndtrm = { N_DECL_ID, NIL, NLF, NNC("Trm"), T_FILENAME, HashByte,
		&ndcon, Basetype(BT_text), 6, sizeof(char), 0, Lmf, -143 };
#else
#define ndtrm ndcon
#endif

symnode ndinp = { N_DECL_ID, NIL, NLF, NNC("Inp"), T_FILENAME, HashByte,
		&ndtrm, Basetype(BT_text), 7, sizeof(char), 0, Lmf, -144 };
symnode ndout = { N_DECL_ID, NIL, NLF, NNC("Out"), T_FILENAME, HashByte,
		&ndinp, Basetype(BT_text), 8, sizeof(char), 0, Lmf, -145 };

#ifndef GEM
symnode nderr = { N_DECL_ID, NIL, NLF, NNC("Err"), T_FILENAME, HashByte,
		&ndout, Basetype(BT_text), 9, sizeof(char), 0, Lmf, -146 };
#else
#define nderr	ndout
#endif

static rout_node r_exit = { 0, do_exit, 0, 0, empty_list };
ss ndexit = {  N_DECL_ID, RD(r_exit), NLF, NNC("Exit"), T_BPROC, HashByte,
	&nderr, Basetype(SP_proc), 0, 0, 0, Mflgs, -139 };
#else

static rout_node r_exit = { 0, do_exit, 0, 0, empty_list };
ss ndexit = {  N_DECL_ID, RD(r_exit), NLF, NNC("Exit"), T_BPROC, HashByte,
	&nd4, Basetype(SP_proc), 0, 0, 0, Mflgs, -139 };
#endif

ssymnode nd6 = {  N_DECL_ID, NIL, NLF, NNC("maxint"), T_BCONST, HashByte,
	&ndexit, CAST(nodep )&BT_integer, 32767, sizeof(int), 0, Mflgs, -6  };
ssymnode nd7 = {  N_DECL_ID, CAST(nodep )&r_new, NLF, NNC("New"), T_BTPROC,
	HashByte, &nd6, Basetype(SP_proc), 0, 0, 0, Mflgs, -7 };
ssymnode ndgm = {  N_DECL_ID, CAST(nodep )&r_getmem,NLF,NNC("GetMem"), T_BTPROC,
	HashByte, &nd7, Basetype(SP_proc), 0, 0, 0, Mflgs, -123 };
ssymnode ndfm = {  N_DECL_ID, CAST(nodep )&r_freemem, NLF, NNC("FreeMem"),
	T_BTPROC, HashByte, &ndgm, Basetype(SP_proc), 0, 0, 0, Mflgs, -124 };
ssymnode ndmma = {  N_DECL_ID, CAST(nodep )&r_memf, NLF, NNC("MemAvail"),
	T_BFUNC, HashByte, &ndfm, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -128 };
ssymnode ndmmx = {  N_DECL_ID, CAST(nodep )&r_memf, NLF, NNC("MaxAvail"),
	T_BFUNC, HashByte, &ndmma, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -129 };
#ifdef LARGE
# define TPTYPE T_BTPROC
# define TFTYPE T_BTFUNC
#else
# define TPTYPE T_BPROC
# define TFTYPE T_BFUNC
#endif

#ifdef GEM
#define TPLIBFUNC	"STLibFunc"
#define TPLIBPROC	"STLibProc"
#else
#define TPLIBFUNC	"TPLibFunc"
#define TPLIBPROC	"TPLibProc"
#endif

ssymnode ndtlf = {  N_DECL_ID, CAST(nodep )&r_tplib, NLF, NNC( TPLIBFUNC ),
	TFTYPE, HashByte, &ndmmx, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -130 };
ssymnode ndtlp = {  N_DECL_ID, CAST(nodep )&r_tplib, NLF, NNC( TPLIBPROC ),
	TPTYPE, HashByte, &ndtlf, Basetype(SP_proc), 0, 0, 0, Mflgs, -131 };
ssymnode ndnvid = {  N_DECL_ID, CAST(nodep )&r_vid, NLF, NNC("NormVideo"),
	T_BPROC, HashByte, &ndtlp, Basetype(SP_proc), 0, 0, 0, Mflgs, -132 };
ssymnode ndhvid = {  N_DECL_ID, CAST(nodep )&r_vid, NLF, NNC("HighVideo"),
	T_BPROC, HashByte, &ndnvid, Basetype(SP_proc), 0, 0, 0, Mflgs, -135 };
ssymnode ndlvid = {  N_DECL_ID, CAST(nodep )&r_vid, NLF, NNC("LowVideo"),
	T_BPROC, HashByte, &ndhvid, Basetype(SP_proc), 0, 0, 0, Mflgs, -133 };
ssymnode ndupc = {  N_DECL_ID, CAST(nodep )&r_upcase, NLF, NNC("UpCase"),
	T_BFUNC, HashByte, &ndlvid, Basetype(BT_char), 0, sizeof(rint), 0, Mflgs, -134 };
ssymnode ndclose = {  N_DECL_ID, CAST(nodep )&r_close, NLF, NNC("Close"),
	T_BTPROC, HashByte, &ndupc, Basetype(SP_proc), 0, 0, 0, Mflgs, -125 };

#ifdef PARSER

/* This guy has the same symbol save ID as "anyfile" so that it will show up
 * that way when the guy loads the file.
 */
ssymnode ndBlockFile = { N_DECL_ID, NIL, NLF, NNC("BlockFile"), T_TYPE,
	HashByte, &ndclose, Basetype(SP_file), 0, 0, 0, Mflgs, -188 };

ssymnode ndconcat = {  N_DECL_ID, NIL, NLF, NNC("Concat"),
	T_BPROC, HashByte, &ndBlockFile, Basetype(SP_proc), 0, 0, 0, Mflgs, SV_CONCAT };
ssymnode nd8 = {  N_DECL_ID, CAST(nodep )&r_dispose, NLF, NNC("Dispose"), T_BTPROC, HashByte,
			&ndconcat, Basetype(SP_proc), 0, 0, 0, Mflgs, -8 };
#else
ssymnode nd8 = {  N_DECL_ID, CAST(nodep )&r_dispose, NLF, NNC("Dispose"), T_BTPROC, HashByte,
			&ndclose, Basetype(SP_proc), 0, 0, 0, Mflgs, -8 };
#endif
ssymnode nd9 = {  N_DECL_ID, CAST(nodep )&r_read, NLF, NNC("read"), T_READLN, HashByte,
			&nd8, Basetype(SP_proc), 0, 0, 0, Mflgs, -9 };
ssymnode nd10 = { N_DECL_ID, CAST(nodep )&r_readln, NLF, NNC("readln"), T_READLN, HashByte,
			&nd9, Basetype(SP_proc), 0, 0, 0, Mflgs, -10 };
ssymnode nd11 = { N_DECL_ID, CAST(nodep )&r_write, NLF, NNC("write"), T_WRITELN, HashByte,
			&nd10, Basetype(SP_proc), 0, 0, 0, Mflgs, -11 };
ssymnode nd12 = { N_DECL_ID, CAST(nodep )&r_lnwrite, NLF, NNC("writeln"), T_WRITELN, HashByte,
			&nd11, Basetype(SP_proc), 0, 0, 0, Mflgs, -12 };
ssymnode nd13 = { N_DECL_ID, NCAST &r_page, NLF, NNC("page"), T_BPROC, HashByte,
			&nd12, Basetype(SP_proc), 0, 0, 0, Mflgs, -13 };
ssymnode nd14 = { N_DECL_ID, NCAST &r_eof, NLF, NNC("eof"), T_BFUNC, HashByte,
			&nd13, CAST(nodep )&BT_boolean, 0, 0, 0, Mflgs, -14 };
ssymnode nd15 = { N_DECL_ID, NCAST &r_eoln, NLF, NNC("eoln"), T_BFUNC, HashByte,
			&nd14, CAST(nodep )&BT_boolean, 0, 0, 0, Mflgs, -15 };
ssymnode nd16 = { N_DECL_ID, NCAST &r_get, NLF, NNC("get"), T_BTPROC, HashByte,
			&nd15, Basetype(SP_proc), 0, 0, 0, Mflgs, -16 };

static rout_node r_address = { 0, do_address, 1, 2, l_addrrot };
static rout_node r_vartomem = { 0, do_vartomem, 2, 3, l_blkmove };
static rout_node r_memtovar = { 0, do_memtovar, 2, 3, l_blkmove };

static rout_node r_delay = { 0, do_delay, 1, 1, l_int_param };
static rout_node r_sound = { 0, do_sound, 1, 1, l_int_param };
static rout_node r_halt = { 0, do_halt, 0, 1, empty_list };
#ifdef GEM
static bargs l_text[] = { NULL, IBC &un_text_param, win_str, NULL };
static rout_node r_clreol = { 0, do_clreol, 0, 1, l_text };
static rout_node r_insdel = { 0, do_insdel, 0, 1, l_text };
static rout_node r_charwait = { 0, do_charwait, 0, 1, l_onefile };
#else
static rout_node r_clreol = { 0, do_clreol, 0, 0, empty_list };
static rout_node r_insdel = { 0, do_insdel, 0, 0, empty_list };
static rout_node r_charwait = { 0, do_charwait, 0, 0, empty_list };
#endif

static rout_node r_assign = { 0, do_assign, 2, 2, l_fnname };
static rout_node r_flpart = { 0, do_flpart, 1, 1, l_real_param };
static bargs l_str_param[] = { NULL, IBC &un_num_param, NULL, IBC &un_str_param,
					NULL, NULL };
static bargs l_valparam[] = { NULL, IBC &un_str_param, NULL, IBC &un_vnum_param,
			NULL, IBC &un_vint_param, "\200Code", NULL };
static rout_node r_str = { 0, do_str, 2, 2, l_str_param };
static rout_node r_val = { 0, do_val, 3, 3, l_valparam };

static bargs l_intr[] = { NULL, IBC &un_int_param, "\200interrupt",
				IBC &un_var_param, "\200registers", NULL };
static rout_node r_intr = { 0, do_intr, 2, 2, l_intr };

static char x1[]	= "\200X1";
static char y1[]	= "\200Y1";
static char x2[]	= "\200X2";
static char y2[]	= "\200Y2";
static char colour[]	= "\200Color";
static bargs l_draw[] = { NULL, IBC &un_int_param, x1,
				IBC &un_int_param, y1,
				IBC &un_int_param, x2,
				IBC &un_int_param, y2,
				IBC &un_int_param, colour,
				NULL };
#ifdef GEM
static rout_node r_draw = { 0, do_draw, 4, 5, l_draw };
#else
static rout_node r_draw = { 0, do_draw, 5, 5, l_draw };
#endif


#ifdef GEM
static bargs l_construct[] = {
		NULL, IBC &un_var_param, "\200Destination",
		/* Rest of parms are of unknown type */
		NULL };

static bargs l_dosound[] = {
		NULL, IBC &un_var_param, "\200Sound Buffer",
		NULL };

static bargs l_winvdi[] = {
		NULL, IBC &un_int_param, "\200NumPoints",
		      IBC &un_var_param, "\200contrl",
		      IBC &un_var_param, "\200intin",
		      IBC &un_var_param, "\200ptsin",
		      IBC &un_var_param, "\200intout",
		      IBC &un_var_param, "\200ptsout",
		NULL };

static bargs l_aescall[] = {
		NULL, IBC &un_var_param, "\200control",
		      IBC &un_var_param, "\200int_in",
		      IBC &un_var_param, "\200addr_in",
		      IBC &un_var_param, "\200int_out",
		      IBC &un_var_param, "\200addr_out",
		NULL };

static bargs l_vdicall[] = {
		NULL, IBC &un_var_param, "\200contrl",
		      IBC &un_var_param, "\200intin",
		      IBC &un_var_param, "\200ptsin",
		      IBC &un_var_param, "\200intout",
		      IBC &un_var_param, "\200ptsout",
		NULL };

static bargs l_wheremouse[] = {
		NULL, IBC &un_ptr_param, "\200WindowPtr",
		      IBC &un_vint_param, "\200X",
		      IBC &un_vint_param, "\200Y",
		      IBC &un_vint_param, "\200AbsX",
		      IBC &un_vint_param, "\200AbsY",
		      IBC &un_vint_param, "\200Buttons",
		NULL };

static bargs l_gsetpallette[] = { NULL,
		IBC &un_int_param, "\200Index",
		IBC &un_int_param, "\200R",
		IBC &un_int_param, "\200G",
		IBC &un_int_param, "\200B",
		NULL };

static bargs l_gsprite[] = {
		NULL, IBC &un_int_param, "\200Action",
		      IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		      IBC &un_var_param, "\200Sprite",
		      IBC &un_var_param, "\200SaveBlock",
		NULL };

static bargs l_movewin[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		NULL };

static bargs l_resizewin[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_int_param, "\200W",
		      IBC &un_int_param, "\200H",
		NULL };

static bargs l_setslider[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_bool_param, "\200Vert/Horiz",
		      IBC &un_int_param, "\200Position",
		      IBC &un_int_param, "\200Size",
		NULL };

static bargs l_setinfo[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_str_param, "\200String",
		NULL };

static bargs l_gwinopt[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_int_param, "\200Options",
		NULL };

static bargs l_gsetcoord[] = {
		NULL, IBC &un_int_param, "\200system",
		NULL };

static bargs l_gellipse[] = {
		NULL, IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		      IBC &un_int_param, "\200XRadius",
		      IBC &un_int_param, "\200YRadius",
		NULL };

static bargs l_gseedfill[] = {
		NULL, IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		NULL };

static bargs l_gsetcolor[] = {
		NULL, IBC &un_int_param, "\200Color",
		NULL };

static bargs l_gcreate[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_int_param, "\200Features",
		      IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		      IBC &un_int_param, "\200W",
		      IBC &un_int_param, "\200H",
		NULL };

static bargs l_gquick[] = {
		NULL, IBC &un_text_param, win_str,
		      IBC &un_int_param, "\200Features",
		      IBC &un_int_param, "\200QuickNumber",
		NULL };

static bargs l_gsetgraph[] = {
		NULL, IBC &un_text_param, win_str,
		NULL };

static bargs l_gmenutext[] = {
		NULL, IBC &un_chr_param, "\200Item",
		      IBC &un_str_param, "\200New Text",
	        NULL };

static bargs l_gmenucheck[] = {
		NULL, IBC &un_chr_param,  "\200Item",
		      IBC &un_bool_param, "\200Flag",
		NULL };

static bargs l_gmouseon[] = {
		NULL, IBC &un_bool_param, "\200On/Off",
		NULL };

static bargs l_gmenugettext[] = {
		NULL, IBC &un_chr_param, "\200Item",
		NULL };

static bargs l_noargs[] = {
		NULL, NULL };

static bargs l_gaddmenu[] = {
		NULL, IBC &un_valptr_param, "\200Menu",
		      IBC &un_int_param, "\200#",
		      IBC &un_str_param, "\200Title",
		      IBC &un_str_param, "\200Items",
		NULL };

static bargs l_gshowmenu[] = {
		NULL, IBC &un_valptr_param, "\200Menu",
		NULL };

static bargs l_gdrmode[] = {
		NULL, IBC &un_int_param, "\200Mode",
		NULL };

static bargs l_getparm[] = {
		NULL, 
			IBC &un_int_param, "\200Parm #",
		NULL };

static bargs l_gtext[] = {
		NULL, IBC &un_int_param, "\200X",
		      IBC &un_int_param, "\200Y",
		      IBC &un_str_param, "\200String",
		      IBC &un_int_param, "\200Style",
	        NULL };

static bargs l_alert[] = {
		NULL,
		IBC &un_int_param, "\200Default",
		IBC &un_str_param, "\200Alert",
		NULL };

static bargs l_filesel[] = {
		NULL, IBC &un_str_param, "\200Pattern",
		      IBC &un_str_param, "\200Directory",
		      IBC &un_str_param, "\200Filename",
		NULL };

static bargs l_event[] = {
		NULL,
		IBC &un_bool_param, "\200Wait",
		NULL };

static bargs l_fillpoly[] = {
		NULL, IBC &un_int_param, "\200NumberPoints",
		      IBC &un_var_param, "\200Points",
		NULL };

static bargs l_getprompt[] = { NULL,
		IBC &un_str_param, "\200Prompt",
		IBC &un_str_param, "\200Answer",
		NULL };

static bargs l_ginit[] = { NULL, NULL };
static bargs l_gterm[] = { NULL, NULL };

static rout_node r_wheremouse = { 0, do_wheremouse, 6, 6, l_wheremouse };
static rout_node r_fillpoly = { 0, do_fillpoly, 2, 2, l_fillpoly };
static rout_node r_gtopwin = { 0, do_gtopwin, 1, 1, l_gsetgraph };
static rout_node r_gdrmode = { 0, do_drmode, 1, 1, l_gdrmode };

static rout_node r_gsprite = { 0, do_gsprite, 5, 5, l_gsprite };

static rout_node r_gwinopt = { 0, do_gwinopt, 2, 2, l_gwinopt };
static rout_node r_gsetcolor = { 0, do_gsetcolor, 1, 1, l_gsetcolor };
static rout_node r_gellipse = { 0, do_gellipse, 4, 4, l_gellipse };
static rout_node r_gseedfill = { 0, do_gseedfill, 2, 2, l_gseedfill };
static rout_node r_gsetcoord = { 0, do_SetCoord, 1, 1, l_gsetcoord };

static rout_node r_getprompt = { 0, do_getprompt, 2, 2, l_getprompt };
static rout_node r_gsetgraph = { 0, do_SetGraph, 1, 1, l_gsetgraph };
static rout_node r_gcreate = { 0, do_gcreate, 6, 6, l_gcreate };
static rout_node r_gquick = { 0, do_quickwin, 3, 3, l_gquick };

static rout_node r_gmenutext = { 0, do_gmenutext, 2, 2, l_gmenutext };
static rout_node r_gmenuenable = { 0, do_gmenuenable, 2, 2, l_gmenucheck };
static rout_node r_gmenucheck = { 0, do_gmenucheck, 2, 2, l_gmenucheck };
static rout_node r_gnewmenu = { 0, do_gnewmenu, 0, 0, l_noargs };
static rout_node r_gaddmenu = { 0, do_gaddmenu, 4, 4, l_gaddmenu };
static rout_node r_gshowmenu = { 0, do_gshowmenu, 1, 1, l_gshowmenu };

static rout_node r_gtext = { 0, do_gtext, 4, 4, l_gtext };
static rout_node r_alert = { 0, do_alert, 2, 2, l_alert };
static rout_node r_event = { 0, do_event, 1, 1, l_event };
static rout_node r_ginit = { 0, do_ginit, 0, 0, l_ginit };
static rout_node r_gterm = { 0, do_gterm, 0, 0, l_gterm };
static rout_node r_getparm = { 0, do_getparm, 1, 1, l_getparm };
static rout_node r_filesel = { 0, do_filesel, 3, 3, l_filesel };

static rout_node r_resizewin = { 0, do_resizewin, 3, 3, l_resizewin };
static rout_node r_movewin = { 0, do_movewin, 3, 3, l_movewin };
static rout_node r_setslider = { 0, do_setslider, 4, 4, l_setslider };
static rout_node r_setinfo = { 0, do_setinfo, 2, 2, l_setinfo };
static rout_node r_mousetype = { 0, do_mousetype, 1, 1, l_int_param };

static rout_node r_gsetpallette = { 0, do_gsetpallette, 4, 4, l_gsetpallette };
static rout_node r_gmouseon = { 0, do_gmouseon, 1, 1, l_gmouseon };
static rout_node r_gfillpat = { 0, do_gfillpat, 1, 1, l_int_param };
static rout_node r_vdicall = { 0, do_vdicall, 3, 5, l_vdicall };
static rout_node r_aescall = { 0, do_aescall, 3, 5, l_aescall };
static rout_node r_winvdi = { 0, do_winvdi, 4, 6, l_winvdi };
static rout_node r_gtopptr = { 0, do_gtopptr, 0, 0, l_noargs };
static rout_node r_ghidewin = { 0, do_hidewin, 0, 0, l_noargs };
static rout_node r_dosound = { 0, do_dosound, 1, 1, l_dosound };
static rout_node r_construct = { 0, do_construct, 2, 255, l_construct };
static rout_node r_gmenugettext = { 0, do_gmenugettext, 1, 1, l_gmenugettext };

ssymnode ndconstruct = { N_DECL_ID, NCAST &r_construct, NLF,
        NNC("Construct"),
	T_BTPROC, HashByte, &nd16, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -259 };

ssymnode nddosound = { N_DECL_ID, NCAST &r_dosound, NLF,
        NNC("DoSound"),
	T_BPROC, HashByte, &ndconstruct, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -258 };

ssymnode ndaes = { N_DECL_ID, NCAST &r_aescall, NLF,
        NNC("AES"),
	T_BPROC, HashByte, &nddosound, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -257 };

ssymnode ndquick = { N_DECL_ID, NCAST &r_gquick, NLF,
        NNC("QuickWindow"),
	T_BPROC, HashByte, &ndaes, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -256 };

ssymnode ndhidewin = { N_DECL_ID, NCAST &r_ghidewin, NLF,
        NNC("RemoveEditWindows"),
	T_BPROC, HashByte, &ndquick, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -255 };

ssymnode ndgtopptr = { N_DECL_ID, NCAST &r_gtopptr, NLF, NNC("TopWindowPtr"),
	T_BFUNC, HashByte, &ndhidewin, CAST(nodep )&BT_pointer, 0,
	 sizeof(pointer), 0, Mflgs, -254 };

ssymnode ndwinvdi = { N_DECL_ID, NCAST &r_winvdi, NLF,
        NNC("WinVDI"),
	T_BPROC, HashByte, &ndgtopptr, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -253 };

ssymnode ndvdicall = { N_DECL_ID, NCAST &r_vdicall, NLF,
        NNC("VDI"),
	T_BPROC, HashByte, &ndwinvdi, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -252 };

ssymnode ndmouseon = { N_DECL_ID, NCAST &r_gmouseon, NLF,
        NNC("MouseOn"),
	T_BPROC, HashByte, &ndvdicall, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -251 };

ssymnode ndgfillpat = { N_DECL_ID, NCAST &r_gfillpat, NLF,
        NNC("FillPattern"),
	T_BPROC, HashByte, &ndmouseon, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -250 };
ssymnode ndgsetpall = { N_DECL_ID, NCAST &r_gsetpallette, NLF,
        NNC("SetPalette"),
	T_BPROC, HashByte, &ndgfillpat, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -249 };
ssymnode ndwheremouse = { N_DECL_ID, NCAST &r_wheremouse, NLF, NNC("WhereMouse"),
	T_BPROC, HashByte, &ndgsetpall, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -248 };
ssymnode ndresizewin = { N_DECL_ID, NCAST &r_resizewin, NLF, NNC("ResizeWindow"),
	T_BPROC, HashByte, &ndwheremouse, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -247 };
ssymnode ndmovewin = { N_DECL_ID, NCAST &r_movewin, NLF, NNC("MoveWindow"),
	T_BPROC, HashByte, &ndresizewin, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -246 };
ssymnode ndmousetype = { N_DECL_ID, NCAST &r_mousetype, NLF, NNC("MouseType"),
	T_BPROC, HashByte, &ndmovewin, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -245 };
ssymnode ndsetinfo = { N_DECL_ID, NCAST &r_setinfo, NLF, NNC("SetInformationLine"),
	T_BPROC, HashByte, &ndmousetype, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -244 };
ssymnode ndsetsl = { N_DECL_ID, NCAST &r_setslider, NLF, NNC("SetSlider"),
	T_BPROC, HashByte, &ndsetinfo, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -243 };
ssymnode ndgetprompt = { N_DECL_ID, NCAST &r_getprompt, NLF, NNC("GetPromptString"),
	T_BTPROC, HashByte, &ndsetsl, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -242 };

/* We want the type, so that we can get the size of the array of points
 * so we can see if there are enough points
 */
ssymnode ndfillpoly = { N_DECL_ID, NCAST &r_fillpoly, NLF, NNC("FillPolygon"),
	T_BTPROC, HashByte, &ndgetprompt, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -241 };

ssymnode ndfilesel = { N_DECL_ID, NCAST &r_filesel, NLF, NNC("FileSelector"),
	T_BTPROC, HashByte, &ndfillpoly, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -240 };

ssymnode ndgmenugt = { N_DECL_ID, NCAST &r_gmenugettext, NLF,
	NNC( "MenuGetText" ), T_BFUNC, HashByte, &ndfilesel,
	Basetype( SP_string ), 0, 8, 0, Mflgs, -238 };

ssymnode ndgtopwin = { N_DECL_ID, NCAST &r_gtopwin, NLF, NNC("TopWindow"),
	T_BPROC, HashByte, &ndgmenugt, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -239 };

ssymnode ndgdrmode = { N_DECL_ID, NCAST &r_gdrmode, NLF, NNC("DrawMode"),
	T_BPROC, HashByte, &ndgtopwin, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -221 };

/* Push the types as well, so this is a T_BTPROC */
ssymnode ndgsprite = { N_DECL_ID, NCAST &r_gsprite, NLF, NNC("DrawSprite"),
	T_BTPROC, HashByte, &ndgdrmode, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -237 };

ssymnode ndgwinopt = { N_DECL_ID, NCAST &r_gwinopt, NLF, NNC("WindowOptions"),
	T_BPROC, HashByte, &ndgsprite, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -236 };

ssymnode ndgseed = { N_DECL_ID, NCAST &r_gseedfill, NLF, NNC("SeedFill"),
	T_BPROC, HashByte, &ndgwinopt, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -235 };

ssymnode ndgellipse = { N_DECL_ID, NCAST &r_gellipse, NLF, NNC("Ellipse"),
	T_BPROC, HashByte, &ndgseed, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -234 };

ssymnode ndsetcolor = { N_DECL_ID, NCAST &r_gsetcolor, NLF, NNC("DrawColor"),
	T_BPROC, HashByte, &ndgellipse, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -233 };

ssymnode ndsetcoord = { N_DECL_ID, NCAST &r_gsetcoord, NLF, NNC("SetCoordinate"),
	T_BPROC, HashByte, &ndsetcolor, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -232 };

ssymnode ndsetgraph = { N_DECL_ID, NCAST &r_gsetgraph, NLF, NNC("GraphicsWindow"),
	T_BPROC, HashByte, &ndsetcoord, Basetype( SP_proc ), 0, 0, 0,
	Mflgs, -231 };

ssymnode ndgcreate = { N_DECL_ID, NCAST &r_gcreate, NLF, NNC("NewWindow"),
	T_BPROC, HashByte, &ndsetgraph, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -230 };

ssymnode ndgetevparm = { N_DECL_ID, NCAST &r_getparm, NLF, NNC("EventParameter"),
	T_BFUNC, HashByte, &ndgcreate, CAST(nodep)&BT_integer, 0, sizeof(rint),
	0, Mflgs, -229 };

ssymnode ndgmen6 = { N_DECL_ID, NCAST &r_gmenutext, NLF, NNC("MenuSetText"),
	T_BPROC, HashByte, &ndgetevparm, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -228 };

ssymnode ndgmen5 = { N_DECL_ID, NCAST &r_gmenuenable, NLF, NNC("MenuEnable"),
	T_BPROC, HashByte, &ndgmen6, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -227 };

ssymnode ndgmen4 = { N_DECL_ID, NCAST &r_gmenucheck, NLF, NNC("MenuCheckmark"),
	T_BPROC, HashByte, &ndgmen5, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -226 };

ssymnode ndgmen3 = { N_DECL_ID, NCAST &r_gnewmenu, NLF, NNC("NewMenuBar"),
	T_BFUNC, HashByte, &ndgmen4, CAST(nodep )&BT_pointer, 0,
	 sizeof(pointer), 0, Mflgs, -225 };

ssymnode ndgmen2 = { N_DECL_ID, NCAST &r_gshowmenu, NLF, NNC("DisplayMenuBar"),
	T_BPROC, HashByte, &ndgmen3, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -224 };

ssymnode ndgmen1 = { N_DECL_ID, NCAST &r_gaddmenu, NLF, NNC("AddMenu"),
	T_BPROC, HashByte, &ndgmen2, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -223 };

ssymnode ndgtext = { N_DECL_ID, NCAST &r_gtext, NLF, NNC("GText"),
	T_BPROC, HashByte, &ndgmen1, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -222 };

ssymnode ndgterm = { N_DECL_ID, NCAST &r_gterm, NLF, NNC("GemFinish"),
	T_BPROC, HashByte, &ndgtext, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -220 };

ssymnode ndginit = { N_DECL_ID, NCAST &r_ginit, NLF, NNC("GemStart"),
	T_BPROC, HashByte, &ndgterm, Basetype( SP_proc ), 0, 0,
	0, Mflgs, -219 };

ssymnode ndgetevt = { N_DECL_ID, NCAST &r_event, NLF, NNC("GetEvent"),
	T_BFUNC, HashByte, &ndginit, CAST(nodep)&BT_integer, 0, sizeof(rint),
	0, Mflgs, -218 };

ssymnode ndalert = { N_DECL_ID, NCAST &r_alert, NLF, NNC("Alert"),
	 T_BFUNC, HashByte,
	&ndgetevt, CAST(nodep)&BT_integer, 0, sizeof(rint), 0, Mflgs, -217};

ssymnode nd17 = { N_DECL_ID, NCAST &r_put, NLF, NNC("put"), T_BTPROC, HashByte,
			&ndalert, Basetype(SP_proc), 0, 0, 0, Mflgs, -17 };
#else GEM

ssymnode nd17 = { N_DECL_ID, NCAST &r_put, NLF, NNC("put"), T_BTPROC, HashByte,
			&nd16, Basetype(SP_proc), 0, 0, 0, Mflgs, -17 };

#endif GEM

static bargs l_plot[] = { NULL, IBC &un_int_param, "\200X",
				IBC &un_int_param, "\200Y",
				IBC &un_int_param, colour,
				NULL };
#ifdef GEM
static rout_node r_plot = { 0, do_plot, 2, 3, l_plot };
#else
static rout_node r_plot = { 0, do_plot, 3, 3, l_plot };
#endif

static bargs l_window[] = { NULL, IBC &un_int_param, x1,
				   IBC &un_int_param, y1,
				   IBC &un_int_param, x2,
				   IBC &un_int_param, y2,
				   NULL };
static rout_node r_window = { 0, do_window, 4, 4, l_window };

static rout_node r_grmode = { 0, do_grmode, 0, 8, empty_list };

static rout_node r_textmode = { 0, do_textmode, 0, 1, l_int_param };

#ifdef GEM
static rout_node r_colour = { 0, do_colour, 1, 2, l_inttext_param };
#else
static rout_node r_colour = { 0, do_colour, 1, 1, l_int_param };
#endif

static rout_node r_ior = { 0, do_ioresult, 0, 0, empty_list };
static rout_node r_iochk = { 0, do_iocheck, 1, 1, l_int_param };

ssymnode nd18 = { N_DECL_ID, NCAST &r_address, NLF, NNC("address"), T_BFUNC, HashByte,
		&nd17, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -18};
ssymnode ndvtm = { N_DECL_ID, NCAST &r_vartomem, NLF, NNC("Var_To_Mem"), T_BTPROC, HashByte,
		&nd18, Basetype(SP_proc), 0, sizeof(rint), 0,Mflgs, -77};
ssymnode ndmtv = { N_DECL_ID, NCAST &r_memtovar, NLF, NNC("Mem_To_Var"), T_BTPROC, HashByte,
		&ndvtm, Basetype(SP_proc), 0, sizeof(rint), 0,Mflgs, -78};
ssymnode nddelay = { N_DECL_ID, NCAST &r_delay, NLF, NNC("Delay"), T_BPROC,
	HashByte, &ndmtv, CAST(nodep )&BT_integer, 0, 0, 0,Mflgs, -79};
ssymnode ndhalt = { N_DECL_ID, NCAST &r_halt, NLF, NNC("Halt"), T_BPROC,
	HashByte, &nddelay, Basetype(SP_proc), 0, 0, 0,Mflgs, -80};
ssymnode ndsound = { N_DECL_ID, NCAST &r_sound, NLF, NNC("Sound"), T_BPROC,
	HashByte, &ndhalt, Basetype(SP_proc), 0, 0, 0,Mflgs, -81};
ssymnode ndclrscr = { N_DECL_ID, NCAST &r_page, NLF, NNC("ClrScr"), T_BPROC,
	HashByte, &ndsound, Basetype(SP_proc), 0, 0, 0,Mflgs, -82};
ssymnode ndgotoxy = { N_DECL_ID, NCAST &r_gotoxy, NLF, NNC("GotoXY"), T_BPROC,
	HashByte, &ndclrscr, Basetype(SP_proc), 0, 0, 0,Mflgs, -83};
ssymnode ndclreol = { N_DECL_ID, NCAST &r_clreol, NLF, NNC("ClrEol"), T_BPROC,
	HashByte, &ndgotoxy, Basetype(SP_proc), 0, 0, 0,Mflgs, -84};
ssymnode ndins = { N_DECL_ID, NCAST &r_insdel, NLF, NNC("InsLine"), T_BPROC,
	HashByte, &ndclreol, Basetype(SP_proc), 0, 0, 0,Mflgs, -85};
ssymnode nddel = { N_DECL_ID, NCAST &r_insdel, NLF, NNC("DelLine"), T_BPROC,
	HashByte, &ndins, Basetype(SP_proc), 0, 0, 0,Mflgs, -86};
ssymnode ndkey = { N_DECL_ID, NCAST &r_charwait, NLF, NNC("KeyPressed"),T_BFUNC,
	HashByte, &nddel, Basetype(BT_boolean), 0, sizeof(rint), 0,Mflgs, -87};
ssymnode ndassign = { N_DECL_ID, NCAST &r_assign, NLF, NNC("Assign"),T_BPROC,
	HashByte, &ndkey, Basetype(SP_proc), 0, 0, 0,Mflgs, -90};
ssymnode ndint = { N_DECL_ID, NCAST &r_flpart, NLF, NNC("Int"),T_BFUNC,
	HashByte, &ndassign,Basetype(BT_real), 0, sizeof(rfloat), 0,Mflgs, -91};
ssymnode ndfrac = { N_DECL_ID, NCAST &r_flpart, NLF, NNC("Frac"),T_BFUNC,
	HashByte, &ndint, Basetype(BT_real), 0, sizeof(rfloat), 0,Mflgs, -92};
ssymnode nddelete = { N_DECL_ID, NCAST &r_delete, NLF, NNC("Delete"),T_BTPROC,
	HashByte, &ndfrac, Basetype(SP_proc), 0, 0, 0,Mflgs, -93};
ssymnode ndinsert = { N_DECL_ID, NCAST &r_strinsert,NLF, NNC("Insert"),T_BTPROC,
	HashByte, &nddelete, Basetype(SP_proc), 0, 0, 0,Mflgs, -94};
ssymnode ndstr = { N_DECL_ID, NCAST &r_str,NLF, NNC("Str"),T_BTPROC,
	HashByte, &ndinsert, Basetype(SP_proc), 0, 0, 0,Mflgs, -95};
ssymnode ndval = { N_DECL_ID, NCAST &r_val,NLF, NNC("Val"),T_BTPROC,
	HashByte, &ndstr, Basetype(SP_proc), 0, 0, 0,Mflgs, -96};
ssymnode ndcopy = { N_DECL_ID, NCAST &r_copy,NLF, NNC("Copy"),T_BFUNC,
	HashByte, &ndval, Basetype(SP_string), 0, 8, 0,Mflgs, -97};
symnode ndkbd = { N_DECL_ID, NIL, NLF, NNC("Kbd"), T_FILENAME, HashByte,
		&ndcopy, Basetype(BT_text), 2, sizeof(char), 0, Mflgs, -101 };
ssymnode ndiores = { N_DECL_ID, NCAST &r_ior, NLF, NNC("ioresult"), T_BFUNC,
	HashByte,&ndkbd,Basetype(BT_integer), 0, sizeof(rint), 0,Mflgs, -120 };
ssymnode ndiochk = { N_DECL_ID, NCAST &r_iochk, NLF, NNC("iochecking"), T_BPROC,
		HashByte, &ndiores, Basetype(SP_proc), 0, 0, 0, Mflgs, -121 };
ssymnode ndmakeptr = { N_DECL_ID, NCAST &r_makeptr, NLF, NNC("MakePointer"), T_BFUNC, HashByte,
	&ndiochk, CAST(nodep )&BT_pointer, 0, sizeof(pointer), 0,Mflgs, -70};
ssymnode nd19 = { N_DECL_ID, CAST(nodep )&r_pause, NLF, NNC("pause"), T_BPROC, HashByte,
			&ndmakeptr, Basetype(SP_proc), 0, 0, 0, Mflgs, -19 };
ssymnode nd20 = { N_DECL_ID, CAST(nodep )&r_peek, NLF, NNC("peek"), T_BFUNC, HashByte,
			&nd19, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -20};
ssymnode nd21 = { N_DECL_ID, NCAST &r_poke, NLF, NNC("poke"), T_BPROC, HashByte,
			&nd20, Basetype(SP_proc), 0, 0, 0, Mflgs, -21 };

ssymnode nd23 = { N_DECL_ID, NIL, NLF, NNC("Alice_Version"), T_BCONST, HashByte,
			&nd21, Basetype(BT_integer), ALICE_VERSION, sizeof(int), 0, Mflgs, -22 };

symnode UnknownSymbol = { N_DECL_ID, NIL, NLF, NNC("UnknownSymbol"), T_BCONST,
HashByte, &nd23, Basetype(BT_integer), 0, sizeof(int), 0, Mflgs, -98 };

#ifndef GEM
ssymnode ndintr = { N_DECL_ID, NCAST &r_intr, NLF, NNC("Intr"), T_BTPROC,
	HashByte, &UnknownSymbol, Basetype(SP_proc), 0, 0, 0, Mflgs, -102 };
#else
#define ndintr UnknownSymbol
#endif

ssymnode nddraw = { N_DECL_ID, NCAST &r_draw, NLF, NNC("Draw"), T_BPROC,
	HashByte, &ndintr, Basetype(SP_proc), 0, 0, 0, Mflgs, -103 };
ssymnode ndplot = { N_DECL_ID, NCAST &r_plot, NLF, NNC("Plot"), T_BPROC,
	HashByte, &nddraw, Basetype(SP_proc), 0, 0, 0, Mflgs, -104 };
ssymnode ndgrphwin = { N_DECL_ID, NCAST &r_window, NLF, NNC("GraphWindow"), T_BPROC,
	HashByte, &ndplot, Basetype(SP_proc), 0, 0, 0, Mflgs, -105 };
ssymnode ndgrmode = { N_DECL_ID, NCAST &r_grmode, NLF, NNC("GraphMode"), T_BPROC,
	HashByte, &ndgrphwin, Basetype(SP_proc), 0, 0, 0, Mflgs, -106 };
ssymnode ndgrcolmode = { N_DECL_ID, NCAST &r_grmode, NLF, NNC("GraphColorMode"), T_BPROC,
	HashByte, &ndgrmode, Basetype(SP_proc), 0, 0, 0, Mflgs, -107 };
ssymnode ndhires = { N_DECL_ID, NCAST &r_grmode, NLF, NNC("HiRes"), T_BPROC,
	HashByte, &ndgrcolmode, Basetype(SP_proc), 0, 0, 0, Mflgs, -108 };
ssymnode ndtextmode = { N_DECL_ID, NCAST &r_textmode, NLF, NNC("TextMode"), T_BPROC,
	HashByte, &ndhires, Basetype(SP_proc), 0, 0, 0, Mflgs, -109 };
ssymnode ndpalette = { N_DECL_ID, NCAST &r_colour, NLF, NNC("Palette"), T_BPROC,
	HashByte, &ndtextmode, Basetype(SP_proc), 0, 0, 0, Mflgs, -110 };
ssymnode ndgrphbkgnd = { N_DECL_ID, NCAST &r_colour, NLF, NNC("GraphBackground"), T_BPROC,
	HashByte, &ndpalette, Basetype(SP_proc), 0, 0, 0, Mflgs, -111 };
ssymnode ndhrcolour = { N_DECL_ID, NCAST &r_colour, NLF, NNC("HiResColor"), T_BPROC,
	HashByte, &ndgrphbkgnd, Basetype(SP_proc), 0, 0, 0, Mflgs, -112 };
ssymnode ndtextcolour = { N_DECL_ID, NCAST &r_colour, NLF, NNC("TextColor"), T_BPROC,
	HashByte, &ndhrcolour, Basetype(SP_proc), 0, 0, 0, Mflgs, -113 };
ssymnode ndtextbkgnd = { N_DECL_ID, NCAST &r_colour, NLF, NNC("TextBackground"), T_BPROC,
	HashByte, &ndtextcolour, Basetype(SP_proc), 0, 0, 0, Mflgs, -114 };
ssymnode ndtextwin = { N_DECL_ID, NCAST &r_window, NLF, NNC("Window"), T_BPROC,
	HashByte, &ndtextbkgnd, Basetype(SP_proc), 0, 0, 0, Mflgs, -115 };

ssymnode nd24 = { N_DECL_ID, NIL, NLF, NNC("StrEnd"), T_BCONST, HashByte,
	&ndtextwin, Basetype(BT_char), 0, sizeof(char), 0, Mflgs, -23 };


static rfloat our_pi = FLOATNUM(3.14159265358979323846);

symnode ndpi = { N_DECL_ID, NIL, NLF, NNC("Pi"), T_BCONST, HashByte,
	&nd24, Basetype(BT_real), (pint)&our_pi, sizeof(rfloat),0, Mflgs, -76 };
ssymnode nd25 = { N_DECL_ID, NCAST &r_strcat, NLF, NNC("StrConcat"), T_BTPROC, HashByte,
			&ndpi, Basetype(SP_proc), 0, 0, 0, Mflgs, -24 };
ssymnode nd26 = { N_DECL_ID, NCAST &r_strdelete, NLF, NNC("StrDelete"), T_BTPROC, HashByte,
			&nd25, Basetype(SP_proc), 0, 0, 0, Mflgs, -25 };
ssymnode nd27 = { N_DECL_ID, NCAST &r_strinsert, NLF, NNC("StrInsert"), T_BTPROC, HashByte,
			&nd26, Basetype(SP_proc), 0, 0, 0, Mflgs, -26 };
ssymnode nd28 = { N_DECL_ID, NCAST &r_substr, NLF, NNC("SubStr"), T_BTPROC, HashByte,
			&nd27, Basetype(SP_proc), 0, 0, 0, Mflgs, -27 };
ssymnode nd29 = { N_DECL_ID, NCAST &r_strlen, NLF, NNC("StrLen"), T_BTFUNC, HashByte,
			&nd28, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -28};
ssymnode ndlen = { N_DECL_ID, NCAST &r_strlen, NLF, NNC("Length"), T_BTFUNC,
	HashByte,&nd29, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -89};
ssymnode nd30 = { N_DECL_ID, NCAST &r_strscan, NLF, NNC("StrScan"), T_BTFUNC, HashByte,
			&ndlen, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -29};
ssymnode ndpos = { N_DECL_ID, NCAST &r_strscan, NLF, NNC("Pos"), T_BTFUNC, HashByte,
			&nd30, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -88};
ssymnode nd31 = { N_DECL_ID, NCAST &r_strsize, NLF, NNC("StrSize"), T_BTFUNC, HashByte,
			&ndpos, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -30};

#ifndef GEM
ssymnode nd32 = { N_DECL_ID, NCAST &r_sysproc, NLF, NNC("SysProc"), T_BPROC, HashByte,
			&nd31, Basetype(SP_proc), 0, 0, 0, Mflgs, -31 };
ssymnode nd33 = { N_DECL_ID, NCAST &r_sysfunc, NLF, NNC("SysFunc"), T_BFUNC, HashByte,
			&nd32, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -32};
ssymnode nd33a = { N_DECL_ID, NCAST &r_sysfunc, NLF, NNC("ISysCall"), T_BFUNC, HashByte,
			&nd33, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -62};
#else
#define	nd33a	nd31
#endif

extern struct list2node Bool_List;
ssymnode ndfalse = { N_DECL_ID, NCAST &Bool_List, NLF,  NNC("false"), T_BENUM,
	HashByte, &nd33a, Basetype(BT_boolean), 0, sizeof(char), 0,Mflgs, -33};
ssymnode ndtrue = { N_DECL_ID, NCAST &Bool_List, NLF, NNC("true"), T_BENUM,
	HashByte, &ndfalse, Basetype(BT_boolean), 1, sizeof(char),0,Mflgs, -34};
struct list2node Bool_List = { N_LIST, &BT_boolean, 2, 2, CAST(nodep )&ndfalse,
			CAST(nodep )&ndtrue };
node BT_boolean = { N_TYP_ENUM, NIL, NLF, CAST(nodep )&Bool_List };
ssymnode nd34 = { N_DECL_ID, CAST(nodep )&r_succ, NLF, NNC("succ"), T_BTFUNC, HashByte,
			&ndtrue, Basetype(SP_passtype), 0, sizeof(rint), 0,Mflgs, -35};
ssymnode nd34a = { N_DECL_ID, CAST(nodep )&r_pred, NLF, NNC("pred"), T_BTFUNC, HashByte,
			&nd34, Basetype(SP_passtype), 0, sizeof(rint), 0, Mflgs, -36 };
ssymnode nd34b = { N_DECL_ID, CAST(nodep )&r_ord, NLF, NNC("ord"), T_BFUNC, HashByte,
			&nd34a, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -37 };
ssymnode nd34c = { N_DECL_ID, CAST(nodep )&r_abs, NLF, NNC("abs"), T_BTFUNC, HashByte,
				&nd34b, Basetype(SP_passtype), 0, 0, 0, Mflgs, -38 };
ssymnode nd34d = { N_DECL_ID, CAST(nodep )&r_sqr, NLF, NNC("sqr"), T_BTFUNC, HashByte,
				&nd34c, Basetype(SP_passtype), 0, 0, 0, Mflgs, -39 };
ssymnode nd34e = { N_DECL_ID, NCAST &r_pack, NLF, NNC("pack"), T_BTPROC, HashByte,
				&nd34d, Basetype(SP_proc), 0, 0, 0, Mflgs, -63 };
ssymnode nd34f = { N_DECL_ID, NCAST &r_unpack, NLF, NNC("unpack"), T_BTPROC, HashByte,
				&nd34e, Basetype(SP_proc), 0, 0, 0, Mflgs, -64 };
ssymnode nd35 = { N_DECL_ID, CAST(nodep )&r_sin, NLF, NNC("sin"), T_BFUNC, HashByte,
			&nd34f, Basetype(BT_real), 0, sizeof(rfloat), 0, Mflgs, -40 };
ssymnode nd36 = { N_DECL_ID, CAST(nodep )&r_cos, NLF, NNC("cos"), T_BFUNC, HashByte,
			&nd35, Basetype(BT_real), 0, sizeof(rfloat),0, Mflgs, -41 };
ssymnode nd37 = { N_DECL_ID, CAST(nodep )&r_arctan, NLF, NNC("arctan"), T_BFUNC, HashByte,
			&nd36, Basetype(BT_real), 0, sizeof(rfloat),0, Mflgs, -42 };
ssymnode nd38 = { N_DECL_ID, CAST(nodep )&r_ln, NLF, NNC("ln"), T_BFUNC, HashByte,
			&nd37, Basetype(BT_real), 0, sizeof(rfloat),0, Mflgs, -43 };
ssymnode nd39 = { N_DECL_ID, CAST(nodep )&r_exp, NLF, NNC("exp"), T_BFUNC, HashByte,
			&nd38, Basetype(BT_real), 0, sizeof(rfloat),0, Mflgs, -44 };
ssymnode nd40 = { N_DECL_ID, CAST(nodep )&r_sqrt, NLF, NNC("sqrt"), T_BFUNC, HashByte,
			&nd39, CAST(nodep )&BT_real, 0, sizeof(rfloat),0, Mflgs, -45 };
ssymnode nd41 = { N_DECL_ID, CAST(nodep )&r_round, NLF, NNC("round"), T_BFUNC, HashByte,
			&nd40, CAST(nodep )&BT_integer, 0, sizeof(rint),0,Mflgs, -46 };
ssymnode nd42 = { N_DECL_ID, CAST(nodep )&r_chr, NLF, NNC("chr"), T_BFUNC, HashByte,
			&nd41, CAST(nodep )&BT_char, 0, sizeof(rint),0, Mflgs, -47 };
ssymnode nd43 = { N_DECL_ID, CAST(nodep )&r_odd, NLF, NNC("odd"), T_BFUNC, HashByte,
			&nd42, CAST(nodep )&BT_boolean, 0, sizeof(rint),0,Mflgs, -48 };
ssymnode nd44 = { N_DECL_ID, CAST(nodep )&r_reset, NLF, NNC("reset"), T_BTPROC, HashByte,
			&nd43, Basetype(SP_proc), 0, 0,0, Mflgs, -49 };
ssymnode nd45 = { N_DECL_ID, CAST(nodep )&r_rewrite, NLF, NNC("rewrite"), T_BTPROC, HashByte,
			&nd44, Basetype(SP_proc), 0, 0,0, Mflgs, -50 };
ssymnode nd46 = { N_DECL_ID, NCAST &r_append, NLF, NNC("append"), T_BTPROC, HashByte,
			&nd45, Basetype(SP_proc), 0, 0,0, Mflgs, -51 };
ssymnode nd47 = { N_DECL_ID, NCAST &r_update, NLF, NNC("update"), T_BTPROC, HashByte,
			&nd46, Basetype(SP_proc), 0, 0,0, Mflgs, -52 };
ssymnode nd48 = { N_DECL_ID, NCAST &r_setnext, NLF, NNC("setnext"), T_BPROC, HashByte,
			&nd47, Basetype(SP_proc), 0, 0,0, Mflgs, -53 };
ssymnode ndsetnext = { N_DECL_ID, NCAST &r_setnext, NLF, NNC("Seek"),
	T_BPROC, HashByte, &nd48, Basetype(SP_proc), 0, 0,0, Mflgs, -100 };
ssymnode nd49 = { N_DECL_ID, CAST(nodep )&r_random, NLF, NNC("random"), T_BFUNC, HashByte,
#ifdef TURBO
			&ndsetnext, Basetype(SP_passtype), 0, 0,0, Mflgs, -54 };
#else
			&ndsetnext, Basetype(BT_integer), 0, sizeof(rint),0, Mflgs, -54 };
#endif
ssymnode nd50 = { N_DECL_ID, NCAST &r_initrandom, NLF, NNC("initrandom"), T_BPROC, HashByte,
			&nd49, Basetype(SP_proc), 0, 0,0, Mflgs, -55 };
#ifndef GEM
ssymnode nd51 = { N_DECL_ID, NIL, NLF, NNC("Null"), T_BPROC, HashByte,
			&nd50, Basetype(SP_proc), 0, 0,0, Mflgs, -57 };
#else
#define nd51	nd50
#endif
static rout_node r_getch = { 0, do_getch, 0, 0, empty_list };

ssymnode nd52 = { N_DECL_ID, NCAST &r_getch, NLF, NNC("Get_Char"), T_BFUNC, HashByte,
			&nd51, Basetype(BT_char), 0, sizeof(rint), 0, Mflgs, -58 };
ssymnode nd53 = { N_DECL_ID, NCAST &r_charwait, NLF, NNC("Char_Waiting"), T_BFUNC, HashByte,
			&nd52, Basetype(BT_boolean), 0, sizeof(rint), 0, Mflgs, -59 };
symnode ndinput = { N_DECL_ID, NIL, NLF, NNC("input"), T_FILENAME, HashByte,
			&nd53, Basetype(BT_text), 0, sizeof(char), 0, Mflgs, -60 };
/* this one indicates the file is indeed output */
symnode ndoutput = { N_DECL_ID, NIL, NLF, NNC("output"), T_FILENAME, HashByte,
			&ndinput, Basetype(BT_text), 1, sizeof(char), 0, Mflgs, -61 };
ssymnode ndcursor = { N_DECL_ID, NCAST &r_cursorto, NLF, NNC("Cursor_To"), T_BPROC, HashByte,
			&ndoutput, Basetype(SP_proc), 0, 0, 0, Mflgs, -65 };

static rout_node r_sizeof = { 0, do_sizeof, 1, 1, l_variable };

ssymnode ndsizeof = { N_DECL_ID, NCAST &r_sizeof, NLF, NNC("SizeOf"), T_BTFUNC, HashByte,

			&ndcursor, CAST(nodep )&BT_integer, 0, sizeof(rint), 0,Mflgs, -66};
#ifdef GEM
static rout_node r_scrfunc = {0, do_scrfunc, 1, 2, l_inttext_param };
static rout_node r_set_attr = { 0, do_setattr, 1, 2, l_inttext_param };
#else
static rout_node r_scrfunc = {0, do_scrfunc, 1, 1, l_int_param };
static rout_node r_set_attr = { 0, do_setattr, 1, 1, l_int_param };
#endif
static bargs l_poin_param[] = { NULL, IBC &un_int_param, offsname,
					IBC &un_int_param, "\200Segment", NULL};
static bargs l_getp_param[] = { NULL, IBC &un_int_param, "\200Pointer #", NULL};

static rout_node r_point = { 0, do_point, 1, 2, l_poin_param };
static rout_node r_getp = {0, do_getptr, 1, 1, l_getp_param };
static bargs l_cfunc[] = {NULL, IBC &un_int_param, "\200Routine #", NULL };
static rout_node r_cifunc = {0, do_cintfunc, 1, 12, l_cfunc };
static rout_node r_cpfunc = {0, do_cptrfunc, 1, 12, l_cfunc };
static rout_node r_clfunc = {0, do_clongfunc, 1, 12, l_cfunc };
static rout_node r_cproc = {0, do_cproc, 1, 12, l_cfunc };

ssymnode ndgetptr = { N_DECL_ID, NCAST &r_getp, NLF, NNC("SysPointer"), T_BFUNC, HashByte,
	&ndsizeof, Basetype(BT_pointer), 0, sizeof(pointer), 0, Mflgs, -75 };
ssymnode ndcifunc = { N_DECL_ID, NCAST &r_cifunc, NLF, NNC("CIntFunc"), T_BTFUNC, HashByte,
	&ndgetptr, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -72 };
ssymnode ndcpfunc = { N_DECL_ID, NCAST &r_cpfunc, NLF, NNC("CPtrFunc"), T_BTFUNC, HashByte,
	&ndcifunc, Basetype(BT_pointer), 0, sizeof(pointer), 0, Mflgs, -73 };
ssymnode ndcproc = { N_DECL_ID, NCAST &r_cproc, NLF, NNC("CProc"), T_BTPROC,
	HashByte, &ndcpfunc, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -74 };
ssymnode ndclfunc = { N_DECL_ID, NCAST &r_clfunc, NLF, NNC("CLongFunc"), T_BTFUNC,
	HashByte, &ndcproc, Basetype(BT_pointer), 0, sizeof(pointer), 0, Mflgs, -122 };
ssymnode ndscrfunc = { N_DECL_ID, NCAST &r_scrfunc, NLF, NNC("ScrXY"), T_BFUNC, HashByte,
	&ndclfunc, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -67 };
ssymnode ndsetattr = { N_DECL_ID, NCAST &r_set_attr, NLF, NNC("Set_Attr"), T_BPROC, HashByte,
	&ndscrfunc, Basetype(BT_integer), 0, sizeof(rint), 0, Mflgs, -68 };
ssymnode ndpointer = { N_DECL_ID, NCAST &r_point, NLF, NNC("RawPointer"), T_BFUNC, HashByte,
	&ndsetattr, Basetype(BT_pointer), 0, sizeof(pointer), 0, Mflgs, -69 };
symnode ndlast = { N_DECL_ID, CAST(nodep )&r_trunc, NLF, NNC("trunc"), T_BFUNC, HashByte,
	&ndpointer, CAST(nodep )&BT_integer, 0, sizeof(rint),0, Mflgs, -56 };

workspace main_ws = {
	N_NULL,
	NIL,
	0, 0, 	/* flags are two bytes */
	NIL, /* to be stub */
	NIL,
	NIL,
	NIL,
	"main",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
	};

struct symbol_node null_undef = {
	N_DECL_ID,
	NIL,
	0, 0,	 /* flags are two bytes now */
	"Undefined",
	T_UNDEF,
	0,	/* hash not needed */
	(symptr)0,
	NIL,
	0,
	0,
	0
	};

struct standard_node master_table = {
	CAST(bits8)N_SYMBOL_TABLE,
	NIL,
	0, 0, /* now two flag bytes */
	CAST(nodep )&ndlast
	};

