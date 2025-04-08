/*
** Copyright (c) 2002-2021, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "common.h"

static SRC_ERROR linear_vari_process (SRC_STATE *state, SRC_DATA *data) ;
static void linear_reset (SRC_STATE *state) ;
static SRC_STATE *linear_copy (SRC_STATE *state) ;
static void linear_close (SRC_STATE *state) ;

/*========================================================================================
*/

#define	LINEAR_MAGIC_MARKER	MAKE_MAGIC ('l', 'i', 'n', 'e', 'a', 'r')

#define	SRC_DEBUG	0

typedef struct
{	int		linear_magic_marker ;
	bool	initialized ;
	float	*last_value ;
} LINEAR_DATA ;

static SRC_STATE_VT linear_state_vt =
{
	linear_vari_process,
	linear_vari_process,
	linear_reset,
	linear_copy,
	linear_close
} ;

/*----------------------------------------------------------------------------------------
*/

static SRC_ERROR
linear_vari_process (SRC_STATE *state, SRC_DATA *data)
{	LINEAR_DATA *priv ;
	double		src_ratio, input_index, rem ;

	if (data->input_frames <= 0)
		return SRC_ERR_NO_ERROR ;

	if (state->private_data == NULL)
		return SRC_ERR_NO_PRIVATE ;

	priv = (LINEAR_DATA*) state->private_data ;

	if (!priv->initialized)
	{	/* If we have just been reset, set the last_value data. */
		for (int ch = 0 ; ch < state->channels ; ch++)
			priv->last_value [ch] = data->data_in [ch] ;
		priv->initialized = true ;
		} ;

	data->input_frames_used = data->output_frames_gen = 0 ;

	src_ratio = state->last_ratio ;

	if (is_bad_src_ratio (src_ratio))
		return SRC_ERR_BAD_INTERNAL_STATE ;

	input_index = state->last_position ;

	/* Calculate samples before first sample in input array. */
	float* current_out = data->data_out;
	while (input_index < 1.0 && data->output_frames_gen < data->output_frames)
	{
		if (data->output_frames > 0 && fabs (state->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = state->last_ratio + data->output_frames_gen * (data->src_ratio - state->last_ratio) / data->output_frames ;

		for (int ch = 0 ; ch < state->channels ; ch++)
		{	*current_out++ = (float) (priv->last_value [ch] + input_index *
										((double) data->data_in [ch] - priv->last_value [ch])) ;
			} ;
		data->output_frames_gen ++ ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		} ;

	rem = fmod_one (input_index) ;
	data->input_frames_used += psf_lrint (input_index - rem) ;
	input_index = rem ;

	/* Main processing loop. */
#if SRC_DEBUG
	assert(data->output_frames_gen >= data->output_frames || data->input_frames_used > 0);
#endif
	while (data->output_frames_gen < data->output_frames && data->input_frames_used + input_index < data->input_frames)
	{
		if (data->output_frames > 0 && fabs (state->last_ratio - data->src_ratio) > SRC_MIN_RATIO_DIFF)
			src_ratio = state->last_ratio + data->output_frames_gen * (data->src_ratio - state->last_ratio) / data->output_frames ;

		const float* current_in = data->data_in + data->input_frames_used * state->channels;
		const float* prev_in = current_in - state->channels;
		for (int ch = 0 ; ch < state->channels ; ch++)
		{
		  *current_out++ = (float) (prev_in[ch] + input_index *
						((double) current_in[ch] - prev_in[ch])) ;
		}
		data->output_frames_gen ++ ;

		/* Figure out the next index. */
		input_index += 1.0 / src_ratio ;
		rem = fmod_one (input_index) ;

		const int num_frame_used = psf_lrint (input_index - rem);
		data->input_frames_used += num_frame_used ;
		input_index = rem ;
        }

	if (data->input_frames_used > data->input_frames)
	{
          input_index += (data->input_frames_used - data->input_frames) ;
          data->input_frames_used = data->input_frames ;
        }

	state->last_position = input_index ;

	if (data->input_frames_used > 0) {
          const float *last_value = data->data_in + (data->input_frames_used - 1) * state->channels;
          for (int ch = 0 ; ch < state->channels ; ch++)
            priv->last_value [ch] = last_value[ch];
        }

	/* Save current ratio rather then target ratio. */
	state->last_ratio = src_ratio ;

	return SRC_ERR_NO_ERROR ;
} /* linear_vari_process */

/*------------------------------------------------------------------------------
*/

LIBSAMPLERATE_DLL_PRIVATE const char*
linear_get_name (int src_enum)
{
	if (src_enum == SRC_LINEAR)
		return "Linear Interpolator" ;

	return NULL ;
} /* linear_get_name */

LIBSAMPLERATE_DLL_PRIVATE const char*
linear_get_description (int src_enum)
{
	if (src_enum == SRC_LINEAR)
		return "Linear interpolator, very fast, poor quality." ;

	return NULL ;
} /* linear_get_descrition */

static LINEAR_DATA *
linear_data_new (int channels)
{
	assert (channels > 0) ;

	LINEAR_DATA *priv = (LINEAR_DATA *) calloc (1, sizeof (LINEAR_DATA)) ;
	if (priv)
	{
		priv->linear_magic_marker = LINEAR_MAGIC_MARKER ;
		priv->last_value = (float *) calloc (channels, sizeof (float)) ;
		if (!priv->last_value)
		{
			free (priv) ;
			priv = NULL ;
		}
	}

	return priv ;
}

LIBSAMPLERATE_DLL_PRIVATE SRC_STATE *
linear_state_new (int channels, SRC_ERROR *error)
{
	assert (channels > 0) ;
	assert (error != NULL) ;

	SRC_STATE *state = (SRC_STATE *) calloc (1, sizeof (SRC_STATE)) ;
	if (!state)
	{
		*error = SRC_ERR_MALLOC_FAILED ;
		return NULL ;
	}

	state->channels = channels ;
	state->mode = SRC_MODE_PROCESS ;

	state->private_data = linear_data_new (state->channels) ;
	if (!state->private_data)
	{
		free (state) ;
		*error = SRC_ERR_MALLOC_FAILED ;
		return NULL ;
	}

	state->vt = &linear_state_vt ;

	linear_reset (state) ;

	*error = SRC_ERR_NO_ERROR ;

	return state ;
}

/*===================================================================================
*/

static void
linear_reset (SRC_STATE *state)
{	LINEAR_DATA *priv = NULL ;

	priv = (LINEAR_DATA*) state->private_data ;
	if (priv == NULL)
		return ;

	priv->initialized = false ;
	memset (priv->last_value, 0, sizeof (priv->last_value [0]) * state->channels) ;

	return ;
} /* linear_reset */

SRC_STATE *
linear_copy (SRC_STATE *state)
{
	assert (state != NULL) ;

	if (state->private_data == NULL)
		return NULL ;

	SRC_STATE *to = (SRC_STATE *) calloc (1, sizeof (SRC_STATE)) ;
	if (!to)
		return NULL ;
	memcpy (to, state, sizeof (SRC_STATE)) ;

	LINEAR_DATA* from_priv = (LINEAR_DATA*) state->private_data ;
	LINEAR_DATA *to_priv = (LINEAR_DATA *) calloc (1, sizeof (LINEAR_DATA)) ;
	if (!to_priv)
	{
		free (to) ;
		return NULL ;
	}

	memcpy (to_priv, from_priv, sizeof (LINEAR_DATA)) ;
	to_priv->last_value = (float *) malloc (sizeof (float) * state->channels) ;
	if (!to_priv->last_value)
	{
		free (to) ;
		free (to_priv) ;
		return NULL ;
	}
	memcpy (to_priv->last_value, from_priv->last_value, sizeof (float) * state->channels) ;

	to->private_data = to_priv ;

	return to ;
} /* linear_copy */

static void
linear_close (SRC_STATE *state)
{
	if (state)
	{
		LINEAR_DATA *linear = (LINEAR_DATA *) state->private_data ;
		if (linear)
		{
			if (linear->last_value)
			{
				free (linear->last_value) ;
				linear->last_value = NULL ;
			}
			free (linear) ;
			linear = NULL ;
		}
		free (state) ;
		state = NULL ;
	}
} /* linear_close */
