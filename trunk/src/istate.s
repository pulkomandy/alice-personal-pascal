
	.globl	istate_
	.globl	ex_stack_
	.globl	new_loc_
	.globl	lt_node_flags_
	.globl	cur_state_

	.shri
istate_:
	move.l	a2, -(a7)	/ save A2

/ test the break bit

/ nobreak:
L1:
	/ Store ex_stack in a register (a2)
	movea.l	ex_stack_, a2

	/ if( new_loc ) {
	tst.l	new_loc_
	beq	L8

	/ espush( eloc );
	move.l	a5, -(a2)

/ godown - while( new_loc ) {
	movea.l	new_loc_, a5

L2: / if( not_a_list( eloc ) && node_flag(eloc) & NF_ERROR ) {
	cmpi.b	$0x65, (a5)
	beq	L3

	move.b	0x6(a5), d0
	andi.w	$0x8, d0
	beq	L3

	/ ex_loc = eloc;
	move.l	a5, ex_loc_

	/ run_error( rttype_err );
	moveq.l	$59, d0
	move.w	d0, -(a7)
	jsr	run_error_
	addq.w	$0x4, a7

L3:
/ isnoerror - cur_state = ntype( eloc );
	clr.l	d0
	move.b	(a5), d0
	move.w	d0, cur_state_

	lsl.l	$0x1, d0
	movea.l	d0, a0
	adda.l	lt_node_flags_, a0	/ lt_node_flags is a ptr for real
	move.w	(a0), d0		/ ALICE, an absolute for SAI

	/ if(F_E1KID|F_E2KID) ...
	andi.w	$0x60, d0
	beq	L5

	/ state_push( cur_state );
	/ d0 still holds el_flags & 0x60
	subq.l	$0x4, a2
	move.w	cur_state_, (a2)
	move.l	a5, -(a2)

	/ if( el_flags & F_E2KIDS ) {
	andi.w	$0x40, d0
	beq	L4

	/ state_push( S_GRABK2 );
	subq.l	$0x4, a2
	moveq.l	$-12, d0
	move.w	d0, (a2)

	/ espush( eloc );
	move.l	a5, -(a2)

L4:
/ nograb2 - new_loc = kid1( eloc );
	movea.l	0x8(a5), a5
	move.l	a5, new_loc_
	bra	L2

L5:
/ no_prepush - new_loc = nil;
	clr.l	new_loc_

L6:
/ terminate
	clr.w	d0	/ return 0

L7:
/ andgo
	move.l	a5, ex_loc_
	move.l	a2, ex_stack_

	movea.l	(a7)+, a2	/ restore A2
	rts

L8:
/ no_new_loc - eloc = espop( e_loc );
	movea.l	(a2)+, a5
	move.l	a5, d0
	beq	L9

	/ cur_state = espop( e_state );
	move.w	(a2), d0
	addq.l	$0x4, a2
	move.w	d0, cur_state_

	bra 	L6

L9:
/ alldone
	moveq.l	$0x1, d0
	bra	L7
