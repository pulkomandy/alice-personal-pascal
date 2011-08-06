#
# Makefile for 520ST Alice
#

#
# The list of Source Files:
#

GEMSRC = stevent.c stinit.c stlib.c stfile.c stcmd.c stwindow.c \
	 stpop.c sthelp.c stgraph.c stsearch.c stimage.c \
	 stdialog.c stmenu.c sttrap.c stunix.c newalloc.c

ISRC = trace.c run2.c runfile.c types.c setup.c interp.c runtime.c \
	typechec.c function.c cfunc.c fullfunc.c
NISRC = nulli.c
CSRC = classes.c scan_dec.c common.c load_nod.c
ESRC =	main.c treesubr.c scan.c subr.c helpfile.c \
	workspac.c search.c menu.c menudat.c load.c save.c \
	history.c mark.c text.c text2.c oledit.c ole.c keyboard.c \
	expparse.c cmds2.c help.c alsym.c \
	treeprin.c output.c outfuncs.c display.c dump.c \
	command.c action.c actrout.c actrout2.c	builtin.c filename.c \
	mem.c kids.c mainloop.c lowstate.c expstate.c support.c

SRC = $(ESRC) $(ISRC) $(CSRC) $(GEMSRC)

#
# The Object
#

GEMOBJS= stevent.o stinit.o stlib.o stfile.o stcmd.o stwindow.o \
	 stpop.o sthelp.o stgraph.o stsearch.o stimage.o \
	 stdialog.o stmenu.o stuser.o sttrap.o stasm.o stunix.o cu.o \
	 stload.o stmdat.o istate.o stdevice.o strsc.o

LANG = lang.o

# GEMOBJS = dummy.o
INTERP = interp.o

IOBJS = trace.o run2.o runfile.o types.o setup.o runtime.o \
	typechec.o function.o cfunc.o fullfunc.o newalloc.o
NIOBJS = nulli.o
COBJS = classes.o scan_dec.o common.o load_nod.o
EOBJS=	main.o treesubr.o scan.o subr.o helpfile.o \
	workspac.o search.o menu.o load.o save.o \
	history.o mark.o text.o text2.o oledit.o ole.o keyboard.o \
	expparse.o cmds2.o help.o \
	treeprin.o output.o outfuncs.o display.o dump.o \
	command.o action.o actrout.o actrout2.o	builtin.o filename.o \
	mem.o kids.o mainloop.o support.o\
	lsbrk.o

OTHOBJS = alsym.o

OBJS= $(EOBJS) $(IOBJS) $(COBJS) $(GEMOBJS) $(OTHOBJS) $(LANG) $(INTERP)

# STUPID = -DSTUPID_COMPILER

DEFINES = -DLARGE -DTURBO -DFULL -DTrace $(STUPID) -DOLMSG \
    	-DLOAD_ERRORS -DLOADABLE_TEMPLATES -DFLOATING \
	-DASM_ICORE -DEMBEDDED_RCS -DGEM

CFLAGS=	$(DBG) -I../h -I../../libmad/ -O $(DEFINES)

# Compile it as a GEM application
LFLAGS= -lcurses -laes -lvdi -lm -f

foo.prg: $(OBJS) makefile
	cc $(OBJS) $(LFLAGS) -o alice.prg

istate.o: istate.s
	as -gx istate.s -o istate.o

#cu.o: cu.s
#	as -gx cu.s -o cu.o

stasm.o: stasm.s
	as -gx stasm.s -o stasm.o

backup: $(SRC)
	kermit s $?
	touch backup

link:
	cc -o alice.prg $(OBJS) unix.o $(LFLAGS)

interp.o: \alice\h\interp.h ilowstat.c mlowstat.c interp.c
	cc $(CFLAGS) -c interp.c

dumpnode.prg: dumpnode.o dumpload.o
	cc dumpnode.o dumpload.o -o dumpnode.prg
