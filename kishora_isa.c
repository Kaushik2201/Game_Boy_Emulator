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

//Increment operations
static void inc_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){   
    LOG_DEBUG("INC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R8(reg, reg_offset);
    uint8_t hc = HALF_CARRY_ADD(x, 1);
    x++;
    x &= UINT8_MASK;
    WRITE_R8(reg, reg_offset, x);

    SET_R_FLAG_VALUE(reg, FLAG_Z, x==0);
    CLEAR_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void inc_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins){   
    LOG_DEBUG("INC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R16(reg, reg_offset);
    x++;

    WRITE_R16(reg, reg_offset, x);

}

static void inc_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("INC (HL): %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint16_t addr = READ_R16(reg, REG_HL);  // Get memory address from HL
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);  // Read 8-bit value from memory

    uint8_t hc = HALF_CARRY_ADD(value, 1);
    value++;  // Increment value

    cpu->mem_write(cpu->mem_data, addr, value);  // Write updated value

    SET_R_FLAG_VALUE(reg, FLAG_Z, value == 0);
    CLEAR_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);
}

//Decrement Operations
static void dec_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins){   
    LOG_DEBUG("DEC r8: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R8(reg, reg_offset);
    uint8_t hc = HALF_CARRY_SUB(x, 1);
    x--;
    x &= UINT8_MASK;
    WRITE_R8(reg, reg_offset, x);

    SET_R_FLAG_VALUE(reg, FLAG_Z, x==0);
    SET_R_FLAG(reg, FLAG_N);
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);

}

static void dec_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins){   
    LOG_DEBUG("DEC r16: %s\n", ins->name);

    size_t reg_offset = (size_t)ins->op1;
    cpu_register_t *reg = &(cpu->reg);

    uint16_t x = READ_R16(reg, reg_offset);
    x--;

    WRITE_R16(reg, reg_offset, x);

}

static void dec_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("DEC (HL): %s\n", ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint16_t addr = READ_R16(reg, REG_HL);  // Get memory address from HL
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);  // Read 8-bit value from memory

    uint8_t hc = HALF_CARRY_SUB(value, 1);
    value--;  // Decrement value

    cpu->mem_write(cpu->mem_data, addr, value);  // Write updated value back to memory

    SET_R_FLAG_VALUE(reg, FLAG_Z, value == 0);  // Set Zero flag if result is 0
    SET_R_FLAG(reg, FLAG_N);  // Set Subtraction flag (DEC is a subtraction)
    SET_R_FLAG_VALUE(reg, FLAG_H, hc);  // Set Half-Carry flag if needed
}

static void rlca(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("RLCA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = (a >> 7) & 1;  // Extract bit 7

    a = (a << 1) | carry;  // Rotate left, setting bit 0 with bit 7
    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);  // Set carry flag based on bit 7
    CLEAR_R_FLAG(reg, FLAG_N);             // Clear subtract flag
    CLEAR_R_FLAG(reg, FLAG_H);             // Clear half-carry flag
    CLEAR_R_FLAG(reg, FLAG_Z);             // RLCA never sets Zero flag
}



static void rla(gbc_cpu_t *cpu, gbc_instruction_t *ins){
    LOG_DEBUG("RLA: %s\n", ins->name);
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = READ_R_FLAG(reg, FLAG_C);

    uint8_t new_carry = a >> 7;
    a = (a << 1) | carry;           // Rotate left, set bit 0 with old bit 7
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
    uint8_t carry = a & 1;  // Extract bit 0

    a = (a >> 1) | (carry << 7);  // Rotate right, setting bit 7 with bit 0
    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);  // Set carry flag based on bit 0
    CLEAR_R_FLAG(reg, FLAG_N);             // Clear subtract flag
    CLEAR_R_FLAG(reg, FLAG_H);             // Clear half-carry flag
    CLEAR_R_FLAG(reg, FLAG_Z);             // RRCA never sets Zero flag
}

