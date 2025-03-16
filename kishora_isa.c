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

static void inc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {   

    LOG_DEBUG("INC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint8_t x = READ_R8(reg, reg_offset);
    uint8_t hc = HALF_CARRY_ADD(x, 1);
    x++;

    WRITE_R8(reg, reg_offset, x);

    SET_R_FLAG_VALUE(reg, FLAG_Z, x == 0);
    CLEAR_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void inc_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {   

    LOG_DEBUG("INC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R16(reg, reg_offset);
    x++;

    WRITE_R16(reg, reg_offset, x);

}

static void inc_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("INC m16: %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint16_t addr = READ_R16(reg, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t hc = HALF_CARRY_ADD(value, 1);
    value++;

    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(reg, FLAG_Z, value == 0);
    CLEAR_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void dec_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {   

    LOG_DEBUG("DEC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint8_t x = READ_R8(reg, reg_offset);
    uint8_t hc = HALF_CARRY_SUB(x, 1);
    x--;

    WRITE_R8(reg, reg_offset, x);

    SET_R_FLAG_VALUE(reg, FLAG_Z, x == 0);
    SET_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void dec_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {   

    LOG_DEBUG("DEC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R16(reg, reg_offset);
    x--;

    WRITE_R16(reg, reg_offset, x);

}

static void dec_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("DEC m16: %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint16_t addr = READ_R16(reg, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t hc = HALF_CARRY_SUB(value, 1);
    value--;

    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(reg, FLAG_Z, value == 0);
    SET_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void rlca(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RLCA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = (a >> 7);

    a = (a << 1) | carry;

    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);
    CLEAR_R_FLAG(reg, FLAG_N);
    CLEAR_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_Z);

}

static void rla(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RLA: %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = READ_R_FLAG(reg, FLAG_C);

    uint8_t new_carry = a >> 7;
    a = (a << 1) | carry;

    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, new_carry);
    CLEAR_R_FLAG(reg, FLAG_N);
    CLEAR_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_Z);

}

static void rrca(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RRCA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = a & 1;

    a = (a >> 1) | (carry << 7);

    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);
    CLEAR_R_FLAG(reg, FLAG_N);
    CLEAR_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_Z);

}

static void rra(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RRA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = a & 1;
    uint8_t old_carry = READ_R_FLAG(reg, FLAG_C) ? 0x80 : 0;

    a = (a >> 1) | old_carry;

    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);
    CLEAR_R_FLAG(reg, FLAG_N);
    CLEAR_R_FLAG(reg, FLAG_H);
    CLEAR_R_FLAG(reg, FLAG_Z);

}

static void daa(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    /* https://ehaskins.com/2018-01-30%20Z80%20DAA/ */
    LOG_DEBUG("DAA: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t v = READ_R8(regs, REG_A);

    uint8_t n = READ_R_FLAG(regs, FLAG_N);
    uint8_t h = READ_R_FLAG(regs, FLAG_H);
    uint8_t c = READ_R_FLAG(regs, FLAG_C);

    uint8_t correction = 0;

    if (h || (!n && (v & 0xf) > 9)) {
        correction |= 0x6;
    }

    if (c || (!n && v > 0x99)) {
        correction |= 0x60;
        SET_R_FLAG(regs, FLAG_C);
    }

    v += n ? -correction : correction;

    WRITE_R8(regs, REG_A, v);

    SET_R_FLAG_VALUE(regs, FLAG_Z, v == 0);
    CLEAR_R_FLAG(regs, FLAG_H);

}

static void scf(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("SCF: %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);

    SET_R_FLAG(reg, FLAG_C);   
    CLEAR_R_FLAG(reg, FLAG_N); 
    CLEAR_R_FLAG(reg, FLAG_H); 

}

static void _jr_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("JR i8: %s\n", ins->name);

    int8_t offset = ins->opcode_ext.i8;
    uint16_t pc = READ_R16(&cpu->reg, REG_PC);
    pc += offset;
    WRITE_R16(&cpu->reg, REG_PC, pc);

}

static void jr_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    _jr_i8(cpu, ins);
}

static void jr_nz_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("JR NZ, i8: %s\n", ins->name);

    if (!READ_R_FLAG(&cpu->reg, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }

}

static void jr_z_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("JR Z, i8: %s\n", ins->name);

    if (READ_R_FLAG(&cpu->reg, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }

}

static void jr_nc_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("JR NC, i8: %s\n", ins->name);

    if (!READ_R_FLAG(&cpu->reg, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }

}

static void jr_c_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("JR C, i8: %s\n", ins->name);

    if (READ_R_FLAG(&cpu->reg, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }

}

static void nop(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("NOP: %s\n", ins->name);
}

static void ld_r16_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD r16, i16: %s\n", ins->name);

    uint16_t value = ins->opcode_ext.i16;

    WRITE_R16(&cpu->reg, (size_t)ins->op1, value);

}

