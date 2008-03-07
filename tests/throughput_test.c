/*
** Copyright (C) 2004-2008 Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <string.h>
#include <math.h>
#include <time.h>

#include <samplerate.h>

#include "util.h"

#define BUFFER_LEN	(1<<16)
#define SNR_LEN		(1<<16)

static float input [BUFFER_LEN] ;
static float output [BUFFER_LEN] ;

int
main (void)
{	SRC_DATA src_data ;
	clock_t start_time, clock_time ;
	double freq ;
	int error ;

	memset (input, 0, sizeof (input)) ;
	freq = 0.01 ;
	gen_windowed_sines (1, &freq, 1.0, input, SNR_LEN) ;

	src_data.data_in = input ;
	src_data.input_frames = ARRAY_LEN (input) ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	src_data.src_ratio = 0.99 ;

	start_time = clock () ;

	if ((error = src_simple (&src_data, SRC_OLD_SINC_BEST_QUALITY, 1)) != 0)
	{	puts (src_strerror (error)) ;
		exit (1) ;
		} ;

	clock_time = clock () - start_time ;

	if (src_data.input_frames_used != ARRAY_LEN (input))
	{	printf ("\n\nLine %d : input frames used %ld should be %d\n", __LINE__, src_data.input_frames_used, ARRAY_LEN (input)) ;
		exit (1) ;
		} ;

	if (fabs (src_data.src_ratio * src_data.input_frames_used - src_data.output_frames_gen) > 2)
	{	printf ("\n\nLine %d : input / output length mismatch.\n\n", __LINE__) ;
		printf ("    input len  : %d\n", ARRAY_LEN (input)) ;
		printf ("    output len : %ld (should be %g +/- 2)\n\n", src_data.output_frames_gen,
				floor (0.5 + src_data.src_ratio * src_data.input_frames_used)) ;
		exit (1) ;
		} ;

	printf ("Time       : %5.2f secs.\n", (1.0 * clock_time) / CLOCKS_PER_SEC) ;
	printf ("Throughput : %d samples/sec\n", (int) floor (src_data.output_frames_gen / ((1.0 * clock_time) / CLOCKS_PER_SEC))) ;
	printf ("SNR        : %6.2f dB\n", calculate_snr (output, SNR_LEN, 1)) ;

	return 0 ;
} /* main */

