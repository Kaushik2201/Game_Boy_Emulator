#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include <cstdint>
#include <functional>
#include <unordered_map>

struct CPU {
    uint8_t A, F; 
    uint8_t B, C, D, E, H, L; 
    uint16_t SP, PC; 

    std::function<uint8_t(uint16_t)> read_memory;
    std::function<void(uint16_t, uint8_t)> write_memory;
};

//FLAGS
constexpr uint8_t FLAG_Z = 0x80; 
constexpr uint8_t FLAG_N = 0x40; 
constexpr uint8_t FLAG_H = 0x20; 
constexpr uint8_t FLAG_C = 0x10; 


#endif 