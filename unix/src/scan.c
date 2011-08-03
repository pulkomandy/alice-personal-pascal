/*
 *DESC: Scanner for tokens from keyboard input
 */

#include "alice.h"
#include "scan.h"
#include "token.h"
#include "class.h"
#include "alctype.h"
#include "keys.h"

#ifdef DB_SCAN
# define PRINTT
#endif
#include "printt.h"

inchar tokentext[MAX_LEN_LINE];
extern char *ScanPtr;
extern int domalloc;

/* Get the next character from the input stream */
static char
GetScanChar()
{
	char c = (*ScanPtr++ & 0x7f);
	printt2( "GetScanChar() '%c' = %x\n", c, c );
	return c;
}

/* Push so many characters back into the parser */
static
PushBack( num )
int num;
{
	ScanPtr -= num;
}

#define scanerror(x)

/*
 * This is a scanner, but used for non-interactive ("batch")
 * uses only.  It expects all tokens to be complete - if the end-of-file
 * comes and it's still reading a token that might continue (e.g identifier,
 * constant, string, comment) it ignores the token completely and returns
 * TOK_EOF.
 */
int
bscan()
{
	register int c;
	register char *cp;
	static int look_kw(reg char * str);

	int f;
	char delim;
	char	nk;

next:
	/*
	 * skip white space
	 */
	printt1( "Scan: '%s'\n", ScanPtr );

	while( (c = GetScanChar()) == ' ' || c == '\t' ) 
		;
	PushBack(1);

	/* Now we try to figure out what sort of token it is */

	cp = tokentext; 	/* Where we store the text of the token */

	for( c=0; c<MAX_LEN_LINE; c++ ) tokentext[c] = 0;

	/* Get the first character so that we can decide what it is */
	c = GetScanChar();
	tokentext[0] = c;

	/* is it an Identifier ? */
	if( isascii(c) && (isalpha( c ) || c == '_' ) ) {
		do {
			*cp++ = c;
			c = GetScanChar();
		} while (isascii(c) && (isalnum(c) || c == '_'));

		/* Terminate the token */
		*cp = 0;

		/* If CASEMAP is on, map uppercase to lowercase */
#ifdef CASEMAP
		for (cp = tokentext; *cp; cp++)
			if (*cp >= 'A' && *cp <= 'Z') {
				*cp |= ' ';
			}
#endif CASEMAP

		/* If we ended at the end of the string, just return
		 * the END OF FILE token
		 */
		if (c == 0) return TOK_EOF;
		PushBack(1);

		/* Now look up the text of the token in the table */
		c = look_kw(tokentext);

		/* If it wasn't there, then it must be an ID */
		if (c == 0) return (TOK_ID);

		/* Otherwise return the token number */
		return c;
		}

	/* Number: integer or floating point */
	else if( isascii(c) && isdigit(c) ) {
		f = 0;
		do {
			*cp++ = c;
			c = GetScanChar();
		} while (isascii(c) && isdigit(c));

		/* Now, was there a decimal point ? */

		if (c == '.') {	

			c = GetScanChar();
			/* If there are two dots... then move back
			 * and return the int
			 */
			if (c == '.') {	
				*cp = 0;
				/* Move back to the first dot */
				PushBack(2);
				return (TOK_INT);
			}
infpnumb:
			f++;

			/* Store the decimal point */
			*cp++ = '.';
			if (!isascii(c) || !isdigit(c)) {
				/* no digits after the decimal point
				 * let's add a zero for the poor user
				 */
				*cp++ = '0';
			} else
				while (isdigit(c)) {
					*cp++ = c;
					c = GetScanChar();
				}
		}
		if (c == 'e' || c == 'E') {
			f++;
			*cp++ = c;
			c = GetScanChar();
			if (c == '+' || c == '-') {
				*cp++ = c;
				c = GetScanChar();
			}
			if (!isascii(c) || !isdigit(c)) {
				scanerror("Digits required in exponent");
				*cp++ = '0';
			} else
				while (isascii(c) && isdigit(c)) {
					*cp++ = c;
					c = GetScanChar();
				}
		}
		*cp = 0;
		printt1( "Terminating char = %d\n", c );
		if (c == 0) return TOK_EOF;

		PushBack(1);
		/* If it is a floating point number, return NUMB
		 * otherwise return an INT
		 */
		if (f)
			return (TOK_NUMB);
		return (TOK_INT);
		}
	/* Various punctuation tokens */
	switch (c) {
	case 0:
		return TOK_EOF;
	/* Strings */
	case '"':
	case '\'':
		{
		int slen;

		delim = c;
		*cp++ = delim;	/* flag to hold delim - no closing quote kept */
		slen = 0;
		/* Now we want to keep on getting chars until end */
		for(;;) {
			c = GetScanChar();
			if( c == delim ) {
				c = GetScanChar();
				if( c == delim ) {
					*cp++ = delim;
					slen++;
					continue;
					}
				if ( c ) {
					PushBack(1);
					break;
					}
				}
			if( c == 0 ) {
				/* If this is the final parse, it's an error*/
				if( domalloc ) {
					PushBack(1);
					return TOK_ILLCH;
					}
				/* Otherwise, just return EOF */
				return TOK_EOF;
				}
			*cp++ = c;
			slen++;
			}
		*cp++ = 0;
#ifdef Notdef
		if( slen == 0 && delim == '\'' ) {
			scanerror( "Null length strings not allowed" );
			tokentext[1] = ' ';
			tokentext[2] = 0;
			slen = 1;
			}
#endif
		printt2("scan string, value ->%s<-, type %s\n", tokentext,
		   delim == '"' || strlen(tokentext) != 2 ? "string" : "char");
		/* strlen==2 means 1 for ' plus one for the char */
		if (delim == '"' || strlen(tokentext) != 2)
			return (TOK_STRING);
		else
			return (TOK_CHAR);
		}
	case '.':
		c = GetScanChar();
		/* Check for .. */
		if (c == '.') {
			*cp++ = c;
			*cp = 0;
			return (TOK_DOTDOT);
		}
		if (isdigit(c)) {
			scanerror("Digits required before decimal point");
			*cp++ = '0';
			goto infpnumb;
		}
		PushBack(1);
		return (TOK_DOT);
	case '{':
		/*
		 * { ... } comment
		 */
		/* *cp++ = c; */
		c = GetScanChar();
		while (c != '}') {
			if (c == 0)
				return TOK_EOF;
			*cp++ = c;
			c = GetScanChar();
		}
		*cp++ = 0;
		return TOK_CMNT;
	case '(':
		return TOK_LPAREN;
	case ';':
		return TOK_SEMI;
	case ',':
		return TOK_COMMA;
	case ':':
#ifdef notdef
		c = GetScanChar();
		if( c == 0 ) return TOK_EOF;

		if( c == '=' ) {
			*++cp = '=';
			*cp = 0;
			return TOK_ASSIGN;
		}

		PushBack(1);
#endif
		return TOK_COLON;

	case '_':
		return TOK_ASSIGN;
	case '=':
		return TOK_EQ;
	case '*':
		return TOK_STAR;
	case '+':
		return TOK_PLUS;
	case '/':
		return TOK_SLASH;
	case '-':
		return TOK_MINUS;
	case ')':
		return TOK_RPAREN;
	case '[':
		return TOK_LBRACKET;
	case ']':
		return TOK_RBRACKET;
	case '?':
		return TOK_QUESTION;
	case '<':
		if ((c=GetScanChar()) == '=') {
			*++cp = c;
			return TOK_LE;
		}
		if (c == '>') {
			*++cp = c;
			return TOK_NE;
		}
		if (c == 0) return TOK_EOF;
		PushBack(1);
		return TOK_LT;
	case '>':
		if ((c=GetScanChar()) == '=') {
			*++cp = c;
			return TOK_GE;
		}
		if (c == 0)
			return TOK_EOF;
		PushBack(1);
		return TOK_GT;
	case '^':
		return TOK_UPARROW;
#ifdef TURBO
	case '$':
		while( isxdigit((c = GetScanChar())) )
			*++cp = c;
		if( c == 0 )
			return TOK_EOF;
		PushBack(1);
		return TOK_INT;
	case '#':
		while( isdigit( (c = GetScanChar() ) ) )
			*++cp = c;
		tokentext[1] = (bits8)atoi( tokentext+1 );
		if( c == 0 )
			return TOK_EOF;
		PushBack(1);
		return TOK_CHAR;
#endif

	default:
		if (c <= 0)
			return (TOK_EOF);
		while ( (nk = GetScanChar()) == c);
		PushBack(1);
		printt1("illegal char in scanner %o\n", c);
		return (TOK_ILLCH);
	}
}

