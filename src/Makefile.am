# This file was automatically generated from the Makefile.am
# DO NOT EDIT!

lib_LTLIBRARIES = libsamplerate.la
include_HEADERS = samplerate.h

EXTRA_DIST = config.h.in Version_script.in

COEFF_HDRS = high_qual_coeffs.h mid_qual_coeffs.h fastest_coeffs.h

noinst_HEADERS = common.h float_cast.h $(COEFF_HDRS)

SRC_SOURCES = samplerate.c src_sinc.c $(COEFF_HDRS) src_zoh.c src_linear.c
				
libsamplerate_la_SOURCES = $(SRC_SOURCES) $(noinst_HEADERS)
libsamplerate_la_LDFLAGS = -version-info @SHARED_VERSION_INFO@ @SHLIB_VERSION_ARG@
libsamplerate_la_LIBADD  = -lm

# Disable autoheader. 
AUTOHEADER=echo
