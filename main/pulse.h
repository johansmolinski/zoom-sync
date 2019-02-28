#ifndef __PULSE__H
#define __PULSE__H

#include <stdint.h>

const int PULSE_TIME = 20;
const int MULTIPLYER = 4;

typedef struct _state {
  uint16_t measure_counter = 0;
  uint16_t beat_counter = 0;
  uint8_t pulse_counter = 0;
  uint8_t sync_on = 0;
  uint8_t flank_read = 0;
  uint16_t tempo = 0;
} state_t;

void pulse(state_t *state, unsigned char *reg_in, unsigned char *reg_out, uint8_t bit_in, uint8_t bit_out);

#endif
