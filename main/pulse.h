#ifndef __PULSE__H
#define __PULSE__H

#include <stdint.h>

typedef struct _state {
  // State variables
  uint16_t measure_counter = 0;
  uint16_t beat_counter = 0;
  uint8_t pulse_counter = 0;
  uint8_t sync_pulse_on = 0;
  uint8_t sync_start_on = 0;
  uint8_t flank_trig_read = 0;
  bool gate_on;
  bool trig_since_gate;
  uint16_t tempo = 0;

  // Configuration
  uint16_t pulse_time;
  uint8_t multiplier;
  unsigned char *register_in, *register_out, *register_debug;
  uint8_t trig_in, play_in, stop_in;
  uint8_t pulse_out, gate_out;
} state_t;

void pulse(state_t *state);

#endif