static void rra(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("RRA: %s\n", ins->name);
    
    cpu_register_t *reg = &(cpu->reg);
    uint8_t a = READ_R8(reg, REG_A);
    uint8_t carry = a & 1;  // Extract bit 0
    uint8_t old_carry = GET_R_FLAG(reg, FLAG_C) ? 0x80 : 0; // Get old carry value

    a = (a >> 1) | old_carry;  // Rotate right, inserting old carry into bit 7
    WRITE_R8(reg, REG_A, a);

    SET_R_FLAG_VALUE(reg, FLAG_C, carry);  // Set carry flag based on bit 0
    CLEAR_R_FLAG(reg, FLAG_N);             // Clear subtract flag
    CLEAR_R_FLAG(reg, FLAG_H);             // Clear half-carry flag
    CLEAR_R_FLAG(reg, FLAG_Z);             // RRA never sets Zero flag
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


static void jr_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR i8: %s\n", ins->name);

    int8_t offset = (int8_t)(uintptr_t)ins->op1;  
    cpu->reg.PC += offset;  

    LOG_INFO("Jumping to address: 0x%04X\n", cpu->reg.PC);
}


static void _jr_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    jr_i8(cpu, ins);
}

static void jr_nz_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR NZ, i8: %s\n", ins->name);

    if (!READ_R_FLAG(&cpu->reg, FLAG_Z)) { 
        int8_t offset = (int8_t)ins->op1;  
        cpu->reg.PC += offset;  
    }
}

static void jr_nc_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR NC, i8: %s\n", ins->name);

    int8_t offset = (int8_t)cpu->mem_read(cpu->mem_data, cpu->pc);
    cpu->reg.PC++;

    if (!READ_R_FLAG(&cpu->reg, FLAG_C)) {  
        cpu->reg.PC += offset;  
        cpu->ins_cycles += 4; 
    }
}

static void jr_z_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR Z, i8: %s\n", ins->name);

    uint16_t pc = READ_R16(&cpu->reg, REG_PC);
    int8_t offset = (int8_t)cpu->mem_read(cpu->mem_data, pc);
    WRITE_R16(&cpu->reg, REG_PC, pc + 1);  

    if (READ_R_FLAG(&cpu->reg, FLAG_Z)) {
        WRITE_R16(&cpu->reg, REG_PC, READ_R16(&cpu->reg, REG_PC) + offset);
        cpu->ins_cycles += 4;  
    }
}

static void jr_c_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("JR C, i8: %s\n", ins->name);

    int8_t offset = (int8_t)cpu->mem_read(cpu->mem_data, cpu->reg.PC); 
    cpu->reg.PC++; 

    if (READ_R_FLAG(&cpu->reg, FLAG_C)) {  
        cpu->reg.PC += offset;  
        cpu->ins_cycles += 4; 
    }
}

void nop(gbc_cpu_t *cpu) {
    cpu->ins_cycles = 4; 
}


static void ld_r16_i16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD r16, i16: %s\n", ins->name);

    uint16_t value = cpu->mem_read(cpu->mem_data, cpu->reg.PC) | (cpu->mem_read(cpu->mem_data, cpu->reg.PC + 1) << 8);  
    cpu->reg.PC += 2;  

    WRITE_R16(&cpu->reg, (size_t)ins->op1, value);  
    cpu->ins_cycles = 12;
}

static void ld_sp_hl(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD SP, HL: %s\n", ins->name);

    uint16_t hl = READ_R16(&cpu->reg, REG_HL);
    WRITE_R16(&cpu->reg, REG_SP, hl);
    
    cpu->ins_cycles += 8;
}

void ld_hl_sp_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD HL, SP+i8: %s\n", ins->name);

    int8_t offset = (int8_t)cpu->mem_read(cpu->mem_data, cpu->reg.PC++);
    uint16_t result = cpu->reg.SP + offset;

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, ((cpu->sp & 0x0F) + (offset & 0x0F)) > 0x0F);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, ((cpu->sp & 0xFF) + (offset & 0xFF)) > 0xFF);


    WRITE_R16(&cpu->reg, REG_HL, result);

    cpu->ins_cycles += 12;
}

