/*
** Copyright (C) 2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include "config.h"
#include "float_cast.h"
#include "common.h"

#define	FIP_MAGIC_MARKER	MAKE_MAGIC ('F', '/', 'I', '/', 'P', ' ')

#define BUFFER_LEN	1024

typedef struct
{	int count ;
	int increment ;
	const float *c ;
} FIP_COEFF_SET ;

#include "fip_best.h"

typedef struct
{	int		fip_magic_marker ;

	int		channels ;
	long	in_count, in_used ;
	long	out_count, out_gen ;

	double	src_ratio, input_index ;

	/* The raw coeff set for this converter. */
	const	FIP_COEFF_SET *coeff_set ;

	/* The current FIR coefficents for the times 2 upsampler. */
	float 	*coeffs0, * coeffs1 ;
	int		max_half_coeff_len, half_coeff_len ;

	/* Buffer definitions. */
	float	*input_buffer ;
	int		in_buf_len, in_current ;

	float	*fir_out ;
	int		fir_out_len, fir_start, fir_end ;

	float	*iir_out ;
	int		iir_out_len, iir_start, iir_end ;

	float	dummy [1] ;
} FIR_IIR_POLY ;

static void fip_reset (SRC_PRIVATE *psrc) ;
static int fip_process (SRC_PRIVATE *psrc, SRC_DATA *data) ;

static int fip_process_var_down (FIR_IIR_POLY * fip, SRC_DATA * data) ;
static int fip_process_const_down (FIR_IIR_POLY * fip, SRC_DATA * data) ;
static int fip_process_var_up (FIR_IIR_POLY * fip, SRC_DATA * data) ;
static int fip_process_const_up (FIR_IIR_POLY * fip, SRC_DATA * data) ;


const char*
fip_get_name (int src_enum)
{
	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
			return "Best FIR/IIR/Polynomial Interpolator" ;
#if 0
		case SRC_FIR_IIR_POLY_MEDIUM :
			return "Medium FIR/IIR/Polynomial Interpolator" ;

		case SRC_FIR_IIR_POLY_FASTEST :
			return "Fastest FIR/IIR/Polynomial Interpolator" ;
#endif
		} ;

	return NULL ;
} /* fip_get_descrition */

const char*
fip_get_description (int src_enum)
{
	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
			return "Three stage FIR/IIR/Polynomial Interpolator, best quality, XXdb SNR, XX% BW." ;
#if 0
		case SRC_FIR_IIR_POLY_MEDIUM :
			return "Three stage FIR/IIR/Polynomial Interpolator, medium quality, XXdb SNR, XX% BW." ;

		case SRC_FIR_IIR_POLY_FASTEST :
			return "Three stage FIR/IIR/Polynomial Interpolator, fastest, XXdb SNR, XX% BW." ;
#endif
		} ;

	return NULL ;
} /* fip_get_descrition */

