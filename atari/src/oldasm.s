
	.globl	Al_Ctrl_
	.globl  AlVdi_
	.globl  ExceptNumber_
	.globl  ExcExtra_
	.globl	ExcRegs_
	.globl  longjmp_
	.globl  err_buf_
	.globl  except_
	.globl	ExcPC_
	.globl	ExcStack_
	.globl	ExcTrace_
	.globl	exc_buserr_
	.globl	exc_addrerr_
	.globl	exc_illerr_
	.globl	exc_priverr_
	.globl	exc_divide_
	.globl	st_libadr_
	.globl	st_parms_
	.globl  st_version_
	.globl	call_lib_

	.bssd

ExceptNumber_: .blkw	1
ExcExtra_:     .blkw	7
ExcRegs_:      .blkl	16
ExcStack_:     .blkl	1
ExcPC_:	       .blkl	1
ExcTrace_:     .blkw	100	/ 100 words of stack traceback

	.shri

AlVdi_:	move.l $Al_Ctrl_,d1
	move.l $0x73,d0
	trap   $0x2
	rts

exc_buserr_:
	move.w	$1, ExceptNumber_
	bra	except_

exc_addrerr_:
	move.w	$2, ExceptNumber_
	bra	except_

exc_priverr_:
	move.w  $3, ExceptNumber_
	bra	except_

exc_illerr_:
	move.w	$4, ExceptNumber_

exc_divide_:
	move.w	$5, ExceptNumber_
	bra	except_

/
/ The exception handler - ExceptNumber holds the exception code
/

except_:

	movem.l	$0xffff,ExcRegs_ / Save all the registers

	move.w	ExceptNumber_, d0 / get the code

/ if it was a bus error or address error, save extra stack stuff

	cmpi.w	$1, d0			/ test for bus error
	beq	L1			/ yes, go save extra stuff
	cmpi.w	$2, d0
	bne	L2

L1:
/
/ We have a bus error or an address error
/
	lea	ExcExtra_, a0
	move.l	(sp)+, (a0)+
	move.l	(sp)+, (a0)+

/
/ Now we can pop the Status Register and PC off the stack
/

L2:
	move.w	(sp)+, d0	/ just throw it away
	move.l	(sp)+, d0
	move.l	d0, ExcPC_	/ save the pc

/
/ We have finished all the processing needed,
/ Now we go back into user mode and do a long jump back to
/ where we belong
/

	andi.w	$0xdfff, sr	/ switch back to user mode

	move.l	sp, d0		/ get stack pointer
	move.l	d0, ExcStack_

	movea.l	d0, a0		/ get ready to copy the stack
	move.w	$99, d0
	lea	ExcTrace_, a1

L5:	move.w	(a0)+, (a1)+
	dbra	d0, L5

/
/ and do a long jump back to err_return
/

	move.w	$2, d0
	move.w  d0, -(a7)
	move.l  $err_buf_, -(a7)
	jsr	longjmp_

/ should never get here

	rts


/ Call a library function, but pass it various parms in registers

call_lib_:	move.l	st_version_, d0
		movea.l  st_parms_, a0
		movea.l	st_libadr_, a1
		jmp	(a1)
