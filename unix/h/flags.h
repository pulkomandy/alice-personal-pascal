/* These flags go on node TYPES, not on nodes themselves */

/* list children are indented */
#define F_INDENT 1
/* has a symbol table child */
#define F_SYMBOL 2
/* takes up a line on its own */
#define F_LINE 4
/* can have a child that is actually a multi-liner (record) */
#define F_PMULTI 8
/* the cursor should not stop here */
#define F_NOSTOP 0x10
/* The first child should be executed immediately */
#define F_E1KID 0x20
/* The first 2 children should be executed immediately */
#define F_E2KIDS 0x40
/* There is a final kid with type information in it */
#define F_TYPE 0x80
/* this guy has declarations under him that must be deleted if he is deleted */
#define F_DECLARE 0x100
/* this guy is a full fledged scope */
#define F_SCOPE 0x200

/* These flags get stored on individual nodes in the tree, in the flags1
   field and the flags2 field */

#define NF_STANDOUT 1
/* 2 bits for 4 modes of dereferencing 
   00 - is an lvalue
   01 - is an rvalue
   10 - structured type rvalue
   11 - is an rvalue and must be made real
*/
#define NF_VALMASK 6
/* the value here should be an rvalue not an lvalue */
#define NF_RVALUE 2
/* the value here must be converted from integer to real when ready */
#define NF_RCONVERT 6
/* is a structured type to be pushed */
#define NF_STPUSH 4
/* this node contains an error of some sort */
#define NF_ERROR 8
/* This block has been typechecked */
#define NF_TCHECKED 0x10
/* this constant is malformed */
#define NF_BADCONST 0x10
/* this array is a string */
#define NF_STRING 0x10
/* this string pointer should be incremented */
#define NF_ISTRING 0x10

/* flags about symbols */

#define	NF_TREEUSED	0x40	/* This decl subtree was used (parser) */

/* this declaration was redclared in this symbol table */
#define	NF_REDECLARED	NF_ERROR
/* this declaration is the real declaration (in case of multiple declarations */
#define NF_IS_DECL	0x20
/* overload this bit with special bit saying passed function */
#define NF_PASSFUNC	0x20
/* this is one of the special array indexes */
#define NF_FUNNYVAR	0x20
/* this char node must be promoted to string */
#define NF_CHRSTR	0x80
/* types of breakpoints */
#define NF_BREAKPOINT 0x40
#define NF_PRINTOINT 0x80

/* in flag2 indicates there are extra words you must ignore in save file
    if you don't understand them */
#define NF2_EXTRA	0x01
#define NF2_KEEP_HIDDEN	0x02
/* overload with parameter flag meaning generic var param */
#define NF2_GENERIC	0x02


/* Symbol flags that go in the extra symbol flags field of declared symbol
 * nodes only.
 */


/* this symbol has been defined */
#define SF_DEFYET	0x20
/* this symbol was used before it was defined */
#define SF_DEFOERR	0x08
/* this real constant is negated */
#define SF_NEGREAL 	0x40
/* large model only symbol */
#define SF_LARGE	0x80
#define SF_ISFORWARD	0x10
/* routine is hidden */
#define SF_HIDDEN	0x20
#define SF_REFERENCED	0x04
/*
 * SF_LIBREF is used by apin.  The flag is set on DECL_IDs in the library
 * when they are referenced by the program being merged.
 */
#define	SF_LIBREF	0x02
#define	SF_MARKED	0x01

/* load/save flags */
#define	LD_LIBRARY	1	/* load the file as a library */
