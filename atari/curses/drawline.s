/	module name drawline

	.bssd

scr_incr_:
	.blkb	0x4

	.shri

	.globl draw_line_
draw_line_:
	link	a6, $-58
	movem.l	$14584, (a7)
	movea.l	8(a6), a5
	movea.l	12(a6), a4
	move.l	$s_nxt_lin_, -(a7)
	move	18(a6), d1
	ext.l	d1
	move.l	d1, -(a7)
	jsr	llmul
	addq	$8, a7
	adda.l	d0, a5

	move	next_char_, d1
	addq	$1, d1
	move.l	s_nxt_lin_, d0
	sub.l	d1, d0
	move.l	d0, scr_incr_

/ the leftmost char

	move.l	a5, -4(a6)
	move.b	(a4)+, d6
	move.b	(a4)+, d7

	move.l	$1, -(a7)
	move.w	d6, -(a7)
	move.w	d7, -(a7)
	pea	-24(a6)
	jsr	draw_char_
	lea	12(a7), a7

/ calc fontptr
	lea	-24(a6), a3
	clr.l	d0
	move.w	18(a6), d0
	adda.l	d0, a3

	move	20(a6), d4
L4:
	tst	d4
	ble.s	L2
	move.b	(a3), d7
	moveq	$16, d5
	move	num_planes_, d3
L7:
	tst	d3
	ble.s	L5
	clr	d1
	move.b	d6, d1
	move	d5, d0
	and	d1, d0
	beq.s	L8
	move.b	(a5), d0
	ext	d0
	clr	d2
	move.b	lmask_, d2
	not	d2
	and	d0, d2
	clr	d1
	move.b	lmask_, d1
	clr	d0
	move.b	d7, d0
	and	d1, d0
	or	d2, d0
	bra.s	L20001
L8:
	move.b	(a5), d1
	ext	d1
	clr	d0
	move.b	lmask_, d0
	not	d0
	and	d1, d0
L20001:
	move.b	d0, (a5)
	addq.l	$2, a5
	lsl	$1, d5
	subq	$1, d3
	bra.s	L7
L5:
	adda.l	scr_incr_, a5
	addq.l	$1, a3
	subq	$1, d4
	bra.s	L4
L2:
	move.l	a5, d0
	andi.l	$1, d0
	beq.s	L10
	move.l	-4(a6), d0
	move.l	d0, -(a7)
	move	next_char_, d0
	bra.s	L20005
L10:
	move.l	-4(a6), d0
	move.l	d0, -(a7)
	move	$1, d0
L20005:
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	movea.l	d0, a5
	cmpi	$1, 16(a6)
	beq	L1
	subq	$1, 16(a6)
	move	16(a6), -26(a6)
L15:
	cmpi	$1, -26(a6)
	ble	L13
	move.l	a5, -4(a6)
	move.b	(a4)+, d6	/ get style
	move.b	(a4)+, d7	/ get character

	move.l	$1, -(a7)	/ draw the character into a temp buffer
	move.w	d6, -(a7)
	move.w	d7, -(a7)
	pea	-24(a6)
	jsr	draw_char_
	lea	12(a7), a7

/ calculate the start of the buffer

	lea	-24(a6), a3
	clr.l	d0
	move.w	18(a6), d0
	adda.l	d0, a3

/ for as many scan lines to do...

	move	20(a6), d4
L18:
	tst	d4
	ble.s	L16		/ if finished scan lines, quit
	move.b	(a3), d7	/ get scan line
	moveq	$16, d5		/ set up initial color plane
	move	num_planes_, d3 / for num_planes do

/ for( k=num_planes; k > 0; k-- ) {

L21:
	tst	d3
	ble.s	L19

	move.b	d6, d1		/ get style
	move	d5, d0		/ get ColBit
	and	d1, d0		/ test
	beq.s	L22		/ if bit not on, then clear (a5)
	move.b	d7, (a5)	/ store the scan line
	bra.s	L23
L22:
	clr.b	(a5)
L23:
	addq.l	$2, a5		/ buffer += 2
	lsl	$1, d5		/ ColBit <<= 1
	subq	$1, d3		/ k--
	bra.s	L21
L19:
	adda.l	scr_incr_, a5
	addq.l	$1, a3
	subq	$1, d4
	bra.s	L18		/ loop for next scan line
L16:
	move.l	a5, d0
	andi.l	$1, d0
	beq.s	L24
	move.l	-4(a6), d0
	move.l	d0, -(a7)
	move	next_char_, d0
	bra.s	L20009
L24:
	move.l	-4(a6), d0
	move.l	d0, -(a7)
	move	$1, d0
L20009:
	ext.l	d0
	add.l	d0, (a7)
	move.l	(a7)+, d0
	movea.l	d0, a5
	subq	$1, -26(a6)
	bra	L15
L13:
	move.b	(a4)+, d6
	move.b	(a4)+, d7
	moveq	$1, d0
	move.l	d0, -(a7)
	clr	d0
	move.b	d6, d0
	move	d0, -(a7)
	clr	d0
	move.b	d7, d0
	move	d0, -(a7)
	pea	-24(a6)
	jsr	draw_char_
	lea	12(a7), a7


/ calc fontptr
	lea	-24(a6), a3
	clr.l	d0
	move.w	18(a6), d0
	adda.l	d0, a3

/ for each line to copy...

	move	20(a6), d4
L28:
	tst	d4
	ble.s	L1
	move.b	(a3), d7
	moveq	$16, d5
	move	num_planes_, d3
L31:
	tst	d3
	ble.s	L29
	clr	d1
	move.b	d6, d1
	move	d5, d0
	and	d1, d0
	beq.s	L32
	move.b	(a5), d0
	ext	d0
	clr	d2
	move.b	rmask_, d2
	not	d2
	and	d0, d2
	clr	d1
	move.b	rmask_, d1
	clr	d0
	move.b	d7, d0
	and	d1, d0
	or	d2, d0
	bra.s	L20010
L32:
	move.b	(a5), d1
	ext	d1
	clr	d0
	move.b	rmask_, d0
	not	d0
	and	d1, d0
L20010:
	move.b	d0, (a5)
	addq.l	$2, a5
	lsl	$1, d5
	subq	$1, d3
	bra.s	L31
L29:
	adda.l	scr_incr_, a5
	addq.l	$1, a3
	subq	$1, d4
	bra.s	L28
L1:
	movem.l	-58(a6), $14584
	unlk	a6
	rts
