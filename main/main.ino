#include "pulse.h"

state_t state;

void setup() {
  state.pulse_time = 200;        // Number of ticks for pulse to be active
  state.multiplier = 16;         // Number of generated pulses per given pulse

  state.register_in = &PIND;     // Input register
  state.bit_in = 2;              // Input bit
  state.register_out = &PORTB;   // Output register
  state.bit_out = 5;             // Output bit

  DDRD &= ~(1 << state.bit_in);  // Set bit_in as input
  PORTD &= ~(1 << state.bit_in); // Disable pullup
  DDRB |= 1 << state.bit_out;    // Set bit_out as output
  
  //Serial.begin(9600);
}


void loop() {
  //Serial.println(String(PIND & 0x04));

  pulse(&state);

  delayMicroseconds(100);
}
