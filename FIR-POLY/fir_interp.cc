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
#include <ctime>
#include <cstring>
#include <complex>

#include <GMatrix.hh>
#include <Minimize.hh>

#include "mag_spectrum.hh"


#define	ARRAY_LEN(x)	(sizeof (x) / sizeof ((x) [0]))

/*
** Number of half cycles of impulse response to the left and right
** of the center.
**		left_half_offset <= right_half_cycles
*/

// static int left_half_cycles = 5 ;
// static int right_half_cycles = 15 ;


typedef struct
{
	double sinc_time_fudge ;

	/* Window coefficients (all in range [0, 1]). */
	double left_ak [5], right_ak [5] ;

} FIR_PARAMS ;

static double fir_error (const GMatrix& gm) ;

int
main (void)
{	static double data [32] ;
	GMatrix gm_start ;
	double error ;
	int k, param_count ;
	
	param_count = sizeof (FIR_PARAMS) / sizeof (double) ;

	srandom (time (NULL)) ;
	for (k = 0 ; k < param_count ; k++)
		data [k] = 3.0 * ((1.0 * random ()) / INT_MAX - 0.5) ;

	gm_start = GMatrix (param_count, 1, data) ;

	fir_error (gm_start) ;

	do
		error = MinDiffEvol (fir_error, gm_start, 1e-15, 2.0, random ()) ;
	while (error > 10.0) ;

	printf ("error : %f\n", error) ;

	return 0 ;
} /* main */

/*==============================================================================
*/

static double
fir_error (const GMatrix& gm)
{	FIR_PARAMS params ;
	double error = 0.0 ;
	unsigned param_count ;
	
	
	param_count = sizeof (params) / sizeof (double) ;

	if (gm.GetData	(param_count, (double*) &params) != param_count)
	{	printf ("\n\nError : GetData should return 2.\n") ;
		exit (1) ;
		} ;

	return error ;
} /* fir_error */

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 3ce7ca6f-394e-432c-aeaa-f228afc05afd
*/
