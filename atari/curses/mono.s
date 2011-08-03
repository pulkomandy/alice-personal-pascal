/	module name mono

	.shri

	.globl mono_refresh_
mono_refresh_:
	link	a6, $-36
	movem.l	$14584, (a7)
	movea.l	8(a6), a5
	move.l	$s_nxt_lin_, -(a7)
	move	charHeight_, d0
	ext.l	d0
	move.l	d0, -(a7)
	jsr	llmul
	addq	$8, a7
	move.l	d0, -4(a6)
	moveq	$2, d0
	move	d0, -(a7)
	jsr	xbios_
	addq	$2, a7
	movea.l	d0, a4
	move.l	$s_nxt_lin_, -(a7)
	move	14(a6), d1
	ext.l	d1
	move.l	d1, -(a7)
	jsr	llmul
	addq	$8, a7
	adda.l	d0, a4
	move	12(a6), d0
	ext.l	d0
	divs	$16, d0
	muls	num_planes_, d0
	lsl	$1, d0
	adda	d0, a4
	move	12(a6), d0
	ext.l	d0
	divs	$8, d0
	andi	$1, d0
	adda	d0, a4
	clr	d7
L4:
	cmp	4(a5), d7
	bge.s	L1
	move.l	a4, d5
	move	d7, d0
	ext.l	d0
	lsl.l	$1, d0
	movea.l	d0, a0
	adda.l	22(a5), a0
	tst	(a0)
	beq.s	L5
	move	d7, d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	18(a5), a0
	move.l	(a0), d0
	movea.l	d0, a3
	move	6(a5), d6
L8:
	tst	d6
	ble.s	L5
	move.b	(a3)+, d4
	move.b	(a3)+, d3
	move.l	s_nxt_lin_, -(a7)
	clr	d0
	move.b	d4, d0
	move	d0, -(a7)
	clr	d0
	move.b	d3, d0
	move	d0, -(a7)
	move.l	a4, -(a7)
	jsr	draw_char_
	lea	12(a7), a7
	addq.l	$1, a4
	subq	$1, d6
	bra.s	L8
L5:
	move.l	d5, d0
	add.l	-4(a6), d0
	movea.l	d0, a4
	addq	$1, d7
	bra.s	L4
L1:
	movem.l	-36(a6), $14584
	unlk	a6
	rts
