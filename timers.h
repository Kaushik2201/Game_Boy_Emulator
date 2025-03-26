#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "memory.h"

#define TICK_DIVIDER 256      /* 16384Hz in cpu normal mode, equivalent to 256 cpu cycles */

#define TAC_TIMER_ENABLE     0x04
#define TAC_TIMER_SPEED_MASK 0x03

#define TAC_TIMER_SPEED_MODE_0 0
#define TAC_TIMER_SPEED_MODE_1 1
#define TAC_TIMER_SPEED_MODE_2 2
#define TAC_TIMER_SPEED_MODE_3 3

#define TAC_MODE_0_CYCLES  1024  // 4096 Hz
#define TAC_MODE_1_CYCLES  16    // 262144 Hz
#define TAC_MODE_2_CYCLES  64   // 65536 Hz
#define TAC_MODE_3_CYCLES  256   // 16384 Hz


typedef struct gbc_timer {
    gbc_memory_t *mem;
    uint16_t div_cycles;
    uint16_t timer_cycles;
    uint8_t *divp;
    uint8_t *timap;
    uint8_t *tmap;
    uint8_t *tacp;

} gbc_timer_t;

void gbc_timer_init(gbc_timer_t *timer);
void gbc_timer_connect(gbc_timer_t *timer, gbc_memory_t *mem);
void gbc_timer_cycle(gbc_timer_t *timer);

#endif 
    