/	module name cu

	.shri

	.globl CU_
CU_:
	link	a6, $-16
	movem.l	$240, (a7)
	move.l	8(a6), d7
	move.l	d7, d4
	sub.l	exec_stack_, d7
	move	d7, d0
	cmp	max_stack_size_, d0
	bgt.s	L1
	move	d7, d0
	blt.s	L1
	move.l	d7, d0
	asr.l	$3, d0
	move.l	d0, d6
	move.l	d7, d1
	andi.l	$7, d1
	moveq	$1, d0
	move	d1, d2
	lsl.l	d2, d0
	move	d0, d5
	movea.l	undef_bitmap_, a0
	adda.l	d6, a0
	clr	d0
	move.b	(a0), d0
	and	d5, d0
	bne.s	L1
	jsr	CUerror_
L1:
	movem.l	(a7), $240
	unlk	a6
	rts