int
fip_set_converter (SRC_PRIVATE *psrc, int src_enum)
{	FIR_IIR_POLY *fip = NULL, temp_fip ;
	int buffer_total ;

	if (psrc->private_data != NULL)
	{	fip = (FIR_IIR_POLY *) psrc->private_data ;
		if (fip->fip_magic_marker != FIP_MAGIC_MARKER)
		{	free (psrc->private_data) ;
			psrc->private_data = NULL ;
			} ;
		} ;

	memset (&temp_fip, 0, sizeof (temp_fip)) ;

	if ((OFFSETOF (FIR_IIR_POLY, dummy) & 0xF) != 0)
	{	printf ("(OFFSETOF (FIR_IIR_POLY, dummy) & 0xF) != 0\n") ;
		exit (1) ;
		} ;

	temp_fip.fip_magic_marker = FIP_MAGIC_MARKER ;
	temp_fip.channels = psrc->channels ;

	psrc->reset = fip_reset ;
	psrc->process = fip_process ;

	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
				temp_fip.coeff_set = &fip_best ;
				break ;
#if 0
		case SRC_FIR_IIR_POLY_MEDIUM :
				temp_fip.coeff_set = &fip_medium ;
				break ;

		case SRC_FIR_IIR_POLY_FASTEST :
				temp_fip.coeff_set = &fip_fastest ;
				break ;
#endif
		default :
				return SRC_ERR_BAD_CONVERTER ;
		} ;

	temp_fip.max_half_coeff_len = SRC_MAX_RATIO * temp_fip.coeff_set->count / temp_fip.coeff_set->increment ;
	temp_fip.in_buf_len = temp_fip.channels * 2 * temp_fip.max_half_coeff_len ;

	temp_fip.fir_out_len = BUFFER_LEN ;
	temp_fip.iir_out_len = 2 * BUFFER_LEN ;

	buffer_total = 4 + temp_fip.max_half_coeff_len + psrc->channels * (temp_fip.in_buf_len + temp_fip.fir_out_len + temp_fip.iir_out_len) ;

	if ((fip = calloc (1, sizeof (FIR_IIR_POLY) + sizeof (fip->dummy [0]) * buffer_total)) == NULL)
		return SRC_ERR_MALLOC_FAILED ;

	*fip = temp_fip ;
	memset (&temp_fip, 0xEE, sizeof (temp_fip)) ;

	psrc->private_data = fip ;

	/* Allocate buffers. */
	fip->coeffs0 = fip->dummy ;
	while ((((unsigned long) fip->coeffs0) & 0xF) > 0)
		fip->coeffs0 ++ ;

	fip->coeffs1 = fip->coeffs0 + fip->max_half_coeff_len ;
	fip->input_buffer = fip->coeffs1 + fip->max_half_coeff_len ;

	fip->fir_out = fip->input_buffer + fip->channels * fip->in_buf_len ;
	fip->iir_out = fip->fir_out + fip->channels * fip->fir_out_len ;

	printf ("total : %d / %d\n", (fip->iir_out + fip->iir_out_len) - fip->dummy, buffer_total) ;

	fip_reset (psrc) ;

	return SRC_ERR_NO_ERROR ;
} /* fip_set_converter */

static void
fip_reset (SRC_PRIVATE *psrc)
{	FIR_IIR_POLY *fip ;

	fip = (FIR_IIR_POLY *) psrc->private_data ;
	if (fip == NULL)
		return ;

	memset (fip->input_buffer, 0, fip->channels * fip->in_buf_len * sizeof (fip->dummy [0])) ;
	memset (fip->fir_out, 0, fip->channels * fip->fir_out_len * sizeof (fip->dummy [0])) ;
	memset (fip->iir_out, 0, fip->channels * fip->iir_out_len * sizeof (fip->dummy [0])) ;

	fip->in_current = 0 ;
	fip->fir_start = 0 ;
	fip->iir_start = fip->iir_end = 0 ;

	fip->src_ratio = fip->input_index = 0.0 ;

	fip->half_coeff_len = 0 ;

	return ;
} /* fip_reset */

/*========================================================================================
*/

