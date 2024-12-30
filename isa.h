#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <string>

// struct CPU {
//     uint8_t A, F; 
//     uint8_t B, C, D, E, H, L; 
//     uint16_t SP, PC; 

//     std::function<uint8_t(uint16_t)> read_memory;
//     std::function<void(uint16_t, uint8_t)> write_memory;
// };
struct Instruction {
    uint8_t opcode;
    uint8_t size;
    uint8_t cycles;
    uint8_t cycles2;
    std::string name;
    std::function<void()> execute;
};


#endif 