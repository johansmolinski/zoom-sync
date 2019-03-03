#include "pulse.h"
#include <stdio.h>

#define read_in(register_in, bit_in) (*register_in & 1 << bit_in)
#define set_high(register_out, bit_out) (*register_out |= 1 << bit_out)
#define set_low(register_out, bit_out) (*register_out &= ~(1 << bit_out))

void makePulse(state_t *state) {
  if (state->gate_on) {
    state->sync_pulse_on = state->pulse_time;
  }
}

void buttonManager(state_t *state) {
  if (read_in(state->buttonline_in, state->line0_in)) {
    state->last_line = 0;
    //set_low(state->register_debug, 5); //debug
  }
  if (read_in(state->buttonline_in, state->line1_in)) {
    state->last_line = 1;
    //set_high(state->register_debug, 5); //debug
  }

  if (read_in(state->register_in, state->play_in) == 0 && state->last_line == 1) { // play
    state->gate_on = true;
  }
  if (read_in(state->register_in, state->stop_in) == 0 && state->last_line == 1) { // stop
    state->trig_since_gate = false;
  }
  if (read_in(state->register_in, state->rewind_in) == 0 && state->last_line == 0) { // rewind
    state->gate_on = false;
    state->trig_since_gate = false;
  }
}

void syncManager(state_t *state, uint8_t *sync, uint8_t out) {
  if (*sync) {
    set_high(state->register_out, out);
    set_high(state->register_debug, 5); //debug
    (*sync)--;
    //Serial.println("On");
  }
  else {
    set_low(state->register_out, out);
    set_low(state->register_debug, 5); //debug
    //Serial.println("Off");
  }
}

void flankResetOnPulseLow(state_t *state, uint8_t *bit_in, uint8_t *flank_read) {
  if (read_in(state->register_in, *bit_in) == 0) {
    *flank_read = 0;
  }
}

void startHostBeat(state_t *state) {
  if (state->trig_since_gate) {
    state->tempo = state->measure_counter / state->multiplier;
  }
  else {
    state->trig_since_gate = true;
  }

  state->sync_start_on = state->pulse_time;
  makePulse(state);
  state->measure_counter = 0;
  state->pulse_counter = state->multiplier - 1;
  state->beat_counter = 0;
  set_high(state->register_out, state->gate_out);
}

void stopHostBeat(state_t *state) {
  set_low(state->register_out, state->gate_out);
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

  buttonManager(state);

  if (state->gate_on) {
    // Flank trigger check
    if (flankTriggerOnPulseHigh(state, &state->trig_in, &state->flank_trig_read)) {
      // Triggered!
      startHostBeat(state);
    }

    // Counter reaching divided Tempo length, trig Pulse
    beatDividerTrig(state);
  }
  else {
    stopHostBeat(state);
  }
  // Pulse is triggered, emit led
  syncManager(state, &(state->sync_pulse_on), state->pulse_out);

  state->measure_counter++;
}
