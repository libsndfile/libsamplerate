#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <samplerate.h>

#include "util.h"
#include "calc_snr.h"

#define BUFFER_LEN	(1<<23)
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
	gen_windowed_sines (input, SNR_LEN, &freq, 1) ;


	src_data.data_in = input ;
	src_data.input_frames = ARRAY_LEN (input) ;

	src_data.data_out = output ;
	src_data.output_frames = ARRAY_LEN (output) ;

	src_data.src_ratio = 0.99 ;

	start_time = clock () ;

	if ((error = src_simple (&src_data, SRC_SINC_BEST_QUALITY, 1)) != 0)
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
	printf ("SNR        : %6.2f dB\n", calculate_snr (output, SNR_LEN)) ;

	return 0 ;
} /* main */

/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: f1c7eab4-7340-49a8-8deb-0f66de593cdb
*/
