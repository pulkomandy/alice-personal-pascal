/	module name char

	.shri

	.globl draw_char_
draw_char_:
	link	a6, $-28
	movem.l	$12536, (a7)
	movea.l	8(a6), a5
	move	12(a6), d7
	move	14(a6), d6
	move.l	16(a6), d5
	move	d7, d0
	swap	d0
	clr	d0
	swap	d0
	add.l	font_base_, d0
	movea.l	d0, a4
	move	d6, d0
	andi	$4, d0
	beq.s	L2
	move.b	$255, d3
	bra.s	L3
L2:
	clr.b	d3
L3:
	move	charHeight_, d0
	subq	$1, d0
	move.b	d0, d4
L6:
	tst.b	d4
	beq.s	L4
	move.b	(a4), d0
	ext	d0
	move	d0, d7
	move	d6, d0
	andi	$8, d0
	beq.s	L7
	move	d7, d1
	lsl	$1, d1
	or	d1, d7
L7:
	clr	d0
	move.b	d3, d0
	eor	d7, d0
	move.b	d0, (a5)
	adda.l	d5, a5
	adda.l	f_nxt_lin_, a4
	subq.b	$1, d4
	bra.s	L6
L4:
	move.b	(a4), d0
	ext	d0
	move	d0, d7
	move	d6, d0
	andi	$8, d0
	beq.s	L8
	move	d7, d1
	lsr	$1, d1
	or	d1, d7
L8:
	move	d6, d0
	andi	$1, d0
	beq.s	L9
	ori	$255, d7
	bra.s	L10
L9:
	move	d6, d0
	andi	$2, d0
	beq.s	L10
	ori	$170, d7
L10:
	clr	d0
	move.b	d3, d0
	eor	d7, d0
	move.b	d0, (a5)
	movem.l	-28(a6), $12536
	unlk	a6
	rts
