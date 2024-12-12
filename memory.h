#ifndef memory_h
#define memory_h

#include <stdint.h>
#include <stdbool.h>

#define rom_bank_00_start   0x0000
#define rom_bank_00_end     0x3FFF
#define rom_bank_switch_start 0x4000
#define rom_bank_switch_end   0x7FFF
#define vram_start  0x8000
#define vram_end    0x9FFF
#define external_ram_start 0xA000
#define external_ram_end   0xBFFF
#define wram_bank_0_start 0xC000
#define wram_bank_0_end   0xCFFF
#define wram_bank_switch_start 0xD000
#define wram_bank_switch_end   0xDFFF
#define echo_ram_start 0xE000
#define echo_ram_end   0xFDFF
#define oam_start  0xFE00
#define oam_end    0xFE9F
#define non_usable_start 0xFEA0
#define non_usable_end   0xFEFF
#define io_registers_start 0xFF00
#define io_registers_end   0xFF7F
#define hram_start 0xFF80
#define hram_end   0xFFFE
#define interrupt_enable_register 0xFFFF

//IDs
#define rom_bank_0_start_id 1
#define rom_bank_switch_start_id 2
#define vram_start_id 3
#define external_ram_start_id 4
#define wram_bank_0_start_id 5
#define wram_bank_switch_start_id 6
#define echo_ram_start_id 7
#define oam_start_id 8
#define non_usable_start_id 9
#define io_registers_start_id 10
#define hram_start_id 11
#define interrupt_enable_register_id 12


#define vram_bank_size 0x2000
#define wram_bank_size 0x1000

#define io_port_base io_registers_start
#define io_port_p1 0x00
#define io_port_sb 0x01
#define io_port_sc 0x02
#define io_port_div 0x04
#define io_port_tima 0x05
#define io_port_tma 0x06
#define io_port_tac 0x07
#define io_port_if 0x0F
#define io_port_nr10 0x10
#define io_port_nr11 0x11
#define io_port_nr12 0x12
#define io_port_nr13 0x13
#define io_port_nr14 0x14
#define io_port_nr21 0x16
#define io_port_nr22 0x17
#define io_port_nr23 0x18
#define io_port_nr24 0x19
#define io_port_nr30 0x1A
#define io_port_nr31 0x1B
#define io_port_nr32 0x1C
#define io_port_nr33 0x1D
#define io_port_nr34 0x1E
#define io_port_nr41 0x20
#define io_port_nr42 0x21
#define io_port_nr43 0x22
#define io_port_nr44 0x23
#define io_port_nr50 0x24
#define io_port_nr51 0x25
#define io_port_nr52 0x26
#define io_port_wave_ram_start 0x30
#define io_port_wave_ram_end 0x3F
#define io_port_lcdc 0x40
#define io_port_stat 0x41
#define io_port_scy 0x42
#define io_port_scx 0x43
#define io_port_ly 0x44
#define io_port_lyc 0x45
#define io_port_dma 0x46
#define io_port_bgp 0x47
#define io_port_obp0 0x48
#define io_port_obp1 0x49
#define io_port_wy 0x4A
#define io_port_wx 0x4B
#define io_port_key1 0x4D
#define io_port_vbk 0x4F
#define io_port_hdma1 0x51
#define io_port_hdma2 0x52
#define io_port_hdma3 0x53
#define io_port_hdma4 0x54
#define io_port_hdma5 0x55
#define io_port_rp 0x56
#define io_port_bcps 0x68
#define io_port_bcpd 0x69
#define io_port_ocps 0x6A
#define io_port_ocpd 0x6B
#define io_port_opri 0x6C
#define io_port_svbk 0x70
#define io_port_pcm12 0x76
#define io_port_pcm34 0x77
#define io_port_ie 0x7F

typedef struct {
    uint8_t rom[0x8000];
    uint8_t vram[vram_bank_size * 2];
    uint8_t external_ram[0x2000];
    uint8_t wram[wram_bank_size * 8];
    uint8_t oam[0xA0];
    uint8_t hram[0x7F];
    uint8_t interrupt_enable;
} memory_t;

void memory_init(memory_t *memory);
uint8_t memory_read8(memory_t *memory, uint16_t address);
void memory_write8(memory_t *memory, uint16_t address, uint8_t value);
uint16_t memory_read16(memory_t *memory, uint16_t address);
void memory_write16(memory_t *memory, uint16_t address, uint16_t value);

#endif 