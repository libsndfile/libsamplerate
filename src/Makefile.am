# This file was automatically generated from the Makefile.am
# DO NOT EDIT!

lib_LTLIBRARIES = libsamplerate.la
include_HEADERS = samplerate.h

EXTRA_DIST = config.h.in Version_script.in check_asm.sh
CLEANFILES = src_sinc.s

COEFF_HDRS = high_qual_coeffs.h mid_qual_coeffs.h fastest_coeffs.h

noinst_HEADERS = common.h float_cast.h $(COEFF_HDRS)

SRC_SOURCES = samplerate.c src_sinc.c $(COEFF_HDRS) src_zoh.c src_linear.c

libsamplerate_la_SOURCES = $(SRC_SOURCES) $(noinst_HEADERS)
libsamplerate_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libsamplerate_la_LIBADD  = -lm

#-------------------------------------------------------------------------------
# An extra check for bad asm.

check-asm : check_asm.sh src_sinc.s
	@echo
	@echo
	$(srcdir)/check_asm.sh src_sinc.s
	@echo
	@echo

src_sinc.s : $(srcdir)/src_sinc.c
	$(CC) -S $(CFLAGS) $(DEFAULT_INCLUDES) $(srcdir)/src_sinc.c -o src_sinc.s

# Disable autoheader.
AUTOHEADER=echo

## Do not edit or modify anything in this comment block.
## The arch-tag line is a file identity tag for the GNU Arch
## revision control system.
##
## arch-tag: 62db1d8d-493b-4874-afb0-e421d3dbb868

