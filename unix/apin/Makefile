
OPT=

HDIR=../h

DOBJECTS= main.o y.tab.o lex.o support.o extra.o lib.o
SRCOBJS= treesubr.o alsym.o save.o load_nodes.o load.o
SAMEOBJS= builtin.o dump.o kids.o classes.o
OBJECTS= $(DOBJECTS) $(SRCOBJS) $(SAMEOBJS)

DSOURCES= main.c pas.y lex.c support.c extra.c nullfunc.c lib.c
SRCSRCS= treesubr.c alsym.c save.c load.c load_nodes.c
SAMESRCS= ../builtin.c ../dump.c
SOURCES= $(DSOURCES) $(SRCSRCS) $(SAMESRCS)

CFLAGS =-I$(HDIR) -DINT32 -DPOINT32 -DTURBO -DFULL -DPARSER -DLOADABLE_TEMPLATES $(OPT)
LFLAGS =

apin: $(OBJECTS) Makefile
	cc $(LFLAGS) -o apin $(OBJECTS) 

y.tab.c: pas.y
	@echo 'expect 30(?) shift/reduce conflicts...'
	yacc -dv pas.y

clean:
	rm -f *.o y.tab.c apin core atr* errs

tags:
	ctags -w $(HDIR)/*.h $(SOURCES)
