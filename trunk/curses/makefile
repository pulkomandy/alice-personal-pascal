
CFLAGS= -DCURSES_VERSION -DFAST_DRAW -I\alice\h

DRAWOBJ = wrefresh.o draw_src.o mono_src.o multi_src.o char.o curinit.o \
	grefresh.o clip.o scroll.o data.o

CURLIBS = $(DRAWOBJ) win.o tty.o winsert.o wadd.o wattr.o misc.o wclr.o \
	  wget.o wmove.o

curses.a: $(CURLIBS)
	ar r c:\lib\libcurses.a $?
	touch curses.a

test.prg: test.o curses.a
	cc test.o -lcurses -laes -lvdi -o test.prg
