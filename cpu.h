#pragma once

#include<stdint.h>

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

} cpu_reg;


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

//INTERRUPTS MASKS
#define INTERRUPT_VBLANK    0x01
#define INTERRUPT_LCD      0x02
#define INTERRUPT_TIMER     0x04
#define INTERRUPT_SERIAL    0x08
#define INTERRUPT_JOYPAD    0x10

//CPU CONSTANTS
#define CLOCK_RATE          4194304         //4.194302 MHz
#define FRAME_RATE          60
#define CLOCK_CYCLE         (1/CLOCK_RATE)
#define FRAME_INTERVAL      (1/FRAME_RATE)
#define CYCLES_PER_FRAME    (CLOCK_RATE/FRAME_RATE)

//CPU STRUCT
typedef struct{
    cpu_reg reg;
    uint8_t IME;            // (INTERRUPT MASTER ENABLE) (write only)IME : 1 (ENABLES INTERRUPT HANDLING) ; IME : 0 (DISABLES INTERRUPT HANDLING)
    uint8_t IE;             // (INTERRUPT ENABLE)
    uint8_t IF;             // (INTERRUPT FLAG)
    uint16_t Cycles;
} cpu;