static void ld_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, i8: %s\n", ins->name, ins->name);

    uint8_t value = cpu->mem_read(cpu->mem_data, cpu->reg.PC++);
    WRITE_R8(&cpu->reg, ins->op1, value);
    cpu->ins_cycles += 8;
}

static void ldi_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDI %s, (HL): %s\n", ins->name, ins->name);

    uint8_t value = cpu->mem_read(cpu->mem_data, READ_R16(&cpu->reg, REG_HL));
    WRITE_R8(&cpu->reg, ins->op1, value);
    WRITE_R16(&cpu->reg, REG_HL, READ_R16(&cpu->reg, REG_HL) + 1);
    cpu->ins_cycles += 8;
}

static void ldi_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDI (HL), %s: %s\n", ins->name, ins->name);

    uint8_t value = READ_R8(&cpu->reg, ins->op2);
    cpu->mem_write(cpu->mem_data, READ_R16(&cpu->reg, REG_HL), value);
    WRITE_R16(&cpu->reg, REG_HL, READ_R16(&cpu->reg, REG_HL) + 1);

    cpu->ins_cycles += 8;
}

static void ldd_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDD %s, (HL): %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, REG_HL);  // Get memory address from HL
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);  // Read value from memory
    WRITE_R8(&cpu->reg, ins->op1, value);  // Load into destination register

    WRITE_R16(&cpu->reg, REG_HL, addr - 1);  // Decrement HL
    cpu->ins_cycles += 8;
}

static void ldd_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LDD (HL), %s: %s\n", ins->name, ins->name);

    uint16_t addr = READ_R16(&cpu->reg, REG_HL);  // Get memory address from HL
    uint8_t value = READ_R8(&cpu->reg, ins->op2);  // Source register
    cpu->mem_write(cpu->mem_data, addr, value);  // Store value in memory

    WRITE_R16(&cpu->reg, REG_HL, addr - 1);  // Decrement HL
    cpu->ins_cycles += 8;
}

static void ld_m16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD (a16), A: %s\n", ins->name);

    uint16_t addr = cpu->mem_read(cpu->mem_data, cpu->reg.PC) |
                    (cpu->mem_read(cpu->mem_data, cpu->reg.PC + 1) << 8);  
    cpu->reg.PC += 2;  // Move past the 16-bit address

    uint8_t value = READ_R8(&cpu->reg, REG_A); 
    cpu->mem_write(cpu->mem_data, addr, value);  

    cpu->ins_cycles += 12;  
}

static void ld_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, (HL): %s\n", ins->name, ins->name);

    cpu_register_t *reg = &(cpu->reg);
    uint16_t addr = READ_R16(reg, REG_HL); 
    uint8_t value = cpu->mem_read(cpu->mem_data, addr);  

    WRITE_R8(reg, ins->op1, value); 

    cpu->ins_cycles += 8; 
}

static void ld_r8_im16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im16: %s\n", ins->name, ins->name);

    uint16_t value = cpu->mem_read(cpu->mem_data, cpu->reg.PC) | (cpu->mem_read(cpu->mem_data, cpu->reg.PC + 1) << 8);
    cpu->reg.PC += 2;  // Move past the 16-bit immediate value

    WRITE_R8(&cpu->reg, ins->op1, (uint8_t)(value & 0xFF));  // Load lower 8 bits
    cpu->ins_cycles += 12;
}

static void ld_m16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD (a16), %s: %s\n", ins->name, ins->name);

    uint16_t addr = cpu->mem_read(cpu->mem_data, cpu->reg.PC) | (cpu->mem_read(cpu->mem_data, cpu->reg.PC + 1) << 8;
    cpu->reg.PC += 2;  

    uint8_t value = READ_R8(&cpu->reg, ins->op2);  // Source register
    cpu->mem_write(cpu->mem_data, addr, value);  // Write to memory

    cpu->ins_cycles += 16;
}

