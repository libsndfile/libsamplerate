/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
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
#include <math.h>

#include <samplerate.h>

#include "util.h"

#define	BUFFER_LEN	1024

/* A NaN ratio fails every ordered comparison, so it must be rejected explicitly. */

static void
nan_ratio_test (int converter)
{	static float input [BUFFER_LEN], output [BUFFER_LEN] ;
	SRC_STATE *src_state ;
	SRC_DATA src_data ;
	int error ;

	printf ("        nan_ratio_test      (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	memset (input, 0, sizeof (input)) ;
	memset (output, 0, sizeof (output)) ;

	if (src_is_valid_ratio (NAN))
	{	printf ("\n\nLine %d : src_is_valid_ratio (NAN) is true.\n\n", __LINE__) ;
		exit (1) ;
		} ;

	memset (&src_data, 0, sizeof (src_data)) ;
	src_data.data_in = input ;
	src_data.input_frames = BUFFER_LEN ;
	src_data.data_out = output ;
	src_data.output_frames = BUFFER_LEN ;
	src_data.src_ratio = NAN ;

	if ((error = src_simple (&src_data, converter, 1)) == 0)
	{	printf ("\n\nLine %d : src_simple() accepted a NaN ratio (error %d).\n\n", __LINE__, error) ;
		exit (1) ;
		} ;

	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	if ((error = src_process (src_state, &src_data)) == 0)
	{	printf ("\n\nLine %d : src_process() accepted a NaN ratio (error %d).\n\n", __LINE__, error) ;
		exit (1) ;
		} ;

	if ((error = src_set_ratio (src_state, NAN)) == 0)
	{	printf ("\n\nLine %d : src_set_ratio() accepted a NaN ratio (error %d).\n\n", __LINE__, error) ;
		exit (1) ;
		} ;

	src_delete (src_state) ;

	puts ("ok") ;
} /* nan_ratio_test */

int
main (void)
{
	nan_ratio_test (SRC_ZERO_ORDER_HOLD) ;
	nan_ratio_test (SRC_LINEAR) ;
	nan_ratio_test (SRC_SINC_FASTEST) ;
	nan_ratio_test (SRC_SINC_MEDIUM_QUALITY) ;
	nan_ratio_test (SRC_SINC_BEST_QUALITY) ;

	return 0 ;
} /* main */
