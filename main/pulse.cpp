#include "pulse.h"

void pulse(state_t *state, unsigned char *reg_in, unsigned char *reg_out, uint8_t bit_in, uint8_t bit_out) {
  // Flank reset
  if ((*reg_in & 1 << bit_in) == 0) {
    state->flank_read = 0;
  }

  // Flank triggered, restart Tempo counting
  if ((*reg_in & 1 << bit_in) && state->flank_read == 0) {
    state->flank_read = 1;
    state->tempo = state->measure_counter / MULTIPLYER;
    state->sync_on = PULSE_TIME;
    state->measure_counter = 0;
    state->pulse_counter = 1;
  }

  // Counter reaching divided Tempo length, trig Pulse
  if (state->beat_counter == state->tempo && state->pulse_counter < MULTIPLYER) {
    state->sync_on = PULSE_TIME;
    state->beat_counter = 0;
    state->pulse_counter++;
  }

  // Pulse is triggered, emit led
  if (state->sync_on) {
    *reg_out |= 1 << bit_out;
    state->sync_on--;
    //Serial.println("On");
  }
  else {
    *reg_out &= ~(1 << bit_out);
    //Serial.println("Off");
  }  

  state->beat_counter++;
  state->measure_counter++;
}
