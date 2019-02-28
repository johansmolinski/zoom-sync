#include "pulse.h"

state_t state;

void setup() {
  state.pulse_time = 200;        // Number of ticks for pulse to be active
  state.multiplier = 16;         // Number of generated pulses per given pulse

  state.register_in = &PIND;     // Input register
  state.trig_in = 2;             // Trig input bit
  state.gate_in = 3;             // Gate input bit
  state.register_out = &PORTB;   // Output register
  state.pulse_out = 5;           // Pulse output bit
  state.start_out = 6;           // Start output bit

  DDRD = 0x00;                   // PD all input
  PORTD = 0x00;                  // No pullups
  DDRB = 0xff;                   // PB all output
  
  //Serial.begin(9600);
}


void loop() {
  //Serial.println(String(PIND & 0x04));

  pulse(&state);

  delayMicroseconds(100);
}
