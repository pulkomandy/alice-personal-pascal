
#include <stdio.h>
#include <obdefs.h>
#include <osbind.h>

int CurHires[] = {
	0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15
	};

int CurMedres[] = {
	0, 3, 1, 2, 3, 3, 3, 3, /* white to magenta */
	0, 3, 1, 2, 3, 3, 3, 3, /* lwhite to lmagenta */
	};

int CurLores[] = {
	0, 15, 1, 2, 4, 6, 3, 5,
	7, 8, 9, 10, 12, 14, 11, 13
	};

int CurColMap[16];

init_colmap()
{

	int rez;
	int i;
	int *ptr;

	rez = Getrez();

	if( rez == 2 )
		ptr = CurHires;
	else if( rez == 1 )
		ptr = CurMedres;
	else
		ptr = CurLores;

	for( i=0; i<16; i++ ) {
		CurColMap[i] = ptr[i];
	}
}

