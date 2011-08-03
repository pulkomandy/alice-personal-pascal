#define clearla()	if (lookaheadchar == ' ') lookaheadchar = '\0';

extern int lookaheadchar;		/* should be an inchar */
extern inchar *macp;			/* If in a macro, this is what's left */
extern inchar *cp;			/* pointer into token text */
extern inchar *scaninptr;		/* For force feeding input */
extern int cantbegexp;		/* "Can't begin with this token number" flag */
extern int canttokno;		/* Token number of token input if cantbegexp */

/* Controllify a character, e.g. CTRL('X') gives control X (030) */
#ifdef CTRL
#undef CTRL
#endif
#define CTRL(c) (c & 037)

#define EOFCHAR 0377		/* End of file (internal end of macro) */

struct token {
	bits8 token_no;		/* token number from token.h */
	bits8 charoff;		/* char offset in oltext of beg of token */
	char *ttext;		/* text of token */
};
