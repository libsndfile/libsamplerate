DIST_SUBDIRS = Win32 @subdirs@
SUBDIRS = @subdirs@
EXTRA_DIST = reconfigure.mk libsamplerate.spec.in samplerate.pc.in Make.bat
 
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = samplerate.pc

m4datadir = $(datadir)/aclocal

# Some people prefer "make test" to "make check".

test:
	make check

check-asm :
	(cd src && make check-asm)

## Do not edit or modify anything in this comment block.
## The arch-tag line is a file identity tag for the GNU Arch 
## revision control system.
##
## arch-tag: 73b721ae-3dbf-4130-997e-923cac695453

