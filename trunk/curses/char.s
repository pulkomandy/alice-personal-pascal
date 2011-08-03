
/ Fast character drawing routines
/ use the style low nibble to offset into a jump
/ table

	.shrd

dr_vectors:			/ 4 3 2 1 Attribute Byte
	.long	dr_normal	/ 0 0 0 0 normal type characters
	.long 	dr_underline	/ 0 0 0 1 underlined characters
	.long	dr_dotunderline / 0 0 1 0 dot underlined characters
	.long	dr_underline	/ 0 0 1 1 underlined characters
	.long 	dr_reverse	/ 0 1 0 0 regular reverse video
	.long	dr_reverse	/ 0 1 0 1
	.long	dr_reverse	/ 0 1 1 0
	.long	dr_reverse	/ 0 1 1 1
	.long	dr_bold		/ 1 0 0 0 regular bold
	.long	dr_bold		/ 1 0 0 1 bold underline is bold
	.long	dr_bold		/ 1 0 1 0 bold dot underline is bold
	.long	dr_bold		/ 1 0 1 1 bold
	.long	dr_boldrev	/ 1 1 0 0 reverse video bold
	.long	dr_boldrev	/ 1 1 0 1
	.long	dr_boldrev	/ 1 1 1 0
	.long	dr_boldrev	/ 1 1 1 1

	.shri

	.globl draw_char_

frame = 8 * 4
pointer = frame
char = frame + 4
style = frame + 6
nextline = frame + 8

draw_char_:

/ save d3-7, a4-5

	movem.l	$0x1f0c, -(a7)

	movea.l	pointer(a7), a5
	move	char(a7), d7
	move	style(a7), d6
	move.l	nextline(a7), d5

/ get the pointer to the font data
	clr.l	d0
	move.b	d7, d0
	add.l	font_base_, d0
	movea.l	d0, a4

	clr.l 	d0
	move.w	d6, d0
	asr.l	$4, d0		/ style is in high nibble
	andi.w	$15, d0
	asl.l	$2, d0		/ convert to offset
	movea.l	d0, a0
	adda.l	$dr_vectors, a0	/ add on base

	movea.l	(a0), a0
	
	jmp	(a0)		/ and call it


dr_normal:
	move.w	charHeight_, d4
	subq.w	$1, d4

L1:	move.b	(a4), (a5)
	adda.l	d5, a5		/ bump buffer pointer
	adda.l	$256, a4	/ jump to next scan line of font
	dbra	d4, L1

	movem.l	(a7)+,$12536
	rts

dr_reverse:
	move.w	charHeight_, d4
	subq.w	$1, d4

L2:	move.b	(a4), d0
	not.b	d0
	move.b	d0, (a5)
	adda.l	d5, a5
	adda.l	$256, a4
	dbra	d4, L2

	movem.l	(a7)+,$12536
	rts

dr_bold:
	move.w	charHeight_, d4
	subq.w	$1, d4

L3:	move.b	(a4), d0
	move.b	d0, d1
	lsr.b	$1, d1
	or.b	d1, d0
	move.b	d0, (a5)
	adda.l	d5, a5
	adda.l	$256, a4
	dbra	d4, L3

	movem.l	(a7)+,$12536
	rts

dr_underline:
	move.w	charHeight_, d4
	subq.w	$2, d4

L4:	move.b	(a4), (a5)
	adda.l	d5, a5
	adda.l	$256, a4
	dbra	d4, L4

	move.b	$255, (a5)
	movem.l	(a7)+,$12536
	rts

dr_dotunderline:
	move.w	charHeight_, d4
	subq.w	$2, d4

L5:	move.b	(a4), (a5)
	adda.l	d5, a5
	adda.l	$256, a4
	dbra	d4, L5

	move.b	(a4), d0
	ori.b	$170, d0
	move.b	d0, (a5)

	movem.l	(a7)+,$12536
	rts


dr_boldrev:
	move.w	charHeight_, d4
	subq.w	$1, d4

L6:	move.b	(a4), d0
	move.b	d0, d1
	lsr.b	$1, d1
	or.b	d1, d0
	not.b	d0
	move.b	d0, (a5)
	adda.l	d5, a5
	adda.l	$256, a4
	dbra	d4, L6

	movem.l	(a7)+,$12536
	rts
