/* all the structure reference macros and memory model based stuff */

#ifdef LOADABLE_TEMPLATES
#define NodeName(typ)	Node_Strings+Node_Names[typ]
#else
/* If we are using the small templates, there are no node names */
#ifndef SMALL_TPL
#define NodeName(typ)   Node_Names[typ]
#endif
#endif

#if defined(msdos) && defined(HYBRID)
# define ER(num,str)	num
#else
# define ER(num,str)	str
# define getERRstr(erm)	erm
#endif

	/* ntype macro works with any node */
#define ntype(NdP)		*( (bits8 *) NdP )
#define s_ntype(NdP,val)	*(bits8 *)(NdP) = (val)
#define node_flag(NdP)	((NdP)->flags1)
#define s_node_flag(NdP,val)	(NdP)->flags1 = (val)
#define or_node_flag(NdP,val)	(NdP)->flags1 |= (val)
#define clr_node_flag(NdP,val)	(NdP)->flags1 &= ~(val)
#define node_2flag(NdP)		((NdP)->flags2)
#define s_2flag(NdP,val)	(NdP)->flags2 = (val)
#define or_2flag(NdP,val)	((NdP)->flags2 |= (val))
#define clr_2flag(NdP,val)	((NdP)->flags2 &= ~(val))

#define node_parent(NdP)	((NdP)->n_parent)
#define s_node_parent(NdP,val) (NdP->n_parent = (val))
#define kid1(NdP)		((NdP)->n_kids[0])
#define kid2(NdP)		((NdP)->n_kids[1])
#define kid3(NdP)		((NdP)->n_kids[2])
#define kid4(NdP)		((NdP)->n_kids[3])
#define kid5(NdP)		((NdP)->n_kids[4])
#define kid6(NdP)		((NdP)->n_kids[5])
#define kid7(NdP)		((NdP)->n_kids[6])
#define kid8(NdP)		((NdP)->n_kids[7])
#define int_kid(num,NdP)	((int)(NdP)->n_kids[num])
#define str_kid(num,NdP)	(*(char **)&((NdP)->n_kids[num]))
#define s_str_kid(num,NdP,val)	(*(char **)&((NdP)->n_kids[num])) = (val)
#define fl_kid(NdP)		(*(rfloat *)&((NdP)->n_kids[1]))
#define s_fl_kid(NdP,val)	(*(rfloat *)&((NdP)->n_kids[1])) = val
#ifdef Stomp_Check
#define s_int_kid(num,NdP,val) s_node_kid(NdP,num,NCAST (val))
#define s_kid1(NdP,val)	s_node_kid(NdP,0,val)
#define s_kid2(NdP,val)	s_node_kid(NdP,1,val)
#define s_kid3(NdP,val)	s_node_kid(NdP,2,val)
#define s_kid4(NdP,val)	s_node_kid(NdP,3,val)
#define s_kid5(NdP,val)	s_node_kid(NdP,4,val)
#define s_kid6(NdP,val)	s_node_kid(NdP,5,val)
#define s_kid7(NdP,val)	s_node_kid(NdP,6,val)
#define s_kid8(NdP,val)	s_node_kid(NdP,7,val)
#else
#define s_int_kid(num,NdP,val) ((NdP)->n_kids[num] = (nodep )(val))
#define s_node_kid(NdP,knum,val)	(NdP)->n_kids[knum] = (val)
#define s_kid1(NdP,val)	(NdP)->n_kids[0]= (val)
#define s_kid2(NdP,val)	(NdP)->n_kids[1]= (val)
#define s_kid3(NdP,val)	(NdP)->n_kids[2]= (val)
#define s_kid4(NdP,val)	(NdP)->n_kids[3]= (val)
#define s_kid5(NdP,val)	(NdP)->n_kids[4]= (val)
#define s_kid6(NdP,val)	(NdP)->n_kids[5]= (val)
#define s_kid7(NdP,val)	(NdP)->n_kids[6]= (val)
#define s_kid8(NdP,val)	(NdP)->n_kids[7]= (val)
#endif
#define kids(NdP)		(NdP)->n_kids
#define node_kid(NdP,knum)	((NdP)->n_kids[knum])
#define kid1adr(NdP)		(&(NdP)->n_kids[0])
#define fpmake( off,seg )	&(((char far *)(((long)seg) << 16))[ (unsigned)(off) ])
#define tfarptr( off )		(off)

/* Generalized Parent */
#define tparent(listornode)	(((nodep )listornode)->n_parent)
#define s_tparent(listornode,val)	((nodep )listornode)->n_parent = (val)

