/*
** Copyright (C) 2002-2004 Erik de Castro Lopo <erikd@mega-nerd.com>
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

#define	FIP_MAGIC_MARKER	MAKE_MAGIC ('f', 'i', 'r', 'p', 'l', 'y')

typedef struct
{	int		fip_magic_marker ;

	int		channels ;
	long	in_count, in_used ;
	long	out_count, out_gen ;

	double	src_ratio, input_index ;
	
	float	*input_buffer ;
	int		in_buf_len ;

	float	*fir_out ;
	int		fir_out_len ;

	float	*iir_out ;
	int		iir_out_len ;

	float	buffer [1] ;
} FIR_IIR_POLY ;

static void fip_reset (SRC_PRIVATE *psrc) ;
static int fip_process (SRC_PRIVATE *psrc, SRC_DATA *data) ;

const char*
fip_get_name (int src_enum)
{
	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
			return "Best FIR/IIR/Polynomial Interpolator" ;

		case SRC_FIR_IIR_POLY_MEDIUM :
			return "Medium FIR/IIR/Polynomial Interpolator" ;

		case SRC_FIR_IIR_POLY_FASTEST :
			return "Fastest FIR/IIR/Polynomial Interpolator" ;
		} ;

	return NULL ;
} /* fip_get_descrition */

const char*
fip_get_description (int src_enum)
{
	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
			return "Three stage FIR/IIR/Polynomial Interpolator, best quality, XXdb SNR, XX% BW." ;

		case SRC_FIR_IIR_POLY_MEDIUM :
			return "Three stage FIR/IIR/Polynomial Interpolator, medium quality, XXdb SNR, XX% BW." ;

		case SRC_FIR_IIR_POLY_FASTEST :
			return "Three stage FIR/IIR/Polynomial Interpolator, fastest, XXdb SNR, XX% BW." ;
		} ;

	return NULL ;
} /* fip_get_descrition */

int
fip_set_converter (SRC_PRIVATE *psrc, int src_enum)
{	FIR_IIR_POLY *fip ;

	fip = (FIR_IIR_POLY *) psrc->private_data ;

	switch (src_enum)
	{	case SRC_FIR_IIR_POLY_BEST :
				break ;

		case SRC_FIR_IIR_POLY_MEDIUM :
				break ;

		case SRC_FIR_IIR_POLY_FASTEST :
				break ;

		default :
				return SRC_ERR_BAD_CONVERTER ;
		} ;

	psrc->reset = fip_reset ;
	psrc->process = fip_process ;

	return SRC_ERR_NO_ERROR ;
} /* fip_set_converter */

static void
fip_reset (SRC_PRIVATE *psrc)
{	FIR_IIR_POLY *fip ;

	fip = (FIR_IIR_POLY *) psrc->private_data ;

} /* fip_reset */

/*========================================================================================
**	Beware all ye who dare pass this point. There be dragons here.
*/

static int
fip_process (SRC_PRIVATE *psrc, SRC_DATA *data)
{	FIR_IIR_POLY *fip ;

	if (psrc->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	fip = (FIR_IIR_POLY*) psrc->private_data ;

	/* If there is not a problem, this will be optimised out. */
	if (sizeof (fip->buffer [0]) != sizeof (data->data_in [0]))
		return SRC_ERR_SIZE_INCOMPATIBILITY ;


	return SRC_ERR_NO_ERROR ;
} /* fip_process */

/*----------------------------------------------------------------------------------------
*/




/*
** Do not edit or modify anything in this comment block.
** The arch-tag line is a file identity tag for the GNU Arch 
** revision control system.
**
** arch-tag: a6c8bad4-740c-4e4f-99f1-e274174ab540
*/

