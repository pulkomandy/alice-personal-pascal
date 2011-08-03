#include <whoami.h>
#include <stdio.h>

#ifndef QNX
#include <ctype.h>
#endif

#include "action.h"
#include "node.h"
#include "token.h"
#include "flags.h"
#include "class.h"

#define UT_ANY		0
#define UT_EXACT	0x10
#define UT_UPKID	0x20
#define UT_PCODE	0x40

typedef struct tabl {
		char *s;
		int  n;
	} table;

#define TAB_END (char *)-1

#define TN_DEFINED	1
#define TN_NAME		2
#define TN_KIDMATRIX	4
#define TN_TEMPLATE	8
#define TN_PRECEDENCE	16
#define TN_FLAGS	32
#define TN_INFIX	64
#define TN_NODECOUNT	128
#define TN_ACTIONS	256
#define TN_CLASS	512

#define TN_NEEDED (TN_DEFINED | TN_NAME | TN_KIDMATRIX | TN_TEMPLATE | TN_FLAGS | TN_INFIX | TN_NODECOUNT | TN_ACTIONS | TN_CLASS )


/* A value higher than all the other classes */
#define C_LIST		64
#define C_DESCEND	128+C_LIST
#define C_DUPLICATE	128
