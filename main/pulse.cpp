#include "pulse.h"
#include <stdio.h>

#define read_in(register_in, bit_in) (*register_in & 1 << bit_in)
#define set_high(register_out, bit_out) (*register_out |= 1 << bit_out)
#define set_low(register_out, bit_out) (*register_out &= ~(1 << bit_out))

void makePulse(state_t *state) {
  state->sync_on = state->pulse_time;
}

void syncManager(state_t *state) {
  if (state->sync_on) {
    set_high(state->register_out, state->pulse_out);
    state->sync_on--;
    //Serial.println("On");
  }
  else {
    set_low(state->register_out, state->pulse_out);
    //Serial.println("Off");
  }
}

void flankResetOnPulseLow(state_t *state, uint8_t *bit_in, uint8_t *flank_read) {
  if (read_in(state->register_in, *bit_in) == 0) {
    *flank_read = 0;
  }
}

void startHostBeat(state_t *state) {
    state->tempo = state->measure_counter / state->multiplier;
    makePulse(state);
    state->measure_counter = 0;
    state->pulse_counter = state->multiplier - 1;
    state->beat_counter = 0;
}

bool flankTriggerOnPulseHigh(state_t *state, uint8_t *bit_in, uint8_t *flank_read) {
  if (read_in(state->register_in, *bit_in) && *flank_read == 0) {
    *flank_read = 1;
    return true;
  }
  return false;
}

void beatDividerTrig(state_t *state) {
  state->beat_counter++;
  if (state->beat_counter == state->tempo && state->pulse_counter) {
    makePulse(state);
    state->beat_counter = 0;
    state->pulse_counter--;
  }
}

void pulse(state_t *state) {
  // Flank reset
  flankResetOnPulseLow(state, &state->trig_in, &state->flank_trig_read);

  // Flank trigger check
  if (flankTriggerOnPulseHigh(state, &state->trig_in, &state->flank_trig_read)) {
    // Triggered!
    startHostBeat(state);
  }

  // Counter reaching divided Tempo length, trig Pulse
  beatDividerTrig(state);

  // Pulse is triggered, emit led
  syncManager(state);

  state->measure_counter++;
}
