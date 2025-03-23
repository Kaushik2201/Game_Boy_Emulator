#include "io.h"
#include "memory.h"
#include <stddef.h>
#include <string.h>

// Connect the I/O system to the memory
void io_connect(gbc_io_t *io, gbc_memory_t *memory) {
    if (io == NULL || memory == NULL) {
        return;
    }
    io->memory = memory; // Link I/O system to memory
}

// Initialize the I/O system
void io_init(gbc_io_t *io) {
    if (io == NULL) {
        return;
    }

    memset(io, 0, sizeof(gbc_io_t));
}

// Poll the keypad for input based on P1 register
void poll_keypad(void *udata) {
    gbc_io_t *io = (gbc_io_t *)udata;
    if (io == NULL || io->memory == NULL || io->poll_keypad == NULL) {
        return;
    }

    uint8_t p1 = io->memory->io_ports[IO_PORT_P1];

    // Default: No keys pressed (active-low, so bits start high)
    uint8_t keys = 0x0F;

    if (!(p1 & KEY_DPAD)) {  // If D-Pad selection bit is 0
        keys &= ~(io->poll_keypad() & DPAD_MASK);
    }
    if (!(p1 & KEY_BUTTON)) {  // If Button selection bit is 0
        keys &= ~(io->poll_keypad() & BUTTON_MASK);
    }

    io->memory->io_ports[IO_PORT_P1] = (p1 & 0xF0) | keys; // Preserve upper bits
}

// Update the I/O system for one CPU cycle
void io_cycle(gbc_io_t *io) {
    if (io == NULL || io->memory == NULL) {
        return;
    }

    // Increment the divider register (simulating real hardware behavior)
    io->memory->io_ports[IO_PORT_DIV]++;
}
