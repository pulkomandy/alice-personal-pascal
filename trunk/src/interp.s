/	module name interp
	.comm	sp_, 4
	.comm	st_display_, 4

	.prvd

	.globl loc_stack_
loc_stack_:
	.long	0
	.globl undef_bitmap_
undef_bitmap_:
	.long	0
	.comm	ex_stack_, 4
	.globl step_delay_
step_delay_:
	.long	0
	.globl must_stop_
must_stop_:
	.word	0

	.bssd

arg_list_:
	.blkb	0x4
built_index_:
	.blkb	0x2
	.comm	cur_state_, 2
	.comm	new_loc_, 4
	.comm	within_hide_, 2
el_flags_:
	.blkb	0x2
intstar_:
	.blkb	0x4
temp2_:
	.blkb	0x2
untemp_:
	.blkb	0x2
fltemp_:
	.blkb	0x8
flstar_:
	.blkb	0x4
btemp1_:
	.blkb	0x1
thescope_:
	.blkb	0x2
nkid_:
	.blkb	0x2
nod_flags_:
	.blkb	0x2
tempptr_:
	.blkb	0x4
resptr_:
	.blkb	0x4
type_ptr_:
	.blkb	0x4
tempnode_:
	.blkb	0x4
dummy_:
	.blkb	0x4
notfirst_:
	.blkb	0x1
	.comm	step_flag_, 2
	.comm	ex_loc_, 4

	.prvd

WasBreak_:
	.byte	0

	.shri

	.globl interp_
interp_:

	.link

L10008:
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.byte	0

	.shri

	link	a6, $-30
	movem.l	$14472, (a7)

	.shrd

	.even
L10003:
	.long	L48
	.long	L57
	.long	L56
	.long	L60
	.long	L62
	.long	L63
	.long	L14
	.long	L55
	.long	L14
	.long	L14
	.long	L14
	.long	L14
	.long	L14
	.long	L14
	.long	L14
	.long	L14
	.long	L53
	.long	L53
L10018:
	.long	L121
	.long	L121
	.long	L119
	.long	L123
	.long	L122
	.long	L124
L10030:
	.long	L129
	.long	L129
	.long	L127
	.long	L131
	.long	L130
	.long	L132
L10038:
	.long	L142
	.long	L142
	.long	L14
	.long	L143
	.long	L14
	.long	L144
L10046:
	.long	L148
	.long	L148
	.long	L20272
	.long	L150
	.long	L149
	.long	L151
L10054:
	.long	L156
	.long	L156
	.long	L159
	.long	L157
	.long	L159
	.long	L158
L10062:
	.long	L163
	.long	L163
	.long	L20272
	.long	L165
	.long	L164
	.long	L166
L10077:
	.long	L248
	.long	L20191
	.long	L256
	.long	L233
	.long	L256
	.long	L256
	.long	L255
	.long	L236
	.long	L254
	.long	L240
	.long	L241
	.long	L238
	.long	L256
	.long	L238
	.long	L242
	.long	L256
	.long	L256
	.long	L256
	.long	L256
	.long	L256
	.long	L256
	.long	L256
	.long	L248
	.long	L235
	.long	L235
	.long	L20191
	.long	L14
	.long	L14
L10078:
	.long	L99
	.long	L71
	.long	L17
	.long	L15
	.long	L87
	.long	L97
	.long	L35
	.long	L189
	.long	L192
	.long	L187
	.long	L20218
	.long	L259
	.long	L32
	.long	L32
	.long	L83
	.long	L76
	.long	L74
	.long	L77
	.long	L22
	.long	L41
	.long	L34
	.long	L14
	.long	L32
	.long	L30
	.long	L31
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L32
	.long	L14
	.long	L14
	.long	L98
	.long	L14
	.long	L29
	.long	L20163
	.long	L32
	.long	L32
	.long	L45
	.long	L32
	.long	L19
	.long	L66
	.long	L68
	.long	L70
	.long	L70
	.long	L186
	.long	L191
	.long	L16
	.long	L18
	.long	L14
	.long	L14
	.long	L32
	.long	L32
	.long	L32
	.long	L114
	.long	L14
	.long	L172
	.long	L32
	.long	L32
	.long	L180
	.long	L181
	.long	L182
	.long	L183
	.long	L194
	.long	L204
	.long	L193
	.long	L231
	.long	L100
	.long	L14
	.long	L115
	.long	L136
	.long	L177
	.long	L102
	.long	L14
	.long	L174
	.long	L208
	.long	L216
	.long	L222
	.long	L224
	.long	L107
	.long	L109
	.long	L111
	.long	L116
	.long	L117
	.long	L125
	.long	L139
	.long	L145
	.long	L153
	.long	L160
	.long	L138
	.long	L223
	.long	L14
	.long	L32
	.long	L20218
	.long	L168
	.long	L133
	.long	L231
	.long	L32
	.long	L257

	.shri

	move	16(a6), step_flag_
	move.l	8(a6), new_loc_
	movea.l	12(a6), a5
	jsr	clear_break_
L3:
	tst.l	new_loc_
	beq	L4
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move.l	a5, (a0)
L6:
	tst.l	new_loc_
	beq	L11
	movea.l	new_loc_, a5
	cmpi.b	$101, (a5)
	beq.s	L7
	clr	d0
	move.b	6(a5), d0
	andi	$8, d0
	beq.s	L7
	move.l	a5, ex_loc_
	moveq	$59, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L7:
	clr	d0
	move.b	(a5), d0
	move	d0, cur_state_
	move	cur_state_, d0
	ext.l	d0
	lsl.l	$1, d0
	movea.l	d0, a0
	adda.l	lt_node_flags_, a0
	move	(a0), d0
	move	d0, el_flags_
	andi	$96, d0
	beq.s	L8
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move	cur_state_, (a0)
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move.l	a5, (a0)
	move	el_flags_, d0
	andi	$64, d0
	beq.s	L9
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-12, d0
	move	d0, (a0)
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move.l	a5, (a0)
L9:
	move.l	8(a5), new_loc_
	bra	L6
L8:
	clr.l	new_loc_
	bra	L6
L11:
	move.l	a5, ex_loc_
	move	cur_state_, d0
	addi	$22, d0
	cmpi	$123, d0
	bhi	L32
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10078, a0
L20139:
	movea.l	(a0), a0
	jmp	(a0)
	bra	L14
L15:
	clr	-(a7)
	jsr	prep_stacks_
L20021:
	addq	$2, a7
	bra	L14
