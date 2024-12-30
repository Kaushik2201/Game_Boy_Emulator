#ifndef IO_H
#define IO_H

#include <stdint.h>

#define KEY_UP       0x10
#define KEY_DOWN     0x20
#define KEY_LEFT     0x40
#define KEY_RIGHT    0x80
#define KEY_A        0x01
#define KEY_B        0x02
#define KEY_START    0x04
#define KEY_SELECT   0x08
// D-Pad and Button bit masks
#define DPAD_MASK    (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)
#define BUTTON_MASK  (KEY_A | KEY_B | KEY_START | KEY_SELECT)
// Memory struct
typedef struct {
    uint8_t *memory;
} IO;

// Function declarations
void io_connect(IO *io, uint8_t *memory);
void io_init(IO *io);
void io_cleanup(IO *io);

#endif 