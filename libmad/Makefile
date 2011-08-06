#
# This Makefile is not exactly a shining example of consistency.
# Nor is it very tidy.
#

CC=	gcc
WARN_EXT=	.warn
CLEAN_WF=	1
CFLAGS=	-g -Wall

ORCS_FILES= madorcs.c madrsr.c
MAD_FILES=	adraw.c ainit.c menu.c amouse.c atos.c avmisc.c gtext.c resource.c form.c event.c fsellin.c

prefix=	/usr/local
LIB_DIR=	$(prefix)/lib
BIN_DIR=	$(prefix)/bin
HEAD_DIR=	$(prefix)/include

XLIBDIR=	/usr/X11R6/lib
XLIBS=  -lXxf86dga -lXxf86vm -lXext -lX11
SLIBS= -lvga
LIBS=	libmad.a -lalleg -lalleg_unsharable $(SLIBS) -L$(XLIBDIR) $(XLIBS)

all:	libmad.a madorcs

libmad.a:	$(MAD_FILES)
	@echo 'compiling libmad.a'
	@$(CC) -c $(MAD_FILES) -Wall 2>libmad$(WARN_EXT) && \
		echo '	libmad.a compiled, building...' || \
		echo '	error compiling libmad.a'
	@ar -rs libmad.a `echo $(MAD_FILES) | sed s/\\\.c/\\\.o/g ` 1>&2 2>>libmad$(WARN_EXT) && \
		echo '	libmad.a built' || \
		echo '	error building libmad.a'

madorcs:	$(ORCS_FILES)
	@echo 'compiling madorcs'
	@$(CC) $(CFLAGS) -c $(ORCS_FILES) 2>madorcs$(WARN_EXT) && \
		echo '	madorcs compiled, building...' || \
		echo '	error compiling madorcs'
	@$(CC) -o madorcs $(CFLAGS) $(ORCS_FILES) $(LIBS) 2>>madorcs$(WARN_EXT) && \
		echo '	madorcs built' || \
		echo '	error building madorcs'
	cp allegro.lin allegro.cfg

test:
	./madorcs

install:
	-install -s -g users -o root -m 4755 madorcs$(PROG_EXT) $(BIN_DIR)
	-install -g users -o root -m 4644 libmad.a $(LIB_DIR)
	-install -g users -o root -m 4644 osbind.h $(HEAD_DIR)
	-install -g users -o root -m 4644 vdi.h $(HEAD_DIR)
	-install -g users -o root -m 4644 aes.h $(HEAD_DIR)

uninstall:
	@if [ ! `whoami` = 'root' ] ; then echo 'must run as root' ; exit ; fi
	@echo "uninstalling libmad.a in $(LIB_DIR)"
	@if [ -f $(LIB_DIR)/libmad.a ] ; then rm -f $(LIB_DIR)/libmad.a 2>/dev/null else echo 'libmad.a not installed'; fi
	@echo "uninstalling headers in $(HEAD_DIR)"
	@if [ -f $(HEAD_DIR)/aes.h ] ; then rm -f $(HEAD_DIR)/aes.h 2>/dev/null else echo 'aes.h not installed'; fi
	@if [ -f $(HEAD_DIR)/osbind.h ] ; then rm -f $(HEAD_DIR)/osbind.h 2>/dev/null else echo 'osbind.h not installed'; fi
	@if [ -f $(HEAD_DIR)/vdi.h ] ; then rm -f $(HEAD_DIR)/vdi.h 2>/dev/null else echo 'vdi.h not installed'; fi
	@echo "uninstalling madorcs in $(BIN_DIR)"
	@if [ -f $(BIN_DIR)/madorcs$(PROG_EXT) ] ; then rm -f $(BIN_DIR)/madorcs$(PROG_EXT) 2>/dev/null else echo 'madorcs not installed'; fi

clean:
	@rm libmad.a 2>/dev/null && echo 'deleting libmad.a' || \
		if [ ! -f libmad.a ] ; \
		then echo 'libmad.a does not exist.' ; \
		else echo 'Failure deleting libmad.a' ; fi
	@rm madorcs$(PROG_EXT) 2>/dev/null && echo 'deleting madorcs' || \
		if [ ! -f madorcs$(PROG_EXT) ] ; \
		then echo 'madorcs does not exist.' ; \
		else echo 'Failure deleting madorcs' ; fi
	@rm *.o 2>/dev/null && echo 'deleting *.o' || echo -n ''
	@rm *$(WARN_EXT) 2>/dev/null && echo "deleting warning files" || echo -n ''

