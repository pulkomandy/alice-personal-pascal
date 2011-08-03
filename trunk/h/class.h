 /* cpp definitions */
#define CLIST(arg) arg+64
#define CLASSMASK 63


/*
 * Classes - these are the actions that are taken for a given token
 * when the cursor is on an unexpanded stub of the given class.
 */

#define C_ROOT 0

#define C_COMMENT 1

#define C_INIT 2

#define C_LABEL 3

#define C_PROC_ID 4

#define C_FUN_ID 5

#define C_PROGRAM 6

#define C_DECLARATIONS 7

#define C_LABEL_DECL 8

#define C_CONST_DECL 9

#define C_TYPE_DECL 10

#define C_VAR_DECL 11

#define C_CONSTANT 12

#define C_TYPE 13

#define C_TYPEID 14

#define C_SIM_TYPE 15

#define C_ST_TYPE 16

#define C_FIELD 17

#define C_FORMAL 18

#define C_STATEMENT 19

#define C_CASE 20

#define C_VAR 22

#define C_EXP 23

#define C_CASECONST 24

#define C_DECL_ID 25

#define C_HIDECL_ID 26

#define C_BLCOMMENT 27

#define C_VARIANT 28

#define C_FLD_NAME 29

#define C_PASSUP 30

#define C_OCONSTANT 31

#define C_PNAME 32

#define C_SPECIAL 33

#define C_ROUTNAME 34

#define C_ABSID 35
