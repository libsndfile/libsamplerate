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
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <complex>

#include <GMatrix.hh>
#include <Minimize.hh>

#include "mag_spectrum.hh"

#define	UPSAMPLE_RATIO	2
#define	ARRAY_LEN(x)	(sizeof (x) / sizeof ((x) [0]))

#define	INTERP_LEN	1600
/*
** Number of half cycles of impulse response to the left and right
** of the center.
**		left_half_offset <= right_len
*/

static int left_len = 150 ;
static int right_len = 750 ;


typedef struct
{
	/* Window coefficients (all in range [0, 1]). */
	double y_left [10], y_right [10] ;

	double sinc_time_fudge ;

} FIR_PARAMS ;

typedef struct
{
	union
	{	FIR_PARAMS fir_params ;
		double data [30] ;
	} ;

	unsigned middle, total_len ;

	double left_window_poly [20] ;
	double right_window_poly [20] ;

	double sinc [INTERP_LEN] ;
	double window [INTERP_LEN] ;
	double filter [INTERP_LEN] ;
	double mag [INTERP_LEN / 2] ;
	double logmag [INTERP_LEN / 2] ;
	double phase [INTERP_LEN / 2] ;
} FIR_INTERP ;

static double fir_error (const GMatrix& gm) ;
static int calc_window (FIR_INTERP *interp, double * error) ;
static void calc_sinc (FIR_INTERP *interp) ;
static void calc_filter (FIR_INTERP *interp) ;

static void oct_save (const FIR_INTERP *interp) ;
static void randomize_data (FIR_INTERP *interp) ;

static double evaluate_filter (FIR_INTERP *interp) ;

int
main (void)
{	FIR_INTERP interp ;
	GMatrix gm_start ;
	double error ;
	int param_count ;

	puts ("Running ........") ;
	param_count = sizeof (FIR_PARAMS) / sizeof (double) ;

	randomize_data (&interp) ;

	gm_start = GMatrix (param_count, 1, interp.data) ;

	fir_error (gm_start) ;

	do
		error = MinDiffEvol (fir_error, gm_start, 1e-15, 2.0, random ()) ;
	while (error > 10.0) ;

	printf ("error : %f\n", error) ;

	return 0 ;
} /* main */

/*==============================================================================
*/

static inline double
poly_evaluate (int order, const double *coeff, double x)
{	double result ;
	int	k ;

	/*
	** Use Horner's Rule for evaluating the value of
	** the polynomial at x.
	*/
	result = coeff [order] ;
	for (k = order - 1 ; k >= 0 ; k--)
		result = result * x + coeff [k] ;

	return result ;
} /* poly_evaluate */

static double
evaluate_filter (FIR_INTERP *interp)
{	double error = 0.0, temp, stop_max = 0.0, stop_sum = 0.0 ;
	unsigned k ;

	mag_spectrum (interp->filter, ARRAY_LEN (interp->filter), interp->mag, interp->logmag, interp->phase) ;
	
	for (k = ARRAY_LEN (interp->logmag) / UPSAMPLE_RATIO ; k < ARRAY_LEN (interp->logmag) ; k++)
	{	temp = interp->logmag [k] + 150.0 ;
		if (temp > stop_max)
			stop_max = temp ;
		if (temp > 0.0)
			stop_sum += temp ;
		} ;

	error = stop_max * stop_sum ;
	return error ;
} /* evaluate_filter */