L16:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	16(a5), (a0)
	move.l	tempptr_, 16(a5)
	move.l	12(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-20, d0
L20045:
	move	d0, (a0)
	bra	L14
L17:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, 16(a5)
	bra	L14
L18:
	moveq	$1, d0
	move	d0, -(a7)
	move.l	12(a5), -(a7)
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), -(a7)
	jsr	find_case_
L20056:
	addq	$8, a7
	bra	L14
L19:
	move.l	12(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-1, d0
	bra.s	L20045
L22:
	movea.l	ex_stack_, a0
	addq	$1, (a0)
	move	(a0), d0
	move	d0, nkid_
	move	nkid_, d1
	subq	$1, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	12(a5), a0
	move.l	8(a0), d0
	move.l	d0, tempnode_
	move.l	tempnode_, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, type_ptr_
	cmpi	$1, nkid_
	bne.s	L23
	movea.l	type_ptr_, a0
	cmpi.b	$23, (a0)
	bne.s	L23
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	movea.l	ex_stack_, a0
	move.l	d0, 4(a0)
	bra.s	L24
L23:
	movea.l	ex_stack_, a0
	move.l	4(a0), -(a7)
	move.l	tempnode_, -(a7)
	move.l	type_ptr_, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	$dummy_, -(a7)
	move.l	a5, -(a7)
	jsr	get_routine_
	addq	$8, a7
	movea.l	d0, a1
	movea.l	2(a1), a0
	movea.l	2(a0), a0
	jsr	(a0)
	lea	14(a7), a7
L24:
	movea.l	12(a5), a0
	clr	d1
	move.b	6(a0), d1
	move	nkid_, d0
	cmp	d1, d0
	bhs.s	L25
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-4, d0
	move	d0, (a0)
	move	nkid_, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	12(a5), a0
L20161:
	move.l	8(a0), d0
L20071:
	move.l	d0, new_loc_
	bra	L14
L25:
	movea.l	ex_stack_, a0
	move.l	4(a0), -(a7)
	clr.l	-(a7)
	clr.l	-(a7)
	clr	-(a7)
	move.l	$dummy_, -(a7)
	move.l	a5, -(a7)
	jsr	get_routine_
	addq	$8, a7
	movea.l	d0, a1
	movea.l	2(a1), a0
	movea.l	2(a0), a0
	jsr	(a0)
	lea	14(a7), a7
	bra	L20118
L29:
	jsr	do_a_goto_
	bra	L14
L30:
	move.l	a5, -(a7)
	moveq	$55, d0
	move	d0, -(a7)
	jsr	run_error_
L20073:
	addq	$6, a7
	bra	L14
L31:
	clr.l	st_display_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move.l	sp_, (a0)
	clr	-(a7)
	move.l	st_display_, -(a7)
	move.l	a5, -(a7)
	clr.l	-(a7)
	move.l	a5, -(a7)
	jsr	act_size_
	addq	$4, a7
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	jsr	make_frame_
	lea	18(a7), a7
	move.l	24(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-19, d0
	bra	L20045
L32:
	move	cur_state_, -(a7)
	moveq	$56, d0
	move	d0, -(a7)
	jsr	run_error_
L20077:
	addq	$4, a7
	bra	L14
L20163:
	move.l	a5, -(a7)
	jsr	start_call_
	bra.s	L20077
L34:
	move.l	a5, -(a7)
	jsr	do_1call_
	bra.s	L20077
L35:
	movea.l	st_display_, a0
	move.b	(a0), d0
	ext	d0
	move	d0, thescope_
	move	thescope_, d1
	neg	d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	st_display_, a0
	move.l	(a0), d0
	move.l	d0, sp_
	subq	$1, call_depth_
	move.l	st_display_, -(a7)
	jsr	get_fpointers_
	addq	$4, a7
	movea.l	d0, a0
	move	10(a0), d0
	move	d0, within_hide_
	move.l	st_display_, -(a7)
	jsr	up_frame_
	addq	$4, a7
	move.l	d0, st_display_
	move.l	$dummy_, -(a7)
	move.l	a5, -(a7)
	jsr	get_routine_
	addq	$8, a7
	movea.l	d0, a4
	move.l	sp_, -(a7)
	move.l	free_top_, -(a7)
	clr	-(a7)
	jsr	closeall_
	lea	10(a7), a7
	cmpi.b	$16, 12(a4)
	bne	L20078
	move.l	18(a4), -4(a6)
	movea.l	-4(a6), a0
	cmpi.b	$20, (a0)
	bne	L37
	movea.l	-4(a6), a0
	move	14(a0), d0
	addq	$2, d0
	move	d0, -(a7)
	jsr	size_adjust_
	addq	$2, a7
	move	d0, -6(a6)
	movea.l	sp_, a0
	move.b	(a0), d0
	ext	d0
	move	d0, -8(a6)
	move	-8(a6), d0
	addq	$1, d0
	move	d0, -(a7)
	clr	-(a7)
	move.l	sp_, d0
	move.l	d0, -(a7)
	move	$1, d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, -(a7)
	jsr	do_undef_
	addq	$8, a7
	move	-8(a6), d0
	andi	$1, d0
	add	-8(a6), d0
	move	d0, -8(a6)
	move	-6(a6), d0
	addi	$-2, d0
	sub	-8(a6), d0
	move	d0, -10(a6)
	beq.s	L38
	move	-8(a6), d0
	addq	$2, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	move.l	sp_, d0
	move.l	d0, -(a7)
	move	-10(a6), d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, -(a7)
	jsr	blk_move_
	lea	10(a7), a7
	movea.l	sp_, a0
	adda	-10(a6), a0
	move.l	a0, sp_
L38:
	subq.l	$4, sp_
	movea.l	sp_, a0
	clr.l	(a0)
	move	-8(a6), d0
	addq	$6, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	jsr	do_undef_
	addq	$8, a7
L37:
	cmpi	$1, 26(a4)
	bne.s	L39
	move.l	sp_, d0
	move.l	d0, -(a7)
	move	$1, d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, -(a7)
	bra.s	L20002
L39:
	move.l	sp_, -(a7)
L20002:
	jsr	CU_
	addq	$4, a7
L20078:
	clr	cur_state_
	bra	L14
L41:
	movea.l	ex_stack_, a0
	addq	$1, (a0)
	move	(a0), d0
	move	d0, nkid_
	move.l	$dummy_, -(a7)
	move.l	a5, -(a7)
	jsr	get_routine_
	addq	$8, a7
	movea.l	d0, a4
	movea.l	12(a5), a0
	clr	d1
	move.b	6(a0), d1
	move	nkid_, d0
	cmp	d1, d0
	bhs.s	L42
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-3, d0
	move	d0, (a0)
	move	nkid_, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	12(a5), a0
	move.l	8(a0), d0
	move.l	d0, new_loc_
	cmpi.b	$19, 12(a4)
	bhs	L14
	subq.l	$4, sp_
	movea.l	sp_, a3
	move.l	new_loc_, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, (a3)
	bra	L14
L42:
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move	(a0), d0
	movea.l	ex_stack_, a0
	move.l	(a0), d0
	move.l	d0, -4(a6)
	move.l	a4, -(a7)
	move.l	-4(a6), -(a7)
	move	nkid_, -(a7)
	movea.l	2(a4), a0
	movea.l	2(a0), a0
	jsr	(a0)
	lea	10(a7), a7
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move.l	(a0), d0
	move.l	d0, sp_
	bra	L20078
L45:
	move	18(a5), d0
	move	d0, d7
	tst	d7
	blt	L46
	move	d7, d0
	andi	$31, d0
	subq	$1, d0
	cmpi	$17, d0
	bhi	L14
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10003, a0
	bra	L20139
L48:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, temp2_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	moveq	$1, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	move	temp2_, d0
	movea.l	tempptr_, a0
	move.b	d0, (a0)
	move	d7, d0
	andi	$32, d0
	beq.s	L50
	move	temp2_, -(a7)
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	sr_check_
	addq	$6, a7
L50:
	move	step_flag_, d0
	andi	$16, d0
	beq	L14
	tst	within_hide_
	bne	L14
	move	temp2_, -(a7)
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	ordname_
	addq	$6, a7
	move.l	d0, -(a7)
	move	$433, -(a7)
L20167:
	jsr	getLDSstr_
	addq	$2, a7
	move.l	d0, -(a7)
	jsr	nonum_message_
	bra	L20056
L53:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, temp2_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, untemp_
	move	temp2_, d0
	andi	$255, d0
	move	d0, -(a7)
	move	untemp_, -(a7)
	jsr	outp_
	addq	$4, a7
	cmpi	$18, d7
	bne	L14
	move	temp2_, d0
	lsr	$8, d0
	move	d0, -(a7)
	move	untemp_, d0
	addq	$1, d0
	move	d0, -(a7)
	jsr	outp_
	bra	L20077
L55:
	movea.l	sp_, a0
	addq.l	$1, (a0)
L56:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, resptr_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	moveq	$4, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	movea.l	tempptr_, a0
	move.l	resptr_, (a0)
	bra	L14
L57:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, temp2_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	moveq	$2, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	movea.l	tempptr_, a0
	move	temp2_, (a0)
	move	d7, d0
	andi	$32, d0
	beq.s	L58
	move	temp2_, -(a7)
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	sr_check_
	addq	$6, a7
L58:
	move	step_flag_, d0
	andi	$16, d0
	beq	L14
	tst	within_hide_
	bne	L14
	move	temp2_, -(a7)
	move	$434, -(a7)
	jsr	getLDSstr_
	addq	$2, a7
	move.l	d0, -(a7)
	jsr	nonum_message_
	bra	L20073
L60:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	moveq	$8, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	move.l	fltemp_, d0
	move.l	fltemp_+4, d1
	movea.l	tempptr_, a0
	move.l	d0, (a0)
	move.l	d1, 4(a0)
	move	step_flag_, d0
	andi	$16, d0
	beq	L14
	tst	within_hide_
	bne	L14
	move.l	fltemp_+4, -(a7)
	move.l	fltemp_, -(a7)
	move	$435, -(a7)
	jsr	getLDSstr_
	addq	$2, a7
	move.l	d0, -(a7)
	jsr	nonum_message_
L20079:
	lea	12(a7), a7
	bra	L14
L62:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, resptr_
	move.l	a5, -(a7)
	move.l	resptr_, -(a7)
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), -(a7)
	jsr	set_assign_
	bra.s	L20079
L63:
	jsr	str_spop_
	move.l	d0, resptr_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	get_stsize_
	addq	$4, a7
	move	d0, -(a7)
	move.l	resptr_, -(a7)
	move.l	tempptr_, -(a7)
	jsr	str_assign_
	lea	10(a7), a7
	move	step_flag_, d0
	andi	$16, d0
	beq	L14
	tst	within_hide_
	bne	L14
	move.l	tempptr_, d0
	move.l	d0, -(a7)
	move	$1, d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, -(a7)
	move	$436, -(a7)
	bra	L20167
L46:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, resptr_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	move	d7, d0
	neg	d0
	move	d0, d7
	move	d7, -(a7)
	move.l	resptr_, -(a7)
	move.l	tempptr_, -(a7)
	jsr	copy_mask_
	lea	10(a7), a7
	move	d7, -(a7)
	move.l	resptr_, -(a7)
	move.l	tempptr_, -(a7)
	jsr	blk_move_
L20168:
	lea	10(a7), a7
	bra	L14
L66:
	movea.l	sp_, a0
	addq.l	$4, sp_
	tst	(a0)
	beq	L14
L20218:
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-1, d0
L20145:
	move	d0, (a0)
	move.l	12(a5), new_loc_
	bra	L14
L68:
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-1, d0
	move	d0, (a0)
	movea.l	sp_, a0
	addq.l	$4, sp_
	tst	(a0)
	beq.s	L10004
	move.l	12(a5), d0
	bra	L20071
L10004:
	move.l	16(a5), d0
	bra	L20071
L70:
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-21, d0
L20170:
	move	d0, (a0)
	move.l	16(a5), new_loc_
	bra	L14
L71:
	movea.l	sp_, a0
	move	4(a0), d0
	move	d0, d7
	move	d7, -(a7)
	move.l	8(a5), -(a7)
	jsr	sr_ccheck_
	addq	$6, a7
	movea.l	sp_, a0
	move	(a0), -(a7)
	move.l	8(a5), -(a7)
	jsr	sr_ccheck_
	addq	$6, a7
	movea.l	8(a5), a1
	movea.l	8(a1), a0
	move	26(a0), d0
	cmpi	$1, d0
	bne.s	L72
	movea.l	sp_, a0
	subq.l	$1, 8(a0)
L72:
	movea.l	sp_, a0
	move.l	8(a0), d0
	move.l	d0, intstar_
	movea.l	intstar_, a0
	move	d7, (a0)
	moveq	$2, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	intstar_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	clr.b	notfirst_
	cmpi.b	$46, (a5)
	bne	L76
	bra.s	L74
L77:
	movea.l	sp_, a0
	move.l	8(a0), d0
	move.l	d0, intstar_
	movea.l	intstar_, a0
	move	(a0), d0
	cmpi	$32767, d0
	beq.s	L79
	movea.l	intstar_, a0
	addq	$1, (a0)
L74:
	movea.l	intstar_, a1
	movea.l	sp_, a0
	move	(a0), d0
	cmp	(a1), d0
	bge.s	L81
L79:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	moveq	$2, d0
	move	d0, -(a7)
	moveq	$2, d0
	move	d0, -(a7)
	move.l	tempptr_, -(a7)
L20171:
	jsr	do_undef_
	bra	L20056
L81:
	tst	most_trlocs_
	beq.s	L82
	tst	within_trace_
	bne.s	L82
	tst.b	notfirst_
	beq.s	L82
	moveq	$2, d0
	move	d0, -(a7)
	move.l	intstar_, -(a7)
	jsr	scan_trace_
	addq	$6, a7
L82:
	moveq	$1, d0
	move.b	d0, notfirst_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-5, d0
L20081:
	move	d0, (a0)
	move.l	20(a5), new_loc_
	bra	L14
L83:
	movea.l	sp_, a0
	move.l	8(a0), d0
	move.l	d0, intstar_
	movea.l	intstar_, a0
	move	(a0), d0
	cmpi	$-32768, d0
	beq	L79
	movea.l	intstar_, a0
	subq	$1, (a0)
L76:
	movea.l	intstar_, a1
	movea.l	sp_, a0
	move	(a0), d0
	cmp	(a1), d0
	bgt	L79
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-8, d0
	bra.s	L20081
L87:
	movea.l	ex_stack_, a0
	addq	$1, (a0)
	move	(a0), d0
	move	d0, nkid_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	tst	d7
	blt.s	L10006
	cmpi	$255, d7
	ble.s	L88
L10006:
	move	d7, -(a7)
	moveq	$57, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$4, a7
L88:
	move.l	8(a5), arg_list_
	move	nkid_, d0
	subq	$1, d0
	ext.l	d0
	lsl.l	$2, d0
	movea.l	d0, a1
	adda.l	arg_list_, a1
	movea.l	8(a1), a0
	cmpi.b	$94, (a0)
	bne.s	L89
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, temp2_
	tst	temp2_
	blt.s	L10007
	cmpi	$255, temp2_
	ble.s	L93
L10007:
	move	temp2_, -(a7)
	moveq	$57, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$4, a7
L93:
	cmp	temp2_, d7
	blt.s	L94
	move	d7, d0
	asr	$3, d0
	movea	d0, a0
	adda.l	sp_, a0
	move	d7, d0
	andi	$7, d0
	moveq	$1, d1
	lsl	d0, d1
	or.b	d1, (a0)
	subq	$1, d7
	bra.s	L93
L89:
	move	d7, d0
	asr	$3, d0
	movea	d0, a0
	adda.l	sp_, a0
	move	d7, d0
	andi	$7, d0
	moveq	$1, d1
	lsl	d0, d1
	or.b	d1, (a0)
L94:
	movea.l	arg_list_, a0
	clr	d1
	move.b	6(a0), d1
	move	nkid_, d0
	cmp	d1, d0
	bhs.s	L95
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-18, d0
	move	d0, (a0)
	move	nkid_, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	arg_list_, a0
	bra	L20161
L95:
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move	(a0), d0
	moveq	$32, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	bra	L20171
L97:
	clr	within_hide_
	bra	L14
L98:
	moveq	$1, d0
	move	d0, -(a7)
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	calc_size_
	addq	$6, a7
	move	d0, -(a7)
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), -(a7)
	jsr	traceon_
	bra	L20073
