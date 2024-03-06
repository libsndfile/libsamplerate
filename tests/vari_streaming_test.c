/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/libsndfile/libsamplerate/blob/master/COPYING
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <math.h>
#include <samplerate.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define BUFFER_LEN (1 << 15)
#define INPUT_BUFFER_LEN (BUFFER_LEN)
#define OUTPUT_BUFFER_LEN (BUFFER_LEN)
#define BLOCK_LEN 192

static void vari_stream_test(int converter, double ratio);

int main(void) {
  static double src_ratios[] = {0.3, 0.9, 1.1, 3.0};

  int k;

  puts("\n    Zero Order Hold interpolator:");
  for (k = 0; k < ARRAY_LEN(src_ratios); k++)
    vari_stream_test(SRC_ZERO_ORDER_HOLD, src_ratios[k]);

  puts("\n    Linear interpolator:");
  for (k = 0; k < ARRAY_LEN(src_ratios); k++)
    vari_stream_test(SRC_LINEAR, src_ratios[k]);

  puts("\n    Sinc interpolator:");
  for (k = 0; k < ARRAY_LEN(src_ratios); k++)
    vari_stream_test(SRC_SINC_FASTEST, src_ratios[k]);

  puts("");

  return 0;
} /* main */

struct generate_waveform_state_s {
  float phase;
  float phase_increment;
};

static void generate_waveform(struct generate_waveform_state_s* state, float* output, size_t len) {
  for(size_t n = 0; n < len; ++n) {
    output[n] = state->phase;
    state->phase += state->phase_increment;
    if (state->phase > 1.0 || state->phase < -1.0)
      state->phase = state->phase > 0 ?  -1.0 : 1.0;
  }
}

static void vari_stream_test(int converter, double src_ratio) {
  float input[INPUT_BUFFER_LEN], output[OUTPUT_BUFFER_LEN];

  SRC_STATE *src_state;

  int error, terminate;

  printf("\tvari_streaming_test   (SRC ratio = %6.4f) ........... ", src_ratio);
  fflush(stdout);

  /* Perform sample rate conversion. */
  if ((src_state = src_new(converter, 1, &error)) == NULL) {
    printf("\n\nLine %d : src_new() failed : %s\n\n", __LINE__,
           src_strerror(error));
    exit(1);
  };

  struct generate_waveform_state_s generator = {
    .phase = 0.0,
    .phase_increment = 2 / 48000, // 1Hz at 48000kHz samplerate.
  };
  
  double resampled_time = 0;
  const size_t input_period_sizes[] = {
    BLOCK_LEN,
    BLOCK_LEN,
    BLOCK_LEN,
    1,
    BLOCK_LEN,
  };
    
  int num_input_frame_used = 0;
  int num_frames_outputed = 0;
  int total_num_input_frames = 0;
  
  int current_num_frames = 0;
  for (size_t n = 0; n < ARRAY_LEN(input_period_sizes); ++n) {
    generate_waveform(&generator, input, input_period_sizes[n]);
    total_num_input_frames += input_period_sizes[n];
    current_num_frames += input_period_sizes[n];
    SRC_DATA src_data = {
      .data_in = input,
      .data_out = output,
      .input_frames = input_period_sizes[n],
      .output_frames = OUTPUT_BUFFER_LEN,
      .input_frames_used = 0,
      .output_frames_gen = 0,
      .end_of_input = n == ARRAY_LEN(input_period_sizes) - 1,
      .src_ratio = src_ratio,
    };
    // printf("eoi: %d\n", src_data.end_of_input); 
    if ((error = src_process(src_state, &src_data))) {
      printf("\n\nLine %d : %s\n\n", __LINE__, src_strerror(error));
      
      printf("src_data.input_frames  : %ld\n", src_data.input_frames);
      printf("src_data.output_frames : %ld\n", src_data.output_frames);
      
      exit(1);
    };
    
    
    num_input_frame_used += src_data.input_frames_used;
    num_frames_outputed += src_data.output_frames_gen;

    memmove(
	    input,
	    input + src_data.input_frames_used,
	    (INPUT_BUFFER_LEN - src_data.input_frames_used) * sizeof(input[0])
	    ); 
    current_num_frames -= src_data.input_frames_used;
    if (src_data.end_of_input) break;
  };

  src_state = src_delete(src_state);

  terminate = (int)ceil((src_ratio >= 1.0) ? src_ratio : 1.0 / src_ratio);

  if (fabs(num_frames_outputed - src_ratio * total_num_input_frames) > 2 * terminate) {
    printf("\n\nLine %d : bad output data length %d should be %d.\n", __LINE__,
           num_frames_outputed, (int)floor(src_ratio * total_num_input_frames));
    printf("\tsrc_ratio  : %.4f\n", src_ratio);
    printf("\tinput_len  : %d\n\toutput_len : %d\n\n", total_num_input_frames, num_frames_outputed);
    exit(1);
  };

  if (num_input_frame_used != total_num_input_frames) {
    printf("\n\nLine %d : unused %d input frames.\n", __LINE__, current_num_frames);
    printf("\tinput_len         : %d\n", total_num_input_frames);
    printf("\tinput_frames_used : %d\n\n", num_input_frame_used);
    exit(1);
  };

  puts("ok");

  return;
} /* stream_test */
