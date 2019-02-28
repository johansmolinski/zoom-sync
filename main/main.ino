#include "pulse.h"

state_t state;

void setup() {

  DDRD &= ~(0b1100); // PD2 and PD3 inputs
  PORTD &= ~(0b1100);
  DDRB |= 1 << 5; // PB5 output
  
  //Serial.begin(9600);
}


void loop() {
  //Serial.println(String(PIND & 0x04));

  pulse(&state, &PIND, &PORTB, 2, 5);

  delay(1);
}