L99:
	clr	within_trace_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, within_hide_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, st_display_
	bra	L14
L100:
	subq.l	$4, sp_
	movea.l	sp_, a0
	clr.l	(a0)
	bra	L14
L102:
	move.l	$dummy_, -(a7)
	move.l	a5, -(a7)
	jsr	get_routine_
	addq	$8, a7
	movea.l	d0, a4
	clr	d0
	move.b	6(a5), d0
	andi	$32, d0
	beq.s	L103
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	dummy_, (a0)
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	a4, (a0)
	bra	L14
L103:
	move	26(a4), d7
	tst	d7
	bne.s	L104
	moveq	$1, d0
	move	d0, -(a7)
	move.l	a5, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	jsr	calc_size_
	addq	$6, a7
	move	d0, d7
L104:
	cmpi	$2, d7
	bgt.s	L105
	subq.l	$4, sp_
	movea.l	sp_, a0
	clr	(a0)
	bra.s	L106
L105:
	move	d7, -(a7)
	jsr	size_adjust_
	addq	$2, a7
	movea.l	sp_, a0
	suba	d0, a0
	move.l	a0, sp_
L106:
	move	d7, -(a7)
	moveq	$2, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	bra	L20163
L107:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$L10008, -(a7)
	move.l	fltemp_+4, -(a7)
	move.l	fltemp_, -(a7)
	jsr	ilcmp
	lea	12(a7), a7
	tst	d0
	bne.s	L108
	moveq	$80, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L108:
	move.l	$fltemp_, -(a7)
	move.l	sp_, -(a7)
	jsr	ddldiv
	bra	L20056
