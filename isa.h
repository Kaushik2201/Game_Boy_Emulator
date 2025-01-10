#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include <cstdint>
#include <cpu.h>


typedef struct instruction instruction_t;
typedef void (*instruction_func)(gbc_cpu_t *cpu, instruction_t *ins);

#define INSTRUCTIONS_SET_SIZE 512

#define PREFIX_CB 0xcb

#define INSTRUCTION_ADD(opcode, size, func, op1, op2, c1, c2, name) {(opcode), (size), (c1), (c2), (c1), (func), ((void*)(op1)), ((void*)(op2)), 0, (name)}

struct instruction {
    uint8_t opcode;
    uint8_t size;
    uint8_t cycles;
    uint8_t cycles2;
    uint8_t r_cycles;
    instruction_func func;
    void *op1;
    void *op2;
    union {
        uint16_t i16;
        uint8_t i8
    } opcode_ext;
    const char* name;
};

void init_instruction_set();
instruction_t* decode(uint8_t *data);
void int_call_i16(gbc_cpu_t *cpu, uint16_t addr);

#endif 
