
#include "alice.h"

#ifdef HAS_LIBRARY

/*
 * Various routines to handle libraries (particularly binary libraries)
 */

typedef struct {
	unsigned int blocksize;	/* Size of the code block */
	unsigned int usecount;  /* Reference count */
	pointer initcode;	/* Ptr to Initialization code */
	pointer termcode;	/* Ptr to Terminatation code */
	pointer	codeblock;	/* Pointer to the code block */
	pointer spare[15];	/* Room for expansion */
	} BinInfo;

/*
 * If there is no library block, then return
 */
freelib( ptr )
BinInfo *ptr;
{
	if( !ptr )
		return;

	/*
	 * If we are not copying the code, but using a reference count
	 * then just decrement the count
	 */
#ifndef REAL_COPY
	if( ptr->usecount > 1 ) {
		ptr->usecount--;
		return;
	}
#endif

	if( ptr->termcode )
		( * (funcptr)( ptr->termcode ) ) ();

	if( ptr->codeblock )
		mfree( ptr->codeblock );

	mfree( ptr );
}

BinInfo *
libcopy( ptr )
BinInfo *ptr;
{
	BinInfo *cpy;
	char *newblock;

	if( !ptr )
		return (BinInfo *)0;

#ifndef REAL_COPY
	ptr->usecount++;
	return ptr;
#else
	cpy = (BinInfo *) checkalloc( sizeof( BinInfo ) );
	memcpy( cpy, ptr, sizeof( BinInfo ));

	newblock = (char *) checkalloc( (unsigned) ptr->blocksize );

	memcpy( newblock, ptr->codeblock, ptr->blocksize );

	cpy->codeblock = newblock;	

	/* Make the init/term pointers relative to the new image */
	cpy->termcode = (ptr->termcode - ptr->codeblock) + cpy->codeblock;
	cpy->initcode = (ptr->initcode - ptr->codeblock) + cpy->codeblock;

	return cpy;
#endif REAL_COPY

}

/*
 * Load a binary library for a N_LIBRARY node
 */
BinInfo *
libfload( fp, p )
FILE *fp;		/* The file pointer to load it from */
char **p;		/* A pointer to the allocated block */
{
	BinInfo *ptr;
	char *ret;
	extern char *lbfload();

	/* Allocate the information block */
	ptr = (BinInfo * ) checkalloc( sizeof( BinInfo ) );

	/* Read in the information from the file */
	fread( ptr, sizeof( BinInfo ), 1, fp );

#ifndef REAL_COPY
	ptr->usecount = 1;
#endif

	/* Now load the executable */
	ret = lbfload( fp );

	/*
	 * Add on the address of the binary
	 */
	ptr->initcode = (char *)((long)ptr->initcode + (long)ret);
	ptr->termcode = (char *)((long)ptr->termcode + (long)ret);

	/* Set up the code block pointer */
	ptr->codeblock = ret;

	*p = ret;

	return ptr;
}
#endif HAS_LIBRARY
