#include "timers.h"
#include "memory.h"
#include "cpu.h"
#include "common.h"
#include <stddef.h>
#include <string.h>

// Define the array of timer modes
static const 
uint16_t timer_modes[4] = {
    TAC_MODE_0_CYCLES,
    TAC_MODE_1_CYCLES,
    TAC_MODE_2_CYCLES,
    TAC_MODE_3_CYCLES
};

// Initialize the timer
void 
gbc_timer_init(gbc_timer_t *timer) 
{
    if (timer == NULL) {
        return;
    }

    memset(timer, 0, sizeof(gbc_timer_t));
}

// Connect the timer to the memory
void 
gbc_timer_connect(gbc_timer_t *timer, gbc_memory_t *mem) 
{
    if (timer == NULL || mem == NULL) {
        return;
    }

    // Store the memory pointer
    timer->mem = mem;

    // Map the timer registers to the memory-mapped I/O addresses
    timer->divp = connect_io_port(mem, IO_PORT_DIV);
    timer->timap = connect_io_port(mem, IO_PORT_TIMA);
    timer->tmap = connect_io_port(mem, IO_PORT_TMA);
    timer->tacp = connect_io_port(mem, IO_PORT_TAC);
}

// Cycle the timer
void 
gbc_timer_cycle(gbc_timer_t *timer) 
{
    if (timer == NULL || timer->mem == NULL) {
        return;
    }

    // Increment the DIV register
    timer->div_cycles++;
    if (timer->div_cycles >= TICK_DIVIDER) {
        timer->div_cycles = 0;
        (*timer->divp)++;
    }

    if (!(*timer->tacp & TAC_TIMER_ENABLE)) {
        return;
    }

    // Check if the timer is enabled
    uint8_t mode = *timer->tacp & TAC_TIMER_SPEED_MASK;
    uint16_t cycles = timer_modes[mode];

    uint16_t tima = *timer->timap;

    if (++timer->timer_cycles == cycles) {
        timer->timer_cycles = 0;
        tima += 1;
        if (tima > 0xFF) {
            *timer->timap = *timer->tmap;
            REQUEST_INTERRUPT(timer->mem, INTERRUPT_TIMER);
        }
        IO_PORT_WRITE(timer->mem, IO_PORT_TIMA, tima);
    }
}