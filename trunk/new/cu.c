
#include "alice.h"
#include "interp.h"

/*
 * Check_Undef
 */
CU( ptr, b )
register long ptr;
int b;
{
	register long off;
	register int bit;
	register long saveptr;

	saveptr = ptr;

	ptr -= (long)exec_stack;

	if( (framesize)ptr > max_stack_size ) return;
	if( (framesize)ptr < (framesize)0 ) return;

	off = ptr >> 3;
	bit = 1 << (ptr & 7);
	if( undef_bitmap[off] & bit )
		return;

	CUerror();
}
