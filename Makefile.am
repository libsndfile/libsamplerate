## Process this file with automake to produce Makefile.in

DISTCHECK_CONFIGURE_FLAGS = --enable-gcc-werror

SUBDIRS = src doc examples tests
DIST_SUBDIRS = $(SUBDIRS)

EXTRA_DIST = autogen.sh libsamplerate.spec.in samplerate.pc.in
 
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = samplerate.pc

m4datadir = $(datadir)/aclocal

# Some people prefer "make test" to "make check".

test:
	make check

check-asm :
	(cd src && make check-asm)

