/*
** Copyright (C) 2003 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include <samplerate.h>

#include "util.h"

#define	BUFFER_LEN		90000
#define CB_READ_LEN		256

static void callback_test (int converter, double ratio) ;

int
main (void)
{	static double src_ratios [] =
	{	1.0, 0.099, 0.1, 0.33333333, 0.789, 1.0001, 1.9, 3.1, 9.9
	} ;

	int k ;

	/* Force output of the Electric Fence banner message. */
	force_efence_banner () ;

	puts ("") ;

	puts ("    Zero Order Hold interpolator :") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		callback_test (SRC_ZERO_ORDER_HOLD, src_ratios [k]) ;

	puts ("    Linear interpolator :") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		callback_test (SRC_LINEAR, src_ratios [k]) ;

	puts ("    Sinc interpolator :") ;
	for (k = 0 ; k < ARRAY_LEN (src_ratios) ; k++)
		callback_test (SRC_SINC_FASTEST, src_ratios [k]) ;

	puts ("") ;

	return 0 ;
} /* main */

/*=====================================================================================
*/

typedef struct
{	int channels ;
	long count, total ;
	float data [BUFFER_LEN] ;
} TEST_CB_DATA ;

static long
test_callback_func (void *cb_data, float **data)
{	TEST_CB_DATA *pcb_data ;

	long frames ;

	if ((pcb_data = cb_data) == NULL)
		return 0 ;

	if (data == NULL)
		return 0 ;

	if (pcb_data->total - pcb_data->count > CB_READ_LEN)
		frames = CB_READ_LEN ;
	else
		frames = pcb_data->total - pcb_data->count ;

	*data = pcb_data->data + pcb_data->count ;
	pcb_data->count += frames ;

	return frames ;
} /* test_callback_func */


static void
callback_test (int converter, double src_ratio)
{	static TEST_CB_DATA test_callback_data ;
	static float output [BUFFER_LEN] ;

	SRC_STATE	*src_state ;

	long	read_count, read_total ;
	int 	input_len, output_len, error ;

	printf ("\tcallback_test    (SRC ratio = %6.4f) ........... ", src_ratio) ;
	fflush (stdout) ;

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
	input_len -= 10 ;

	if (output_len > BUFFER_LEN)
	{	printf ("\n\nLine %d : output_len > BUFFER_LEN\n\n", __LINE__) ;
		exit (1) ;
		} ;

	test_callback_data.channels = 1 ;
	test_callback_data.count = 0 ;
	test_callback_data.total = input_len ;

	if ((src_state = src_callback_new (test_callback_func, converter, 1, &error, &test_callback_data)) == NULL)
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	read_total = 0 ;
	do
	{	read_count = (ARRAY_LEN (output) - read_total > CB_READ_LEN) ? CB_READ_LEN : ARRAY_LEN (output) - read_total ;
		read_count = src_callback_read (src_state, src_ratio, read_count, output + read_total) ;
		read_total += read_count ;
		}
	while (read_count > 0) ;

	if ((error = src_error (src_state)) != 0)
	{	printf ("\n\nLine %d : %s\n\n", __LINE__, src_strerror (error)) ;
		exit (1) ;
		} ;

	src_state = src_delete (src_state) ;

	if (fabs (read_total - src_ratio * input_len) > 2)
	{	printf ("\n\nLine %d : input / output length mismatch.\n\n", __LINE__) ;
		printf ("    input len  : %d\n", input_len) ;
		printf ("    output len : %ld (should be %g +/- 2)\n\n", read_total, floor (0.5 + src_ratio * input_len)) ;
		exit (1) ;
		} ;

	puts ("ok") ;

	return ;
} /* callback_test */


/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: bf1edd96-b137-48bc-a3ab-194007fadb55
*/

