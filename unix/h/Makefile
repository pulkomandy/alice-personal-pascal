
SDIR=../src

all: node.h class.h typecodes.h

node.h : node.m4
	m4 node.m4  >/tmp/nodeout
	csplit -f nsplit /tmp/nodeout "/BEGIN_ARRAY/" "/BEGIN_DEFINE/"

class.h : class.m4
	m4 class.m4 | sed -n -e '1,/cpp/p' -e '/cpp/,$$w class.h' >$(SDIR)/classes.c

typecodes.h : typecodes.seq
	sequence <typecodes.seq typecodes.h


links: 
	ln $(SDIR)/nodes.c nsplit00 
	ln $(SDIR)/nodes2.c nsplit01
	ln node.h nsplit02
