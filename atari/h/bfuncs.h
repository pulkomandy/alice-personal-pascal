#ifdef NOBFUNC
#define do_write	0
#define do_lnwrite	1
#define do_read		2
#define do_lnread	3
#define do_trunc	4
#define do_round	5
#define do_sin		6
#define do_cos		7
#define do_arctan	8
#define do_ln		9
#define do_exp		10
#define do_sqrt		11
#define do_peek		12
#define do_chr		13
#define do_odd		14
#define do_abs		15
#define do_sqr		16
#define do_succ		17
#define do_pred		18
#define do_ord		19
#define do_poke		20
#define	do_strcat	21
#define do_strdelete	22
#define do_strinsert	23
#define do_substr	24
#define do_strlen	25
#define do_strscan	26
#define do_stsize	27
#define do_pause	28
#define do_sysproc	29
#define do_new		30
#define do_dispose	31
#define do_eof		32
#define do_eoln		33
#define do_get		34
#define do_put		35
#define do_reset	36
#define do_rewrite	37
#define do_sysfunc	38
#define do_initrandom	39
#define do_random	40
#define do_getch	41
#define do_charwait	42
#define do_page		43
#define do_address	44
#define do_append	45
#define do_update	46
#define do_setnext	47
#define do_cursorto	48
#define do_sizeof	49
#define do_scrfunc	50
#define do_setattr	51
#define do_point	52
#define do_makeptr	53
#define do_cintfunc	54
#define do_cptrfunc	55
#define do_cproc	56
#define do_getptr	57
#define do_pack		58
#define	do_vartomem	59
#define	do_memtovar	60
#define do_delay	61
#define do_sound	62
#define do_halt		63
/* gap at 64 for hook-in */
#define do_insdel	65
#define do_clreol	66
#define do_assign	67
#define do_flpart	68
#define do_str		69
#define do_val		70
#define do_copy		71
#define do_intr		72
#define do_draw		73
#define	do_plot		74
#define do_window	75
#define	do_grmode	76
#define	do_textmode	77
#define	do_colour	78
#define do_ioresult	79
#define do_iocheck	80
#define do_clongfunc	81
#define do_close	82
#define do_memfunc	83
#define do_tplib	84
#define do_vid		85
#define do_upcase	86
#define do_exit		87
# ifdef PARSER
/* only for apin */
#define do_exit		87
#define do_nop		87
#define do_hilo		87
#define do_where	87
#define do_filefunc	87
#define do_rndomize	87
#define do_ptr		87
#define do_segoff	87
#define do_move		87
#define do_fillch	87
#define do_rename	87
#define do_dirfunc	87
#define do_getdir	87
#define do_markrel	87
#define do_blockio	87
#define do_longseek	87
#define do_parstr	87
#define do_parcnt	87
#define do_unpack	87
#define do_opendir	87
#define do_readdir	87
#define do_strtoar	87
#define do_artostr	87
#ifdef GEM
#define do_alert	87
#define do_ginit        87
#define do_gterm        87
#define do_event        87
#define do_gtext        87
#define do_style        87
#define do_gnewmenu     87
#define do_gaddmenu     87
#define do_gshowmenu    87
#define do_gmenucheck   87
#define do_gmenuenable  87
#define do_gmenutext    87
#define do_getparm      87
#define do_gcreate      87
#define do_SetGraph	87
#define do_SetCoord     87
#define do_gellipse     87
#define do_gseedfill    87
#define do_gsetcolor    87
#define do_gwinopt      87
#define do_gsprite      87
#define do_gtopwin      87
#define do_drmode       87
#define do_filesel      87
#define do_getprompt    87
#define do_fillpoly	87
#define do_mousetype    87
#define do_movewin	87
#define do_resizewin	87
#define do_setslider	87
#define do_setinfo	87
#define do_wheremouse	87
#define do_gsetpallette 87
#define do_gmouseon	87
#define do_gfillpat	87
#define do_vdicall	87
#define do_aescall	87
#define do_winvdi	87
#define do_gtopptr	87
#define do_gmenugettext	87
#define do_hidewin	87
#define do_quickwin	87
#define do_dosound	87
#define do_construct	87
# endif GEM
# endif PARSER

#else

