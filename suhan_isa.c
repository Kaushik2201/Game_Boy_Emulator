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
static void ldh_im8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t regs=&(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;
    cpu->mem_write(cpu->mem_data, addr, READ_R8(cpu->reg, reg_offset)); 
  
}
static void ldh_r8_im8(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t regs=&(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;
    WRITE_R8(reg, reg_offset, cpu->mem_read(cpu->mem_data, addr));  
}
static void ldh_r8_m8(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t regs=&(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;
    WRITE_R8(reg, reg_offset, cpu->mem_read(cpu->mem_data, addr));  
}
static void ldh_m8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    size_t reg_offset=(size_t)(ins->op1);
    cpu_register_t regs=&(cpu->reg);
    uint16_t addr = 0xFF00 +ins->opcode_ext.i8;
    cpu->mem_write(cpu->mem_data, addr, READ_R8(cpu->reg, reg_offset)); 
}
static void  _call_addr(gbc_cpu_t *cpu,gbc_instruction_t *ins,uint16_t addr){
LOG_DEBUG("\t_CALL ADDR: %X\n",addr);
cpu_register_t regs=&(cpu->reg);
uint16_t SP=READ_16(regs,REG_SP);
SP--;
uint16_t PC=READ_16(regs,REG_PC)
cpu->mem_write(cpu->mem_data, SP,PC>>8);
SP--;
cpu->mem_write(cpu->mem_data, SP,PC & UINT8_MASK);
WRITE_16(cpu->reg,REG_SP,SP);
_jp_addr16(cpu,ins,addr);
}

static void rst(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    uint16_t addr = ins->opcode & 0x38;
    LOG_DEBUG("\t_RST ADDR: %X\n", addr);
    _call_addr(cpu, ins, addr);
}

static void _call_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    uint16_t addr = ins->opcode_ext.i16;
    LOG_DEBUG("\t_CALL I16: %X\n", addr); 
    _call_addr(cpu, ins, addr);
}
static void int_call_i16(gbc_cpu_t *cpu,gbc_instruction_t *ins){
    uint16_t addr = ins->opcode_ext.i16;
    LOG_DEBUG("\t_CALL I16: %X\n", addr); 
    _call_addr(cpu, ins, addr);
    cpu->ime = 0;
}


static void call_nz_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu_register_t *regs = &(cpu->reg);
    LOG_DEBUG("\tCALL NZ ADDR: %s\n", ins->name);
    if (!READ_R_FLAG(regs, FLAG_Z)) {
        _call_i16(cpu, ins);
    }
}
static void call_nc_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu_register_t *regs = &(cpu->reg);
    LOG_DEBUG("\tCALL NC ADDR: %s\n", ins->name);
    if (!READ_R_FLAG(regs, FLAG_C)) {
        _call_i16(cpu, ins);
    }
}
static void call_z_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu_register_t *regs = &(cpu->reg);
    LOG_DEBUG("\tCALL Z ADDR: %s\n", ins->name);
    if (READ_R_FLAG(regs, FLAG_Z)) {
        _call_i16(cpu, ins);
    }
}


static void call_c_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu_register_t *regs = &(cpu->reg);
    LOG_DEBUG("\tCALL C ADDR: %s\n", ins->name);
    if (READ_R_FLAG(regs, FLAG_C)) {
        _call_i16(cpu, ins);
    }
}

static void call_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    uint16_t addr = ins->opcode_ext.i16;  
    LOG_DEBUG("\tCALL ADDR: %X\n", addr);
    _call_i16(cpu, ins);
    cpu->ime = 0;   

}


static void cpl(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("\tCPL : %s\n", ins->name);
    uint8_t value = READ_R8(cpu->reg, REG_A);
    value = ~value; 
    cpu_register_t *regs = &(cpu->reg);     
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
    cpu->ime = 0; 
    LOG_DEBUG("\t IME DISABLED : %s\n",ins->name);
}


