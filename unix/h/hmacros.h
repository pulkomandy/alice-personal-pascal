
/* Hybrid model master macro file */

#define ER(num,str)	num
#define OLMSG 1
/* all the structure reference macros */
typedef unsigned char far * baseptype;

extern baseptype tr_far, tf_n_parent, tf_flags1, tf_flags2;
extern baseptype tf_n_k0, tf_n_k1, tf_n_k2, tf_n_k3, tf_n_k4,
		tf_n_k5, tf_n_k6, tf_n_k7;

#define tfb(np)		(tr_far + (unsigned)np)

	/* ntype macro works with any node */
#define ntype(NdP)		(*(tr_far + (unsigned)NdP))
#define s_ntype(NdP,val)	*(tr_far + (unsigned)NdP) = (val)
#define node_flag(NdP)		(*(((unsigned)NdP)+ tf_flags1))
#define s_node_flag(NdP,val)	*(((unsigned)NdP)+ tf_flags1) = (val)
#define or_node_flag(NdP,val)	*(((unsigned)NdP)+ tf_flags1) |= (val)
#define clr_node_flag(NdP,val)	*(((unsigned)NdP)+ tf_flags1) &= ~(val)
#define node_2flag(NdP)		(*(((unsigned)NdP)+ tf_flags2))
#define s_2flag(NdP,val)	*(((unsigned)NdP)+ tf_flags2) = (val)
#define or_2flag(NdP,val)	*(((unsigned)NdP)+ tf_flags2) |= (val)
#define clr_2flag(NdP,val)	*(((unsigned)NdP)+ tf_flags2) &= ~(val)

#define NP(arg)		(*(nodep far *)(arg))

#define node_parent(NdP)	NP (((unsigned)NdP)+ tf_n_parent)
#define s_node_parent(NdP,val)	NP((unsigned)(NdP)+ tf_n_parent) = (val)
#define kid1(NdP)		NP (((unsigned)NdP)+ tf_n_k0)
#define kid2(NdP)		NP (((unsigned)NdP)+ tf_n_k1)
#define kid3(NdP)		NP (((unsigned)NdP)+ tf_n_k2)
#define kid4(NdP)		NP (((unsigned)NdP)+ tf_n_k3)
#define kid5(NdP)		NP (((unsigned)NdP)+ tf_n_k4)
#define kid6(NdP)		NP (((unsigned)NdP)+ tf_n_k5)
#define kid7(NdP)		NP (((unsigned)NdP)+ tf_n_k6)
#define kid8(NdP)		NP (((unsigned)NdP)+ tf_n_k7)
#define int_kid(num,NdP)	((int far *)(((unsigned)NdP)+ tf_n_k0))[num]
#define str_kid(num,NdP)	((char * far *)(((unsigned)NdP)+ tf_n_k0))[num]
#define s_str_kid(num,NdP,val)	((char * far *)(((unsigned)NdP)+ tf_n_k0))[num] = val
#ifdef Stomp_Check
#define s_int_kid(num,NdP,val)	s_node_kid(NdP,num,NCAST (val))
#define s_kid1(NdP,val)		s_node_kid(NdP,0,val)
#define s_kid2(NdP,val)		s_node_kid(NdP,1,val)
#define s_kid3(NdP,val)		s_node_kid(NdP,2,val)
#define s_kid4(NdP,val)		s_node_kid(NdP,3,val)
#define s_kid5(NdP,val)		s_node_kid(NdP,4,val)
#define s_kid6(NdP,val)		s_node_kid(NdP,5,val)
#define s_kid7(NdP,val)		s_node_kid(NdP,6,val)
#define s_kid8(NdP,val)		s_node_kid(NdP,7,val)
#else
#define s_int_kid(num,NdP,val)	((int far *)(((unsigned)NdP)+ tf_n_k0))[num] = val
#define s_node_kid(NdP,num,val)	((nodep far *)(((unsigned)NdP)+ tf_n_k0))[num] = val
#define s_kid1(NdP,val)		NP(((unsigned)NdP)+ tf_n_k0)= (val)
#define s_kid2(NdP,val)		NP(((unsigned)NdP)+ tf_n_k1)= (val)
#define s_kid3(NdP,val)		NP(((unsigned)NdP)+ tf_n_k2)= (val)
#define s_kid4(NdP,val)		NP(((unsigned)NdP)+ tf_n_k3)= (val)
#define s_kid5(NdP,val)		NP(((unsigned)NdP)+ tf_n_k4)= (val)
#define s_kid6(NdP,val)		NP(((unsigned)NdP)+ tf_n_k5)= (val)
#define s_kid7(NdP,val)		NP(((unsigned)NdP)+ tf_n_k6)= (val)
#define s_kid8(NdP,val)		NP(((unsigned)NdP)+ tf_n_k7)= (val)
#endif
#define node_kid(NdP,num)	((nodep far *)(((unsigned)NdP)+ tf_n_k0))[num]
#define kid1adr(NdP)		((nodep far *)(((unsigned)NdP)+ tf_n_k0))

