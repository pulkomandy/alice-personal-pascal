/	module name multi_src

	.bssd

next_line_:
	.blkb	0x4

	.shri

	.globl fast_multi_
fast_multi_:
	link	a6, $-74
	movem.l	$12536, (a7)
	move	next_char_, -4(a6)
	move	8(a6), d1
	muls	$46, d1
	movea.l	d1, a0
	adda.l	$WinInfo_+14, a0
	move	(a0), d0
	move	d0, -6(a6)
	move	8(a6), d1
	muls	$46, d1
	movea.l	d1, a0
	adda.l	$WinInfo_+16, a0
	move	(a0), d0
	move	d0, -8(a6)
	move	-4(a6), d1
	addq	$1, d1
	move.l	s_nxt_lin_, d0
	move.l	d0, -(a7)
	move	d1, d0
	ext.l	d0
	sub.l	d0, (a7)
	move.l	(a7)+, d0
	move.l	d0, next_line_
	movea.l	10(a6), a0
	move	34(a0), d1
	ext.l	d1
	lsl.l	$1, d1
	movea.l	d1, a0
	adda.l	$CurColMap_, a0
	move	(a0), d0
	move	d0, d6
	moveq	$2, d0
	move	d0, -(a7)
	jsr	xbios_
	addq	$2, a7
	movea.l	d0, a5
	move.l	$s_nxt_lin_, -(a7)
	move	-8(a6), d1
	ext.l	d1
	move.l	d1, -(a7)
	jsr	llmul
	addq	$8, a7
	adda.l	d0, a5
	movea.l	la_init_+4, a0
	move	-6(a6), d0
	ext.l	d0
	divs	$16, d0
	muls	(a0), d0
	lsl	$1, d0
	adda	d0, a5
	move	-6(a6), d0
	ext.l	d0
	divs	$8, d0
	andi	$1, d0
	adda	d0, a5
	clr	-44(a6)
L4:
	movea.l	10(a6), a0
	move	-44(a6), d0
	cmp	4(a0), d0
	bge	L1
	move.l	a5, -42(a6)
	movea.l	10(a6), a1
	move	-44(a6), d0
	ext.l	d0
	lsl.l	$1, d0
	movea.l	d0, a0
	adda.l	22(a1), a0
	tst	(a0)
	beq	L5
	movea.l	10(a6), a1
	move	-44(a6), d1
	ext.l	d1
	lsl.l	$2, d1
	movea.l	d1, a0
	adda.l	18(a1), a0
	move.l	(a0), d0
	move.l	d0, -36(a6)
	movea.l	10(a6), a0
	move	6(a0), d0
	move	d0, -46(a6)
L8:
	tst	-46(a6)
	ble	L5
	move.l	a5, -32(a6)
	movea.l	-36(a6), a0
	addq.l	$1, -36(a6)
	move.b	(a0), -2(a6)
	clr	d1
	move.b	-2(a6), d1
	andi	$15, d1
	swap	d1
	clr	d1
	swap	d1
	lsl.l	$1, d1
	movea.l	d1, a0
	adda.l	$CurColMap_, a0
	move	(a0), d0
	move	d0, d5
	movea.l	-36(a6), a0
	addq.l	$1, -36(a6)
	move.b	(a0), -38(a6)
	moveq	$1, d0
	move.l	d0, -(a7)
	clr	d0
	move.b	-2(a6), d0
	move	d0, -(a7)
	clr	d0
	move.b	-38(a6), d0
	move	d0, -(a7)
	pea	-28(a6)
	jsr	draw_char_
	lea	12(a7), a7
	lea	-28(a6), a4
	move	charHeight_, d3
L11:
	tst	d3
	ble.s	L9

/ Okay, here is the stuff that is executed lots of times...

	moveq	$1, d7
	move	num_planes_, d4
L14:
	tst	d4		/ finished all the planes ?
	ble.s	L12

/ check for this bit being on in the background colour

	move.b	d6, d0		/ d6 is the color mask
	and.b	d7, d0		/ d7 is the current bit plane number
	beq.s	L15

/ Bit is on for background color, so set the color to be the inverse of
/ the current font byte

	move.b	(a4), (a5)	/ *scrptr = ~*ptr;
	not.b	(a5)

	bra.s	L16
L15:
/ Otherwise, just set it to zero
/ *scrptr = 0;
	clr.b	(a5)
L16:
/ Now OR in the font byte if we should for this color plane

	move.b	d7, d0	/ d7 - ColBit
	and.b	d5, d0	/ d5 - col
	beq.s	L17

/ OR in the mask

	move.b	(a4), d0
	or.b	d0, (a5)  / OR on the character scan line

L17:
	lsl	$1, d7	/ ColBit <<= 1;
	addq.l	$2, a5  / scrptr += 2;

/ go onto the next plane

	subq	$1, d4
	bra.s	L14
/
L12:
	adda.l	next_line_, a5
	addq.l	$1, a4
	subq	$1, d3
	bra.s	L11	/ Go do next scan line...

/ Finished all the scan lines...

L9:
/ If scrptr & 1

	move.l	a5, d0
	andi.l	$1, d0
	beq.s	L18

/ Add on 'next'
/ scrptr = saveptr + next;
	clr.w	d0
	move.w	-4(a6), d0	/ get 'next'
	add.l	-32(a6), d0	/ add on 'saveptr'
	bra.s	L20001
L18:
	move.l	-32(a6), d0
	addq.l	$1, d0
L20001:
	movea.l	d0, a5
	subq	$1, -46(a6)

	bra	L8	/ Go do next character

L5:
	move.l	$s_nxt_lin_, -(a7)
	move	charHeight_, d0
	ext.l	d0
	move.l	d0, -(a7)
	jsr	llmul
	addq	$8, a7
	add.l	-42(a6), d0
	movea.l	d0, a5
	addq	$1, -44(a6)
	bra	L4
L1:
	movem.l	-74(a6), $12536
	unlk	a6
	rts
