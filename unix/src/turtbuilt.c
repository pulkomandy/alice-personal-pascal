
/*
 *DESC: More built in symbols, to support turtle graphics
 */

static rout_node r_back = { 0, Fbackward, 1, 1, l_int_param };

ss ndt0 = {N_DECL_ID, RD(r_back), NLF, NNC("Back"), T_BPROC, HashByte,
	&nd4, Basetype(SP_proc), 0, 0, 0, Lmf, -260 };

static rout_node r_forwd = { 0, Fforward, 1, 1, l_int_param };

ss ndt1 = {N_DECL_ID, RD(r_forwd), NLF, NNC("Forwd"), T_BPROC, HashByte,
	&ndt0, Basetype(SP_proc), 0, 0, 0, Lmf, -261 };

static rout_node r_heading = { 0, Fgetheading, 0, 0, empty_list };

ss ndt2 = {N_DECL_ID, RD(r_heading), NLF, NNC("Heading"), T_BFUNC, HashByte,
	&ndt1, Basetype(BT_integer), 0, sizeof(rint), 0, Lmf, -262 };

static rout_node r_home = { 0, Fhome, 0, 0, empty_list };

ss ndt3 = {N_DECL_ID, RD(r_home), NLF, NNC("Home"), T_BPROC, HashByte,
	&ndt2, Basetype(SP_proc), 0, 0, 0, Lmf, -263 };

static rout_node r_pen = { 0, Fpen, 0, 0, empty_list };

ss ndt4 = {N_DECL_ID, RD(r_pen), NLF, NNC("PenDown"), T_BPROC, HashByte,
	&ndt3, Basetype(SP_proc), 0, 0, 0, Lmf, -264 };
ss ndt5 = {N_DECL_ID, RD(r_pen), NLF, NNC("PenUp"), T_BPROC, HashByte,
	&ndt4, Basetype(SP_proc), 0, 0, 0, Lmf, -265 };

static rout_node r_sethead = { 0, Fheading, 1, 1, l_int_param };

ss ndt6 = {N_DECL_ID, RD(r_sethead), NLF, NNC("SetHeading"), T_BPROC, HashByte,
	&ndt5, Basetype(SP_proc), 0, 0, 0, Lmf, -266 };

static rout_node r_setpenc = { 0, Fsetpencolor, 1, 1, l_int_param };

ss ndt7 = {N_DECL_ID, RD(r_setpenc), NLF, NNC("SetPenColor"), T_BPROC, HashByte,
	&ndt6, Basetype(SP_proc), 0, 0, 0, Lmf, -267 };

static rout_node r_setpos = { 0, Fsetposition, 2, 2, l_twoint };

ss ndt8 = {N_DECL_ID, RD(r_setpos), NLF, NNC("SetPosition"), T_BPROC, HashByte,
	&ndt7, Basetype(SP_proc), 0, 0, 0, Lmf, -268 };

static rout_node r_tleft = { 0, Fleft, 1, 1, l_int_param };

ss ndt9 = {N_DECL_ID, RD(r_tleft), NLF, NNC("TurnLeft"), T_BPROC, HashByte,
	&ndt8, Basetype(SP_proc), 0, 0, 0, Lmf, -269 };

static bargs l_setpal[] = { NULL, IBC &un_int_param, "\200Table #",
	IBC &un_int_param, "\200entry", NULL };
static rout_node r_setpal = { 0, do_setpal, 2, 2, l_setpal };

ss ndsetpal = {N_DECL_ID, RD(r_setpal), NLF, NNC("SetPalette"), T_BPROC,
	HashByte, &ndt9, Basetype(SP_proc), 0, 0, 0, Lmf, -273 };

#ifdef XORLINE
static rout_node r_hasturt = { 0, Fleft, 0, 0, empty_list };
ss ndtht = {N_DECL_ID, RD(r_hasturt), NLF, NNC("HideTurtle"), T_BPROC, HashByte,
	&ndsetpal, Basetype(SP_proc), 0, sizeof(rint), 0, Lmf, -271 };
ss ndtnt = {N_DECL_ID, RD(r_hasturt), NLF, NNC("ShowTurtle"), T_BPROC, HashByte,
	&ndtht, Basetype(SP_proc), 0, sizeof(rint), 0, Lmf, -272 };
#endif

static rout_node r_tright = { 0, Fright, 1, 1, l_int_param };

ss ndturtle= {N_DECL_ID, RD(r_tright), NLF, NNC("TurnRight"), T_BPROC, HashByte,
	&ndsetpal, Basetype(BT_integer), 0, sizeof(rint), 0, Lmf, -270 };
