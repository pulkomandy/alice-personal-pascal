
/* There are many different types of identifiers in a program.  These are
 * the code numbers for them.
 */
#define T_LABEL 1
#define T_CONST	2
#define T_ENUM_ELEMENT 3
#define T_TYPE 4
#define T_VAR 5
#define T_BTYPE 6
#define T_BBTYPE 7
#define T_UNDEF 8
#define T_FIELD 9
#define T_FILENAME 10
#define T_FORM_VALUE 11
#define T_FORM_REF 12
#define T_FORM_PROCEDURE 13
#define T_FORM_FUNCTION 14
#define T_PROCEDURE 15
#define T_FUNCTION 16
#define T_USERCALL T_FUNCTION		
#define T_BTFUNC 17
#define T_BTPROC 18
#define T_BPROC 19
#define T_BFUNC 20
#define T_WRITELN 21
#define T_READLN 22
#define T_PROGRAM 23
#define T_BCONST 24
#define T_ABSVAR 25
#define T_INIT 26
#define T_BENUM 27
#define T_MEMVAR 28
#define T_PORTVAR 29
#define T_LFUNC 30
#define T_LPROC 31

#define CC_SUBRANGE 32


#define CC_BYTE 1
#define CC_INTEGER 2
#define CC_POINTER 3
#define CC_REAL 4
#define CC_SET 5
#define CC_STRING 6
#define CC_GENERIC 7
#define CC_STRPOINT 8
#define CC_CHRSTR 9
#define CC_ARRAY 10
#define CC_FILE 11
#define CC_RECORD 12
#define CC_ENUM 13
#define CC_ORDVALUE 14
#define CC_REALVALUE 15
#define CC_PORT 16
#define CC_PBYTE CC_PORT+CC_BYTE
#define CC_PWORD CC_PORT+CC_INTEGER