#define list_parent(listp)	(listp)->n_parent
#define s_listparent(listp, val)	((listp)->n_parent = (val))
#define listcount(listp)	(listp)->flags1
#define listacount(listp)	(listp)->flags2
#define s_listcount(listp,val)	(listp)->flags1 = (val)
#define s_listacount(listp,val)	(listp)->flags2 = (val)
/* testing macros */

#define is_root(NdP)		(!(NdP)->n_parent)
#define is_not_root(NdP)		((NdP)->n_parent)
#define is_a_list(NdP)	(*((bits8 *)NdP)==N_LIST)
#define not_a_list(NdP)	(*((bits8 *)NdP) != N_LIST)

#define is_a_hide(NdP)	(*((bits8 *)NdP)==N_HIDE||*((bits8 *)NdP)==N_REVEAL)
#define is_a_symbol(NdP)	(*((bits8 *)NdP) == N_DECL_ID)
#define t_is_standard(type)	((type)<=LAST_STANDARD&&(type)>=FIRST_STANDARD)
#define is_standard(NdP)	(*((bits8 *)NdP)<=LAST_STANDARD&&*((bits8 *)NdP)>=FIRST_STANDARD)
#define is_a_stub(NdP)	(*((bits8 *)NdP) == N_STUB)
#define not_a_stub(NdP)	(*((bits8 *)NdP) != N_STUB)
#define is_stub_class(NdP,cl)	(*((bits8 *)NdP) == N_STUB && (int) NdP->n_kids[0] == cl)
#define is_descend(strin)	(strin[0] == '!' && strin[2]=='l')
#define is_a_subscope(NdP)	(*((bits8 *)NdP) == N_TYP_RECORD)
#define is_a_comment(NdP)	(*((bits8 *)NdP) == N_ST_COMMENT||*((bits8 *)NdP)==N_NOTDONE)
#define is_a_file(NdP)	(*((bits8 *)(NdP)) == N_TYP_FILE)
/* current form of a stub */
#define make_stub(clsno)	tree( N_STUB, NCAST (clsno) )

/* screen line structure */
#define line_headnode(scl)	l_hcalc( &(scl) )

/* child of N_ID that is symbol */
#define kid_id(NdP)		((symptr)(NdP->n_kids[0]))
#define s_kid_id(NdP,val)	(NdP->n_kids[0] = (nodep )val)
#define sym_name(sNdP)	(sNdP)->s_name
#define	sym_nhash(sNdP)	(sNdP)->s_namhash
#define sym_declaration(sNdP) (sNdP)->n_parent
#define sym_mflags(sNdP)	((sNdP)->s_mflags)
#define sym_type(sNdP)	((sNdP)->s_type)
#define sym_next(sNdP)	((sNdP)->s_next)
#define sym_offset(sNdP)	((int)(sNdP)->s_value)
#define sym_value(sNdP)	((sNdP)->s_value)
#define sym_ivalue(sNdP)	((int)(sNdP)->s_value)
#define sym_framesize(sNdP)	(sNdP)->s_value
#define sym_size(sNdP)	(sNdP)->s_size
#define sym_dtype(sNdP)	(sNdP)->s_decl_type
#define sym_scope(sNdP)	(sNdP)->s_scope
#define sym_saveid(sNdP)	(sNdP)->s_saveid

#define s_sym_name(sNdP, val)		((sNdP)->s_name = (val))
#define	s_sym_nhash(sNdP, val)	((sNdP)->s_namhash = (val))
#define s_sym_declaration(sNdP,val)	((sNdP)->n_parent = (val))
#define s_sym_mflags(sNdP, val)	((sNdP)->s_mflags = (val))
#define or_sym_mflags(sNdP, val)	((sNdP)->s_mflags |= (val))
#define clr_sym_mflags(sNdP, val)	((sNdP)->s_mflags &= ~(val))
#define s_sym_type(sNdP, val)	((sNdP)->s_type = (val))
#define s_sym_next(sNdP, val)	((sNdP)->s_next = (val))
#define s_sym_offset(sNdP,val)	(sNdP)->s_value = (val)
#define s_sym_dtype(sNdP,val)		(sNdP)->s_decl_type = (val)
#define s_sym_value(sNdP,val)		(sNdP)->s_value = (val)
#define s_sym_ivalue(sNdP,val)		(sNdP)->s_value = (pint)(val)
#define s_sym_framesize(sNdP,val)	(sNdP)->s_value = (val)
#define s_sym_size(sNdP,val)		(sNdP)->s_size = (val)
#define s_sym_scope(sNdP,val)		(sNdP)->s_scope = (val)
#define s_sym_saveid(sNdP,val)	(sNdP)->s_saveid = (val)

#ifndef PARSER
#define	namestring(x)		x
#endif

/* language description tables */

#define print_codes(typ)	node_table[typ]->printcodes
#define kid_codes(typ)		node_table[typ]->kidcodes

