/*
** Copyright (C) 2006 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <samplerate.h>

#include "util.h"

#define	BUFFER_LEN		((1 << 16) - 20)

static void varispeed_test (int converter) ;

int
main (void)
{
	puts ("    Zero Order Hold interpolator: ") ;
	varispeed_test (SRC_ZERO_ORDER_HOLD) ;

	puts ("    Linear interpolator: ") ;
	varispeed_test (SRC_LINEAR) ;

	puts ("    Sinc interpolator: ") ;
	varispeed_test (SRC_SINC_FASTEST) ;

	return 0 ;
} /* main */

static void
varispeed_test (int converter)
{	static float input [BUFFER_LEN], output [BUFFER_LEN] ;
	double src_ratio ;

	SRC_STATE	*src_state ;
	SRC_DATA	src_data ;

	int input_len, output_len, current_in, current_out ;
	int k, error, terminate ;

/* Erik */
for (k = 0 ; k < BUFFER_LEN ; k++) input [k] = k * 1.0 ;

	/* Calculate maximun input and output lengths. */
	if (src_ratio >= 1.0)
	{	output_len = BUFFER_LEN ;
		input_len = (int) floor (BUFFER_LEN / src_ratio) ;
		}
	else
	{	input_len = BUFFER_LEN ;
		output_len = (int) floor (BUFFER_LEN * src_ratio) ;
		} ;

	/* Reduce input_len by 10 so output is longer than necessary. */
	input_len -= 20 ;

	if (output_len > BUFFER_LEN)
	{	printf ("\n\nLine %d : output_len > BUFFER_LEN\n\n", __LINE__) ;
		exit (1) ;
		} ;

	current_in = current_out = 0 ;

	/* Perform sample rate conversion. */
	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_data.end_of_input = 0 ; /* Set this later. */

	src_data.data_in = input ;

	src_data.src_ratio = src_ratio ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) / 10 ;

	terminate = 1 + (int) ceil ((src_ratio >= 1.0) ? src_ratio : 1.0 / src_ratio) ;

	while (1)
	{
		src_data.input_frames = next_block_length (0) ;
		src_data.input_frames = MIN (src_data.input_frames, input_len - current_in) ;

		src_data.output_frames = ARRAY_LEN (output) - current_out ;
		/*-Erik MIN (src_data.output_frames, output_len - current_out) ;-*/

		src_data.end_of_input = (current_in >= input_len) ? 1 : 0 ;

		if ((error = src_process (src_state, &src_data)))
		{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
			printf ("  src_data.input_frames  : %ld\n", src_data.input_frames) ;
			printf ("  src_data.output_frames : %ld\n\n", src_data.output_frames) ;
			exit (1) ;
			} ;

		if (src_data.end_of_input && src_data.output_frames_gen == 0)
			break ;

		if (src_data.input_frames_used > src_data.input_frames)
		{	printf ("\n\nLine %d : input_frames_used > input_frames\n\n", __LINE__) ;
			printf ("  src_data.input_frames      : %ld\n", src_data.input_frames) ;
			printf ("  src_data.input_frames_used : %ld\n", src_data.input_frames_used) ;
			printf ("  src_data.output_frames     : %ld\n", src_data.output_frames) ;
			printf ("  src_data.output_frames_gen : %ld\n\n", src_data.output_frames_gen) ;
			exit (1) ;
			} ;

		if (src_data.input_frames_used < 0)
		{	printf ("\n\nLine %d : input_frames_used (%ld) < 0\n\n", __LINE__, src_data.input_frames_used) ;
			exit (1) ;
			} ;

		if (src_data.output_frames_gen < 0)
		{	printf ("\n\nLine %d : output_frames_gen (%ld) < 0\n\n", __LINE__, src_data.output_frames_gen) ;
			exit (1) ;
			} ;

		current_in	+= src_data.input_frames_used ;
		current_out += src_data.output_frames_gen ;

		if (current_in > input_len + terminate)
		{	printf ("\n\nLine %d : current_in (%d) > input_len (%d + %d)\n\n", __LINE__, current_in, input_len, terminate) ;
			exit (1) ;
			} ;

		if (current_out > output_len)
		{	printf ("\n\nLine %d : current_out (%d) > output_len (%d)\n\n", __LINE__, current_out, output_len) ;
			exit (1) ;
			} ;

		if (src_data.input_frames_used > input_len)
		{	printf ("\n\nLine %d : input_frames_used (%ld) > %d\n\n", __LINE__, src_data.input_frames_used, input_len) ;
			exit (1) ;
			} ;

		if (src_data.output_frames_gen > output_len)
		{	printf ("\n\nLine %d : output_frames_gen (%ld) > %d\n\n", __LINE__, src_data.output_frames_gen, output_len) ;
			exit (1) ;
			} ;

		if (src_data.data_in == NULL && src_data.output_frames_gen == 0)
			break ;


		src_data.data_in	+= src_data.input_frames_used ;
		src_data.data_out	+= src_data.output_frames_gen ;
		} ;

	src_state = src_delete (src_state) ;

	if (fabs (current_out - src_ratio * input_len) > terminate)
	{	printf ("\n\nLine %d : bad output data length %d should be %2.1f +/- %d.\n", __LINE__,
					current_out, src_ratio * input_len, terminate) ;
		printf ("\tsrc_ratio  : %.4f\n", src_ratio) ;
		printf ("\tinput_len  : %d\n\tinput_used : %d\n", input_len, current_in) ;
		printf ("\toutput_len : %d\n\toutput_gen : %d\n\n", output_len, current_out) ;
		exit (1) ;
		} ;

	if (current_in != input_len)
	{	printf ("\n\nLine %d : unused input.\n", __LINE__) ;
		printf ("\tinput_len         : %d\n", input_len) ;
		printf ("\tinput_frames_used : %d\n\n", current_in) ;
		exit (1) ;
		} ;

	puts ("ok") ;

	return ;
} /* varispeed_test */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 0c9492de-9c59-4690-ba38-f384b547dc74
*/

