#include "alice.h"

#ifndef RELEASE
# define PRINTT
#endif
#include "printt.h"

#ifndef QNX
min(x, y)
int	x;
int	y;
{
	if (x <= y)
		return x;
	else
		return y;
}

max(x, y)
int	x;
int	y;
{
	if (x >= y)
		return x;
	else
		return y;
}
#endif

mfree(p)
char	*p;
{
	printt1("MEM: %lx freed\n", p);
	free(p);
}

/* dummy memory cleanup vector */
funcptr mem_cleanup = 0;
/* Extra routines from Alice editor needed by lister */

char *
checkalloc(size)
int	size;
{
	char *calloc(), *p;

	p = calloc(1, size);
	if (p == NULL)
		fatal("Not enough memory to convert program");

	printt2("MEM: %lx alloc (%d)\n", p, size);

	return p;
}

char *
trealloc( ptr, oldval, newval )
char *ptr;
int oldval, newval;
{
	char *realloc(), *p;
#ifdef QNX
	free( ptr );
#endif

	printt3("checkrealloc(ptr=%x, oldval %d, newval %d\n", ptr, oldval,
		newval);

	p = realloc(ptr, newval);
	if (p == NULL)
		fatal("out of memory (realloc)");
	return p;
}

char *
allocstring(str)
char *str;
{
	char	*new = checkalloc(strlen(str) + 1);

	strcpy(new, str);

	printt2("allocstring(%s) returns %lx\n", str, new);
	return new;
}

bug( str, a1, a2, a3, a4, a5 )
{
	fprintf( stderr, "Internal error:" );
	printt0("\nInternal error: ");
	fatal( str, a1, a2, a3, a4, a5 );
}

char *turbolib[] = {
"crtexit",
"crtinit",
"highvideo",
"msdos",
"nosound",
"erase",
"flush",
"randomize",
"move",
"fillchar",
"rename",
"chdir",
"mkdir",
"rmdir",
"getdir",
"mark",
"release",
"blockread",
"hi",
"lo",
"swap",
"wherex",
"wherey",
"dseg",
"cseg",
"sseg",
"addr",
"ptr",
"ofs",
"seg",
"heapptr",
"filepos",
"filesize",
"seekeof",
"seekeoln",
"longfIlesize",
"longfileposition",
"longseek",
"blockread",
"blockwrite",
"chain",
"execute",
"paramcount",
"paramstr",
"black",
"blue",
"green",
"cyan",
"red",
"magenta",
"brown",
"lightgray",
"darkgray",
"lightblue",
"lightgreen",
"lightcyan",
"lightred",
"lightmagenta",
"yellow",
"white",
"blink",
"bw40",
"c40",
"bw80",
"c80",
"arc",
"circle",
"colortable",
"fillscreen",
"fillshape",
"fillpattern",
"getpic",
"pattern",
"putpic",
"getdotcolor",
"back",
"clearscreen",
"forw",
"hideturtle",
"home",
"nowrap",
"pendown",
"penup",
"setheading",
"setpencolor",
"setposition",
"showturtle",
"turnleft",
"turnright",
"turtlewindow",
"wrap",
"heading",
"xcor",
"ycor",
"turtlethere",
"north",
"east",
"south",
"west",
0
};

char *undone_turbo[] = {
"aux",
"auxinptr",
"auxoutptr",
"buflen",
"con",
"coninptr",
"conoutptr",
"constptr",
"lst",
"lstoutptr",
"mem",
"port",
"portw",
"memw",
"trm",
"usr",
"usrinptr",
"usroutptr",
"exit",
"ovrpath",
"inp",
"out",
"err",
"form",
"ovrdrive",
"bdos",
"bdoshl",
"bios",
"bioshl",
0
};

scanids( str, tab )
char *str;
char **tab;
{
	char **tp;

	for( tp = tab; *tp; tp++ )
		if( case_equal( str, *tp ) )
			return TRUE;
	return FALSE;
}

extern char undstring[];	/* undefined symbol error message */

/* check for unknown turbo id */
#ifdef GERMAN
char seeman[] ="Anhang des Benutzerhandbuches fuer Hinweise.\n";
#else
char seeman[] ="See Appendix H in the User Guide for more details.\n";
#endif

static int hadlib = FALSE;
extern int smallflag;
static int hadnotsupported = FALSE;
turboid( id )
char *id;
{

	if( scanids( id, turbolib ) ) {
		if( hadlib ) 
			nonfatal( "Symbol '%s' undefined, try ALICE's Turbo Pascal library\n",id);
		 else {
#ifdef GERMAN
			nonfatal("The Turbo Pascal symbol '%s' is not built into ALICE,\n",id);
			fprintf(stderr,"but exists in Pascal source form for inclusion in your Turbo Pascal program.\n" );
			if( smallflag )
				fprintf(stderr,"You might also try the LARGE model ALICE.\n" );
			fprintf(stderr,seeman);
#else
			nonfatal("The Turbo Pascal symbol '%s' is not built into ALICE,\n",id);
			fprintf(stderr,"but exists in Pascal source form for inclusion in your Turbo Pascal program.\n" );
			if( smallflag )
				fprintf(stderr,"You might also try the LARGE model ALICE.\n" );
			fprintf(stderr,seeman);
#endif
			hadlib = TRUE;
			}
		}
	else if( scanids( id, undone_turbo ) ) {
#ifdef GERMAN
		unsupported("Turbo Pascal symbol '%s' is not supported\n", id);
		if (!hadnotsupported ) {
			fprintf(stderr,"It will be necessary to modify your program for Alice Pascal\n");
#else
		unsupported("Turbo Pascal symbol '%s' is not supported\n", id);
		if (!hadnotsupported ) {
			fprintf(stderr,"It will be necessary to modify your program for Alice Pascal\n");
#endif
			fprintf(stderr, seeman );
			hadnotsupported = TRUE;
			}
		}
	else 
		nonfatal( undstring, id );

}

