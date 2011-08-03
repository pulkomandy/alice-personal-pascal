

dist:
	find . src apin help help/misc help/p* adoc aplib tem h howto com \
	spec notes samples ! -name '*.o' ! -name '*.Z' !  -perm -111 \
	! -name core -maxdepth 1  | tar -c -T - | gzip >/tmp/alice.tar.gz

