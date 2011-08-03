
	import  <sp>:word
	export	<str_spop>
<str_spop>:
	mov	bx,<sp>
	mov	ax,[bx]
	add	bx,#2
	or	ax,ax
	jnz	gogo
b	mov	cl,[bx]			; the length byte
	sub	ch,ch			; clear upper half
	add	cx,#2			; two extra bytes
	mov	ax,bx			; the pointer
	add	bx,cx			; 
gogo:
	mov	<sp>,bx			; store stack
	ret

	export <get_DS>
<get_DS>:
	mov	ax,ds
	ret
