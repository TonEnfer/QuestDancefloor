#pragma once

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

extern "C" void output_rbg(uint8_t * ptr, uint16_t count);
