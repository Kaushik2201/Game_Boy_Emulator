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


static void ldh_im8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){

    LOG_DEBUG("LDH m8, r8: %s\n", ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;

    cpu->mem_write(cpu->mem_data, addr, READ_R8(regs, reg_offset)); 
  
}

static void ldh_r8_im8(gbc_cpu_t *cpu, gbc_instruction_t *ins){

    LOG_DEBUG("LDH r8, m8: %s\n", ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;

    WRITE_R8(regs, reg_offset, cpu->mem_read(cpu->mem_data, addr));  

}

static void ldh_r8_m8(gbc_cpu_t *cpu, gbc_instruction_t *ins){

    LOG_DEBUG("LDH r8, C: %s\n", ins->name);

    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t *regs=&(cpu->reg);
    size_t reg_offset2 = (size_t)ins->op2;
    uint16_t addr = 0xFF00 + READ_R8(regs, reg_offset2);

    WRITE_R8(regs, reg_offset, cpu->mem_read(cpu->mem_data, addr));

}

static void ldh_m8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){

    LOG_DEBUG("LDH C, r8: %s\n", ins->name);

    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t *regs=&(cpu->reg);
    size_t reg_offset2 = (size_t)ins->op2;

    uint16_t addr = 0xFF00 + READ_R8(regs, reg_offset);

    cpu->mem_write(cpu->mem_data, addr, READ_R8(regs, reg_offset2));

}

static void  _call_addr(gbc_cpu_t *cpu, gbc_instruction_t *ins, uint16_t addr){

    LOG_DEBUG("\t_CALL ADDR: %X\n",addr);

    cpu_register_t *regs=&(cpu->reg);
    uint16_t SP=READ_R16(regs,REG_SP);

    SP--;
    uint16_t PC=READ_R16(regs,REG_PC);

    cpu->mem_write(cpu->mem_data, SP,PC>>8);
    SP--;
    cpu->mem_write(cpu->mem_data, SP,PC & UINT8_MASK);
    WRITE_R16(regs,REG_SP,SP);
    _jp_addr16(cpu,ins,addr);

}

static void rst(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RST: %s\n", ins->name);

    uint16_t addr = (uint16_t)(uintptr_t)ins->op1;
    _call_addr(cpu, ins, addr);

}

static void _call_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\t_CALL I16: %X\n", ins->name); 

    uint16_t addr = ins->opcode_ext.i16;
    _call_addr(cpu, ins, addr);

}

void int_call_i16(gbc_cpu_t *cpu, uint16_t addr)
{
    LOG_DEBUG("INT CALL I16\n");
    cpu_register_t *regs = &(cpu->reg);

    uint16_t pc = READ_R16(regs, REG_PC);
    uint16_t sp = READ_R16(regs, REG_SP);

    cpu->mem_write(cpu->mem_data, sp - 1, pc >> 8);
    cpu->mem_write(cpu->mem_data, sp - 2, pc & UINT8_MASK);

    WRITE_R16(regs, REG_SP, sp - 2);
    WRITE_R16(regs, REG_PC, addr);
    /* costs 20cycles (5 M-cycles), this function counts as 1 cycle */
    cpu->ins_cycles = 19;

}


static void call_nz_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CALL NZ: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);

    if (!READ_R_FLAG(regs, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }

}

static void call_nc_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CALL NC: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);

    if (!READ_R_FLAG(regs, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }

}

static void call_z_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CALL Z: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);

    if (READ_R_FLAG(regs, FLAG_Z)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }

}


static void call_c_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CALL C: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);

    if (READ_R_FLAG(regs, FLAG_C)) {
        ins->r_cycles = ins->cycles2;
        _call_i16(cpu, ins);
    }

}

static void call_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCALL ADDR: %X\n", ins->name);

    _call_i16(cpu, ins);

}


static void cpl(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCPL : %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);     
    uint8_t value = READ_R8(regs, REG_A);
    value = ~value; 

    WRITE_R8(regs, REG_A, value);

    SET_R_FLAG(regs, FLAG_N); 
    SET_R_FLAG(regs, FLAG_H);  

}

static void ccf(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCCF : %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg); 
    uint8_t carry = READ_R_FLAG(regs, FLAG_C);

    SET_R_FLAG_VALUE(regs, FLAG_C, !carry);  
    CLEAR_R_FLAG(regs, FLAG_N);              
    CLEAR_R_FLAG(regs, FLAG_H);  

}

static void di(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\t IME DISABLED : %s\n",ins->name);
    cpu->ime = 0; 

}


static void ei(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\t IME ENABLED %s\n",ins->name);
    cpu->ime_insts = 1;

}

static void halt(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tHALT :%s\n",ins->name);
    cpu->halt = 1;

}

static void cb_rlc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCB RLC R8 : %s\n", ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value >> 7;  

    value = (value << 1) | carry;  

    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0); 
    CLEAR_R_FLAG(regs, FLAG_N);      
    CLEAR_R_FLAG(regs, FLAG_H);              
    SET_R_FLAG_VALUE(regs, FLAG_C, carry); 
}

static void cb_rlc_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB RLC M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value >> 7;

    value = (value << 1) | carry;

    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_rrc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCB RRC r8 %s\n",ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (carry << 7); 

    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry); 

}

static void cb_rrc_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB RRC M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (carry << 7);

    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}


static void cb_rl_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCB RL r8 : %s\n", ins->name);
    
    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value >> 7;

    value = (value << 1) | READ_R_FLAG(regs, FLAG_C);  

    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_rl_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB RL M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value >> 7;

    value = (value << 1) | READ_R_FLAG(regs, FLAG_C);

    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}


static void cb_rr_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCB RR r8: %s\n", ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (READ_R_FLAG(regs, FLAG_C) << 7);

    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_rr_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB RR M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (READ_R_FLAG(regs, FLAG_C) << 7);
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_sla_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("\tCB SLA r8 : %s\n", ins->name);

    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);

    uint8_t carry = value >> 7;  
    value <<= 1; 
    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_sla_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SLA M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value >> 7;

    value <<= 1;
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_sra_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SRA r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (value & 0x80); 

    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_sra_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SRA M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value & 0x01;

    value = (value >> 1) | (value & 0x80);  
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_swap_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SWAP r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);

    value = (value >> 4) | ((value << 4) & 0xF0);
    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);

}


static void cb_swap_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SWAP M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    value = (value >> 4) | ((value << 4) & 0xF0);
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);

}

static void cb_srl_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SRL r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = value & 0x01;

    value >>= 1;
    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}


static void cb_srl_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("CB SRL M16:%s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op1);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = value & 0x01;

    value >>= 1;
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);

}

static void cb_bit_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("BIT: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value & (1 << bit);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);

}


static void cb_bit_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("BIT: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value & (1 << bit);

    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_Z, result == 0);

}



static void cb_res_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RES: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value & ~(1 << bit);

    WRITE_R8(regs, reg_offset, result);

}


static void cb_res_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("RES: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value & ~(1 << bit);

    cpu->mem_write(cpu->mem_data, addr, result);

}


static void cb_set_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("SET: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;

    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t result = value | (1 << bit);

    WRITE_R8(regs, reg_offset, result);

}


static void cb_set_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {

    LOG_DEBUG("SET: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint8_t bit = (uint8_t)(uintptr_t)ins->op1;
    size_t reg_offset = (size_t)ins->op2;
    uint16_t addr = READ_R16(regs, reg_offset);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    uint8_t result = value | (1 << bit);

    cpu->mem_write(cpu->mem_data, addr, result);

}