static void ei(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu->ime_insts = 1;
    LOG_DEBUG("\t IME ENABLED %s\n",ins->name);
}

static void halt(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    cpu->halt = 1;
    LOG_DEBUG("\tHALT :%s\n",ins->name);
}
static void cb_rlc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("\tCB RLC R8 : %s\n", ins->name);
    size_t reg_offset = (size_t)(ins->op1);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = (value & 0x80) >> 7;  

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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = (value & 0x80) >> 7;

    value = (value << 1) | carry;
    cpu->mem_write(cpu->mem_data, addr, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}
static void cb_rrc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    size_t reg_offset = (size_t)(ins->op1);
    LOG_DEBUG("\tCB RRC r8 %s\n",ins->name);
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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
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
    size_t reg_offset = (size_t)(ins->op1);
    LOG_DEBUG("\tCB RL r8 : %s\n", ins->name);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = (value & 0x80) >> 7;

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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = (value & 0x80) >> 7;

    value = (value << 1) | READ_R_FLAG(regs, FLAG_C);
    cpu->mem_write(cpu->mem_data, addr, value);
    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    SET_R_FLAG_VALUE(regs, FLAG_C, carry);
}


static void cb_rr_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    size_t reg_offset = (size_t)(ins->op1);
    LOG_DEBUG("\tCB RR r8: %s\n", ins->name);
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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
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
    size_t reg_offset = (size_t)(ins->op1);
    LOG_DEBUG("\tCB SLA r8 : %s\n", ins->name);
    cpu_register_t *regs = &(cpu->reg);
    uint8_t value = READ_R8(regs, reg_offset);
    uint8_t carry = (value & 0x80) >> 7;  
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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    uint8_t carry = (value & 0x80) >> 7;

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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
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

    value = ((value >> 4) & 0x0F) | ((value & 0x0F) << 4);
    WRITE_R8(regs, reg_offset, value);

    SET_R_FLAG_VALUE(regs, FLAG_Z, value == 0);
    CLEAR_R_FLAG(regs, FLAG_N);
    CLEAR_R_FLAG(regs, FLAG_H);
    CLEAR_R_FLAG(regs, FLAG_C);
}


static void cb_swap_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB SWAP M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);

    value = ((value >> 4) & 0x0F) | ((value & 0x0F) << 4);
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
    uint16_t addr = READ_R16(regs, (size_t)ins->op2);
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
    LOG_DEBUG("CB BIT r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3); 
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);    

    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);

    SET_R_FLAG_VALUE(regs, FLAG_Z, !(value & (1 << n)));
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
}


static void cb_bit_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB BIT M16: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3); 
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);     

    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    SET_R_FLAG_VALUE(regs, FLAG_Z, !(value & (1 << n)));
    CLEAR_R_FLAG(regs, FLAG_N);
    SET_R_FLAG(regs, FLAG_H);
}



static void cb_res_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB RES r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3); 
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);     

    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);

    value &= ~(1 << n);
    WRITE_R8(regs, reg_offset, value);
}


static void cb_res_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB RES M16 %s\n", ins->name);
    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3); 
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);     
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    value &= ~(1 << n);
    cpu->mem_write(cpu->mem_data, addr, value);
}


static void cb_set_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB SET  r8: %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3);  
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);   

    size_t reg_offset = (size_t)(ins->op1);
    uint8_t value = READ_R8(regs, reg_offset);

    value |= (1 << n);
    WRITE_R8(regs, reg_offset, value);
}


static void cb_set_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("CB SET M16 %s\n", ins->name);

    cpu_register_t *regs = &(cpu->reg);
    uint16_t addr = READ_R16(regs, (size_t)ins->op3);  
    uint8_t n = cpu->mem_read(cpu->mem_data, addr);     

    uint8_t value = cpu->mem_read(cpu->mem_data, addr);
    value |= (1 << n);
    cpu->mem_write(cpu->mem_data, addr, value);
}
