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

#define	UPSAMPLE_RATIO	2.0
#define	ARRAY_LEN(x)	(sizeof (x) / sizeof ((x) [0]))

/*
** Number of half cycles of impulse response to the left and right
** of the center.
**		left_half_offset <= right_half_cycles
*/

static int left_half_cycles = 6 ;
static int right_half_cycles = 20 ;


typedef struct
{
	/* Window coefficients (all in range [0, 1]). */
	double y_left [3], y_right [3] ;

	double sinc_time_fudge ;

} FIR_PARAMS ;

typedef struct
{
	union
	{	FIR_PARAMS fir_params ;
		double data [20] ;
	} ;

	unsigned middle, total_len ;

	double left_window_poly [5] ;
	double right_window_poly [5] ;

	double sinc [2048] ;
	double window [2048] ;
	double filter [2048] ;
	double mag [1024] ;
	double phase [1024] ;
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
{
	mag_spectrum (interp->filter, ARRAY_LEN (interp->filter), interp->mag, interp->phase) ;

	return 1.0 ;
} /* evaluate_filter */

static double
fir_error (const GMatrix& gm)
{	static FIR_INTERP interp ;
	static double best_error = 1e200 ;
	double error = 0.0 ;
	unsigned param_count ;

	memset (&interp, 0, sizeof (interp)) ;

	param_count = sizeof (FIR_PARAMS) / sizeof (double) ;

	if (ARRAY_LEN (interp.data) < param_count)
	{	printf ("\n\nError : ARRAY_LEN (interp.data) < param_count.\n") ;
		exit (1) ;
		} ;

	if (gm.GetData	(param_count, interp.data) != param_count)
	{	printf ("\n\nError : GetData should return %d.\n", param_count) ;
		exit (1) ;
		} ;

	/* Eval error in sinc_time_fudge. */
	if (interp.fir_params.sinc_time_fudge < 1.0)
		return 1e30 * (fabs (interp.fir_params.sinc_time_fudge - 1.0)) ;

	if (interp.fir_params.sinc_time_fudge > 1.25)
		return 1e30 * (fabs (interp.fir_params.sinc_time_fudge - 1.25)) ;

	interp.middle = lrint (floor (left_half_cycles * UPSAMPLE_RATIO / interp.fir_params.sinc_time_fudge)) ;
	interp.total_len = interp.middle + lrint (floor (right_half_cycles * UPSAMPLE_RATIO / interp.fir_params.sinc_time_fudge)) ;

	if (interp.total_len > ARRAY_LEN (interp.sinc))
	{	printf ("\n\nError : interp.total_len > ARRAY_LEN (interp.sinc).\n") ;
		exit (1) ;
		} ;

	if (calc_window (&interp, &error))
		return error ;

	calc_sinc (&interp) ;
	calc_filter (&interp) ;

	error = evaluate_filter (&interp) ;

	oct_save (&interp) ;
	printf ("%s %d : exit\n", __func__, __LINE__) ;
	exit (1) ;

	if (error < best_error)
	{	oct_save (&interp) ;
		best_error = error ;
		} ;

	return error ;
} /* fir_error */

static int
calc_window (FIR_INTERP *interp, double * returned_error)
{	unsigned k ;
	double error, temp, x ;

	error = 0 ;
	for (k = 0 ; k < ARRAY_LEN (interp->fir_params.y_left) ; k++)
	{	temp = fabs (interp->fir_params.y_left [k] - 0.5) ;
		if (temp > 0.5 && temp > error)
			error = temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e20 * error ;
		return 1 ;
		} ;

	error = 0 ;
	for (k = 0 ; k < ARRAY_LEN (interp->fir_params.y_right) ; k++)
	{	temp = fabs (interp->fir_params.y_right [k] - 0.5) ;
		if (temp > 0.5 && temp > error)
			error = temp ;
		} ;

	if (error > 0.0)
	{	*returned_error = 1e18 * error ;
		return 1 ;
		} ;

    interp->left_window_poly [0] = interp->fir_params.y_left [0] ;
    interp->left_window_poly [1] = 13.0/2 - 13.0/2 * interp->fir_params.y_left [0] + 27.0/2 * interp->fir_params.y_left [1] - 27.0/2 * interp->fir_params.y_left [2] ;
    interp->left_window_poly [2] = -139.0/4 + 29.0/2 * interp->fir_params.y_left [0] - 189.0/4 * interp->fir_params.y_left [1] + 135.0/2 * interp->fir_params.y_left [2] ;
    interp->left_window_poly [3] = 54 - 27.0/2 * interp->fir_params.y_left [0] + 54 * interp->fir_params.y_left [1] - 189.0/2 * interp->fir_params.y_left [2] ;
    interp->left_window_poly [4] = -99.0/4 + 9.0/2 * interp->fir_params.y_left [0] - 81.0/4 * interp->fir_params.y_left [1] + 81.0/2 * interp->fir_params.y_left [2] ;

	/* Generate left side of window. */
	error = 0.0 ;
	for (k = 0 ; k < interp->middle ; k++)
	{	x = k / (interp->middle * 1.0) ;
		interp->window [k] = poly_evaluate (ARRAY_LEN (interp->left_window_poly) - 1, interp->left_window_poly, x) ;
		} ;

	if (fabs (interp->window [0] - 0.2) > 0.2)
	{	*returned_error = 1e16 * fabs (interp->window [0] - 0.1) ;
		return 1 ;
		} ;

    interp->right_window_poly [0] = 1 ;
    interp->right_window_poly [1] = 0 ;
    interp->right_window_poly [2] = -85.0/4 + 27 * interp->fir_params.y_right [0] - 27.0/4 * interp->fir_params.y_right [1] + interp->fir_params.y_right [2] ;
    interp->right_window_poly [3] = 45 - 135.0/2 * interp->fir_params.y_right [0] + 27 * interp->fir_params.y_right [1] - 9.0/2 * interp->fir_params.y_right [2] ;
    interp->right_window_poly [4] = -99.0/4 + 81.0/2 * interp->fir_params.y_right [0] - 81.0/4 * interp->fir_params.y_right [1] + 9.0/2 * interp->fir_params.y_right [2] ;

	/* Generate right side of window. */
	error = 0.0 ;
	for (k = interp->middle ; k < interp->total_len ; k++)
	{	x = (k - interp->middle) / (1.0 * (interp->total_len - interp->middle)) ;
		interp->window [k] = poly_evaluate (ARRAY_LEN (interp->right_window_poly) - 1, interp->right_window_poly, x) ;
		} ;

	k = interp->total_len - 1 ;
	if (fabs (interp->window [k] - 0.2) > 0.2)
	{	*returned_error = 1e16 * fabs (interp->window [k] - 0.1) ;
		return 1 ;
		} ;

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
