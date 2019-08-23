/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <samplerate.h>
#include "util.h"

void chunked_input_test(int converter, int channels, long chunkSize, double ratio)
{
	printf("chunked_input_test(converter=%d, channels=%d, chunkSize=%ld, ratio=%g)\n",
		converter, channels, chunkSize, ratio);

	const long NUM_FRAMES = 30*256;
	int error;
	
	float *input = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(input);
	float *output = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(output);
	float *chunkedOutput = calloc(NUM_FRAMES * channels, sizeof(float));
	TEST(chunkedOutput);

	for (int ch = 0 ; ch < channels ; ch++)
	{	double freq = 0.0111 ;
		gen_windowed_sines (1, &freq, 1.0, output + ch * NUM_FRAMES, NUM_FRAMES) ;
	}
	interleave_data(output, input, NUM_FRAMES, channels);

	// Calculate in 1 pass
	SRC_DATA onePassData;
	memset (&onePassData, 0, sizeof (onePassData)) ;
	onePassData.data_in = input;
	onePassData.data_out = output;
	onePassData.input_frames = NUM_FRAMES;
	onePassData.output_frames = NUM_FRAMES;
	onePassData.src_ratio = ratio;
	CHECKED_CALL(src_simple(&onePassData, converter, channels));

	SRC_STATE *state;
	if ((state = src_new (converter, channels, &error)) == NULL)
	{
		printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
	}
	// Calculate in chunks
	SRC_DATA data;
	memset (&data, 0, sizeof (data)) ;
	data.data_in = input;
	data.data_out = chunkedOutput;
	data.output_frames = NUM_FRAMES;
	data.src_ratio = ratio;

	long input_frames = NUM_FRAMES;
	long input_frames_used = 0;
	long output_frames_gen = 0;

	while(input_frames && data.output_frames)
	{
		data.input_frames = MIN(input_frames, chunkSize);
		data.end_of_input = data.input_frames == input_frames;

		CHECKED_CALL(src_process (state, &data));
		input_frames_used += data.input_frames_used;
		output_frames_gen += data.output_frames_gen;

		data.input_frames -= data.input_frames_used;
		data.data_in += data.input_frames_used * channels;
		data.output_frames -= data.output_frames_gen;
		data.data_out += data.output_frames_gen * channels;

		input_frames -= data.input_frames_used;
	}
	state = src_delete (state);

	TEST(onePassData.output_frames_gen == output_frames_gen);
	for(long i = 0; i < output_frames_gen * channels; i++)
	{
		if(output[i] != chunkedOutput[i])
			TEST(output[i] == chunkedOutput[i]);
	}
	
	free(input);
	free(output);
	free(chunkedOutput);
}

int
main (void)
{
	int CONVERTERS[] = {SRC_SINC_FASTEST, SRC_ZERO_ORDER_HOLD, SRC_LINEAR};
	double RATIOS[] = {1., 2., 256., 1./2., 1./256.};

	for(int iConverter = 0; iConverter < ARRAY_LEN(CONVERTERS); iConverter++)
		for(int iRatio=0; iRatio < ARRAY_LEN(RATIOS); iRatio++)
			chunked_input_test(CONVERTERS[iConverter], 1, 128, RATIOS[iRatio]);
	return 0 ;
}
