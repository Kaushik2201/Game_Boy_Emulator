#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

#define TAC_MODE_0_CYCLES  1024  // 4096 Hz
#define TAC_MODE_1_CYCLES  16    // 262144 Hz
#define TAC_MODE_2_CYCLES  64   // 65536 Hz
#define TAC_MODE_3_CYCLES  256   // 16384 Hz

void timer_connect(void);
void timer_init(void);
void timer_update(uint16_t cycles);
void timer_cleanup(void);

typedef struct {
    uint16_t DIV;   
    uint8_t TIMA;   
    uint8_t TMA;    
    uint8_t TAC;    
    uint16_t counter; 
} gbc_timer_t;
#endif 
    