static double
fir_error (const GMatrix& gm)
{	static FIR_INTERP interp ;
	static int test_count = 0 ;
	static double best_error = 1e200 ;
	double error = 0.0 ;
	unsigned param_count ;

	memset (&interp, 0, sizeof (interp)) ;

	param_count = sizeof (FIR_PARAMS) / sizeof (double) ;

	if (ARRAY_LEN (interp.data) < param_count)
	{	printf ("\n\nError : ARRAY_LEN (interp.data) < param_count.\n\n") ;
		exit (1) ;
		} ;

	if (gm.GetData	(param_count, interp.data) != param_count)
	{	printf ("\n\nError : GetData should return %d.\n\n", param_count) ;
		exit (1) ;
		} ;

	/* Eval error in sinc_time_fudge. */
	if (fabs (interp.fir_params.sinc_time_fudge - 1.0) > 0.5)
		return 1e30 * (fabs (interp.fir_params.sinc_time_fudge - 1.0)) ;


	interp.middle = left_len ;
	interp.total_len = left_len + right_len ;

	if (interp.total_len > ARRAY_LEN (interp.sinc))
	{	printf ("\n\nError : interp.total_len > ARRAY_LEN (interp.sinc).\n") ;
		exit (1) ;
		} ;

	if (calc_window (&interp, &error))
		return error ;

	calc_sinc (&interp) ;
	calc_filter (&interp) ;

	error = evaluate_filter (&interp) ;

	test_count ++ ;
// 	printf ("%s %d : exit\n", __func__, __LINE__) ;
// 	exit (1) ;

	if (error < best_error)
	{	oct_save (&interp) ;
		best_error = error ;
		printf ("%12d    best : %f\n", test_count, best_error) ;
		} ;

	return error ;
} /* fir_error */

