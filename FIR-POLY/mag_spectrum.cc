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
mag_spectrum (double *input, int len, double *magnitude, double scale)
{	static fftw_plan plan = NULL ;

	double	maxval ;
	int		k ;

	if (input == NULL || magnitude == NULL)
		return ;

	if (plan == NULL)
	{	plan = fftw_plan_r2r_1d (len, input, magnitude, FFTW_R2HC, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT) ;
		if (plan == NULL)
		{	printf ("%s : line %d : create plan failed.\n", __FILE__, __LINE__) ;
			exit (1) ;
			} ;
		} ;

	fftw_execute (plan) ;

	/* (k < N/2 rounded up) */
	maxval = 0.0 ;
	for (k = 0 ; k < len / 2 ; k++)
		magnitude [k] = scale * sqrt (magnitude [k] * magnitude [k] + magnitude [len - k - 1] * magnitude [len - k - 1]) ;

	memset (magnitude + len / 2, 0, len / 2 * sizeof (magnitude [0])) ;

	return ;
} /* mag_spectrum */

// Do not edit or modify anything in this comment block.
// The following line is a file identity tag for the GNU Arch
// revision control system.
//
// arch-tag: 58aa43ee-b8e1-4178-8861-7621399fa049
