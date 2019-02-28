#ifndef __PULSE__H
#define __PULSE__H

#include <stdint.h>

const int PULSE_TIME = 20;
const int MULTIPLYER = 4;

void pulse(unsigned char *reg_in, unsigned char *reg_out, uint8_t bit_in, uint8_t bit_out);

#endif