static int
calc_window (FIR_INTERP *interp, double * returned_error)
{	unsigned k ;
	double error, temp, x ;

	/* Make sure left poly params are in range [0, 1]. */
	error = 0.0 ;
	for (k = 0 ; k < ARRAY_LEN (interp->fir_params.y_left) ; k++)
	{	temp = fabs (interp->fir_params.y_left [k] - 0.5) ;
		if (temp > 0.5 && temp > error)
			error = temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e20 * error ;
		return 1 ;
		} ;

	/* Make sure right poly params are in range [0, 1]. */
	error = 0.0 ;
	for (k = 0 ; k < ARRAY_LEN (interp->fir_params.y_right) ; k++)
	{	temp = fabs (interp->fir_params.y_right [k] - 0.5) ;
		if (temp > 0.5 && temp > error)
			error = temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e18 * error ;
		return 1 ;
		} ;

	/* Generate polynomial for left side of window. */
	interp->left_window_poly [0] = interp->fir_params.y_left [0] ;
	interp->left_window_poly [1] = 12.4166666667 - 12.4166666667 * interp->fir_params.y_left [0] + 31.25 * interp->fir_params.y_left [1] - 41.6666666667 * interp->fir_params.y_left [2] + 41.6666666667 * interp->fir_params.y_left [3] - 31.25 * interp->fir_params.y_left [4] ;
	interp->left_window_poly [2] = -140.756944444 + 58.2916666667 * interp->fir_params.y_left [0] - 231.770833333 * interp->fir_params.y_left [1] + 413.194444444 * interp->fir_params.y_left [2] - 447.916666667 * interp->fir_params.y_left [3] + 348.958333333 * interp->fir_params.y_left [4] ;
	interp->left_window_poly [3] = 571.614583333 - 135.416666667 * interp->fir_params.y_left [0] + 662.760416667 * interp->fir_params.y_left [1] - 1395.83333333 * interp->fir_params.y_left [2] + 1682.29166667 * interp->fir_params.y_left [3] - 1385.41666667 * interp->fir_params.y_left [4] ;
	interp->left_window_poly [4] = -1062.93402778 + 166.666666667 * interp->fir_params.y_left [0] - 917.96875 * interp->fir_params.y_left [1] + 2152.77777778 * interp->fir_params.y_left [2] - 2838.54166667 * interp->fir_params.y_left [3] + 2500. * interp->fir_params.y_left [4] ;
	interp->left_window_poly [5] = 917.96875 - 104.166666667 * interp->fir_params.y_left [0] + 618.489583333 * interp->fir_params.y_left [1] - 1562.5 * interp->fir_params.y_left [2] + 2213.54166667 * interp->fir_params.y_left [3] - 2083.33333333 * interp->fir_params.y_left [4] ;
	interp->left_window_poly [6] = -297.309027778 + 26.0416666667 * interp->fir_params.y_left [0] - 162.760416667 * interp->fir_params.y_left [1] + 434.027777778 * interp->fir_params.y_left [2] - 651.041666667 * interp->fir_params.y_left [3] + 651.041666667 * interp->fir_params.y_left [4] ;

	/* Generate left side of window. */
	error = 0.0 ;
	for (k = 0 ; k < interp->middle ; k++)
	{	x = k / (interp->middle * 1.0) ;
		interp->window [k] = poly_evaluate (ARRAY_LEN (interp->left_window_poly) - 1, interp->left_window_poly, x) ;
		} ;

	/* Ensure start value in range [0, 0,2]. */
	if (fabs (interp->window [0] - 0.1) > 0.1)
	{	*returned_error = 1e16 * fabs (interp->window [0] - 0.1) ;
		return 1 ;
		} ;

	/* Generate polynomial for right side of window. */
	interp->right_window_poly [0] = 1 ;
	interp->right_window_poly [1] = 0 ;
	interp->right_window_poly [2] = -83.4652777778 + 125. * interp->fir_params.y_right [0] - 62.5 * interp->fir_params.y_right [1] + 27.7777777778 * interp->fir_params.y_right [2] - 7.8125 * interp->fir_params.y_right [3] + interp->fir_params.y_right [4] ;
	interp->right_window_poly [3] = 446.614583333 - 802.083333333 * interp->fir_params.y_right [0] + 557.291666667 * interp->fir_params.y_right [1] - 270.833333333 * interp->fir_params.y_right [2] + 79.4270833333 * interp->fir_params.y_right [3] - 10.4166666667 * interp->fir_params.y_right [4] ;
	interp->right_window_poly [4] = -932.725694444 + 1848.95833333 * interp->fir_params.y_right [0] - 1536.45833333 * interp->fir_params.y_right [1] + 850.694444444 * interp->fir_params.y_right [2] - 266.927083333 * interp->fir_params.y_right [3] + 36.4583333333 * interp->fir_params.y_right [4] ;
	interp->right_window_poly [5] = 865.885416667 - 1822.91666667 * interp->fir_params.y_right [0] + 1692.70833333 * interp->fir_params.y_right [1] - 1041.66666667 * interp->fir_params.y_right [2] + 358.072916667 * interp->fir_params.y_right [3] - 52.0833333333 * interp->fir_params.y_right [4] ;
	interp->right_window_poly [6] = -297.309027778 + 651.041666667 * interp->fir_params.y_right [0] - 651.041666667 * interp->fir_params.y_right [1] + 434.027777778 * interp->fir_params.y_right [2] - 162.760416667 * interp->fir_params.y_right [3] + 26.0416666667 * interp->fir_params.y_right [4] ;

	/* Generate right side of window. */
	error = 0.0 ;
	for (k = interp->middle ; k < interp->total_len ; k++)
	{	x = (k - interp->middle) / (1.0 * (interp->total_len - interp->middle)) ;
		interp->window [k] = poly_evaluate (ARRAY_LEN (interp->right_window_poly) - 1, interp->right_window_poly, x) ;
		} ;

	/* Ensure end value in range [0, 0,4]. */
	k = interp->total_len - 1 ;
	if (fabs (interp->window [k] - 0.1) > 0.1)
	{	*returned_error = 1e16 * fabs (interp->window [k] - 0.1) ;
		return 1 ;
		} ;

	/* Ensure whole window is in [0, 1] range. */
	error = 0.0 ;
	for (k = 0 ; k < interp->total_len ; k++)
	{	temp = fabs (interp->window [k] - 0.5) ;
		if (temp > 0.5 && temp > error)
			error = temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e16 * error ;
		return 1 ;
		} ;

	/* Monotonicity of left side of window. */
	error = 0.0 ;
	for (k = 1 ; k < interp->middle ; k++)
	{	temp = (interp->window [k - 1] - interp->window [k]) ;
		if (temp > 0.0)
			error += temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e14 * error ;
		return 1 ;
		} ;

	/* Monotonicity of right side of window. */
	error = 0.0 ;
	for (k = interp->middle + 1 ; k < interp->total_len ; k++)
	{	temp = (interp->window [k] - interp->window [k - 1]) ;
		if (temp > 0.0)
			error += temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e14 * error ;
		return 1 ;
		} ;

	return 0 ;
} /* calc_window */

