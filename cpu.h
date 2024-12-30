#pragma once

#include<stdint.h>
#include"memory.h"

//FLAG BIT MASKS
#define FLAG_Z  0x80     // ZERO FLAG
#define FLAG_N  0x40    // SUBTRACTION FLAG
#define FLAG_H  0x20     // HALF CARRY FLAG
#define FLAG_C  0x10     //  CARRY FLAG

typedef union{
    uint16_t full;
    struct{
        uint8_t low;
        uint8_t high;
    };
} reg16_t;

//CPU REGISTERS
typedef struct{
    
    reg16_t AF;                   // ACCUMULATOR AND FLAGS (AF REGISTER)
    reg16_t BC;                   // BC REGISTER
    reg16_t DE;                   // DE REGISTER
    reg16_t HL;                   // HL REGISTER
    uint16_t SP;                  // STACK POINTER
    uint16_t PC;                  // PROGRAM COUNTER / POINTER

} cpu_reg_t;


//FLAG HELPER FUNCTIONS
#define SET_FLAG(reg,flag) ((reg)|=(flag))                  // SET FLAG BIT
#define CLEAR_FLAG(reg,flag) ((reg) & = ~(flag))            // CLEAR FLAG BIT
#define CHECK_FLAG(reg,flag) (((reg) & (flag))!=0)          // CHECK FLAG BIT   

//READ AND WRITE 8-BIT REGISTERS 
#define READ_8(reg) (reg)
#define WRITE_8(reg,value) ((reg)=((value)))

//READ AND WRITE 16_BIT REGISTERS
#define READ_16(reg) ((reg).full)
#define WRITE_16(reg,value) ((reg).full=(value))

//READ AND WRITE LSB
#define READ_1(reg) (reg & 0x1)
#define WRITE_1(reg, value) (reg) = (value & 0x1)

//INTERRUPTS MASKS
#define INTERRUPT_VBLANK    0x01
#define INTERRUPT_LCD      0x02
#define INTERRUPT_TIMER     0x04
#define INTERRUPT_SERIAL    0x08
#define INTERRUPT_JOYPAD    0x10
#define CPU_REQUEST_INTERRUPT(cpu, flag) (*(cpu)->ifp |= (flag))

//INTERRUPT HANDLER BITMASKS
#define INT_HANDLER_VBLANK   0x40
#define INT_HANDLER_LCD_STAT 0x48
#define INT_HANDLER_TIMER    0x50
#define INT_HANDLER_SERIAL   0x58
#define INT_HANDLER_JOYPAD   0x60

//CPU CONSTANTS
#define CLOCK_RATE          4194304         //4.194302 MHz
#define FRAME_RATE          60
#define CLOCK_CYCLE         (1000000000/CLOCK_RATE)
#define FRAME_INTERVAL      (1000000000/FRAME_RATE)
#define CYCLES_PER_FRAME    (CLOCK_RATE/FRAME_RATE)

//CPU STRUCT
typedef struct{
    cpu_reg_t reg;
    memory_read mem_read;  /* memory op */
    memory_write mem_write;
    void *mem_data;
    uint8_t *ifp;           /* interrupt flag 'pointer'(it is a pointer to io port) */

    uint64_t cycles;
    uint16_t ins_cycles;   /* current instruction cost */
    uint8_t ime;           /* interrupt master enable */
    uint8_t ier;           /* interrupt enable register */
    uint8_t ime_insts:4;   /* instruction count to set ime */
    uint8_t halt:2;        /* halt state */
    uint8_t dspeed:1;      /* doublespeed state */
} gbc_cpu_t;

/* https://gbdev.io/pandocs/CGB_Registers.html#ff4d--key1-cgb-mode-only-prepare-speed-switch */
#define KEY1_CPU_SWITCH_ARMED 0x1
#define KEY1_CPU_CURRENT_MODE 0x80

void gbc_cpu_init(gbc_cpu_t *cpu);
void gbc_cpu_connect(gbc_cpu_t *cpu, gbc_memory_t *mem);
void gbc_cpu_cycle(gbc_cpu_t *cpu);

/* ORDER: "PC", "SP", "A", "F", "B", "C", "D", "E", "H", "L", "Z", "N", "H", "C", "IME", "IE", "IF" */
#define DEBUG_CPU_REGISTERS_SIZE 17
void debug_get_all_registers(gbc_cpu_t *cpu, int values[DEBUG_CPU_REGISTERS_SIZE]);
