#ifndef MBC_H
#define MBC_H

#include <stdint.h>
#include "memory.h"
#include "cartridge.h"

#define MAX_ROM_BANKS 512
#define MAX_RAM_BANKS 16
#define ROM_BANK_SIZE 0x4000    /* 16KB */
#define RAM_BANK_SIZE 0x2000    /* 8KB */

#define MBC1_ROM_BEGIN 0x0000
#define MBC1_ROM_END   0x7fff

#define MBC_RAM_ENABLE_START 0x0000
#define MBC_RAM_ENABLE_END 0x1FFF

#define MBC1_ROM_BANK_NUMBER_START 0x2000
#define MBC1_ROM_BANK_NUMBER_END 0x3FFF

#define MBC1_RAM_BANK_NUMBER_START 0x4000
#define MBC1_RAM_BANK_NUMBER_END 0x5FFF

#define MBC1_BANKING_MODE_START 0x6000
#define MBC1_BANKING_MODE_END 0x7FFF

#define MBC1_ROM_BANK0_BEGIN 0x0000
#define MBC1_ROM_BANK0_END   0x3fff

#define MBC1_ROM_BANK_N_BEGIN 0x4000
#define MBC1_ROM_BANK_N_END   0x7fff

#define MBC1_RAM_BEGIN EXTERNAL_RAM_START
#define MBC1_RAM_END   EXTERNAL_RAM_END

#define MBC1_RAM_ENABLE 0x0a

#define MBC1_BANKING_MODE_ROM 0
#define MBC1_BANKING_MODE_RAM 1

#define MBC1_ROM_BANK_MASK_SHIFT 5
#define MBC1_ROM_BANK_MASK ((1 << MBC1_ROM_BANK_MASK_SHIFT) - 1)

#define MBC1_RAM_BANK_MASK_SHIFT 2
#define MBC1_RAM_BANK_MASK ((1 << MBC1_RAM_BANK_MASK_SHIFT) - 1)

#define MBC5_ROM_BANK_LSB_START 0x2000
#define MBC5_ROM_BANK_LSB_END 0x2FFF

#define MBC5_ROM_BANK_MSB_START 0x3000
#define MBC5_ROM_BANK_MSB_END 0x3FFF

#define MBC5_RAM_BANK_MASK      0x0f
#define MBC5_ROM_BANK_MASK      0x1ff

#define MBC5_REG_ROM_BANK_MSB_MASK 0x1
#define MBC5_REG_ROM_BANK_MSB_SHIFT 8

typedef uint8_t (*mbc_read_func)(gbc_mbc_t *mbc, uint16_t addr);
typedef uint8_t (*mbc_write_func)(gbc_mbc_t *mbc, uint16_t addr, uint8_t data);

typedef struct
{
    uint16_t rom_bank;
    uint16_t rom_bank_size;
    uint8_t ram_bank;
    uint8_t ram_bank_size;
    uint8_t ram_enabled;
    uint8_t mode;
    uint8_t type;

    mbc_read_func read;
    mbc_write_func write;

    gbc_memory_t *mem;
    cartridge_t *cart;

    uint8_t *rom_banks;
    uint8_t ram_banks[MAX_RAM_BANKS * RAM_BANK_SIZE];

} gbc_mbc_t;

void gbc_mbc_init(gbc_mbc_t *mbc);
void gbc_mbc_connect(gbc_mbc_t *mbc, gbc_memory_t *mem);
void gbc_mbc_init_with_cart(gbc_mbc_t *mbc, cartridge_t *cart);

#endif 
