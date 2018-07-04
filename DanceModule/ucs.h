#pragma once
#include <avr/io.h> 

#ifdef __ASSEMBLER__

#else

void output_rbg(uint8_t * ptr, uint16_t count);
#endif