#ifdef TURBO
bits8 extra_tokens[] = { TOK_STTYPE, TOK_SHL, TOK_SHR, TOK_XOR, TOK_ABSOLUTE, 0 };
#endif

struct kwtab {
	char *word;
	int ttype;
} kwtab[] = {
	"..",		TOK_DOTDOT,
	":=",		TOK_ASSIGN,
	"<Enter>",	TOK_CR,
	"Real Constant",TOK_NUMB,
	"(",	TOK_LPAREN,
	";",	TOK_SEMI,
	",",	TOK_COMMA,
	":",	TOK_COLON,
	"*",	TOK_STAR,
	"+",	TOK_PLUS,
	"/",	TOK_SLASH,
	"-",	TOK_MINUS,
	")",	TOK_RPAREN,
	"[",	TOK_LBRACKET,
	"]",	TOK_RBRACKET,
	"=",	TOK_EQ,
	"<",	TOK_LT,
	"<=", TOK_LE,
	"<>", TOK_NE,
	">",	TOK_GT,
	">=", TOK_GE,
	"^",	TOK_UPARROW,
	"writeln(",	TOK_QUESTION,
	".",		TOK_DOT,
	"String Constant",	TOK_STRING,
	"Char Constant", TOK_CHAR,
	"{Comment}",	TOK_CMNT,
	"Integer Constant",	TOK_INT,
	"A Symbol",	TOK_ID,
	"Illegal Char",	TOK_ILLCH,
	/* this must be approximately the start */
#define START_REAL 27
#ifdef TURBO
	"absolute",	TOK_ABSOLUTE,
#endif
	"and",		TOK_AND,
	"array",	TOK_ARRAY,
	"begin",	TOK_BEGIN,
	"case",		TOK_CASE,
	"const",	TOK_CONST,
	"div",		TOK_DIV,
	"do",		TOK_DO,
	"downto",	TOK_DOWNTO,
	"else",		TOK_ELSE,
	"end",		TOK_END,
	"file",		TOK_FILE,
	"for",		TOK_FOR,
	"function",	TOK_FUNCTION,
	"goto",		TOK_GOTO,
	"if",		TOK_IF,
	"in",		TOK_IN,
	"label",	TOK_LABEL,
	"mod",		TOK_MOD,
	"nil",		TOK_NIL,
	"not",		TOK_NOT,
	"of",		TOK_OF,
	"or",		TOK_OR,
	"packed",	TOK_PACKED,
	"procedure",	TOK_PROCEDURE,
	"program",	TOK_PROG,
	"record",	TOK_RECORD,
	"repeat",	TOK_REPEAT,
	"set",		TOK_SET,
#ifdef TURBO
	"shl",		TOK_SHL,
	"shr",		TOK_SHR,
	"string",	TOK_STTYPE,
#endif
	"then",		TOK_THEN,
	"to",		TOK_TO,
	"type",		TOK_TYPE,
	"until",	TOK_UNTIL,
	"var",		TOK_VAR,
	"while",	TOK_WHILE,
	"with",		TOK_WITH,
#ifdef TURBO
	"xor",		TOK_XOR,
#endif
	0,		0,
};


