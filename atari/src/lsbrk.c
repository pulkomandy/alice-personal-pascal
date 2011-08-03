
#include <stdio.h>
#include <basepage.h>
#include <osbind.h>

long sys_fence = 0x400;

#define SAFETY	sys_fence		/* Leave 2K for the system */

char *
lsbrk( amount )
register long amount;
{
	register long ptr;

	if( amount < 0L ) return -1L;

	if( amount == 0 ) return BP->p_hitpa;

	amount += 1L;
	amount &= 0xFFFFFFFE;

	if( amount > (Malloc( -1L ) - SAFETY) )
		ptr = 0L;
	else
		ptr = Malloc( amount );

	if( ptr == 0L ) return -1L;

	BP->p_hitpa = amount + ptr;

	return ptr;

}

char *
sbrk( amount )
int amount;
{
	return lsbrk( (long)amount );
}

