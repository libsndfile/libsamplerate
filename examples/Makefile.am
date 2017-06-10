# This file was automatically generated
# DO NOT EDIT!

if HAVE_LIBSNDFILE
bin_PROGRAMS = sndfile-resample
endif

check_PROGRAMS = varispeed-play timewarp-file

SAMPLERATEDIR =../src
AM_CPPFLAGS = -I$(srcdir)/$(SAMPLERATEDIR) $(OS_SPECIFIC_INCLUDES)
AM_CFLAGS = $(SNDFILE_CFLAGS)

#===============================================================================

if HAVE_LIBSNDFILE
sndfile_resample_SOURCES = sndfile-resample.c
sndfile_resample_LDADD = $(SAMPLERATEDIR)/libsamplerate.la $(SNDFILE_LIBS) $(ALSA_LIBS)
endif

varispeed_play_SOURCES = varispeed-play.c audio_out.c audio_out.h
varispeed_play_LDADD = $(SAMPLERATEDIR)/libsamplerate.la $(SNDFILE_LIBS) $(ALSA_LIBS) $(OS_SPECIFIC_LINKS)

timewarp_file_SOURCES = timewarp-file.c
timewarp_file_LDADD = $(SAMPLERATEDIR)/libsamplerate.la $(SNDFILE_LIBS) $(ALSA_LIBS)

