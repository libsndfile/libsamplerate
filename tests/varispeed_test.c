/*
** Copyright (c) 2006-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include "src_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <samplerate.h>

#include "util.h"

#if HAVE_FFTW3
#include <fftw3.h>
#else
#define fftw_cleanup ()
#endif

#define	BUFFER_LEN		(1 << 16)

static void varispeed_test (int converter, double target_snr) ;
static void varispeed_hammer_test (int converter) ;
static void set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio) ;

int
main (void)
{
	puts ("") ;
	printf ("    Zero Order Hold interpolator    : ") ;
	varispeed_test (SRC_ZERO_ORDER_HOLD, 10.0) ;
	varispeed_hammer_test (SRC_ZERO_ORDER_HOLD) ;
	puts ("ok") ;

	printf ("    Linear interpolator             : ") ;
	varispeed_test (SRC_LINEAR, 10.0) ;
	varispeed_hammer_test (SRC_LINEAR) ;
	puts ("ok") ;

	printf ("    Sinc interpolator               : ") ;
	varispeed_test (SRC_SINC_FASTEST, 115.0) ;
	varispeed_hammer_test (SRC_SINC_FASTEST) ;
	puts ("ok") ;

	fftw_cleanup () ;
	puts ("") ;

	return 0 ;
} /* main */

static void
varispeed_test (int converter, double target_snr)
{	static float input [BUFFER_LEN], output [BUFFER_LEN] ;
	double sine_freq, snr ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;

	int input_len, error ;

	memset (input, 0, sizeof (input)) ;

	input_len = ARRAY_LEN (input) / 2 ;

	sine_freq = 0.0111 ;
	gen_windowed_sines (1, &sine_freq, 1.0, input, input_len) ;

	/* Perform sample rate conversion. */
	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new () failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 1 ;

	src_data.data_in = input ;
	src_data.input_frames = input_len ;

	src_data.src_ratio = 3.0 ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	if ((error = src_set_ratio (src_state, 1.0 / src_data.src_ratio)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	if (src_data.input_frames_used != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %ld\n\n", src_data.input_frames_used) ;
		exit (1) ;
		} ;

	/* Copy the last output to the input. */
	memcpy (input, output, sizeof (input)) ;
	reverse_data (input, src_data.output_frames_gen) ;

	if ((error = src_reset (src_state)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 1 ;

	src_data.data_in = input ;
	input_len = src_data.input_frames = src_data.output_frames_gen ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	if ((error = src_set_ratio (src_state, 1.0 / src_data.src_ratio)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
		printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
		exit (1) ;
		} ;

	if (src_data.input_frames_used != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %ld\n\n", src_data.input_frames_used) ;
		exit (1) ;
		} ;

	src_state = src_delete (src_state) ;

	snr = calculate_snr (output, src_data.output_frames_gen, 1) ;

	if (target_snr > snr)
	{	printf ("\n\nLine %d : snr (%3.1f) does not meet target (%3.1f)\n\n", __LINE__, snr, target_snr) ;
		save_oct_float ("varispeed.mat", input, src_data.input_frames, output, src_data.output_frames_gen) ;
		exit (1) ;
		} ;

	return ;
} /* varispeed_test */

static void
varispeed_hammer_test (int converter)
{	double ratios [] = { 1.0, 0.01, 20 } ;
	int chan, r1, r2 ;

	for (chan = 1 ; chan <= 9 ; chan ++)
		for (r1 = 0 ; r1 < ARRAY_LEN (ratios) ; r1++)
			for (r2 = 0 ; r1 != r2 && r2 < ARRAY_LEN (ratios) ; r2 ++)
				set_ratio_test (converter, 1, ratios [r1], ratios [r2]) ;
} /* varispeed_hammer_test */

static void
set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio)
{	static float input [BUFFER_LEN] ;
	static float output [8 * BUFFER_LEN] ;
	static float temp_buffer [BUFFER_LEN] ;
	static char details [128] ;

	const int num_frames = BUFFER_LEN / channels ;
	const int chunk_size = 128 ;

	SRC_STATE *src_state ;
	SRC_DATA src_data ;

	long input_len, output_len, current_in, current_out ;
	int error, k, ch ;



	snprintf (details, sizeof (details), "%d channels, ratio %g -> %g", channels, initial_ratio, second_ratio) ;

	input_len = ARRAY_LEN (input) / channels ;
	output_len = ARRAY_LEN (output) / channels ;

	for (ch = 0 ; ch < channels ; ch++)
	{	double freq = 0.01 * (ch + 1) ;
		gen_windowed_sines (1, &freq, 1.0, temp_buffer + ch * num_frames, num_frames) ;
		} ;

	interleave_data (temp_buffer, input, num_frames, channels) ;

	if ((src_state = src_new (converter, channels, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new () failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	current_in = 0 ;
	current_out = 0 ;

	memset (&src_data, 0, sizeof (src_data)) ;
	src_data.end_of_input = 0 ;
	src_data.src_ratio = initial_ratio ;
	src_data.data_in = input ;
	src_data.data_out = output ;
	src_data.input_frames = chunk_size ;
	src_data.output_frames = ARRAY_LEN (output) / channels ;

	/* Process one chunk at initial_ratio. */
	if ((error = src_process (src_state, &src_data)))
	{	printf ("\n\nLine %d : %s : %s\n\n", __LINE__, details, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Now switch to second_ratio and process the remaining. */
	src_data.src_ratio = second_ratio ;

	while (1)
	{	if ((error = src_process (src_state, &src_data)))
		{	printf ("\n\nLine %d : %s : %s\n\n", __LINE__, details, src_strerror (error)) ;
			exit (1) ;
			} ;

		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		current_in	+= src_data.input_frames_used ;
		current_out += src_data.output_frames_gen ;

		src_data.data_in	+= src_data.input_frames_used ;
		src_data.data_out	+= src_data.output_frames_gen ;

		src_data.input_frames	= input_len - current_in ;
		src_data.output_frames	= output_len - current_out ;

		src_data.end_of_input = (current_in >= input_len) ? 1 : 0 ;
		} ;


	for (k = 0 ; k < current_out * channels ; k ++)
		ASSERT (!isnan (output [k])) ;

	src_state = src_delete (src_state) ;

	return ;
} /* set_ratio_test */
