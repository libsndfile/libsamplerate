/*
** Copyright (C) 2004 Erik de Castro Lopo <erikd@mega-nerd.com>
**
** The copyright above and this notice must be preserved in all
** copies of this source code. The copyright above does not
** evidence any actual or intended publication of this source code.
**
** This is unpublished proprietary trade secret of Erik de Castro
** Lopo.  This source code may not be copied, disclosed,
** distributed, demonstrated or licensed except as authorized by
** Erik de Castro Lopo.
**
** No part of this program or publication may be reproduced,
** transmitted, transcribed, stored in a retrieval system,
** or translated into any language or computer language in any
** form or by any means, electronic, mechanical, magnetic,
** optical, chemical, manual, or otherwise, without the prior
** written permission of Erik de Castro Lopo.
*/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>

#include <fftw3.h>

void
mag_spectrum (double *input, int len, double *magnitude, double *phase)
{	static fftw_plan plan = NULL ;
	static int saved_len = -1 ;
	static double *fft_data = NULL ;

	double	maxval ;
	int		k, pi_factor ;

	if (input == NULL || magnitude == NULL || phase == NULL)
	{	printf ("%s %d : input == NULL || magnitude == NULL || phase == NULL\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	if (saved_len > 0 && saved_len != len)
	{	printf ("%s %d : saved_len != len.\n", __func__, __LINE__) ;
		exit (1) ;
		} ;

	if (fft_data == NULL)
		fft_data = (double *) calloc (sizeof (double), len) ;

	if (plan == NULL)
	{	plan = fftw_plan_r2r_1d (len, input, fft_data, FFTW_R2HC, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT) ;
		if (plan == NULL)
		{	printf ("%s %d : create plan failed.\n", __func__, __LINE__) ;
			exit (1) ;
			} ;
		} ;

	fftw_execute (plan) ;

	/* (k < N/2 rounded up) */
	maxval = 0.0 ;
	for (k = 0 ; k < len / 2 ; k++)
	{	magnitude [k] = sqrt (fft_data [k] * fft_data [k] + fft_data [len - k - 1] * fft_data [len - k - 1]) ;
		phase [k] = atan2 (fft_data [len - k - 1], fft_data [k]) ;
		} ;

	pi_factor = 0 ;
	for (k = 1 ; k < len / 2 ; k++)
	{	phase [k] -= pi_factor * M_PI ;
		if (fabs (phase [k] - phase [k - 1]) > 3.0)
		{	phase [k] -= 2 * M_PI ;
			pi_factor += 2 ;
			}
		} ;

	return ;
} /* mag_spectrum */

// Do not edit or modify anything in this comment block.
// The following line is a file identity tag for the GNU Arch
// revision control system.
//
// arch-tag: 58aa43ee-b8e1-4178-8861-7621399fa049
