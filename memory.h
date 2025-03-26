#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>


#define MEMORY_MAP_ENTRIES 14

#define ROM_BANK_00_START 0x0000
#define ROM_BANK_00_END 0x3FFF
#define ROM_BANK_SWITCH_START 0x4000
#define ROM_BANK_SWITCH_END 0x7FFF
#define VRAM_START 0x8000
#define VRAM_END 0x9FFF
#define EXTERNAL_RAM_START 0xA000
#define EXTERNAL_RAM_END 0xBFFF
#define WRAM_BANK_0_START 0xC000
#define WRAM_BANK_0_END 0xCFFF
#define WRAM_BANK_SWITCH_START 0xD000
#define WRAM_BANK_SWITCH_END 0xDFFF
#define ECHO_RAM_START 0xE000
#define ECHO_RAM_END 0xFDFF
#define OAM_START 0xFE00
#define OAM_END 0xFE9F
#define NON_USABLE_START 0xFEA0
#define NON_USABLE_END 0xFEFF
#define IO_REGISTERS_START_1 0xFF00
#define IO_REGISTERS_END_1 0xFF0F
#define AUDIO_BEGIN 0xFF10
#define AUDIO_END 0xFF3F
#define IO_REGISTERS_START_2 0xFF40
#define IO_REGISTERS_END_2 0xFF7F
#define HRAM_START 0xFF80
#define HRAM_END 0xFFFE
#define INTERRUPT_ENABLE_REGISTER 0xFFFF

// IDs
#define ROM_BANK_0_START_ID 1
#define ROM_BANK_SWITCH_START_ID 2
#define VRAM_START_ID 3
#define EXTERNAL_RAM_START_ID 4
#define WRAM_BANK_0_START_ID 5
#define WRAM_BANK_SWITCH_START_ID 6
#define ECHO_RAM_START_ID 7
#define OAM_START_ID 8
#define NON_USABLE_START_ID 9
#define IO_REGISTERS_START_ID 10
#define AUDIO_ID 11
#define IO_REGISTERS_START_ID_2 12
#define HRAM_START_ID 13
#define INTERRUPT_ENABLE_REGISTER_ID 14

#define VRAM_BANK_SIZE 0x2000
#define WRAM_BANK_SIZE 0x1000

#define IO_PORT_BASE IO_REGISTERS_START_1
#define IO_PORT_P1 0x00
#define IO_PORT_SB 0x01
#define IO_PORT_SC 0x02
#define IO_PORT_DIV 0x04
#define IO_PORT_TIMA 0x05
#define IO_PORT_TMA 0x06
#define IO_PORT_TAC 0x07
#define IO_PORT_IF 0x0F
#define IO_PORT_NR10 0x10
#define IO_PORT_NR11 0x11
#define IO_PORT_NR12 0x12
#define IO_PORT_NR13 0x13
#define IO_PORT_NR14 0x14
#define IO_PORT_NR21 0x16
#define IO_PORT_NR22 0x17
#define IO_PORT_NR23 0x18
#define IO_PORT_NR24 0x19
#define IO_PORT_NR30 0x1A
#define IO_PORT_NR31 0x1B
#define IO_PORT_NR32 0x1C
#define IO_PORT_NR33 0x1D
#define IO_PORT_NR34 0x1E
#define IO_PORT_NR41 0x20
#define IO_PORT_NR42 0x21
#define IO_PORT_NR43 0x22
#define IO_PORT_NR44 0x23
#define IO_PORT_NR50 0x24
#define IO_PORT_NR51 0x25
#define IO_PORT_NR52 0x26
#define IO_PORT_WAVE_RAM_START 0x30
#define IO_PORT_WAVE_RAM_END 0x3F
#define IO_PORT_LCDC 0x40
#define IO_PORT_STAT 0x41
#define IO_PORT_SCY 0x42
#define IO_PORT_SCX 0x43
#define IO_PORT_LY 0x44
#define IO_PORT_LYC 0x45
#define IO_PORT_DMA 0x46
#define IO_PORT_BGP 0x47
#define IO_PORT_OBP0 0x48
#define IO_PORT_OBP1 0x49
#define IO_PORT_WY 0x4A
#define IO_PORT_WX 0x4B
#define IO_PORT_KEY1 0x4D
#define IO_PORT_VBK 0x4F
#define IO_PORT_DISABLE_BOOT_ROM 0x50
#define IO_PORT_HDMA1 0x51
#define IO_PORT_HDMA2 0x52
#define IO_PORT_HDMA3 0x53
#define IO_PORT_HDMA4 0x54
#define IO_PORT_HDMA5 0x55
#define IO_PORT_RP 0x56
#define IO_PORT_BCPS 0x68
#define IO_PORT_BCPD 0x69
#define IO_PORT_OCPS 0x6A
#define IO_PORT_OCPD 0x6B
#define IO_PORT_OPRI 0x6C
#define IO_PORT_SVBK 0x70
#define IO_PORT_PCM12 0x76
#define IO_PORT_PCM34 0x77
#define IO_PORT_IE 0x7F

#define GBC_BOOT_ROM_SIZE 0x8ff

typedef struct {
  uint16_t id;
  uint16_t addr_begin;
  uint16_t addr_end;
  memory_read read;
  memory_write write;
  void *udata;
} memory_map_entry_t;

typedef struct {
  uint16_t c[4];
} gbc_palette_t;

typedef struct {
  memory_read read;
  memory_write write;
  memory_map_entry_t map[MEMORY_MAP_ENTRIES];
  uint8_t wram[WRAM_BANK_SIZE * 8]; /* 8 WRAM banks */
  uint8_t hram[HRAM_END - HRAM_START + 1];

  uint8_t io_ports[IO_REGISTERS_END_2 - IO_REGISTERS_START_1 + 1];
  uint8_t oam[OAM_END - OAM_START + 1];

  gbc_palette_t bg_palette[8];
  gbc_palette_t obj_palette[8];

  uint8_t boot_rom_enabled;
  uint8_t boot_rom[GBC_BOOT_ROM_SIZE];
} gbc_memory_t;

#define IO_ADDR_PORT(addr) ((addr) - IO_PORT_BASE)
#define IO_PORT_ADDR(port) ((port) + IO_PORT_BASE)

#define IO_PORT_READ(mem, port) ((mem)->io_ports[(port)])
#define IO_PORT_WRITE(mem, port, data) ((mem)->io_ports[(port)] = (data))

#define REQUEST_INTERRUPT(mem, intp) ((mem)->io_ports[IO_PORT_IF] |= (intp))

#define BG_PALETTE_READ(mem, idx) ((mem)->bg_palette + ((idx)))
#define OBJ_PALETTE_READ(mem, idx) ((mem)->obj_palette + ((idx)))
#define OAM_ADDR(mem) ((mem)->oam)

void mem_init(gbc_memory_t *memory);
void register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry);
void *connect_io_port(gbc_memory_t *mem, uint16_t addr);

typedef uint8_t (*memory_read)(void *udata, uint16_t addr);
typedef uint8_t (*memory_write)(void *udata, uint16_t addr, uint8_t data);

#endif