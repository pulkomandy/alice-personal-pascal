/* Declarations of functions that return something other than an int */
extern curspos uc_nextpos();
extern curspos uc_prevpos();
extern curspos c_nextpos();
extern curspos c_prevpos();
extern curspos c_parent();
extern curspos c_child();
extern curspos c_lsib();
extern curspos c_rsib();
extern curspos makecp();/* makecp(par, childno) => a curspos */
extern curspos n_to_c();/* nodecp(n) => cp such that cu_node(cp) => n */

extern nodep n_child();
extern nodep sym_kid();			/* special children of nodep */
extern listp decl_kid();
extern nodep next_symtab();/* find next symbol table in block chain */
extern listp newlist();		/* create the first element in a list */
extern nodep find_true_parent();	/* find node class parent of element */
extern nodep l_hcalc();			/* line head calculator */
extern nodep tree();			/* routine to build tree elements */
extern nodep label_declare();		/* setup declared label */
extern nodep enum_declare();
extern nodep const_declare();
extern nodep const_init_declare();
extern nodep hide_block();
extern nodep type_declare();
extern nodep var_declare();
extern nodep abs_declare();
extern nodep param_declare();
extern nodep fparam_declare();		/* declared function parameter */
extern nodep pparam_declare();		/* declared procedure parameter */	
extern nodep symref();			/* produce a symbol reference */
extern char * sym_complete();
extern char *Safep();			/* safe string to printf */
extern symptr slookup();		/* the symbol for a name */	
extern symptr s_declare();		/* declares a symbol */
extern symptr sym_link();		/* links a declaration into symtab */
extern char *allocstring();		/* allocate a string in memory */
extern listp __exp_list();
extern char *strc_alloc();		/* allocate for constant strings */
extern char *strchr();			/* System III string find routine */
extern ClassNum ex_class();		/* find expected class */
extern curspos up_to_line();		/* get line level node for cursor */
extern nodep t_up_to_line();		/* tiny up to line */
extern nodep my_block();
extern curspos c_rightpos();		/* move to the right */
extern curspos realcp();		/* get a "real" cursor (non list *) */
extern bits8 *table_lookup();		/* search in byte tables */
extern curspos find_alice_cursor();	/* get cursor from row and column */
extern nodep look_multi();		/* look for multi-line nodep */
extern char *checkalloc();		/* checking malloc routine */
extern curspos ins_node();		/* insert a node in the tree */
extern nodep get_type();		/* get the type of expression */
extern nodep decl_typetree();		/* get the typetree of a decl */
extern nodep gexp_type();		/* type from a value nodep */
extern nodep find_realtype();		/* the type tree for this type */
extern nodep comp_type();		/* type for type comparisons */
extern nodep check_numeric();		/* check a type is numeric */
extern nodep ch_real_coerce();		/* handle int to real in subtrees */
extern nodep check_setop();		/* check for set operators */
extern nodep typecheck();		/* do type checking on a tree */
extern nodep hidebound();		/* make sure not inside a hide */
extern nodep l_lower();			/* lower a list */
extern char *tokname();			/* find token name */
extern nodep transmog();		/* alter the type of a node */
extern listp growlist();		/* grow list by one element */
extern nodep tcopy();			/* tree copy routine */
extern nodep copy_graft();		/* copy workspace onto stub */
extern nodep arg_indtype();		/* interpreter routine for type of
						formal param */
extern nodep c_typecheck();
extern nodep exprid();			/* parser routine for isolated ids */
extern pint eval_const();		/* evaluate const tree */
extern char *ordname();			/* string for ordinal value */
extern listp sized_list();		/* get a list of certain size */
extern listp list_range();		/* prepare the range in a list */
extern	nodep my_symtab();
extern nodep lookupFor();
extern	symptr	search_symtab();
extern nodep find_r_symtab();

#ifdef ES
extern char	*getESString();
extern char	*ESString();
extern treep	putESString();
extern char	*esdsstrcpy();
extern treep	dsesstrcpy();
extern treep	allocESString();
#endif
#ifdef HYBRID
extern char	*getESString();
extern char * allocESString();
extern char 	*getSegString();
#endif
#ifdef ES_TREE
extern nodep	make_stub();
#endif
#ifdef LARGE
extern pointer talloc();
#endif

#ifdef PARSER
extern nodep	pushSymtabStack();
#endif