#define barglist(list,index,subel)	((bargs far *)(tr_far + (unsigned)list))[1+2*(index)+subel]
/* Generalized Parent */
#define tparent(listornode)	NP((unsigned)(listornode)+ tf_n_parent)
#define s_tparent(listornode,val)	NP((unsigned)(listornode)+ tf_n_parent) = (val)

#define list_parent		tparent
#define s_listparent		s_tparent
#define listcount		node_flag
#define listacount		node_2flag
#define s_listcount		s_node_flag
#define s_listacount		s_2flag
/* testing macros */

/* child of N_ID that is symbol */


#define kid_id(NdP)		(*(symptr far *)((unsigned)NdP+ tf_n_k0))
#define s_kid_id(NdP,val)	(*(symptr far *)((unsigned)NdP+ tf_n_k0)) = val

#define sfp(symp) ((realsymptr)(tr_far + (unsigned)symp))
#define sym_name(sNdP)		sfp(sNdP)->s_name
#define	sym_nhash(sNdP)		sfp(sNdP)->s_namhash
#define sym_declaration(sNdP)	 sfp(sNdP)->n_parent
#define sym_mflags(sNdP)	(sfp(sNdP)->s_mflags)
#define sym_type(sNdP)		(sfp(sNdP)->s_type)
#define sym_next(sNdP)		(sfp(sNdP)->s_next)
#define sym_offset(sNdP)	(sfp(sNdP)->s_value)
#define sym_value(sNdP)		(sfp(sNdP)->s_value)
#define sym_ivalue(sNdP)	(sfp(sNdP)->s_value)
#define sym_framesize(sNdP)	sfp(sNdP)->s_value
#define sym_size(sNdP)		sfp(sNdP)->s_size
#define sym_dtype(sNdP)		sfp(sNdP)->s_decl_type
#define sym_scope(sNdP)		sfp(sNdP)->s_scope
#define sym_saveid(sNdP)	sfp(sNdP)->s_saveid

#define s_sym_name(sNdP, val)		(sfp(sNdP)->s_name = (val))
#define	s_sym_nhash(sNdP, val)		(sfp(sNdP)->s_namhash = (val))
#define s_sym_declaration(sNdP,val)	(sfp(sNdP)->n_parent = (val))
#define s_sym_mflags(sNdP, val)		(sfp(sNdP)->s_mflags = (val))
#define or_sym_mflags(sNdP, val)	(sfp(sNdP)->s_mflags |= (val))
#define clr_sym_mflags(sNdP, val)	(sfp(sNdP)->s_mflags &= ~(val))
#define s_sym_type(sNdP, val)		(sfp(sNdP)->s_type = (val))
#define s_sym_next(sNdP, val)		(sfp(sNdP)->s_next = (val))
#define s_sym_offset(sNdP,val)		sfp(sNdP)->s_value = (val)
#define s_sym_dtype(sNdP,val)		sfp(sNdP)->s_decl_type = (val)
#define s_sym_value(sNdP,val)		sfp(sNdP)->s_value = (val)
#define s_sym_ivalue(sNdP,val)		sfp(sNdP)->s_value = (val)
#define s_sym_framesize(sNdP,val)	sfp(sNdP)->s_value = (val)
#define s_sym_size(sNdP,val)		sfp(sNdP)->s_size = (val)
#define s_sym_scope(sNdP,val)		sfp(sNdP)->s_scope = (val)
#define s_sym_saveid(sNdP,val)		sfp(sNdP)->s_saveid = (val)


extern unsigned int tr_segment;
#define fpmake( off,seg )	&(((char far *)(((long)seg) << 16))[ (unsigned)(off) ])
#define tfarptr( off )	((realnodep)(tr_far + (unsigned)(off)))

	/* ntype macro works with any node */
#define fl_kid(NdP)		(*(rfloat far *)((unsigned)(NdP)+tf_n_k1))
#define s_fl_kid(NdP,val)	(*(rfloat far *)((unsigned)(NdP)+tf_n_k1)) = val


#define is_root(NdP)		(!tparent(NdP))
#define is_not_root(NdP)		tparent(NdP)
#define is_a_list(NdP)	(ntype(NdP)==N_LIST)
#define not_a_list(NdP)	(ntype(NdP) != N_LIST)

