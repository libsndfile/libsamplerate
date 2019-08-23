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
#define fftw_cleanup()
#endif

#define	BUFFER_LEN		(1 << 16)

static void varispeed_test (int converter, double target_snr) ;
static void set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio) ;

int
main (void)
{
	int CONVERTERS[] = {SRC_SINC_FASTEST, SRC_ZERO_ORDER_HOLD, SRC_LINEAR};
	double RATIOS[] = {0.1, 0.01, 20};//{1./256., .01, 1.1, 256};

	puts ("") ;
	printf ("    Zero Order Hold interpolator    : ") ;
	varispeed_test (SRC_ZERO_ORDER_HOLD, 10.0) ;

	printf ("    Linear interpolator             : ") ;
	varispeed_test (SRC_LINEAR, 10.0) ;

	printf ("    Sinc interpolator               : ") ;
	varispeed_test (SRC_SINC_FASTEST, 115.0) ;

	for(int i = 0; i < ARRAY_LEN(CONVERTERS); i++)
		for(int channels=1; channels <=16; channels*=2)
			for(int iFirst=0; iFirst < ARRAY_LEN(RATIOS); iFirst++)
				for(int iSecond=0; iSecond < ARRAY_LEN(RATIOS); iSecond++)
				{
					if(iFirst != iSecond)
						set_ratio_test(CONVERTERS[i], channels, RATIOS[iFirst], RATIOS[iSecond]);
				}

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
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
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

	puts ("ok") ;

	return ;
} /* varispeed_test */

static void adjustData(SRC_DATA *statistic, SRC_DATA *curData, int channels)
{
	TEST(curData->input_frames_used <= statistic->input_frames);
	TEST(curData->output_frames_gen <= statistic->output_frames);

	statistic->input_frames_used += curData->input_frames_used;
	statistic->output_frames_gen += curData->output_frames_gen;
	curData->input_frames -= curData->input_frames_used;
	curData->data_in += curData->input_frames_used * channels;
	curData->output_frames -= curData->output_frames_gen;
	curData->data_out += curData->output_frames_gen * channels;
}

static void
set_ratio_test (int converter, int channels, double initial_ratio, double second_ratio)
{
	printf("set_ratio_test(converter=%d, channels=%d, initial ratio=%g, 2nd ratio=%g)\n",
		converter, channels, initial_ratio, second_ratio);

	const int NUM_FRAMES = 40*256;
	const int CHUNK_SIZE = 128;
	
	float *input = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(input);
	float *output = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(output);
	float *tmpBuffer = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(tmpBuffer);

	SRC_DATA data, curData;
	memset (&data, 0, sizeof (data));
	data.data_in = input;
	data.data_out = output;
	data.input_frames = NUM_FRAMES;
	data.output_frames = NUM_FRAMES;
	data.end_of_input = 0;
	data.src_ratio = initial_ratio;

	for (int ch = 0 ; ch < channels ; ch++)
	{	double freq = 0.01;
		gen_windowed_sines (1, &freq, 1.0, tmpBuffer + ch * NUM_FRAMES, NUM_FRAMES);
	}
	interleave_data(tmpBuffer, input, NUM_FRAMES, channels);

	SRC_STATE *state;
	int error;
	if ((state = src_new (converter, channels, &error)) == NULL)
	{
		printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error));
		exit (1);
	}

	long input_frames = data.input_frames;
	curData = data;
	curData.input_frames = CHUNK_SIZE;

	CHECKED_CALL(src_process (state, &curData));
	adjustData(&data, &curData, channels);
	input_frames -= curData.input_frames_used;
	curData.src_ratio = second_ratio;
	CHECKED_CALL(src_set_ratio (state, curData.src_ratio));
	while(input_frames && curData.output_frames)
	{
		CHECKED_CALL(src_process (state, &curData));
		adjustData(&data, &curData, channels);
		input_frames -= curData.input_frames_used;

		curData.input_frames =  MIN(input_frames, CHUNK_SIZE);
		curData.end_of_input = curData.input_frames == input_frames;
	}

	TEST(data.input_frames_used > 0);
	TEST(data.input_frames_used <= data.input_frames);
	TEST(data.output_frames_gen > 0);
	TEST(data.output_frames_gen <= data.output_frames);

	for (long i = 0 ; i < data.output_frames_gen * channels ; i++)
	{
		TEST(!isnan(output[i]));
	}
	
	state = src_delete (state) ;
	free(input);
	free(output);
	free(tmpBuffer);
}
