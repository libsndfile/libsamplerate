#!/usr/bin/make -f

Makefile.am: configure
	automake --copy --add-missing

configure: configure.ac src/config.h.in libtool ltmain.sh
	autoconf

src/config.h.in: configure.ac libtool
	autoheader

libtool ltmain.sh: aclocal.m4
	libtoolize --copy --force
	
# Need to re-run aclocal whenever acinclude.m4 is modified.
aclocal.m4: acinclude.m4
	aclocal

clean:
	rm -f libtool ltmain.sh aclocal.m4 Makefile.in src/config.h.in config.cache


