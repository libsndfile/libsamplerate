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


static const float fip_best_coeffs [250] =
{	0.0, 1.2, 2.3
} ;

/* FIP_COEFF_SET needs to be defined before this header is included. */
static const FIP_COEFF_SET fip_best =
{
	/* count */
	ARRAY_LEN (fip_best_coeffs),

	/* increment */
	1,

	/* c */
	fip_best_coeffs
} ;

/*
** Do not edit or modify anything in this comment block.
** The following line is a file identity tag for the GNU Arch
** revision control system.
**
** arch-tag: 86739f0c-c446-45f9-8ad0-35b6903c6135
*/
