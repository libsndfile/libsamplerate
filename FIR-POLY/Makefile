TARGETS = fir_interp

CXX = g++
CXXFLAGS = -g -O3 -Wall -W -Werror -Wstrict-prototypes -Wmissing-prototypes

GMATH_INC = $(shell pkg-config --cflags GMath)
GMATH_LIB = $(shell pkg-config --libs GMath)

FFTW_INC = $(shell pkg-config --cflags fftw3)
FFTW_LIB = $(shell pkg-config --libs fftw3)



all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

check: $(TARGETS)
	./fir_interp

fir_interp: fir_interp.o mag_spectrum.o demin.o
	$(CXX) $(CXXFLAGS) $+ $(GMATH_LIB) $(FFTW_LIB) -o $@

#---------------------------------------------------------------------

fir_interp.o : fir_interp.cc mag_spectrum.hh
	$(CXX) $(CXXFLAGS) $(GMATH_INC) -c fir_interp.cc -o $@

mag_spectrum.o : mag_spectrum.cc mag_spectrum.hh
	$(CXX) $(CXXFLAGS) $(FFTW_INC) -c mag_spectrum.cc -o $@

demin.o : demin.cc
	$(CXX) $(CXXFLAGS) $(GMATH_INC) -c $+ -o $@

# Do not edit or modify anything in this comment block.
# The arch-tag line is a file identity tag for the GNU Arch
# revision control system.
#
# arch-tag: 34b6b183-dca1-4085-9089-c8b7d5b09099
