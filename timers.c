#include "timers.h"
#include "memory.h"
#include "cpu.h"
#include "common.h"
#include<stddef.h>
#include<string.h>

void gbc_timer_init(gbc_timer_t *timer){
    if( timer == NULL){
        return;
    }
    memset(timer, 0, sizeof(gbc_timer_t));
}
void gbc_timer_connect(gbc_timer_t *timer, gbc_memory_t *mem){
    if(timer == NULL || mem == NULL){
        return ;
    }
    timer -> DIV = 0;
    timer -> TIMA = 0;
    timer -> TMA = 0;
    timer -> TAC = 0;
    timer ->counter =0;

}
void gbc_timer_cycle(gbc_timer_t *timer){
    if(timer == NULL){
        return ;
    }

    timer->DIV++;

    if ( timer->TAC & 0x4){
        uint8_t cycles = 0;
        switch (timer->TAC & 0x3){
            case 0: cycles = TAC_MODE_0_CYCLES; break;
            case 1: cycles = TAC_MODE_1_CYCLES; break;
            case 2: cycles = TAC_MODE_2_CYCLES; break;
            case 3: cycles = TAC_MODE_3_CYCLES; break;
        }
        timer->counter += cycles;
        if(timer->counter >= 256){
            timer->TIMA++;
            timer->counter -= 256;
            if(timer->TIMA == 0){
                timer->TIMA = timer->TMA;
                CPU_REQUEST_INTERRUPT(timer->mem, INTERRUPT_TIMER);
            }
        }
    }
}