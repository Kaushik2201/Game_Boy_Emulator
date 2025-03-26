#include "io.h"
#include "memory.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Connect the I/O system to the memory
void io_connect(gbc_io_t *io, gbc_memory_t *memory) 
{
    if (io == NULL || memory == NULL) {
        return;
    }
    io->memory = memory;
}

// Initialize the I/O system
void io_init(gbc_io_t *io) 
{
    if (io == NULL) {
        return;
    }

    memset(io, 0, sizeof(gbc_io_t));
}

// Poll the keypad for input based on P1 register
void poll_keypad(void *udata) 
{
    gbc_io_t *io = (gbc_io_t *)udata;
    if (io == NULL || io->memory == NULL || io->poll_keypad == NULL) {
        return;
    }

    uint8_t p1 = IO_PORT_READ(io->memory, IO_PORT_P1);

    // Default: No keys pressed (active-low, so bits start high)
    uint8_t key = io->poll_keypad();

    if (!(p1 & KEY_DPAD))
        key >>= 4;
    key = ~key & 0xF;
    p1 = (p1 & 0xF0) | key;
    IO_PORT_WRITE(io->memory, IO_PORT_P1, p1);
}

// Update the I/O system for one CPU cycle
void io_cycle(gbc_io_t *io) 
{
    if (io == NULL || io->memory == NULL) {
        return;
    }

    uint8_t sc = IO_PORT_READ(io->memory, IO_PORT_SC);
    uint8_t sb = IO_PORT_READ(io->memory, IO_PORT_SB);

    poll_keypad(io);

    /* TODO: THIS IS FOR DEBUGGING PURPOSES */    
    if (sc == 0x81) {
        IO_PORT_WRITE(io->memory, IO_PORT_SC, 0x01);
        fprintf(stderr, "%c", sb);
    }
}
