
/* printt functions are debug printf functions that are found all over
 * the code.   You have to include the number of args in the macro name
 * because of the way CPP macros work.   Undefine DEBUG and these all
 * go away.
 */
#ifdef PRINTT
# define printt0(f) (tracing? fprintf(dtrace, f) :0)
# define printt1(fmt,a) (tracing? fprintf(dtrace, fmt, (a)) :0)
# define printt2(fmt,a,b) (tracing? fprintf(dtrace, fmt, (a), (b)) :0)
# define printt3(fmt,a,b,c) (tracing? fprintf(dtrace, fmt, (a), (b), (c)) :0)
# define printt4(fmt,a,b,c,d) (tracing? fprintf(dtrace, fmt, (a), (b), (c), (d)) :0)
# define printt5(fmt,a,b,c,d,e) (tracing? fprintf(dtrace, fmt, (a), (b), (c), (d), (e)) :0)
# define printt6(fmt,a,b,c,d,e,f) (tracing? fprintf(dtrace, fmt, (a), (b), (c), (d), (e), (f)) :0)
# define printt8(fmt,a,b,c,d,e,f,g,h) (tracing? fprintf(dtrace, fmt, (a), (b), (c), (d), (e), (f), (g), (h)) :0)
# define flusht()	(tracing? fflush(dtrace) :0)
#else
# define printt0(f)
# define printt1(fmt,a)
# define printt2(fmt,a,b)
# define printt3(fmt,a,b,c)
# define printt4(fmt,a,b,c,d)
# define printt5(fmt,a,b,c,d,e)
# define printt6(fmt,a,b,c,d,e,f)
# define printt8(fmt,a,b,c,d,e,f,g,h)
# define flusht()
#endif