L109:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	tst	d7
	bne.s	L110
	moveq	$81, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L110:
	movea.l	sp_, a0
	move	(a0), d0
	ext.l	d0
	divs	d7, d0
	bra	L20045
L111:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	tst	d7
	bgt.s	L112
	moveq	$82, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L112:
	movea.l	sp_, a0
	move	(a0), d0
	ext.l	d0
	divs	d7, d0
	swap	d0
	move	d0, (a0)
	movea.l	sp_, a0
	tst	(a0)
	bge	L14
	movea.l	sp_, a0
	add	d7, (a0)
	bra	L14
L114:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a0
	move	(a0), d0
	lsl	d7, d0
	bra	L20045
L115:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a0
	move	(a0), d0
	lsr	d7, d0
	bra	L20045
L116:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a0
	and	d7, (a0)
	bra	L14
L117:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L14
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10018, a0
	bra	L20139
L119:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	cmp.l	tempptr_, d0
L20313:
	bne	L20305
L20300:
	moveq	$1, d0
L20278:
	move	d0, d7
	subq.l	$4, sp_
	movea.l	sp_, a0
	move	d7, (a0)
	bra	L14
L121:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
L20259:
	bne	L20219
L20234:
	moveq	$1, d0
L20185:
	movea.l	sp_, a0
	bra	L20045
