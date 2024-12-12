#ifndef memory_h
#define memory_h

#include <stdint.h>
#include <stdbool.h> 


//Memory Map
#define rom_bank_start 0x0000
#define rom_bank_end 0x7FFF
#define rom_bank_switch_start 0x4000
#define rom_bank_switch_end 0x7FFF
#define vram_start 0x8000
#define vram_end 0x9FFF
#define external_ram_start 0xA000
#define external_ram_end 0xBFFF
#define wram_start 0xC000
#define wram_end 0xCFFF
#define wram_2_start 0xD000
#define wram_2_end 0xD000
#define echo_ram_start 0xE000
#define echo_ram_end 0xFDFF
#define oam_start 0xFE00
#define oam_end 0xFE9F
#define unusable_start 0xFEA0
#define unusable_end 0xFEFF
#define io_register_start 0xFF00
#define io_register_ end 0xFF7F
#define hram_start 0xFF80
#define hram_end 0xFFFE
#define interrupt_reg_start 0xFFFF
#define interrupt_reg_end 0xFFFF

#define rom_size 0x8000
#define vram_bank_size 0x2000
#define wram_bank_size 0x1000

typedef struct{
    uint8_t rom[rom_size];
    uint8_t vram[vram_bank_size*2];
    uint8_t external_ram[0x2000];
    uint8_t wram[wram_bank_size*2];
    uint8_t oam[0xA0];
    uint8_t hram[0x7F];
    uint8_t interrupt_enable;
}memory_t;

void memory_init(memory_t *mem);
uint8_t memory_read8(memory_t *mem, uint16_t addr);
void memory_write8(memory_t *mem, uint16_t value);
uint16_t memory_read16(memory_t *mem, uint16_t addr);
void memory_write16(memory_t *mem, uint16_t value);


#endif memory_h