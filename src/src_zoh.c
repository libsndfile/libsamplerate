/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "src_config.h"
#include "common.h"

static enum SRC_ERR zoh_vari_process (SRC_STATE *state, SRC_DATA *data) ;
static void zoh_reset (SRC_STATE *state) ;
static enum SRC_ERR zoh_copy (SRC_STATE *from, SRC_STATE *to) ;
static void zoh_close (SRC_STATE *state) ;

/*========================================================================================
*/

#define	ZOH_MAGIC_MARKER	MAKE_MAGIC ('s', 'r', 'c', 'z', 'o', 'h')

typedef struct
{	int		zoh_magic_marker ;
	bool	dirty ;
	long	in_count, in_used ;
	long	out_count, out_gen ;
	float	last_value [] ;
} ZOH_DATA ;

/*----------------------------------------------------------------------------------------
*/

static enum SRC_ERR
zoh_vari_process (SRC_STATE *state, SRC_DATA *data)
{	ZOH_DATA 	*priv ;
	double		src_ratio, input_index, rem ;
	int			ch ;

	if (data->input_frames <= 0)
		return SRC_ERR_NO_ERROR ;

	if (state->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	priv = (ZOH_DATA*) state->private_data ;

	if (!priv->dirty)
	{	/* If we have just been reset, set the last_value data. */
		for (ch = 0 ; ch < state->channels ; ch++)
			priv->last_value [ch] = data->data_in [ch] ;
		priv->dirty = true ;
		} ;

	priv->in_count = data->input_frames * state->channels ;
	priv->out_count = data->output_frames * state->channels ;
	priv->in_used = priv->out_gen = 0 ;

	src_ratio = state->last_ratio ;

	if (is_bad_src_ratio (src_ratio))
		return SRC_ERR_BAD_INTERNAL_STATE ;

	input_index = state->last_position ;

	/* Calculate samples before first sample in input array. */
	while (input_index < 1.0 && priv->out_gen < priv->out_count)
	{
		if (priv->in_used + state->channels * input_index >= priv->in_count)
			break ;

		if (priv->out_count > 0 && fabs (state->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = state->last_ratio + priv->out_gen * (data->src_ratio - state->last_ratio) / priv->out_count ;

		for (ch = 0 ; ch < state->channels ; ch++)
		{	data->data_out [priv->out_gen] = priv->last_value [ch] ;
			priv->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		} ;

	rem = fmod_one (input_index) ;
	priv->in_used += state->channels * lrint (input_index - rem) ;
	input_index = rem ;

	/* Main processing loop. */
	while (priv->out_gen < priv->out_count && priv->in_used + state->channels * input_index <= priv->in_count)
	{
		if (priv->out_count > 0 && fabs (state->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = state->last_ratio + priv->out_gen * (data->src_ratio - state->last_ratio) / priv->out_count ;

		for (ch = 0 ; ch < state->channels ; ch++)
		{	data->data_out [priv->out_gen] = data->data_in [priv->in_used - state->channels + ch] ;
			priv->out_gen ++ ;
			} ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		rem = fmod_one (input_index) ;

		priv->in_used += state->channels * lrint (input_index - rem) ;
		input_index = rem ;
		} ;

	if (priv->in_used > priv->in_count)
	{	input_index += (priv->in_used - priv->in_count) / state->channels ;
		priv->in_used = priv->in_count ;
		} ;

	state->last_position = input_index ;

	if (priv->in_used > 0)
		for (ch = 0 ; ch < state->channels ; ch++)
			priv->last_value [ch] = data->data_in [priv->in_used - state->channels + ch] ;

	/* Save current ratio rather then target ratio. */
	state->last_ratio = src_ratio ;

	data->input_frames_used = priv->in_used / state->channels ;
	data->output_frames_gen = priv->out_gen / state->channels ;

	return SRC_ERR_NO_ERROR ;
} /* zoh_vari_process */

/*------------------------------------------------------------------------------
*/

const char*
zoh_get_name (int src_enum)
{
	if (src_enum == SRC_ZERO_ORDER_HOLD)
		return "ZOH Interpolator" ;

	return NULL ;
} /* zoh_get_name */

const char*
zoh_get_description (int src_enum)
{
	if (src_enum == SRC_ZERO_ORDER_HOLD)
		return "Zero order hold interpolator, very fast, poor quality." ;

	return NULL ;
} /* zoh_get_descrition */

enum SRC_ERR
zoh_set_converter (SRC_STATE *state, int src_enum)
{	ZOH_DATA *priv = NULL ;

	if (src_enum != SRC_ZERO_ORDER_HOLD)
		return SRC_ERR_BAD_CONVERTER ;

	zoh_close (state) ;

	if (state->private_data == NULL)
	{	priv = ZERO_ALLOC (ZOH_DATA, sizeof (*priv) + state->channels * sizeof (float)) ;
		state->private_data = priv ;
		} ;

	if (priv == NULL)
		return SRC_ERR_MALLOC_FAILED ;

	priv->zoh_magic_marker = ZOH_MAGIC_MARKER ;

	state->const_process = zoh_vari_process ;
	state->vari_process = zoh_vari_process ;
	state->reset = zoh_reset ;
	state->copy = zoh_copy ;
	state->close = zoh_close ;

	zoh_reset (state) ;

	return SRC_ERR_NO_ERROR ;
} /* zoh_set_converter */

/*===================================================================================
*/

static void
zoh_reset (SRC_STATE *state)
{	ZOH_DATA *priv ;

	priv = (ZOH_DATA*) state->private_data ;
	if (priv == NULL)
		return ;

	priv->dirty = false ;
	memset (priv->last_value, 0, sizeof (priv->last_value [0]) * state->channels) ;

	return ;
} /* zoh_reset */

static enum SRC_ERR
zoh_copy (SRC_STATE *from, SRC_STATE *to)
{
	if (from->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	ZOH_DATA *to_priv = NULL ;
	ZOH_DATA* from_priv = (ZOH_DATA*) from->private_data ;
	size_t private_size = sizeof (*to_priv) + from->channels * sizeof (float) ;

	if ((to_priv = ZERO_ALLOC (ZOH_DATA, private_size)) == NULL)
		return SRC_ERR_MALLOC_FAILED ;

	memcpy (to_priv, from_priv, private_size) ;
	to->private_data = to_priv ;

	return SRC_ERR_NO_ERROR ;
} /* zoh_copy */

static void
zoh_close (SRC_STATE *state)
{
	if (state)
	{
		if (state->private_data)
		{
			free (state->private_data) ;
			state->private_data = NULL ;
		}
	}
} /* zoh_close */