static int
fip_process (SRC_PRIVATE *psrc, SRC_DATA *data)
{	FIR_IIR_POLY *fip ;
	int error ;

	if (psrc->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	fip = (FIR_IIR_POLY*) psrc->private_data ;

	/* If there is not a problem, this will be optimised out. */
	if (sizeof (fip->dummy [0]) != sizeof (data->data_in [0]))
		return SRC_ERR_SIZE_INCOMPATIBILITY ;

	/* Just after an fip_reset(), the src_ratio field is invalid. */
	if (fip->src_ratio < 1.0 / SRC_MAX_RATIO)
		fip->src_ratio = data->src_ratio ;

	fip->in_count = data->input_frames * fip->channels ;
	fip->out_count = data->output_frames * fip->channels ;
	fip->in_used = fip->out_gen = 0 ;

	/* Choose a more specialised process() function. */
	if (fip->src_ratio < 1.0 || data->src_ratio < 1.0)
	{	if (fabs (fip->src_ratio - data->src_ratio) > 1e-20)
			error = fip_process_var_down (fip, data) ;
		else
			error = fip_process_const_down (fip, data) ;
		}
	else if (fabs (fip->src_ratio - data->src_ratio) > 1e-20)
		error = fip_process_var_up (fip, data) ;
	else
		error = fip_process_const_up (fip, data) ;

	data->input_frames_used = fip->in_used / fip->channels ;
	data->output_frames_gen = fip->out_gen / fip->channels ;

	return error ;
} /* fip_process */

/*----------------------------------------------------------------------------------------
*/

static void
fip_generate_fir_current_coeffs (FIR_IIR_POLY * fip)
{	int k ;

	if (fip->src_ratio >= 1.0)
	{	fip->half_coeff_len = fip->coeff_set->count / fip->coeff_set->increment / 2 ;

		for (k = 0 ; k < fip->half_coeff_len ; k++)
		{	fip->coeffs0 [k] = fip->coeff_set->c [fip->coeff_set->increment * k] ;
			fip->coeffs1 [k] = fip->coeff_set->c [fip->coeff_set->increment * k + fip->coeff_set->increment / 2] ;
			} ;

		/* Round it up to a multiple of 4. */
		while (fip->half_coeff_len & 3)
		{	fip->coeffs0 [fip->half_coeff_len] = 0.0 ;
			fip->coeffs1 [fip->half_coeff_len] = 0.0 ;
			fip->half_coeff_len ++ ;
			} ;

		return ;
		} ;

	printf ("%s : not implemented yet.\n", __func__) ;
	exit (1) ;
} /* fip_generate_fir_current_coeffs */

static int
fip_process_var_down (FIR_IIR_POLY * fip, SRC_DATA * data)
{	fip = NULL ;
	data = NULL ;
	printf ("%s : not implemented yet.\n", __func__) ;
	return SRC_ERR_NO_ERROR ;
} /* fip_process_var_down */

static int
fip_process_const_down (FIR_IIR_POLY * fip, SRC_DATA * data)
{
	if (fip->half_coeff_len == 0)
		fip_generate_fir_current_coeffs (fip) ;

	data = NULL ;
	printf ("%s : not implemented yet.\n", __func__) ;
	return SRC_ERR_NO_ERROR ;
} /* fip_process_const_down */

static int
fip_process_var_up (FIR_IIR_POLY * fip, SRC_DATA * data)
{
	if (fip->half_coeff_len == 0)
		fip_generate_fir_current_coeffs (fip) ;

	data = NULL ;
	printf ("%s : not implemented yet.\n", __func__) ;
	return SRC_ERR_NO_ERROR ;
} /* fip_process_var_up */

static int
fip_process_const_up (FIR_IIR_POLY * fip, SRC_DATA * data)
{	long max_in_used ;
	int ch, indx ;

	/* If we don't yet have the coeffs, generate them. */
	if (fip->half_coeff_len == 0)
		fip_generate_fir_current_coeffs (fip) ;

	max_in_used = lrintf (fip->in_count / fip->src_ratio - 0.5) ;
	max_in_used -= max_in_used & 1 ;

	if (fip->channels != 1)
	{	printf ("%s : fip->channels != 1\n", __func__) ;
		exit (1) ;
		}

	/* Main processing loop. */
	while (fip->out_gen < fip->out_count && fip->in_used < max_in_used)
	{
		for (ch = 0 ; ch < fip->channels ; ch ++)
		{	indx = (fip->in_current + fip->half_coeff_len + ch) % fip->in_buf_len ;
			fip->input_buffer [indx] = data->data_in [fip->in_used ++] ;
			} ;




		fip->in_current = (fip->in_current + 1) % fip->in_buf_len ;
		} ;

	return SRC_ERR_NO_ERROR ;
} /* fip_process_const_up */


/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: a6c8bad4-740c-4e4f-99f1-e274174ab540
*/
