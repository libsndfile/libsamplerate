/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#if LIBSAMPLERATE_SINGLE_PRECISION
typedef float fp_t;
#else
typedef double fp_t;
#endif

#define	ABS(a)			(((a) < 0) ? - (a) : (a))

#ifndef MAX
#define	MAX(a,b)		(((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define	MIN(a,b)		(((a) < (b)) ? (a) : (b))
#endif

#define	ARRAY_LEN(x)	((int) (sizeof (x) / sizeof ((x) [0])))

void gen_windowed_sines (int freq_count, const fp_t *freqs, fp_t max, float *output, int output_len) ;

void save_oct_float (char *filename, float *input, int in_len, float *output, int out_len) ;
void save_oct_double (char *filename, fp_t *input, int in_len, fp_t *output, int out_len) ;

void interleave_data (const float *in, float *out, int frames, int channels) ;

void deinterleave_data (const float *in, float *out, int frames, int channels) ;

void reverse_data (float *data, int datalen) ;

fp_t calculate_snr (float *data, int len, int expected_peaks) ;

const char * get_cpu_name (void) ;

#define ASSERT(condition) \
	if (!(condition)) \
	{	printf ("Condition failed on Line %d : %s\n\n", __LINE__, #condition) ; \
		exit (1) ; \
	    } ;

