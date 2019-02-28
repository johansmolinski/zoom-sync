#include "pulse.h"

uint16_t g_measure_counter = 0;
uint16_t g_beat_counter = 0;
uint8_t g_pulse_counter = 0;
uint8_t g_sync_on = 0;
uint8_t g_flank_read = 0;
uint16_t g_tempo = 0;

void pulse(unsigned char *reg_in, unsigned char *reg_out, uint8_t bit_in, uint8_t bit_out) {
  // Flank reset
  if ((*reg_in & 1 << bit_in) == 0) {
    g_flank_read = 0;
  }

  // Flank triggered, restart Tempo counting
  if ((*reg_in & 1 << bit_in) && g_flank_read == 0) {
    g_flank_read = 1;
    g_tempo = g_measure_counter / MULTIPLYER;
    g_sync_on = PULSE_TIME;
    g_measure_counter = 0;
    g_pulse_counter = 1;
  }

  // Counter reaching divided Tempo length, trig Pulse
  if (g_beat_counter == g_tempo && g_pulse_counter < MULTIPLYER) {
    g_sync_on = PULSE_TIME;
    g_beat_counter = 0;
    g_pulse_counter++;
  }

  // Pulse is triggered, emit led
  if (g_sync_on) {
    *reg_out |= 1 << bit_out;
    g_sync_on--;
    //Serial.println("On");
  }
  else {
    *reg_out &= ~(1 << bit_out);
    //Serial.println("Off");
  }  

  g_beat_counter++;
  g_measure_counter++;
}