static void
calc_sinc (FIR_INTERP *interp)
{	unsigned k ;
	double x ;

	for (k = 0 ; k < interp->middle ; k++)
	{	x = M_PI * (interp->middle - k) * interp->fir_params.sinc_time_fudge / UPSAMPLE_RATIO ;
		interp->sinc [k] = sin (x) / x ;
		} ;
	interp->sinc [interp->middle] = 1.0 ;

	for (k = interp->middle + 1 ; k < interp->total_len ; k++)
	{	x = M_PI * (k - interp->middle) * interp->fir_params.sinc_time_fudge / UPSAMPLE_RATIO ;
		interp->sinc [k] = sin (x) / x ;
		} ;

} /* calc_sinc */

static void
calc_filter (FIR_INTERP *interp)
{	unsigned k ;
	double sum = 0.0 ;

	for (k = 0 ; k < interp->total_len ; k++)
	{	interp->filter [k] = interp->sinc [k] * interp->window [k] ;
		sum += interp->filter [k] ;
		} ;

	/* Now normalize. */
	for (k = 0 ; k < interp->total_len ; k++)
		interp->filter [k] /= sum ;

} /* calc_filter */

static void
oct_save (const FIR_INTERP *interp)
{	const char * filename = "a.mat" ;
	FILE * file ;
	unsigned k ;

	unlink (filename) ;

	if ((file = fopen (filename, "w")) == NULL)
	{	printf ("\nError : fopen failed.\n") ;
		exit (1) ;
		} ;

	fprintf (file, "# Not created by Octave\n") ;

	fprintf (file, "# name: sinc_time_fudge\n") ;
	fprintf (file, "# type: scalar\n%16.14f\n", interp->fir_params.sinc_time_fudge) ;

	fprintf (file, "# name: middle\n") ;
	fprintf (file, "# type: scalar\n%d\n", interp->middle) ;

	fprintf (file, "# name: total_len\n") ;
	fprintf (file, "# type: scalar\n%d\n", interp->total_len) ;

	fprintf (file, "# name: sinc\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", interp->total_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < interp->total_len ; k++)
		fprintf (file, "% f\n", interp->sinc [k]) ;

	fprintf (file, "# name: win\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", interp->total_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < interp->total_len ; k++)
		fprintf (file, "% f\n", interp->window [k]) ;

	fprintf (file, "# name: filt\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", interp->total_len) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < interp->total_len ; k++)
		fprintf (file, "% f\n", interp->filter [k] * UPSAMPLE_RATIO) ;

	fprintf (file, "# name: mag\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", ARRAY_LEN (interp->mag)) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < ARRAY_LEN (interp->mag) ; k++)
		fprintf (file, "% f\n", interp->mag [k]) ;

	fprintf (file, "# name: logmag\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", ARRAY_LEN (interp->logmag)) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < ARRAY_LEN (interp->logmag) ; k++)
		fprintf (file, "% f\n", interp->logmag [k]) ;

	fprintf (file, "# name: phase\n") ;
	fprintf (file, "# type: matrix\n") ;
	fprintf (file, "# rows: %d\n", ARRAY_LEN (interp->phase)) ;
	fprintf (file, "# columns: 1\n") ;

	for (k = 0 ; k < ARRAY_LEN (interp->phase) ; k++)
		fprintf (file, "% f\n", interp->phase [k]) ;

	fclose (file) ;
} /* oct_save */

static void
randomize_data (FIR_INTERP *interp)
{	FILE * file ;
	unsigned k, param_count, seed ;

	file = fopen ("/dev/urandom", "r") ;
	fread (&seed, 1, sizeof (seed), file) ;
	fclose (file) ;

	srandom (seed) ;

	param_count = sizeof (FIR_PARAMS) / sizeof (double) ;

	for (k = 0 ; k < param_count ; k++)
		interp->data [k] = 3.0 * ((1.0 * random ()) / INT_MAX - 0.5) ;

} /* randomize_data */

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 3ce7ca6f-394e-432c-aeaa-f228afc05afd
*/
