#include "isa.h"
#include "common.h"
#include "cpu.h"
static void log_instruction(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  LOG_DEBUG("STOP: %s\n", ins->name);
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
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t carry = READ_R_FLAG(&(cpu->reg), FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}
static void adc_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t carry = READ_R_FLAG(&(cpu->reg), FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, hc);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}
static void adc_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = (uint16_t)(ins->opcode_ext.i16);
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint8_t carry = READ_R_FLAG(&(cpu->reg), FLAG_C);
  uint8_t hc = HALF_CARRY_ADC(x, y, carry);
  uint16_t result = x + y + carry;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, hc);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

// Subtract instructions
static void sub_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint16_t result = x - y;
  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, result < 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}

static void sub_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint16_t result = x - y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

static void sub_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint16_t result = x - y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

// Subtraction with Carry instructions

static void subc_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t carry = READ_R_FLAG(regs, FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);
  uint16_t result = x - y - carry;

  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, result < 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, hc);
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}

static void subc_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t carry = READ_R_FLAG(&(cpu->reg), FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);

  uint16_t result = x - y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, hc);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

static void subc_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint8_t carry = READ_R_FLAG(&(cpu->reg), FLAG_C);
  uint8_t hc = HALF_CARRY_SBC(x, y, carry);
  uint16_t result = x - y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, hc);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

static void and_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x & y;
  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, result < 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 1);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}
static void and_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint8_t result = x & y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 1);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}
static void and_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x & y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 1);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}

// OR instructions
static void or_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x | y;
  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}
static void or_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x | y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}

static void or_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint8_t result = x | y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}

// XOR instructions

static void xor_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint8_t result = x ^ y;
  WRITE_R8(regs, x, result);
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, 0);
  SET_R_FLAG_VALUE(regs, FLAG_C, 0);
}

static void xor_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint8_t result = x ^ y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}

static void xor_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint8_t result = x ^ y;
  WRITE_R8(&(cpu->reg), x, result);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, 0);
}

// Compare Instructions

static void cp_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  size_t reg_offset2 = (size_t)(ins->op2);
  cpu_register_t *regs = &(cpu->reg);
  uint8_t x = READ_R8(regs, reg_offset);
  uint8_t y = READ_R8(regs, reg_offset2);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(regs, FLAG_N, result < 0);
  SET_R_FLAG_VALUE(regs, FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(regs, FLAG_C, result > 0xFF);
}

static void cp_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint8_t y = (uint8_t)(ins->opcode_ext.i8);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

static void cp_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  size_t reg_offset = (size_t)(ins->op1);
  uint8_t x = READ_R8(&(cpu->reg), reg_offset);
  uint16_t addr = READ_I16(&(cpu->reg));
  uint8_t y = READ_R16(cpu->mem_data, addr);
  uint16_t result = x - y;
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_Z, result == 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_N, result < 0);
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_H, HALF_CARRY_SUB(x, y));
  SET_R_FLAG_VALUE(&(cpu->reg), FLAG_C, result > 0xFF);
}

// Return Functions
static void _ret(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
  WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
  WRITE_R16(&(cpu->reg), REG_PC, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}

static void ret_nz(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_Z) == 0) {
    uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
    WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}

static void ret_nc(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_C) == 0) {
    uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
    WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}
static void ret_z(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_Z) != 0) {
    uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
    WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}

static void ret_c(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_C) != 0) {
    uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
    WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}
static void ret(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  _ret(cpu, ins);
}
static void reti(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
  WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
  WRITE_R16(&(cpu->reg), REG_PC, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
  cpu->ime = 1;
}

// Jump instructions

static void _jp_addr16(gbc_cpu_t *cpu, gbc_instruction_t *ins, uint16_t addr) {
  log_instruction(cpu, ins);

  WRITE_R16(&(cpu->reg), REG_PC, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}
static void _jp_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_I16(&(cpu->reg));
  WRITE_R16(&(cpu->reg), REG_PC, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}

static void _jp_nz_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_Z) == 0) {
    uint16_t addr = READ_I16(&(cpu->reg));
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}
static void jp_nc_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_C) == 0) {
    uint16_t addr = READ_I16(&(cpu->reg));
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}
static void jp_z_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_Z) != 0) {
    uint16_t addr = READ_I16(&(cpu->reg));
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}
static void jp_c_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  if (READ_R_FLAG(&(cpu->reg), FLAG_C) != 0) {
    uint16_t addr = READ_I16(&(cpu->reg));
    WRITE_R16(&(cpu->reg), REG_PC, addr);
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  } else {
    cpu->ins_cycles = 0;
    cpu->cycles += 4;
  }
}

static void jp_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  _jp_i16(cpu, ins);
}
static void jp_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_R16(&(cpu->reg), (uint16_t)ins->op1);
  WRITE_R16(&(cpu->reg), REG_PC, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}

// MISC Instructions
static void pop_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
  WRITE_R16(&(cpu->reg), REG_SP, addr + 2);
  WRITE_R16(&(cpu->reg), (uint16_t)ins->op1, addr);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}

static void push_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
  log_instruction(cpu, ins);
  uint16_t addr = READ_R16(&(cpu->reg), REG_SP);
  WRITE_R16(&(cpu->reg), REG_SP, addr - 2);
  WRITE_R16(&(cpu->reg), addr, (uint16_t)ins->op1);
  cpu->ins_cycles = 0;
  cpu->cycles += 4;
}