#ifdef LOADABLE_TEMPLATES

#define	valid_node(typ)		(typ >= 0 && typ < NodeCount && node_table[typ] != 0)
#define ntype_info(typ)		lt_node_flags[typ]
#define kid_count(typ)		lt_kid_count[typ]

#else

#ifdef SMALL_TPL
#define	valid_node(typ)		(typ >= 0 && typ < NodeCount)
#define ntype_info(typ)		lt_node_flags[typ]
#define kid_count(typ)		lt_kid_count[typ]
#define full_kids(typ)		lt_full_kids[typ]
#define	kid_class(typ,kid)	(lt_class_val[lt_K_Class[typ]+kid])
#else

#define	valid_node(typ)		(typ >= 0 && typ <= N_DECL_ID && node_table[typ] != 0)
#define ntype_info(typ)		node_table[typ]->ni_flags
#define kid_count(typ)		node_table[typ]->kidcount
#endif
#endif

#ifndef SMALL_TPL

#define kid_descend(typ)	node_table[typ]->ni_descend
#define full_kids(typ)		node_table[typ]->fullkids
#define kid_class(typ,kid)	(K_Classes[typ][kid])
#define node_actions(typ)	N_Actions[typ]
#define bind_strength(typ)	((typ>=FIRST_EXPRESSION)&&(typ<=LAST_EXPRESSION) ? expr_prec[typ-FIRST_EXPRESSION] : 0)

#endif

#define classtypes(ctyp)	Class_Types[ctyp]
#define classname(ctyp)		Class_Names[(ctyp)&63]

/* general macros */
#ifndef HYBRID
#define allocESString(str)	allocstring(str)
#define getESString(str)	str
#define getSegString(str,seg)	str
#endif

#if !defined(HYBRID) && !(defined(LARGE) && !defined(PARSER) && defined(msdos) )
# define	talloc(sz)	checkalloc(sz)
#define	tfree(p)	mfree(p)
#endif

#define fresh(type)		(type *)talloc( sizeof(type) )
/* copy screen line struct */
#ifdef STR_ASSIGN
#define scrl_copy(a1,a2)	a1 = a2
#define stru_assign(a1,a2)	a1 = a2
#else
#define scrl_copy(a1,a2)	blk_move(&(a1), &(a2),sizeof(struct scr_line))
#define stru_assign(a1,a2)	blk_move(&(a1), &(a2), sizeof(a1) )
#endif

/* Macros to manipulate cursor positions */

/* Get and set the value pointed to by a curspos */
/* to set the node pointed at by a curspos, call fn setchild(cp, val) */
#define cursparent	(cursor->n_parent)
#define cursnode	cursor

/* Get and set the fields of a curspos */
#define cu_parent(cp) (cp)->n_parent	/* faster than parent(c_to_n(cp)) */
#define s_cu_parent(cp, val) cp->n_parent = (val)

/* debugging prints */
#define PCURS		cursor
#define PCP(cp)		cp
#define Basetype(tname)	((nodep )&tname)
#define Tptr(tname)	&tname

/* This check could probably be improved using sbrk */
#if BIG && REALLY_BIG
#define saneptr(p)	(((long)p) > 0x1000 && ((long)p) < 0xff0000l)
#else
#define saneptr(p)	(((unsigned)p) > 0x1000 && ((unsigned)p) < 0xff00)
#endif
#define warning		message

#define STAR(x) *(x)
#define PTS(arg,fld,typ) arg->fld
#define BPTS ->

#define prnull(str)	((str) ? (str) : "(null)")
#define barglist(list,index,subel)	((bargs *)(list))[1+2*(index)+subel]

#ifdef QNX
# ifndef STACKCHECK
#  define stackcheck()
# else STACKCHECK
#  define stackcheck()	asm("	cmp	sp,stack_base"); asm( "	jb	<stackerr>" );
# endif STACKCHECK
#else QNX
# ifndef msdos
# define stackcheck()
# endif
#endif QNX

#ifdef UDEBUG
#if defined(QNX) || defined(DEMO) || (defined(LARGE) && defined(msdos))
# define check_undef(l,b) CU(l)
#else
#ifdef LARGE
#ifdef ATARI520ST
# define check_undef(l,b) CU(l)
#else
# define check_undef(l,b) {register unsigned CU;if((long)l>>16==SSeg&&(CU=(pointer)(l)-(pointer)exec_stack)<=max_stack_size&&!(*(pointer)(undef_bitmap+(CU>>3))&(1<<(CU&7))) )CUerror();}
#endif
#else
#endif LARGE
#endif QNX
#else UDEBUG
#  define check_undef(l,b)
#endif UDEBUG
