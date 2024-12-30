#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>

#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_ERROR 3

#define LOG_LEVEL LOG_LEVEL_DEBUG

#if LOG_LEVEL == LOG_LEVEL_DEBUG
#define LOG_INFO(fmt, ...) printf("[info]" fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) printf("[debug]" fmt, ##__VA_ARGS__)
#endif

#if LOG_LEVEL == LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) printf("[info]" fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)
#endif

#if LOG_LEVEL == LOG_LEVEL_ERROR
#define LOG_INFO(fmt, ...)
#define LOG_DEBUG(fmt, ...) 
#endif

#define LOG_ERROR(fmt, ...) printf("[error]" fmt, ##__VA_ARGS__)

#define UINT4_MASK  0xF
#define UINT8_MASK  0xFF
#define UINT16_MASK 0xFFFF


#define HALF_CARRY_ADD(a, b) ((((a & UINT4_MASK) + (b & UINT4_MASK)) & 0x10) == 0x10)
#define HALF_CARRY_ADC(a, b, c) ((((a & UINT4_MASK) + (b & UINT4_MASK) + (c & UINT4_MASK)) & 0x10) == 0x10)

#define HALF_CARRY_ADD_16(a, b) ((((a & 0xFFF) + (b & 0xFFF)) & 0x1000 )== 0x1000)
#define HALF_CARRY_ADC_16(a, b, c) ((((a & 0xFFF) + (b & 0xFFF) + (c & 0xFFF)) & 0x1000 )== 0x1000)

#define HALF_CARRY_SUB(a, b) (((a & UINT4_MASK) < (b & UINT4_MASK)) ? 1 : 0)
#define HALF_CARRY_ADC(a, b, c) ((((a & UINT4_MASK) - (b & UINT4_MASK) - (c & UINT4_MASK)) < 0) ? 1 : 0)

#define HALF_CARRY_ADD_16(a, b) (((a & 0xFFF) < (b & 0xFFF)) ? 1 : 0)
#define HALF_CARRY_ADC_16(a, b, c) ((((a & 0xFFF) - (b & 0xFFF) - (c & 0xFFF)) < 0) ? 1 : 0)

#define IN_RANGE(addr, begin, end) ((addr) >= (begin) && (addr) <= (end))

#endif