static void ld_sp_hl(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD SP, HL: %s\n", ins->name);

    uint16_t hl = READ_R16(&cpu->reg, REG_HL);

    WRITE_R16(&cpu->reg, REG_SP, hl);

}

void ld_hl_sp_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD HL, SP+i8: %s\n", ins->name);

    int8_t offset = ins->opcode_ext.i8;
    uint16_t sp = READ_R16(&cpu->reg, REG_SP);
    uint16_t result = sp + offset;
    
    uint8_t carry = ((sp & UINT8_MASK) + (uint8_t)offset) > UINT8_MASK;
    uint8_t halfc = HALF_CARRY_ADD(sp, offset);

    WRITE_R16(&cpu->reg, REG_HL, result);

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, halfc);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}

static void ld_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD r8, i8: %s\n", ins->name);

    uint8_t value = ins->opcode_ext.i8;

    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

}

static void ldi_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LDI r8, m16: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op2, addr + 1);

}

static void ldi_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LDI m16, %s: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);

    cpu->mem_write(cpu->mem_data, addr, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, addr + 1);

}

static void ldd_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LDD r8, m16: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op2, addr - 1);

}

static void ldd_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LDD m16, r8: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);

    cpu->mem_write(cpu->mem_data, addr, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, addr - 1);

}

static void ld_m16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD m16, 8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)ins->op1;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = ins->opcode_ext.i8;

    cpu->mem_write(cpu->mem_data, addr, value);

}

static void ld_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD r8, m16: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

}

static void ld_r8_im16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    OG_DEBUG("LD r8, im16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = ins->opcode_ext.i16;
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    WRITE_R8(regs, REG_A, value);
}

static void ld_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD m16, r8: %s\n", ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);

    cpu->mem_write(cpu->mem_data, addr, value);

}

static void ld_im16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD im16, r16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = ins->opcode_ext.i16;
    uint16_t value = READ_R16(regs, reg_offset);

    cpu->mem_write(cpu->mem_data, addr, value & UINT8_MASK);
    cpu->mem_write(cpu->mem_data, addr + 1, value >> 8);

}

static void ld_im16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD im16, r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = ins->opcode_ext.i16;
    uint8_t value = READ_R8(regs, reg_offset);

    cpu->mem_write(cpu->mem_data, addr, value);

}

static void ld_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("LD r8, r8: %s\n", ins->name);

    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);

    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

}

static void add_r16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("ADD r16, r16: %s\n", ins->name);

    uint16_t a = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint16_t b = READ_R16(&cpu->reg, (size_t)ins->op2);

    uint8_t carry = (a > UINT16_MASK - b);
    uint8_t half_carry = HALF_CARRY_ADD_16(a, b);

    uint16_t result = a + b;
    
    WRITE_R16(&cpu->reg, (size_t)ins->op1, result);

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_N, 0);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}

static void add_r16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("ADD r16, i8: %s\n", ins->name);

    int8_t offset = ins->opcode_ext.i8;
    uint16_t value = READ_R16(&cpu->reg, (size_t)ins->op1);

    uint8_t carry = ((value & UINT8_MASK) + (offset & UINT8_MASK)) > UINT8_MASK;
    uint8_t half_carry = HALF_CARRY_ADD(offset, value);

    uint16_t result = value + offset;

    WRITE_R16(&cpu->reg, (size_t)ins->op1, result);

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}

static void add_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("ADD r8, r8: %s\n", ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint8_t b = READ_R8(&cpu->reg, (size_t)ins->op2);

    uint8_t carry = (a > UINT8_MASK - b);
    uint8_t half_carry = HALF_CARRY_ADD(a, b);

    uint8_t result = a + b;
    
    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}

static void add_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("ADD r8, i8: %s\n", ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint8_t b = ins->opcode_ext.i8;

    uint8_t carry = (a > UINT8_MASK - b);
    uint8_t half_carry = HALF_CARRY_ADD(a, b);

    uint8_t result = a + b;

    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}

static void add_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("ADD r8, m16: %s\n", ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t b = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = (a > UINT8_MASK - b);
    uint8_t half_carry = HALF_CARRY_ADD(a, b);
    uint8_t result = a + b;

    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

}