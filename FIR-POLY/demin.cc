/*
** Copyright (C) 1998-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#include	<stdio.h>
#include	<unistd.h>

#include	<Minimize.hh>
#include	<PsuedoRand.hh>

class	DEmin
{	private :
		u_int		PopSize, FuncCalcs, Generations ;

		PsuedoRand	Rand ;

		GMatrix		*MainVector, *Vector2 ;
		double		*MainCost, *Cost2 ;
		double		*ScaleFactor, *ScaleFactor2 ;
		double		*Crossover, *Crossover2 ;

		MinFunc		Function ;

	public :
				DEmin	(void) ;
				~DEmin	(void) ;

		void	Setup	(MinFunc func, const GMatrix& start, u_int popsize, double range, u_long seed) ;

		GMatrix	Minimize (double tol, u_int maxgen) ;

		void	PrintStats	(void) ;

} ; // class DEmin

//======================================================================================

inline
void	Swap	(GMatrix **a, GMatrix **b)
{	GMatrix *temp ;
	temp = *a ; *a = *b ; *b = temp ;
} ; //

inline void
Swap	(double **a, double **b)
{	double *temp ;
	temp = *a ; *a = *b ; *b = temp ;
} ; // Swap

inline double
Max	(double a, double b)
{	return (a > b) ? a : b ;
} ; // Max

inline double
Min	(double a, double b)
{	return (a < b) ? a : b ;
} ; // Max

//======================================================================================

double
MinDiffEvol (MinFunc Function, GMatrix& start, double Ftol, double range, u_long seedval)
{	DEmin		*pfmin ;
	u_int		dimensions ;

	dimensions = start.GetElements () ;
	if (dimensions < 20)
		dimensions = 20 ;

	pfmin = new DEmin ;

	pfmin -> Setup (Function, start, dimensions * 5, range, seedval) ;

	Ftol = (Ftol < 1e-8) ? 1e-8 : Ftol ;

	start = pfmin -> Minimize (Ftol, dimensions * 5000) ;

	delete	pfmin ;

	return Function (start) ;
} ; // MinDiffEvol


//======================================================================================

DEmin::DEmin	(void)
{	PopSize		= 0 ;
	FuncCalcs	= 0 ;
	Generations	= 0 ;
	MainVector	= NULL ;
	MainCost	= NULL ;
	ScaleFactor	= NULL ;
	Crossover	= NULL ;
	Vector2		= NULL ;
	Cost2		= NULL ;
	ScaleFactor2 = NULL ;
	Crossover2	= NULL ;
} ; // DEmin constructor

DEmin::~DEmin	(void)
{	if (PopSize)
	{	delete [] MainVector ;
		delete MainCost ;
		delete ScaleFactor ;
		delete Crossover ;
		delete [] Vector2 ;
		delete Cost2 ;
		delete ScaleFactor2 ;
		delete Crossover2 ;
		} ;
	PopSize = 0 ;
} ; // DEmin destructor

void
DEmin::Setup (MinFunc func, const GMatrix& start, u_int popsize, double range, u_long seed)
{	u_int		k ;
	u_short		rows, cols ;

	Generations = 0 ;
	Function = func ;

	PopSize = (popsize < 2) ? popsize + 2 : popsize ;

	MainVector	= new GMatrix [PopSize] ;
	MainCost	= new double [PopSize] ;
	ScaleFactor	= new double [PopSize] ;
	Crossover	= new double [PopSize] ;
	Vector2		= new GMatrix [PopSize] ;
	Cost2		= new double [PopSize] ;
	ScaleFactor2 = new double [PopSize] ;
	Crossover2	= new double [PopSize] ;

	Rand.Seed (seed + 1234567) ;

	rows = start.GetRows () ;
	cols = start.GetCols () ;

	GMatrix::PsuedoRandSeed (123123) ;

	if (IsComplex (start))
		for (k = 0 ; k < PopSize ; k++)
		{	MainVector [k].SetSizeComplex (rows, cols) ;
			MainVector [k].PsuedoRandomize () ;
			MainVector [k] = (2.0 * range) * (MainVector [k] - Complex (0.5, 0.5)) ;
			}
	else
		for (k = 0 ; k < PopSize ; k++)
		{	MainVector [k].SetSizeDouble (rows, cols) ;
			MainVector [k].PsuedoRandomize () ;
			MainVector [k] = (2.0 * range) * (MainVector [k] - 0.5) ;
			} ;

	MainVector [0] = start ;	// Must include given vector.

	for (k = 0 ; k < PopSize ; k++)
	{	MainCost [k] = Function (MainVector [k]) ;
		FuncCalcs ++ ;
		ScaleFactor [k] = 1.2 * Rand.UDDouble () ;
		Crossover [k] = Rand.UDDouble () ;
		} ;

	return ;
} ; // DEmin::Setup

GMatrix
DEmin::Minimize (double Ftol, u_int maxgen)
{	u_int		k, a, b, c ;
	GMatrix		temp ;
	double		scalefactor, crossover, cost, improvement, best ;

	improvement = 100 * Ftol ;
	best = 10e10 ;

	while (Generations < maxgen && best > Ftol)
	{	Swap (&MainVector, &Vector2) ;
		Swap (&MainCost, &Cost2) ;
		Swap (&ScaleFactor, &ScaleFactor2) ;
		Swap (&Crossover, &Crossover2) ;

		improvement = 0.0 ;

		for (k = 0 ; k < PopSize ; k++)
		{	do
				a = u_int (double (PopSize) * Rand.UDDouble ()) ;
			while 	(a == k || a >= PopSize) ;
			do
				b = u_int (double (PopSize) * Rand.UDDouble ()) ;
			while	(b == k || b == a || b >= PopSize) ;
			do
				c = u_int (double (PopSize) * Rand.UDDouble ()) ;
			while	(c == k || c == a || c == b || c >= PopSize) ;

if (a >= PopSize)	{	printf ("a - very bad (%u, %u).\n", a, PopSize) ;	exit (0) ; } ;
if (b >= PopSize)	{	printf ("b - very bad (%u, %u).\n", b, PopSize) ;	exit (0) ; } ;
if (c >= PopSize)	{	printf ("c - very bad (%u, %u).\n", c, PopSize) ;	exit (0) ; } ;

			scalefactor = 0.5 * (ScaleFactor2 [b] + ScaleFactor2 [c]) ;

			temp = Vector2 [a] + scalefactor * (Vector2 [b] - Vector2 [c]) ;

			crossover = 0.5 * (Crossover2 [k] + Crossover2 [a]) ;

			temp.CrossOver (Vector2 [k], crossover) ;

			cost = Function (temp) ;
			FuncCalcs ++ ;

			if (cost < Cost2 [k])
			{	MainVector	[k] = temp ;
				MainCost	[k] = cost ;
				ScaleFactor [k] = scalefactor ;
				Crossover	[k] = crossover ;
				improvement = 0.5 * (improvement + Max (improvement, Cost2 [k] - cost)) ;
				best = Min (best, cost) ;
				}
			else
			{	MainVector	[k] = Vector2 [k] ;
				MainCost	[k] = Cost2 [k] ;
				ScaleFactor	[k] = ScaleFactor2 [k] ;
				Crossover	[k] = Crossover2 [k] ;
				} ;	// if
			} ; // for k

		Generations ++ ;
		} ; // while

	// Now find best vector.

	for (a = 0, k = 1 ; k < PopSize ; k++)
		if (MainCost [k] < MainCost [a])
			a = k ;
	temp = MainVector [a] ;

	return temp ;
} ; // DEmin::Minimize

void
DEmin::PrintStats (void)
{	printf ("Generations          : %8d\n", Generations) ;
	printf ("Function evaluations : %8d\n", FuncCalcs) ;
	putchar ('\n') ;
} ; // DEmin::PrintStats

//======================================================================================



// Do not edit or modify anything in this comment block.
// The arch-tag line is a file identity tag for the GNU Arch
// revision control system.
//
// arch-tag: e874ad11-7294-4757-9370-fd1e6e1261d0

