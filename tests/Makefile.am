noinst_PROGRAMS = misc_test termination_test simple_test callback_test \
					reset_test multi_channel_test snr_bw_test \
					float_short_test varispeed_test src-evaluate benchmark

SAMPLRATEDIR =../src
INCLUDES = -I$(srcdir)/$(SAMPLRATEDIR)
noinst_HEADERS = calc_snr.h util.h

#===============================================================================

misc_test_SOURCES = misc_test.c util.c util.h
misc_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

termination_test_SOURCES = termination_test.c util.c util.h
termination_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

simple_test_SOURCES = simple_test.c util.c util.h
simple_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

reset_test_SOURCES = reset_test.c util.c util.h
reset_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

multi_channel_test_SOURCES = multi_channel_test.c util.c calc_snr.c
multi_channel_test_CFLAGS = @FFTW3_CFLAGS@
multi_channel_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la $(FFTW3_LIBS)

snr_bw_test_SOURCES = snr_bw_test.c calc_snr.c util.c util.h
snr_bw_test_CFLAGS = @FFTW3_CFLAGS@
snr_bw_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la $(FFTW3_LIBS)

callback_test_SOURCES = callback_test.c util.c util.h
callback_test_CFLAGS = @FFTW3_CFLAGS@
callback_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

float_short_test_SOURCES = float_short_test.c util.c util.h
float_short_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

varispeed_test_SOURCES = varispeed_test.c util.c util.h
varispeed_test_LDADD = $(SAMPLRATEDIR)/libsamplerate.la

# This program is for evaluating other sample rate converters.

src_evaluate_SOURCES = src-evaluate.c calc_snr.c util.c
src_evaluate_CFLAGS = @SNDFILE_CFLAGS@ @FFTW3_CFLAGS@
src_evaluate_LDADD = $(SNDFILE_LIBS) $(FFTW3_LIBS)

benchmark_CFLAGS = $(AM_CFLAGS) @FFTW3_CFLAGS@
benchmark_SOURCES = benchmark.c  util.c calc_snr.c
benchmark_LDADD  = $(SAMPLRATEDIR)/libsamplerate.la $(FFTW3_LIBS)

#===============================================================================

check: $(noinst_PROGRAMS)
	date
	uname -a
	./misc_test
	./termination_test
	./simple_test
	./callback_test
	./reset_test
	./multi_channel_test
	./float_short_test
	./snr_bw_test
	@echo "-----------------------------------------------------------------"
	@echo "  ${PACKAGE}-${VERSION} passed all tests."
	@echo "-----------------------------------------------------------------"

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch 
# revision control system.
#
# arch-tag: ec9d19f8-523c-48e5-8929-809c2641a8dc
