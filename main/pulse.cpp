#include "pulse.h"

#define read_in(register_in, bit_in) (*register_in & 1 << bit_in)
#define set_high(register_out, bit_out) (*register_out |= 1 << bit_out)
#define set_low(register_out, bit_out) (*register_out &= ~(1 << bit_out))

void pulse(state_t *state) {
  // Flank reset
  if (read_in(state->register_in, state->bit_in) == 0) {
    state->flank_read = 0;
  }

  // Flank triggered, restart Tempo counting
  if (read_in(state->register_in, state->bit_in) && state->flank_read == 0) {
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
    set_high(state->register_out, state->bit_out);
    state->sync_on--;
    //Serial.println("On");
  }
  else {
    set_low(state->register_out, state->bit_out);
    //Serial.println("Off");
  }  

  state->beat_counter++;
  state->measure_counter++;
}