L122:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, -(a7)
	jsr	set_eq_
L20289:
	addq	$8, a7
	bra.s	L20278
L123:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	sp_, d0
	addq.l	$8, sp_
	move.l	d0, -(a7)
	move.l	fltemp_+4, -(a7)
	move.l	fltemp_, -(a7)
	jsr	ilcmp
	lea	12(a7), a7
	tst	d0
	beq	L20300
L20305:
	clr	d0
	bra	L20278
L124:
	jsr	str_spop_
	move.l	d0, tempptr_
	jsr	str_spop_
	move.l	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	ustrcmp_
L20321:
	addq	$8, a7
	tst	d0
	bra	L20313
L125:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L14
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10030, a0
	bra	L20139
L127:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	cmp.l	tempptr_, d0
L20322:
	beq.s	L20305
	bra	L20300
L129:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
	bne	L20234
L20219:
	clr	d0
	bra	L20185
L130:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, -(a7)
	jsr	set_eq_
	bra	L20321
L131:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	sp_, d0
	addq.l	$8, sp_
	move.l	d0, -(a7)
	move.l	fltemp_+4, -(a7)
	move.l	fltemp_, -(a7)
	jsr	ilcmp
	lea	12(a7), a7
L20325:
	tst	d0
	bra	L20322
L132:
	jsr	str_spop_
	move.l	d0, tempptr_
	jsr	str_spop_
	move.l	d0, -(a7)
	move.l	tempptr_, -(a7)
	jsr	ustrcmp_
	addq	$8, a7
	bra.s	L20325
L133:
	move	10(a5), d0
	move	d0, -(a7)
	move.l	$Stub_Exec_, -(a7)
	jsr	strchr_
	addq	$6, a7
	tst.l	d0
	bne	L14
	moveq	$83, d0
L20215:
	move	d0, -(a7)
	jsr	run_error_
	bra	L20021
L136:
	movea.l	sp_, a0
	suba	$32, a0
	move.l	a0, sp_
	moveq	$32, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	jsr	zero_
	addq	$6, a7
	movea.l	8(a5), a0
	tst.b	6(a0)
	beq	L14
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	clr	(a0)
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-18, d0
	move	d0, (a0)
	movea.l	8(a5), a0
	bra	L20161
L138:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), -(a7)
	jsr	set_in_
	addq	$6, a7
	bra	L20278
L139:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L14
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10038, a0
	bra	L20139
L142:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
	ble	L20219
	bra	L20234
L143:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	4(a0), -(a7)
	move.l	(a0), -(a7)
	jsr	ilcmp
	lea	12(a7), a7
L20324:
	tst	d0
	bge	L20305
	bra	L20300
L144:
	jsr	str_spop_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	jsr	str_spop_
	move.l	d0, -(a7)
	jsr	ustrcmp_
	addq	$8, a7
	bra.s	L20324
L145:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L20272
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10046, a0
	bra	L20139
L148:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
	blt	L20219
	bra	L20234
L149:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, -(a7)
	jsr	set_le_
	bra	L20289
L150:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	4(a0), -(a7)
	move.l	(a0), -(a7)
	jsr	ilcmp
	lea	12(a7), a7
	tst	d0
	bgt	L20305
	bra	L20300
L151:
	jsr	str_spop_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	jsr	str_spop_
	move.l	d0, -(a7)
	jsr	ustrcmp_
	addq	$8, a7
	tst	d0
	ble	L20300
	bra	L20305
L20272:
	moveq	$84, d0
	bra	L20215
L153:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L159
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10054, a0
	bra	L20139
L156:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
	bge	L20219
	bra	L20234
L157:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	4(a0), -(a7)
	move.l	(a0), -(a7)
	jsr	ilcmp
	lea	12(a7), a7
L20317:
	tst	d0
	ble	L20305
	bra	L20300
L158:
	jsr	str_spop_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	jsr	str_spop_
	move.l	d0, -(a7)
	jsr	ustrcmp_
	addq	$8, a7
	bra.s	L20317
L159:
	moveq	$84, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
	move.l	a5, cursor_
	bra	L14
L160:
	move	18(a5), d0
	subq	$1, d0
	cmpi	$5, d0
	bhi	L20272
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10062, a0
	bra	L20139
L163:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a1
	cmp	(a1), d7
	bgt	L20219
	bra	L20234
L164:
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, -(a7)
	jsr	set_ge_
	bra	L20289
L165:
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	4(a0), -(a7)
	move.l	(a0), -(a7)
	jsr	ilcmp
	lea	12(a7), a7
L20319:
	tst	d0
	blt	L20305
	bra	L20300
L166:
	jsr	str_spop_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	jsr	str_spop_
	move.l	d0, -(a7)
	jsr	ustrcmp_
	addq	$8, a7
	bra.s	L20319
L168:
	tst	within_hide_
	bne	L20218
	moveq	$1, d0
	move	d0, within_hide_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-17, d0
	bra	L20145
L172:
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-1, d0
	bra	L20170
L174:
	move.l	a5, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	cmpi.l	$BT_real_, d0
	bne.s	L175
L20226:
	movea.l	sp_, a0
	move.l	(a0), d0
	move.l	4(a0), d1
	eori.l	$-2147483648, d0
	movea.l	sp_, a0
L20159:
	move.l	d0, (a0)
	move.l	d1, 4(a0)
	bra	L14
L175:
	movea.l	sp_, a0
	move	(a0), d0
	neg	d0
	bra	L20185
L177:
	cmpi.l	$BT_boolean_, 12(a5)
	bne.s	L178
	movea.l	sp_, a0
	tst	(a0)
	bra	L20259
L178:
	movea.l	sp_, a0
	move	(a0), d0
	not	d0
	bra	L20185
L180:
	subq.l	$4, sp_
	movea.l	sp_, a0
	move	14(a5), d0
	bra	L20045
L181:
	subq.l	$8, sp_
	movea.l	sp_, a0
	move.l	12(a5), d0
	move.l	16(a5), d1
	bra.s	L20159
L182:
	subq.l	$4, sp_
	movea.l	sp_, a0
	move	10(a5), d0
	bra	L20045
