#include "pulse.h"

void setup() {
  // put your setup code here, to run once:

  DDRD &= ~(0b1100); // PD2 and PD3 inputs
  PORTD &= ~(0b1100);
  DDRB |= 1 << 5; // PB5 output
  
  Serial.begin(9600);
}


void loop() {
  // put your main code here, to run repeatedly:
  //Serial.println(String(PIND & 0x04));

  pulse(&PIND, &PORTB, 2, 5);

  delay(1);
}
