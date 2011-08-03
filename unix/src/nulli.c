erinterp(loc,restart, setstep) {}
run_prog(loc,setstep) {}
int count_suspend;
char *curr_suspend;
resume(){}
ex_immediate( loc, stepmode ) {}
do_write( args, type, value, width, precision ) {}
do_sizeof(){}
do_lnwrite( args, type, value, width, precision ) {}
do_read( args, type, ptr ) {}
do_lnread( args, type, ptr ) {}
do_trunc( argc, x ) {}
do_round( argc, x ) {}
do_sin( argc, x ) {}
do_cos( argc, x ) {}
do_arctan( argc, x ) {}
do_ln( argc, x ) {}
do_exp( argc, x ) {}
do_sqrt( argc, x ) {}
do_peek( argc, argv ) {}
do_chr( argc, argv ) {}
do_odd( argc, argv ) {}
do_abs( argc, argv ) {}
do_sqr( argc, argv ) {}
do_succ( argc, argv ) {}
do_pred( argc, argv ) {}
do_ord( argc, argv ) {}
do_poke() {}
do_strcat() {}
do_strdelete() {}
do_strinsert() {}
do_substr() {}
do_strlen() {}
do_strscan() {}
do_stsize() {}
do_pause() {}
do_sysproc() {}
do_superstep(){}
get_sblock(){ return 0; }
typecheck( tree, theclass ) {}
type_error() {}
c_typecheck() {}
int had_type_error;
do_new() {}
do_dispose() {}
do_eof() {}
do_eoln() {}
do_get() {}
do_put() {}
do_reset() {}
do_rewrite() {}
do_sysfunc() {}
do_initrandom() {}
do_random() {}
do_getch() {}
do_charwait() {}
do_page(){}
do_address(){}
do_cursorto(){}
decl_typetree(xdecl ){}
find_realtype(xstart ){}
typ_bound(xtypetree, lastflag ){}
calc_size( typetree, runtime ){}
gexp_type(xetree ){}
get_damn_exp_type(etree) {}
eval_const(xcontree ){}
fd_func(xthedecl, so_far, xtype_parent ){}
comp_type(xatype ){}
range_check(xtree, lower ){}
init_interp(){}
c_comp_decls( scope, loc ){}
comp_decls( scope, xblock ){}
ccheck_func( theconst, dummy, thecase, swtype ){}
size_adjust( size ){}
do_append() {}
do_update() {}
do_setnext() {}
tracepop() {}
pop_susp() {}
int	call_depth	= 0;
int	call_current	= 0;
clear_resume(){}
go_raw(){}
do_scrfunc(){}
acthelp( selected, menu ){}
token_help(){}
new_tk_menu( m ){}
m_token_add( token, m ){}
pas_help(){}
helpfile( hdir, subject ){}
int in_execws;
exec_ws(){}
do_getptr(argc, argv ){}
do_cintfunc( argc, argv ){}
do_cptrfunc( argc, argv ){}
do_cproc( argc, argv ){}
do_makeptr(){}
do_setattr(){}
do_pack() {}
do_point(){}
tst_cha() {}

int code_damaged = FALSE;
