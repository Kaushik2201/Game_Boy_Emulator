#include "isa.h"
#include "common.h"
#include "cpu.h"
#include "isa.h"



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

static void adc_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins)
{
    LOG_DEBUG("ADC r8, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)ins->op1;
    size_t reg2_offset = (size_t)ins->op2;
    uint8_t v1 = READ_R8(regs, reg_offset);
    uint8_t v2 = READ_R8(regs, reg2_offset);
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);
    uint8_t hc = HALF_CARRY_ADC(v1, v2, carry);

    uint8_t result = v1 + v2 + carry;
    carry = (v1 + v2 + carry) > UINT8_MASK;

    WRITE_R8(regs, reg_offset, result);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG_VALUE(regs, FLAG_H, hc);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
}