static void ld_im16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im16: %s\n", ins->name, ins->name);

    uint16_t value = cpu->mem_read(cpu->mem_data, cpu->reg.PC) | (cpu->mem_read(cpu->mem_data, cpu->reg.PC + 1) << 8;
    cpu->reg.PC += 2;  // Move past the 16-bit immediate value

    WRITE_R16(&cpu->reg, ins->op1, value);  // Load into 16-bit register
    cpu->ins_cycles += 12;
}

static void ld_im16_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, im8: %s\n", ins->name, ins->name);

    uint8_t value = cpu->mem_read(cpu->mem_data, cpu->reg.PC++);  // Read 8-bit immediate value
    WRITE_R8(&cpu->reg, ins->op1, value);  // Load into 8-bit register
    cpu->ins_cycles += 8;
}

static void ld_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("LD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint8_t value = READ_R8(&cpu->reg, ins->op2);  // Source register
    WRITE_R8(&cpu->reg, ins->op1, value);  // Destination register
    cpu->ins_cycles += 4;
}

static void add_r16_r16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint16_t a = READ_R16(&cpu->reg, ins->op1);  // Destination register (e.g., HL)
    uint16_t b = READ_R16(&cpu->reg, ins->op2);  // Source register (e.g., BC)
    uint16_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_N, 0);  // Clear subtraction flag
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, HALF_CARRY_ADD_16(a, b));
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, (result < a));  // Set carry flag if overflow

    WRITE_R16(&cpu->reg, ins->op1, result);  // Write result to destination register
    cpu->ins_cycles += 8;
}

static void add_r16_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, i8: %s\n", ins->name, ins->name);

    int8_t offset = (int8_t)cpu->mem_read(cpu->mem_data, cpu->reg.PC++);  // Read 8-bit immediate value
    uint16_t result = READ_R16(&cpu->reg, ins->op1) + offset;

    CLEAR_R_FLAG(&cpu->reg, FLAG_Z);  // Zero flag is unaffected
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);  // Clear subtraction flag
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, ((READ_R16(&cpu->reg, ins->op1) & 0xFFF) + (offset & 0xFFF) > 0xFFF);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, ((READ_R16(&cpu->reg, ins->op1) & 0xFFFF) + (offset & 0xFFFF) > 0xFFFF);

    WRITE_R16(&cpu->reg, ins->op1, result);  // Write result to destination register
    cpu->ins_cycles += 16;
}

static void add_r8_r8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, %s: %s\n", ins->name, ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, ins->op1);  // Destination register (e.g., A)
    uint8_t b = READ_R8(&cpu->reg, ins->op2);  // Source register (e.g., B)
    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, HALF_CARRY_ADD(a, b));
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, (result < a));

    WRITE_R8(&cpu->reg, ins->op1, result);  // Write result to destination register
    cpu->ins_cycles += 4;
}

static void add_r8_i8(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, i8: %s\n", ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, ins->op1);  // Destination register (e.g., A)
    uint8_t b = cpu->mem_read(cpu->mem_data, cpu->reg.PC++);  // Read 8-bit immediate value
    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, HALF_CARRY_ADD(a, b));
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, (result < a));

    WRITE_R8(&cpu->reg, ins->op1, result);  // Write result to destination register
    cpu->ins_cycles += 8;
}

static void add_r8_m16(gbc_cpu_t *cpu, gbc_instruction_t *ins) {
    LOG_DEBUG("ADD %s, (HL): %s\n", ins->name, ins->name);

    uint8_t a = READ_R8(&cpu->reg, ins->op1);  // Destination register (e.g., A)
    uint16_t addr = READ_R16(&cpu->reg, REG_HL);  // Memory address in HL
    uint8_t b = cpu->mem_read(cpu->mem_data, addr);  // Read value from memory
    uint8_t result = a + b;

    SET_R_FLAG_VALUE(&cpu->reg, FLAG_Z, result == 0);
    CLEAR_R_FLAG(&cpu->reg, FLAG_N);
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_H, HALF_CARRY_ADD(a, b));
    SET_R_FLAG_VALUE(&cpu->reg, FLAG_C, (result < a));

    WRITE_R8(&cpu->reg, ins->op1, result);  // Write result to destination register
    cpu->ins_cycles += 8;
}


