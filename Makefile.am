## Process this file with automake to produce Makefile.in

DISTCHECK_CONFIGURE_FLAGS = --enable-werror

AM_CPPFLAGS = -I$(top_srcdir)/src $(OS_SPECIFIC_INCLUDES)

EXTRA_DIST = autogen.sh libsamplerate.spec.in samplerate.pc.in Make.bat Win32 \
    Octave/generate_filter.m Octave/make_filter.m Octave/measure_filter.m \
    Octave/Readme.md

pkgconfig_DATA = samplerate.pc

# Some people prefer "make test" to "make check".
test:
	$(MAKE) check

########
# src/ #
########

lib_LTLIBRARIES = src/libsamplerate.la
include_HEADERS = src/samplerate.h

EXTRA_DIST += src/config.h.in src/Version_script.in src/check_asm.sh
CLEANFILES = src/src_sinc.s

# MinGW requires -no-undefined if a DLL is to be built.
src_libsamplerate_la_LDFLAGS = -no-undefined -version-info $(SHARED_VERSION_INFO) $(SHLIB_VERSION_ARG)
src_libsamplerate_la_SOURCES = src/samplerate.c src/src_sinc.c src/src_zoh.c src/src_linear.c \
	src/common.h src/fastest_coeffs.h src/mid_qual_coeffs.h src/high_qual_coeffs.h \
	src/src_config.h

#-------------------------------------------------------------------------------
# An extra check for bad asm.

check-asm: src/check_asm.sh src/src_sinc.s src/src_linear.s src/src_zoh.s
	@echo
	@echo
	$(top_srcdir)/src/check_asm.sh src/src_sinc.s
	$(top_srcdir)/src/check_asm.sh src/src_linear.s
	$(top_srcdir)/src/check_asm.sh src/src_zoh.s
	@echo
	@echo

.c.s:
	$(CC) -S $(CFLAGS) $(CPPFLAGS) $(DEFAULT_INCLUDES) $< -o $@

# Disable autoheader.
AUTOHEADER=echo

########
# doc/ #
########

dist_html_DATA = doc/SRC.png doc/SRC.css doc/index.html doc/license.html doc/history.html \
	doc/download.html doc/lists.html doc/quality.html doc/win32.html doc/faq.html doc/api.html \
	doc/api_simple.html doc/api_callback.html doc/api_full.html doc/api_misc.html

#############
# examples/ #
#############

if HAVE_LIBSNDFILE
if HAVE_LIBALSA
noinst_PROGRAMS = examples/varispeed-play examples/timewarp-file

examples_varispeed_play_SOURCES = examples/varispeed-play.c examples/audio_out.c examples/audio_out.h
examples_varispeed_play_CFLAGS = $(SNDFILE_CFLAGS) $(ALSA_CFLAGS)
examples_varispeed_play_LDADD = src/libsamplerate.la $(SNDFILE_LIBS) $(ALSA_LIBS) $(OS_SPECIFIC_LINKS)

examples_timewarp_file_SOURCES = examples/timewarp-file.c
examples_timewarp_file_CFLAGS = $(SNDFILE_CFLAGS) $(ALSA_CFLAGS)
examples_timewarp_file_LDADD = src/libsamplerate.la $(SNDFILE_LIBS) $(ALSA_LIBS)
endif
endif

##########
# tests/ #
##########

check_PROGRAMS = tests/misc_test tests/termination_test tests/simple_test tests/callback_test \
	tests/reset_test tests/multi_channel_test tests/snr_bw_test tests/float_short_test \
	tests/varispeed_test tests/callback_hang_test tests/src-evaluate tests/throughput_test \
	tests/multichan_throughput_test tests/downsample_test tests/clone_test tests/nullptr_test

check: $(check_PROGRAMS)
	date
	uname -a
	tests/misc_test
	tests/termination_test
	tests/callback_hang_test
	tests/downsample_test
	tests/simple_test
	tests/callback_test
	tests/reset_test
	tests/clone_test
	tests/nullptr_test
	tests/multi_channel_test
	tests/varispeed_test
	tests/float_short_test
	tests/snr_bw_test
	tests/throughput_test
	@echo "-----------------------------------------------------------------"
	@echo "  ${PACKAGE}-${VERSION} passed all tests."
	@echo "-----------------------------------------------------------------"

#===============================================================================

tests_misc_test_SOURCES = tests/misc_test.c tests/util.c tests/util.h
tests_misc_test_LDADD = src/libsamplerate.la

tests_termination_test_SOURCES = tests/termination_test.c tests/util.c tests/util.h
tests_termination_test_LDADD = src/libsamplerate.la

tests_callback_hang_test_SOURCES = tests/callback_hang_test.c tests/util.c tests/util.h
tests_callback_hang_test_LDADD = src/libsamplerate.la

tests_simple_test_SOURCES = tests/simple_test.c tests/util.c tests/util.h
tests_simple_test_LDADD = src/libsamplerate.la

tests_reset_test_SOURCES = tests/reset_test.c tests/util.c tests/util.h
tests_reset_test_LDADD = src/libsamplerate.la

tests_multi_channel_test_SOURCES = tests/multi_channel_test.c tests/util.c tests/calc_snr.c
tests_multi_channel_test_CFLAGS = $(FFTW3_CFLAGS)
tests_multi_channel_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_snr_bw_test_SOURCES = tests/snr_bw_test.c tests/calc_snr.c tests/util.c tests/util.h
tests_snr_bw_test_CFLAGS = $(FFTW3_CFLAGS)
tests_snr_bw_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_callback_test_SOURCES = tests/callback_test.c tests/util.c tests/util.h
tests_callback_test_LDADD = src/libsamplerate.la

tests_float_short_test_SOURCES = tests/float_short_test.c tests/util.c tests/util.h
tests_float_short_test_LDADD = src/libsamplerate.la

tests_downsample_test_SOURCES = tests/downsample_test.c tests/util.c tests/util.h
tests_downsample_test_LDADD = src/libsamplerate.la

tests_varispeed_test_SOURCES = tests/varispeed_test.c tests/util.c tests/util.h tests/calc_snr.c
tests_varispeed_test_CFLAGS = $(FFTW3_CFLAGS)
tests_varispeed_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_clone_test_SOURCES = tests/clone_test.c tests/util.c tests/util.h
tests_clone_test_LDADD = src/libsamplerate.la

tests_nullptr_test_SOURCES = tests/nullptr_test.c tests/util.c tests/util.h
tests_nullptr_test_LDADD = src/libsamplerate.la

# This program is for evaluating other sample rate converters.

tests_throughput_test_SOURCES = tests/throughput_test.c tests/util.c tests/calc_snr.c
tests_throughput_test_CFLAGS = $(FFTW3_CFLAGS)
tests_throughput_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_multichan_throughput_test_SOURCES = tests/multichan_throughput_test.c tests/util.c tests/calc_snr.c
tests_multichan_throughput_test_CFLAGS = $(FFTW3_CFLAGS)
tests_multichan_throughput_test_LDADD = src/libsamplerate.la $(FFTW3_LIBS)

tests_src_evaluate_SOURCES = tests/src-evaluate.c tests/calc_snr.c tests/util.c
tests_src_evaluate_CFLAGS = $(SNDFILE_CFLAGS) $(FFTW3_CFLAGS)
tests_src_evaluate_LDADD = $(SNDFILE_LIBS) $(FFTW3_LIBS)
