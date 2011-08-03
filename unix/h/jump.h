
/* Different operating systems have different exception handling */

#include <setjmp.h>

extern jmp_buf exit_buf;
extern jmp_buf err_buf;

/* unix routines to simulate icon jumps */
#define setexit() !setjmp( exit_buf )

#define reset() longjmp( exit_buf, 1 )

#define errset() !setjmp( err_buf )

#define err_return() longjmp( err_buf, 1 )
