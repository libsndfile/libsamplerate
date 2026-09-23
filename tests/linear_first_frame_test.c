/*
** Copyright (c) 2008-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <samplerate.h>

#include "util.h"

/*
** A fresh or reset state whose first src_process() call is given exactly one
** input frame.  The linear converter interpolates between the previous and the
** current input frame; with a single frame the previous one does not exist, so
** the main loop used to read data_in [in_used - channels + ch] with in_used==0,
** i.e. a negative index (heap-buffer-overflow, up to channels*4 bytes).
*/

static void
first_frame_test (int converter, int channels)
{
	float *in, *out ;
	SRC_DATA data ;
	SRC_STATE *state ;
	int k, error ;

	in = (float *) malloc (sizeof (float) * channels) ;
	out = (float *) malloc (sizeof (float) * channels * 64) ;
	if (in == NULL || out == NULL)
	{	puts ("malloc failed.") ;
		exit (1) ;
		} ;

	for (k = 0 ; k < channels ; k++)
		in [k] = 0.5f ;

	printf ("        first_frame_test    (%-28s ch %d) ....... ", src_get_name (converter), channels) ;
	fflush (stdout) ;

	state = src_new (converter, channels, &error) ;
	if (state == NULL)
	{	puts ("src_new failed.") ;
		exit (1) ;
		} ;

	memset (&data, 0, sizeof (data)) ;
	data.data_in = in ;
	data.data_out = out ;
	data.input_frames = 1 ;
	data.output_frames = 32 ;
	data.src_ratio = 1.0 ;

	if (src_process (state, &data))
	{	puts ("src_process failed.") ;
		exit (1) ;
		} ;

	if (data.input_frames_used != 1)
	{	printf ("\n        input_frames_used is %ld, expected 1.\n", (long) data.input_frames_used) ;
		exit (1) ;
		} ;

	if (data.output_frames_gen != 1)
	{	printf ("\n        output_frames_gen is %ld, expected 1.\n", (long) data.output_frames_gen) ;
		exit (1) ;
		} ;

	/* The first output sample comes from interpolating the single input frame
	** against itself, so it must equal that frame. */
	for (k = 0 ; k < channels ; k++)
	{	if (data.data_out [k] != 0.5f)
		{	printf ("\n        output [%d] is %f, expected 0.5.\n", k, (double) data.data_out [k]) ;
			exit (1) ;
			} ;
		} ;

	src_delete (state) ;
	free (in) ;
	free (out) ;

	puts ("ok") ;
} /* first_frame_test */

int
main (void)
{
	puts ("") ;

	first_frame_test (SRC_LINEAR, 1) ;
	first_frame_test (SRC_LINEAR, 2) ;
	first_frame_test (SRC_LINEAR, 8) ;
	first_frame_test (SRC_LINEAR, 128) ;

	puts ("") ;

	return 0 ;
} /* main */
