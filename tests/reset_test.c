/*
** Copyright (C) 2002,2003 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#define	BUFFER_LEN		1024

static void reset_test (int converter) ;

int
main (void)
{	/* Force output of the Electric Fence banner message. */
	force_efence_banner () ;

	puts ("") ;

	reset_test (SRC_ZERO_ORDER_HOLD) ;
	reset_test (SRC_LINEAR) ;
	reset_test (SRC_SINC_FASTEST) ;

	puts ("") ;

	return 0 ;
} /* main */

static void
reset_test (int converter)
{	static float data_one [BUFFER_LEN] ;
	static float data_zero [BUFFER_LEN] ;
	static float output [BUFFER_LEN] ;

	SRC_STATE *src_state ;
	SRC_DATA src_data ;
	int k, error ;

	printf ("\treset_test (%-28s) ....... ", src_get_name (converter)) ;
	fflush (stdout) ;

	for (k = 0 ; k < BUFFER_LEN ; k++)
	{	data_one [k] = 1.0 ;
		data_zero [k] = 0.0 ;
		} ;

	/* Get a converter. */
	if ((src_state = src_new (converter, 1, &error)) == NULL)
	{	printf ("\n\nLine %d : src_new() failed : %s.\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Process a bunch of 1.0 valued samples. */
	src_data.data_in		= data_one ;
	src_data.data_out		= output ;
	src_data.input_frames	= BUFFER_LEN ;
	src_data.output_frames	= BUFFER_LEN ;
	src_data.src_ratio		= 0.9 ;
	src_data.end_of_input	= 1 ;

	if ((error = src_process (src_state, &src_data)) != 0)
	{	printf ("\n\nLine %d : src_simple () returned error : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Reset the state of the converter.*/
	src_reset (src_state) ;

	/* Now process some zero data. */
	src_data.data_in		= data_zero ;
	src_data.data_out		= output ;
	src_data.input_frames	= BUFFER_LEN ;
	src_data.output_frames	= BUFFER_LEN ;
	src_data.src_ratio		= 0.9 ;
	src_data.end_of_input	= 1 ;

	if ((error = src_process (src_state, &src_data)) != 0)
	{	printf ("\n\nLine %d : src_simple () returned error : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	/* Finally make sure that the output data is zero ie reset was sucessful. */
	for (k = 0 ; k < BUFFER_LEN / 2 ; k++)
		if (output [k] != 0.0)
		{	printf ("\n\nLine %d : output [%d] should be 0.0, is %f.\n", __LINE__, k, output [k]) ;
			exit (1) ;
			} ;

	/* Make sure that this function has been exported. */
	src_set_ratio (src_state, 1.0) ;

	/* Delete converter. */
	src_state = src_delete (src_state) ;

	puts ("ok") ;
} /* reset_test */
/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: 4840a60d-e20c-4fa5-bec0-df004b2ff215
*/