#define is_a_hide(NdP)	(ntype(NdP)==N_HIDE||ntype(NdP)==N_REVEAL)
#define is_a_symbol(NdP)	(ntype(NdP) == N_DECL_ID)
#define t_is_standard(type)	((type)<=LAST_STANDARD&&(type)>=FIRST_STANDARD)
#define is_standard(NdP)	(ntype(NdP)<=LAST_STANDARD&&ntype(NdP)>=FIRST_STANDARD)
#define is_a_stub(NdP)	(ntype(NdP) == N_STUB)
#define not_a_stub(NdP)	(ntype(NdP) != N_STUB)
#define is_stub_class(NdP,cl)	(ntype(NdP) == N_STUB && int_kid(0,NdP) == cl)
#define is_descend(strin)	(strin[0] == '!' && strin[2]=='l')
#define is_a_subscope(NdP)	(ntype(NdP) == N_TYP_RECORD)
#define is_a_comment(NdP)	(ntype(NdP) == N_ST_COMMENT||ntype(NdP)==N_NOTDONE)
#define is_a_file(NdP)	(ntype(NdP) == N_TYP_FILE)

/* stub is created by a function */
/* screen line structure */
#define line_headnode(scl)	l_hcalc( &(scl) )

/* child of N_ID that is symbol */


#ifndef PARSER
#define	namestring(x)		x
#endif

/* language description tables */

#define print_codes(typ)	node_table[typ]->printcodes

#ifdef LOADABLE_TEMPLATES

  #define valid_node(typ) (typ >= 0 && typ < NodeCount && node_table[typ] != 0)
  #define ntype_info(typ)		lt_node_flags[typ]
  #define kid_count(typ)		lt_kid_count[typ]

#else

  #ifdef SMALL_TPL

    #define valid_node(typ)		(typ >= 0 && typ < NodeCount)
    #define ntype_info(typ)		lt_node_flags[typ]
    #define kid_count(typ)		lt_kid_count[typ]

    #define full_kids(typ)		lt_full_kids[typ]
    #define kid_class(typ,kid)		(lt_class_val[lt_K_Class[typ]+kid])

  #else

    #define valid_node(typ)		(typ >= 0 && typ <= N_DECL_ID && node_table[typ] != 0)
    #define ntype_info(typ)		node_table[typ]->ni_flags
    #define kid_count(typ)		node_table[typ]->kidcount

  #endif
#endif

#ifndef SMALL_TPL 

#define kid_codes(typ)		node_table[typ]->kidcodes
#define kid_descend(typ)	node_table[typ]->ni_descend
#define full_kids(typ)		node_table[typ]->fullkids
#define kid_class(typ,kid)	(K_Classes[typ][kid])
#define node_actions(typ)	N_Actions[typ]
#define classname(ctyp)		Class_Names[(ctyp)&63]

#endif

#ifndef ATARI520ST
#define ncerror error
#endif

#ifdef LOADABLE_TEMPLATES
#define NodeName(typ)	Node_Strings+Node_Names[typ]
#else
  #ifndef SMALL_TPL
	#define NodeName(typ)   Node_Names[typ]
  #else
	#define NodeName(typ)	("Node")
  #endif
#endif
#define classtypes(ctyp)	Class_Types[ctyp]
#define bind_strength(typ)	((typ>=FIRST_EXPRESSION)&&(typ<=LAST_EXPRESSION) ? expr_prec[typ-FIRST_EXPRESSION] : 0)

/* general macros */
#define fresh(type)		(type *)talloc( sizeof(type) )
/* copy screen line struct */
#ifdef STR_ASSIGN
#define scrl_copy(a1,a2)	a1 = a2
#define stru_assign(a1,a2)	a1 = a2
#else
#define scrl_copy(a1,a2)	blk_move(&(a1), &(a2),sizeof(struct scr_line))
#define stru_assign(a1,a2)	blk_move(&(a1), &(a2), sizeof(a1) )
#endif
#define tfree(np)	segFree(SEG_TREE, np, nodeSize(np))
/* Macros to manipulate cursor positions */

/* Get and set the value pointed to by a curspos */
/* to set the node pointed at by a curspos, call fn setchild(cp, val) */
#define cursparent	tparent(cursor)
#define cursnode	cursor

/* Get and set the fields of a curspos */
#define cu_parent(cp) tparent(cp)	/* faster than parent(c_to_n(cp)) */
#define s_cu_parent(cp, val) s_tparent(cp, val)

/* debugging prints */
#define PCURS		cursor
#define PCP(cp)		cp
#define Basetype(tname)	(nodep)(tname)
#define Tptr(tname)	tname

/* This check could probably be improved using sbrk */
#if BIG && REALLY_BIG
#define saneptr(p)	(((long)p) > 0x1000 && ((long)p) < 0xff0000l)
#else
#define saneptr(p)	(((unsigned)p) > 0x1000 && ((unsigned)p) < 0xff00)
#endif
#define warning		message

#define STAR(arg)	*(arg)
#define PTS(arg,fld,stype)	((stype)tfarptr(arg))->fld
#define BPTS ->

#define prnull(str)	((str) ? (str) : "(null)")
#define BC (bargs)

/* # define stackcheck() is a function */

#ifdef UDEBUG
# define check_undef(l,b) CU(l)
#else
# define check_undef(l,b)
#endif