extern int do_write();
extern int do_lnwrite();
extern int do_read();
extern int do_lnread();
extern int do_trunc();
extern int do_round();
extern int do_sin();
extern int do_cos();
extern int do_arctan();
extern int do_ln();
extern int do_exp();
extern int do_sqrt();
extern int do_peek();
extern int do_chr();
extern int do_odd();
extern int do_abs();
extern int do_sqr();
extern int do_succ();
extern int do_pred();
extern int do_ord();
extern int do_poke();
extern int do_strcat();
extern int do_strdelete();
extern int do_strinsert();
extern int do_substr();
extern int do_strlen();
extern int do_strscan();
extern int do_stsize();
extern int do_pause();
extern int do_sysproc();
extern int do_new();
extern int do_dispose();
extern int do_eof();
extern int do_eoln();
extern int do_get();
extern int do_put();
extern int do_reset();
extern int do_rewrite();
extern int do_sysfunc();
extern int do_initrandom();
extern int do_random();
extern int do_getch();
extern int do_charwait();
extern int do_page();
extern int do_address();
extern int do_append();
extern int do_update();
extern int do_setnext();
extern int do_cursorto();
extern int do_sizeof();
extern int do_scrfunc();
extern int do_setattr();
extern int do_point();
extern int do_makeptr();
extern int do_cintfunc();
extern int do_cptrfunc();
extern int do_cproc();
extern int do_getptr();
extern int do_pack();
extern int do_vartomem();
extern int do_memtovar();
extern int do_delay();
extern int do_sound();
extern int do_halt();
extern int do_clreol();
extern int do_insdel();
extern int do_assign();
extern int do_flpart();
extern int do_str();
extern int do_val();
extern int do_copy();
extern int do_intr();
extern int do_draw();
extern int do_plot();
extern int do_window();
extern int do_grmode();
extern int do_textmode();
extern int do_colour();
extern int do_ioresult();
extern int do_iocheck();
extern int do_clongfunc();
extern int do_close();
extern int do_memfunc();
extern int do_tplib();
extern int do_vid();
extern int do_upcase();
extern int do_exit();
# ifdef FULL
extern int do_nop();
extern int do_hilo();
extern int do_where();
extern int do_filefunc();
extern int do_rndomize();
extern int do_ptr();
extern int do_segoff();
extern int do_move();
extern int do_fillch();
extern int do_rename();
extern int do_dirfunc();
extern int do_getdir();
extern int do_markrel();
extern int do_blockio();
extern int do_longseek();
extern int do_parstr();
extern int do_parcnt();
extern int do_unpack();
extern int do_opendir();
extern int do_readdir();
extern int do_strtoar();
extern int do_artostr();
#ifdef GEM
	extern do_alert(), do_ginit(), do_gterm(), do_event();
	extern do_gtext(), do_style(), do_gnewmenu(), do_gaddmenu();
	extern do_gshowmenu(), do_gmenucheck(), do_gmenuenable();
	extern do_gmenutext(), do_getparm();
	extern do_gcreate(), do_SetGraph();
	extern do_SetCoord(), do_gellipse(), do_gseedfill();
	extern do_gsetcolor(), do_gwinopt();
	extern do_gsprite(), do_gtopwin(), do_drmode(), do_filesel();
	extern do_getprompt(), do_fillpoly();
	extern do_mousetype(), do_movewin(), do_resizewin(), do_setslider();
	extern do_setinfo(), do_wheremouse();
	extern do_gsetpallette();
	extern do_gmouseon(), do_gfillpat(), do_vdicall(), do_aescall();
	extern do_winvdi(), do_gtopptr(), do_gmenugettext();
	extern do_hidewin(), do_quickwin();
	extern do_dosound();
	extern do_construct();
# endif GEM
# endif FULL

#endif NOBFUNC

#if defined(HYBRID) && defined(ES_TREE)
extern funcptr builtins[];

#define bfptr(s)  (*builtins[((realrout_np)tfarptr(tparent(s)))->b_func])
#else

# ifdef NOBFUNC
# define bfptr(sym)	(*builtins[((rout_node *)(sym BPTS n_parent)) BPTS b_func])
extern funcptr builtins[];		/* array of builtin routines */
# else
# define bfptr(sym)	(*((rout_node *)(sym BPTS n_parent))->b_func)
# endif

#endif HYBRID
