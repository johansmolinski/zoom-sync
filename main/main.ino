#include "pulse.h"

state_t state;

void setup() {
  state.pulse_time = 100;        // Number of ticks for pulse to be active
  state.multiplier = 2;         // Number of generated pulses per given pulse

  state.register_in = &PINB;     // Input register
  state.register_debug = &PORTB; // Debug (builtin led PB5)
  state.trig_in = 2;             // Trig input bit
  // stop = 3, play = 4
  state.gate_in = 3;             // Gate input bit
  state.register_out = &PORTD;   // Output register
  state.pulse_out = 2;           // Pulse output bit
  state.start_out = 3;           // Start output bit

  DDRD = 0xff;                   // PD all output
  DDRB = 0b00100000;             // PB all input except PB5 (builtin led)
  PORTB = 0x00;                  // No pullups
  
  //Serial.begin(9600);
}


void loop() {
  //Serial.println(String(PIND & 0x04));

  pulse(&state);

  delayMicroseconds(100);
}
