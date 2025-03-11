#include "isa.h"
#include "common.h"
#include "cpu.h"



static void stop(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("STOP: %s\n", ins->name);
    gbc_memory_t *mem = (gbc_memory_t*)cpu->mem_data;
    uint8_t key1 = IO_PORT_READ(mem, IO_PORT_KEY1);
    if (key1 & KEY1_CPU_SWITCH_ARMED) {
        cpu->dspeed = !cpu->dspeed;
        if (cpu->dspeed)
            key1 |= KEY1_CPU_CURRENT_MODE;
        else
            key1 &= ~KEY1_CPU_CURRENT_MODE;

        key1 &= ~KEY1_CPU_SWITCH_ARMED;
        IO_PORT_WRITE(mem, IO_PORT_KEY1, key1);
    }

    LOG_INFO("[CPU] Speed Switch %s -> %s\n",
     (cpu->dspeed ? "NORMAL":"DOUBLE"), 
     (cpu->dspeed ? "DOUBLE":"NORMAL"));
} 

static void inc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    
}