/*
 * Return the character string name for a given token number.
 * This is mostly useful for debugging.
 */
char *
tokname(tnum)
int tnum;
{
	register int i;
	static char retbuf[LONG_KEYWORD+5];

	for( i = 0; i < sizeof(kwtab)/sizeof(kwtab[0]); i++ )
		if( kwtab[i].ttype == tnum ) {
			return kwtab[i].word;
			}
	sprintf(retbuf, "?<%d>?", tnum);
	return retbuf;
}
	
/* Look up a keyword in the table above, returning the token number or 0 */
static int
look_kw(str)
reg char *str;
{
	register int l;
	char lstr[LONG_KEYWORD+2];		/* suitable to hold a keyword */
	char *lcpy;
	int h, m, r;
#ifdef TURBO
	extern int turbo_flag;
#endif

	if(strlen(str) > LONG_KEYWORD )
		return 0;
	 else {
		strcpy( lstr, str );
		/* lower case lstr */
		for( lcpy = lstr; *lcpy; lcpy++ )
			*lcpy |= 0x20;
		}

	l=START_REAL; h=sizeof(kwtab) / sizeof(kwtab[0]) - 2;
	while (l <= h) {
		m = (l+h)/2;
		r = strcmp(kwtab[m].word, lstr);
		if (r < 0)
			l = m+1;
		else if (r > 0)
			h = m-1;
		else {
#ifdef TURBO
			/* pass over extended reserved words */
			if( !turbo_flag && strchr( extra_tokens, kwtab[m].ttype ) )
				return 0;
#endif
			return kwtab[m].ttype;
			}
	}
	return 0;
}

/*
 * Prompt for one token from keyboard, then pass to in_token.
 */
key_token()
{
	char tokbuf[256];
	register int tn;

	getprline("token: ", tokbuf, 255);
	setmac(0);
	scaninptr = tokbuf;
	printt1("key_token, got string '%s'\n", tokbuf);
	tn = bscan();
	setmac(1);
	printt1("key_token, got token %d\n", tn);
	in_token(tn,cursor);
}
