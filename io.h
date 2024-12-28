
#ifndef IO_H
#define IO_H

#include <stdint.h>

// I/O functions for reading and writing registers
uint8_t io_read(uint16_t address);
void io_write(uint16_t address, uint8_t value);

#endif // IO_H