L183:
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	8(a5), d0
	move.l	d0, -(a7)
	move	$1, d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, (a0)
	bra	L14
L186:
	move.l	8(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-13, d0
	bra	L20045
L187:
	movea.l	sp_, a0
	addq.l	$4, sp_
	tst	(a0)
	beq	L14
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$48, d0
	bra	L20145
L189:
	movea.l	sp_, a0
	addq.l	$4, sp_
	tst	(a0)
	bne	L14
L191:
	move.l	8(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-14, d0
	bra	L20045
L192:
	move.l	12(a5), new_loc_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-15, d0
	bra	L20045
L193:
	movea.l	12(a5), a0
	move.l	8(a0), d0
	movea.l	d0, a4
	move	24(a4), d1
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, -(a7)
	move	d1, d0
	ext.l	d0
	add.l	d0, (a7)
L20301:
	move.l	(a7)+, d0
L20287:
	move.l	d0, tempptr_
L20266:
	move.l	18(a4), -(a7)
	move	26(a4), -(a7)
L20227:
	move.l	tempptr_, -(a7)
L20160:
	jsr	do_pushing_
	bra	L20168
L194:
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, type_ptr_
	movea.l	type_ptr_, a0
	cmpi.b	$20, (a0)
	bne.s	L195
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	tst	d7
	blt.s	L10065
	movea.l	type_ptr_, a0
	move	14(a0), d1
	move	d7, d0
	cmp	d1, d0
	ble.s	L196
L10065:
	move	d7, -(a7)
	move.l	$BT_integer_, -(a7)
	jsr	ordname_
	addq	$6, a7
	move.l	d0, -(a7)
	moveq	$60, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$6, a7
L196:
	move.l	$BT_char_, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	jsr	str_spop_
	move.l	d0, d1
	move	d7, d0
	ext.l	d0
	add.l	d1, d0
L20235:
	move.l	d0, -(a7)
	bra	L20160
L195:
	movea.l	type_ptr_, a0
	move	18(a0), d0
	move	d0, temp2_
	clr	d0
	move.b	6(a5), d0
	andi	$32, d0
	beq	L197
	movea.l	8(a5), a1
	movea.l	8(a1), a0
	cmpi.b	$29, 12(a0)
	bne.s	L198
	clr	d0
	move.b	6(a5), d0
	andi	$2, d0
	beq	L14
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	subq.l	$4, sp_
	movea.l	sp_, a3
	cmpi	$1, temp2_
	ble.s	L10066
	move	d7, d0
	addq	$1, d0
	move	d0, -(a7)
	jsr	inp_
	addq	$2, a7
	move	d0, d3
	bra.s	L10067
L10066:
	clr	d3
L10067:
	move	d7, -(a7)
	jsr	inp_
	addq	$2, a7
	or	d3, d0
	move	d0, (a3)
	bra	L14
L198:
	movea.l	sp_, a1
	addq.l	$4, sp_
	movea	(a1), a0
	move.l	a0, tempptr_
	movea.l	12(a5), a0
	cmpi.b	$58, (a0)
	bne.s	L201
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	lsl	$8, d0
	lsl	$8, d0
	ext.l	d0
	or.l	tempptr_, d0
	move.l	d0, tempptr_
L201:
	move.l	a5, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	move	temp2_, -(a7)
	bra	L20227
L197:
	movea.l	type_ptr_, a0
	move	22(a0), d1
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	sub	d1, d0
	move	d0, d7
	tst	d7
	blt.s	L10068
	movea.l	type_ptr_, a0
	move	26(a0), d1
	move	d7, d0
	cmp	d1, d0
	blt.s	L203
L10068:
	movea.l	type_ptr_, a0
	move	22(a0), d1
	move	d7, d0
	add	d1, d0
	move	d0, -(a7)
	movea.l	type_ptr_, a0
	move.l	8(a0), -(a7)
	jsr	ordname_
	addq	$6, a7
	move.l	d0, -(a7)
	moveq	$60, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$6, a7
L203:
	move.l	a5, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, -(a7)
	move	temp2_, -(a7)
	movea.l	sp_, a0
	addq.l	$4, sp_
	cmpi	$2, temp2_
	bne.s	L10069
	move	d7, d0
	lsl	$1, d0
	bra.s	L10070
L10069:
	cmpi	$1, temp2_
	bne.s	L10071
	move	d7, d0
	bra.s	L10070
L10071:
	move	d7, d0
	muls	temp2_, d0
L10070:
	ext.l	d0
	add.l	(a0), d0
	bra	L20235
L204:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move.l	(a0), d0
	move.l	d0, tempptr_
	move.l	8(a5), -(a7)
	jsr	gexp_type_
	addq	$4, a7
	movea.l	d0, a0
	cmpi.b	$23, (a0)
	bne.s	L205
	move.l	tempptr_, -4(a6)
	clr	d0
	move.b	6(a5), d0
	andi	$2, d0
	beq.s	L206
	movea.l	-4(a6), a0
	move	6(a0), d0
	andi	$16, d0
	beq.s	L206
	moveq	$1, d0
	move	d0, -(a7)
	move.l	-4(a6), -(a7)
	jsr	_a_get_
	addq	$6, a7
L206:
	move.l	-4(a6), d0
	move.l	d0, -(a7)
	move	$14, d0
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, tempptr_
L205:
	tst.l	tempptr_
	bne.s	L207
	moveq	$61, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L207:
	move.l	a5, -(a7)
	jsr	gexp_type_
	addq	$4, a7
	move.l	d0, type_ptr_
	move.l	type_ptr_, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	type_ptr_, -(a7)
	jsr	calc_size_
	addq	$6, a7
	move	d0, -(a7)
	bra	L20227
L208:
	move.l	16(a5), type_ptr_
	cmpi.l	$BT_integer_, type_ptr_
	bne.s	L209
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	ext.l	d0
	move.l	d0, -4(a6)
	movea.l	sp_, a0
	move	(a0), d0
	ext.l	d0
	add.l	d0, -4(a6)
	move	$437, -(a7)
L20264:
	jsr	getLDSstr_
	addq	$2, a7
	move.l	d0, -(a7)
	move.l	-4(a6), -(a7)
	jsr	IntCheck_
	addq	$8, a7
	bra	L20185
L209:
	cmpi.l	$BT_real_, type_ptr_
	bne.s	L211
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	move.l	sp_, -(a7)
	jsr	ddladd
	bra	L20056
L211:
	cmpi.l	$SP_string_, type_ptr_
	bne.s	L213
	jsr	string_add_
	bra	L14
L213:
	move.l	type_ptr_, -(a7)
	jsr	is_set_type_
	addq	$4, a7
	tst	d0
	beq	L14
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, -(a7)
	jsr	set_union_
	bra	L20056
L216:
	move.l	16(a5), type_ptr_
	cmpi.l	$BT_integer_, type_ptr_
	bne.s	L217
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	ext.l	d0
	move.l	d0, -4(a6)
	movea.l	sp_, a0
	move	(a0), d0
	ext.l	d0
	sub.l	-4(a6), d0
	move.l	d0, -4(a6)
	move	$454, -(a7)
	bra	L20264
L217:
	cmpi.l	$BT_real_, type_ptr_
	bne.s	L219
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	move.l	sp_, -(a7)
	jsr	ddlsub
	bra	L20056
L219:
	move.l	type_ptr_, -(a7)
	jsr	is_set_type_
	addq	$4, a7
	tst	d0
	beq	L14
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, -(a7)
	jsr	set_subtract_
	bra	L20056
L222:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a0
	or	d7, (a0)
	bra	L14
L223:
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	movea.l	sp_, a0
	eor	d7, (a0)
	bra	L14
L224:
	move.l	16(a5), type_ptr_
	cmpi.l	$BT_integer_, type_ptr_
	bne.s	L225
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	ext.l	d0
	move.l	d0, -4(a6)
	pea	-4(a6)
	movea.l	sp_, a0
	move	(a0), d0
	ext.l	d0
	move.l	d0, -(a7)
	jsr	llmul
	addq	$8, a7
	move.l	d0, -4(a6)
	move	$438, -(a7)
	bra	L20264
L225:
	cmpi.l	$BT_real_, type_ptr_
	bne.s	L227
	movea.l	sp_, a0
	addq.l	$8, sp_
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, fltemp_
	move.l	d1, fltemp_+4
	move.l	$fltemp_, -(a7)
	move.l	sp_, -(a7)
	jsr	ddlmul
	bra	L20056
L227:
	move.l	type_ptr_, -(a7)
	jsr	is_set_type_
	addq	$4, a7
	tst	d0
	beq	L14
	move.l	sp_, d0
	addi.l	$32, sp_
	move.l	d0, tempptr_
	move.l	tempptr_, -(a7)
	move.l	sp_, -(a7)
	jsr	set_intersect_
	bra	L20056
L231:
	movea.l	8(a5), a4
	clr	d0
	move.b	28(a4), d0
	move	d0, thescope_
	clr	d0
	move.b	12(a4), d0
	subq	$2, d0
	cmpi	$27, d0
	bhi	L256
	lsl	$2, d0
	movea	d0, a0
	adda.l	$L10077, a0
	movea.l	(a0), a0
	jmp	(a0)
	bra	L20266
L233:
	move	24(a4), d1
	move	thescope_, d0
L20158:
	ext.l	d0
	lsl.l	$2, d0
	movea.l	d0, a0
	adda.l	st_display_, a0
	move.l	(a0), d0
	move.l	d0, -(a7)
	move	d1, d0
	ext.l	d0
	sub.l	d0, (a7)
	bra	L20301
L235:
	move.l	22(a4), tempptr_
	bra	L20266
L236:
	movea.l	12(a5), a0
	move	24(a4), d0
	ext.l	d0
	add.l	16(a0), d0
	bra	L20287
L238:
	pea	-8(a6)
	move.l	a4, -(a7)
	jsr	get_routine_
	addq	$8, a7
	move.l	d0, -4(a6)
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	-8(a6), (a0)
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	-4(a6), (a0)
	bra	L14
L240:
	move	24(a4), d1
	move	thescope_, d0
	neg	d0
	bra.s	L20158
L241:
	move	thescope_, d3
	neg	d3
	ext.l	d3
	lsl.l	$2, d3
	movea.l	d3, a0
	adda.l	st_display_, a0
	move.l	(a0), d2
	move	24(a4), d1
	movea.l	d2, a0
	suba	d1, a0
	move.l	(a0), d0
	bra	L20287
L242:
	move	thescope_, d0
	neg	d0
	ext.l	d0
	lsl.l	$2, d0
	movea.l	d0, a0
	adda.l	st_display_, a0
	cmpi	$1, 26(a4)
	bne.s	L10074
	moveq	$1, d0
	bra.s	L10075
L10074:
	clr	d0
L10075:
	ext.l	d0
	add.l	(a0), d0
	bra	L20287
L20191:
	subq.l	$4, sp_
	movea.l	sp_, a0
	move	24(a4), d0
	bra	L20045
L248:
	move.l	18(a4), type_ptr_
	cmpi.l	$BT_integer_, type_ptr_
	beq.s	L20191
	cmpi.l	$BT_char_, type_ptr_
	beq.s	L20191
	cmpi	$2, 26(a4)
	blo.s	L20191
	cmpi.l	$BT_real_, type_ptr_
	bne.s	L251
	subq.l	$8, sp_
	movea.l	sp_, a1
	movea.l	22(a4), a0
	move.l	(a0), d0
	move.l	4(a0), d1
	move.l	d0, (a1)
	move.l	d1, 4(a1)
	clr	d0
	move.b	29(a4), d0
	andi	$64, d0
	beq	L14
	bra	L20226
L251:
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	22(a4), (a0)
	bra	L14
L254:
	subq.l	$4, sp_
	movea.l	sp_, a1
	move.l	22(a4), d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	$predef_files_, a0
	move.l	(a0), d0
	move.l	d0, (a1)
	bra	L14
L255:
	moveq	$59, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$2, a7
L256:
	clr	d0
	move.b	12(a4), d0
	move	d0, -(a7)
	moveq	$62, d0
	move	d0, -(a7)
	jsr	error_
	addq	$4, a7
	bra	L20266
L257:
	tst.b	6(a5)
	beq	L14
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	move.l	sp_, (a0)
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	clr	(a0)
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-11, d0
	move	d0, (a0)
	move.l	8(a5), new_loc_
	bra.s	L14
L259:
	movea.l	ex_stack_, a0
	addq	$1, (a0)
	move	(a0), d0
	move	d0, nkid_
	clr	d1
	move.b	6(a5), d1
	move	nkid_, d0
	cmp	d1, d0
	bhs.s	L20118
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-11, d0
	move	d0, (a0)
	move	nkid_, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	a5, a0
	bra	L20161
L20118:
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move	(a0), d0
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	movea.l	(a0), a0
L14:
	jsr	got_break_
	tst	d0
	beq.s	L263
	jsr	breakkey_
	jsr	clear_break_
L263:
	tst	step_flag_
	beq	L264
	tst.l	new_loc_
	beq	L264
	movea.l	new_loc_, a0
	cmpi.b	$101, (a0)
	beq	L264
	movea.l	new_loc_, a0
	clr	d0
	move.b	(a0), d0
	ext.l	d0
	lsl.l	$1, d0
	movea.l	d0, a0
	adda.l	lt_node_flags_, a0
	move	(a0), d0
	andi	$4, d0
	beq.s	L10080
	tst	within_hide_
	beq.s	L10079
L10080:
	tst	must_stop_
	beq	L264
L10079:
	movea.l	new_loc_, a0
	clr	d0
	move.b	6(a0), d0
	andi	$64, d0
	move	d0, d7
	bne.s	L10081
	move	step_flag_, d0
	andi	$16, d0
	beq.s	L265
L10081:
	tst	d7
	beq.s	L266
	move	$213, -(a7)
	jsr	message_
	addq	$2, a7
	bra.s	L267
L266:
	tst.b	WasBreak_
	beq.s	L267
	move	$212, -(a7)
	jsr	message_
	addq	$2, a7
	clr.b	WasBreak_
L267:
	moveq	$1, d0
	move	d0, -(a7)
	move.l	a5, -(a7)
	move.l	new_loc_, -(a7)
	jsr	do_suspend_
	lea	10(a7), a7
	bra	L1
L265:
	move	step_flag_, d0
	andi	$2, d0
	beq.s	L264
	move.l	step_delay_, -4(a6)
L273:
	tst.l	-4(a6)
	beq.s	L271
	subq.l	$1, -4(a6)
	bra.s	L273
L271:
	clr	-(a7)
	move.l	new_loc_, -(a7)
	move.l	curr_window_, -(a7)
	jsr	find_phys_cursor_
	lea	10(a7), a7
	move	srch_real_, -(a7)
	move	srch_row_, -(a7)
	movea.l	curr_window_, a0
	move.l	(a0), -(a7)
	jsr	wmove_
	addq	$8, a7
	movea.l	curr_window_, a0
	move.l	(a0), -(a7)
	jsr	wrefresh_
	addq	$4, a7
L264:
	tst.l	new_loc_
	bne	L3
	cmpi.b	$101, (a5)
	beq	L3
	tst	cur_state_
	blt	L3
	clr	d0
	move.b	6(a5), d0
	andi	$6, d0
	cmpi	$6, d0
	bne.s	L275
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	subq.l	$8, sp_
	movea.l	sp_, a3
	move	d7, -(a7)
	jsr	dicvt
	addq	$2, a7
	move.l	d0, (a3)
	move.l	d1, 4(a3)
	bra.s	L276
L275:
	clr	d0
	move.b	6(a5), d0
	andi	$128, d0
	beq.s	L276
	movea.l	sp_, a0
	addq.l	$4, sp_
	move	(a0), d0
	move	d0, d7
	subq.l	$4, sp_
	movea.l	sp_, a0
	moveq	$1, d0
	move.b	d0, (a0)
	movea.l	sp_, a0
	move.b	d7, 1(a0)
	movea.l	sp_, a0
	clr.b	2(a0)
	moveq	$3, d0
	move	d0, -(a7)
	moveq	$1, d0
	move	d0, -(a7)
	move.l	sp_, -(a7)
	jsr	do_undef_
	addq	$8, a7
	subq.l	$4, sp_
	movea.l	sp_, a0
	clr.l	(a0)
L276:
	tst	trace_queue_size_
	beq	L3
	tst	within_trace_
	bne	L3
	subq	$1, trace_queue_size_
	move	trace_queue_size_, d1
	ext.l	d1
	lsl.l	$1, d1
	movea.l	d1, a0
	adda.l	$trace_excepts_, a0
	move	(a0), d0
	move	d0, -2(a6)
	subq.l	$4, sp_
	movea.l	sp_, a0
	move.l	st_display_, (a0)
	subq.l	$4, sp_
	movea.l	sp_, a0
	move	within_hide_, (a0)
	move	-2(a6), d1
	muls	$18, d1
	movea.l	d1, a0
	adda.l	$tr_locs_+8, a0
	move.l	(a0), d0
	move.l	d0, new_loc_
	move	-2(a6), d1
	muls	$18, d1
	movea.l	d1, a0
	adda.l	$tr_locs_+12, a0
	move.l	(a0), d0
	move.l	d0, st_display_
	move	-2(a6), d1
	muls	$18, d1
	movea.l	d1, a0
	adda.l	$tr_locs_+16, a0
	move	(a0), d0
	move	d0, within_hide_
	moveq	$1, d0
	move	d0, within_trace_
	subq.l	$4, ex_stack_
	movea.l	ex_stack_, a0
	moveq	$-22, d0
	move	d0, (a0)
	bra	L3
L4:
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move.l	(a0), d1
	movea.l	d1, a5
	move.l	a5, d0
	beq.s	L12
	movea.l	ex_stack_, a0
	addq.l	$4, ex_stack_
	move	(a0), d0
	move	d0, cur_state_
	bra	L11
L12:
	jsr	terminated_
L1:
	movem.l	-30(a6), $14472
	unlk	a6
	rts
	.globl IntCheck_
IntCheck_:
	link	a6, $-4
	movem.l	$128, (a7)
	move.l	8(a6), d7
	cmpi.l	$-32768, d7
	blt.s	L10082
	cmpi.l	$32767, d7
	ble.s	L280
L10082:
	move.l	12(a6), -(a7)
	moveq	$63, d0
	move	d0, -(a7)
	jsr	run_error_
	addq	$6, a7
L280:
	move	d7, d0
	movem.l	-4(a6), $128
	unlk	a6
	rts
	.globl breakkey_
breakkey_:
	link	a6, $0
	tst	break_disable_
	beq.s	L282
	moveq	$1, d0
	move	d0, -(a7)
	move.l	$1, -(a7)
	jsr	scan_trace_
	addq	$6, a7
	bra.s	L281
L282:
	moveq	$1, d0
	move.b	d0, WasBreak_
	ori	$16, step_flag_
	moveq	$1, d0
	move	d0, must_stop_
L281:
	unlk	a6
	rts
