#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TAC_MODE_0  256  // 4096 Hz
#define TAC_MODE_1  4    // 262144 Hz
#define TAC_MODE_2  16   // 65536 Hz
#define TAC_MODE_3  64   // 16384 Hz

#define TIMER_CYCLES_PER_TICK 4

void timer_connect(void);
void timer_init(void);
void timer_update(uint16_t cycles);
void timer_cleanup(void);

#endif 
