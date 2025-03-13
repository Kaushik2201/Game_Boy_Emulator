#include "isa.h"
#include "common.h"
#include "cpu.h"
static void log_instruction(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("%s\n", ins->name);
}
static void stop(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("STOP: %s\n", ins->name);
  gbc_memory_t *mem = (gbc_memory_t *)cpu->mem_data;
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

  LOG_INFO("[CPU] Speed Switch %s -> %s\n", (cpu->dspeed ? "NORMAL" : "DOUBLE"),
           (cpu->dspeed ? "DOUBLE" : "NORMAL"));
}

static void inc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {}

// ADD instructions
static void adc_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("ADC R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}
static void adc_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("ADC R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}
static void adc_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("ADC R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}

// Subtract instructions
static void sub_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("Sub R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint16_t result = x - y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
}

static void sub_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("Sub R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint16_t result = x - y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
}

static void sub_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("Sub R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint16_t result = x - y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
}

// Subtraction with Carry instructions

static void subc_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("SUBC R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);
  uint16_t result = x - y - carry;

  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < (y + carry));
}

static void subc_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("SUBC R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);

  uint16_t result = x - y - carry;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < (y + carry));
}

static void subc_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("SUBC R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);
  uint16_t result = x - y - carry;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, x < (y + carry));
}

static void and_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("AND R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x & y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}
static void and_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("AND R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint8_t result = x & y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}
static void and_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("AND R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x & y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

// OR instructions
static void or_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("OR R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x | y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}
static void or_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("OR R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x | y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

static void or_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("OR R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint8_t result = x | y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

// XOR instructions

static void xor_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("XOR R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x ^ y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

static void xor_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("XOR R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x ^ y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

static void xor_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("XOR R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint8_t result = x ^ y;
  WRITE_R8(regs, reg_offset, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

// Compare Instructions

static void cp_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("CP R8 R8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
}

static void cp_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("CP R8 I8: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
}

static void cp_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("CP R8 M16: %s\n", ins->name);
  size_t reg_offset = (size_t)(ins->op1);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint16_t addr = READ_R16(regs, (size_t)ins->op2);
  uint8_t y = cpu->mem_read(cpu->mem_data, addr);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 1);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(regs, FLAG_C, x < y);
}

// Return Functions
static void _ret(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  uint16_t addr = READ_R16(regs, REG_SP);
  WRITE_R16(regs, REG_SP, addr + 2);
  WRITE_R16(regs, REG_PC, addr);
}

static void ret_nz(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("RET NZ: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_Z) == 0) {
    _ret(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void ret_nc(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("RET NC: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_C) == 0) {
    _ret(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void ret_z(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("RET Z: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_Z) != 0) {
    _ret(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void ret_c(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("RET C: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_C) != 0) {
    _ret(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void ret(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("RET: %s\n", ins->name);
  _ret(cpu, ins);
}

static void reti(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("RETI: %s\n", ins->name);
  _ret(cpu, ins);
  cpu->ime = 1;
}
// Jump instructions
static void _jp_addr16(gbc_cpu_t *cpu, gbc_instruction_t *ins, uint16_t addr) {
  cpu_register_t *regs = &(cpu->reg);
  WRITE_R16(regs, REG_PC, addr);
}

static void _jp_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  uint16_t addr = ins->opcode_ext.i16;
  _jp_addr16(cpu, ins, addr);
}

static void jp_nz_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP NZ: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_Z) == 0) {
    _jp_i16(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void jp_nc_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP NC: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_C) == 0) {
    _jp_i16(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void jp_z_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP Z: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_Z) != 0) {
    _jp_i16(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void jp_c_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP C: %s\n", ins->name);
  if (READ_R_FLAG(regs, FLAG_C) != 0) {
    _jp_i16(cpu, ins);
    ins->r_cycles = ins->cycles2;
  }
}

static void jp_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP I16: %s\n", ins->name);
  _jp_i16(cpu, ins);
}

static void jp_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("JP R16: %s\n", ins->name);
  uint16_t addr = READ_R16(regs, (uint16_t)ins->op1);
  _jp_addr16(cpu, ins, addr);
}
// MISC Instructions
static void pop_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  LOG_DEBUG("POP R16: %s\n", ins->name);
  uint16_t addr = READ_R16(regs, REG_SP);
  WRITE_R16(regs, REG_SP, addr + 2);
  WRITE_R16(regs, (uint16_t)ins->op1, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}

static void push_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  cpu_register_t *regs = &(cpu->reg);
  uint16_t sp = READ_R16(regs, REG_SP);
  uint16_t value = READ_R16(regs, (uint16_t)ins->op1);
  WRITE_R16(regs, REG_SP, sp - 2);

  // Write to memory
  cpu->mem_write(cpu->mem_data, sp - 2, value & 0xFF);
  cpu->mem_write(cpu->mem_data, sp - 1, value >> 8);

  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}