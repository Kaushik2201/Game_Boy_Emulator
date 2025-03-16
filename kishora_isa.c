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

    cpu->ins_cycles = 8;
}

static void inc_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("INC (HL): %s\n", ins->name);

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

    cpu->ins_cycles = 8;
}

static void dec_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("DEC (HL): %s\n", ins->name);

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
    uint8_t carry = (a >> 7) & 1;

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
    LOG_DEBUG("DAA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t correction = 0;
    uint8_t carry = 0;

    if (READ_R_FLAG(reg, FLAG_H) || (!READ_R_FLAG(reg, FLAG_N) && (a & 0x0F) > 9)) {
        correction |= 0x06;
    }
    if (READ_R_FLAG(reg, FLAG_C) || (!READ_R_FLAG(reg, FLAG_N) && a > 0x99)) {
        correction |= 0x60;
        carry = 1;
    }

    if (READ_R_FLAG(reg, FLAG_N)) {
        a -= correction;
    } else {
        a += correction;
    }

    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);
    CLEAR_R_FLAG(reg, FLAG_H);
    SET_R_FLAG_VALUE(reg, FLAG_Z, a == 0);
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

    LOG_INFO("Jumping to address: 0x%04X\n", pc);
}

static void jr_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    _jr_i8(cpu, ins);
}

static void jr_nz_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR NZ, i8: %s\n", ins->name);

    if (!READ_R_FLAG(&cpu->reg, FLAG_Z)) {
        ins->rcycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void jr_z_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR Z, i8: %s\n", ins->name);

    if (READ_R_FLAG(&cpu->reg, FLAG_Z)) {
        ins->rcycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void jr_nc_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR NC, i8: %s\n", ins->name);

    if (!READ_R_FLAG(&cpu->reg, FLAG_C)) {
        ins->rcycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

static void jr_c_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR C, i8: %s\n", ins->name);

    if (READ_R_FLAG(&cpu->reg, FLAG_C)) {
        ins->rcycles = ins->cycles2;
        _jr_i8(cpu, ins);
    }
}

void nop(gbc_cpu_t *cpu) {
    cpu->ins_cycles = 4;
}

static void ld_r16_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD r16, i16: %s\n", ins->name);

    uint16_t value = ins->opcode_ext.i16;
    WRITE_R16(&cpu->reg, (size_t)ins->op1, value);
    cpu->ins_cycles = 12;
}

static void ld_sp_hl(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD SP, HL: %s\n", ins->name);

    uint16_t hl = READ_R16(&cpu->reg, REG_HL);
    WRITE_R16(&cpu->reg, REG_SP, hl);
    cpu->ins_cycles = 8;
}

void ld_hl_sp_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD HL, SP+i8: %s\n", ins->name);

    int8_t offset = ins->opcode_ext.i8;
    uint16_t sp = READ_R16(&cpu->reg, REG_SP);
    uint16_t result = sp + offset;

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, ((sp & 0x0F) + (offset & 0x0F)) > 0x0F);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, ((sp & 0xFF) + (offset & 0xFF)) > 0xFF);

    WRITE_R16(&cpu->reg, REG_HL, result);
    cpu->ins_cycles = 12;
}

static void ld_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, i8: %s\n", ins->name, ins->name);

    uint8_t value = ins->opcode_ext.i8;
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);
    cpu->ins_cycles = 8;
}

static void ldi_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDI %s, (HL): %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op2, addr + 1);
    cpu->ins_cycles = 8;
}

static void ldi_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDI (HL), %s: %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);
    cpu->mem_write(cpu->mem_data, addr, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, addr + 1);
    cpu->ins_cycles = 8;
}

static void ldd_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDD %s, (HL): %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op2, addr - 1);
    cpu->ins_cycles = 8;
}

static void ldd_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDD (HL), %s: %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);
    cpu->mem_write(cpu->mem_data, addr, value);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, addr - 1);
    cpu->ins_cycles = 8;
}

static void ld_m16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD (a16), A: %s\n", ins->name);

    uint16_t addr = ins->opcode_ext.i16;
    uint8_t value = READ_R8(&cpu->reg, REG_A);
    cpu->mem_write(cpu->mem_data, addr, value);

    cpu->ins_cycles = 12;
}

static void ld_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, (HL): %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);

    cpu->ins_cycles = 8;
}

static void ld_r8_im16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im16: %s\n", ins->name, ins->name);

    uint16_t value = ins->opcode_ext.i16;
    WRITE_R8(&cpu->reg, (size_t)ins->op1, (uint8_t)(value & 0xFF));
    cpu->ins_cycles = 12;
}

static void ld_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD (a16), %s: %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);
    cpu->mem_write(cpu->mem_data, addr, value);

    cpu->ins_cycles = 16;
}

static void ld_im16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im16: %s\n", ins->name, ins->name);

    uint16_t value = ins->opcode_ext.i16;
    WRITE_R16(&cpu->reg, (size_t)ins->op1, value);
    cpu->ins_cycles = 12;
}

static void ld_im16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im8: %s\n", ins->name, ins->name);

    uint8_t value = ins->opcode_ext.i8;
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);
    cpu->ins_cycles = 8;
}

static void ld_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint8_t value = READ_R8(&cpu->reg, (size_t)ins->op2);
    WRITE_R8(&cpu->reg, (size_t)ins->op1, value);
    cpu->ins_cycles = 4;
}

static void add_r16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint16_t a = READ_R16(&cpu->reg, (size_t)ins->op1);
    uint16_t b = READ_R16(&cpu->reg, (size_t)ins->op2);

    uint8_t carry = (a > 0xFFFF - b);
    uint8_t half_carry = HALF_CARRY_ADD_16(a, b);

    uint16_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_N, 0);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, result);
    cpu->ins_cycles = 8;
}

static void add_r16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, i8: %s\n", ins->name, ins->name);

    int8_t offset = ins->opcode_ext.i8;
    uint16_t value = READ_R16(&cpu->reg, (size_t)ins->op1);

    uint8_t carry = ((value & 0xFF) + (offset & 0xFF)) > 0xFF;
    uint8_t half_carry = ((value & 0xFFF) + (offset & 0xFFF)) > 0xFFF;

    uint16_t result = value + offset;

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

    WRITE_R16(&cpu->reg, (size_t)ins->op1, result);
    cpu->ins_cycles = 16;
}

static void add_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint8_t b = READ_R8(&cpu->reg, (size_t)ins->op2);

    uint8_t carry = (a > 0xFF - b);
    uint8_t half_carry = HALF_CARRY_ADD(a, b);

    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);
    cpu->ins_cycles = 4;
}

static void add_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, i8: %s\n", ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint8_t b = ins->opcode_ext.i8;

    uint8_t carry = (a > 0xFF - b);
    uint8_t half_carry = HALF_CARRY_ADD(a, b);

    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, half_carry);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, carry);

    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);
    cpu->ins_cycles = 8;
}

static void add_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, (HL): %s\n", ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, (size_t)ins->op1);
    uint16_t addr = READ_R16(&cpu->reg, (size_t)ins->op2);
    uint8_t b = cpu->mem_read(cpu->mem_data, addr);
    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, HALF_CARRY_ADD(a, b));
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, (result < a));

    WRITE_R8(&cpu->reg, (size_t)ins->op1, result);
    cpu->ins_cycles = 8;
}