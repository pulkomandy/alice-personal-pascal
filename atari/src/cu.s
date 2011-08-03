
	.shri

	.globl CU_
CU_:
	link	a6, $-12
	movem.l	$224, (a7)
	move.l	8(a6), d7		/ get pointer
	sub.l	exec_stack_, d7		/ subtract off exec_stack
	clr.l	d1
	move	max_stack_size_, d1
	cmp.l	d1, d7
	bhi.s	L1			/ if bigger than stack, return

	move.l	d7, d6
	asr.l	$3, d6			/ get byte offset in d6

	andi.l	$7, d7			/ get bit in d7

	moveq	$1, d5
	lsl.l	d7, d5			/ get mask in d5

	movea.l	d6, a0			/ get offset
	adda.l	undef_bitmap_, a0	/ add on base address
	move.b	(a0), d1		/ get the byte

	and	d5, d1			/ test the bit
	bne.s	L1

	jsr	CUerror_

L1:
	movem.l	-12(a6), $224
	unlk	a6
	rts

