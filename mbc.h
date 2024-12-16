#ifndef MBC_H
#define MBC_H

#include <stdint.h>
#include <stdbool.h>

// Macros
#define MBC_RAM_ENABLE_START 0x0000
#define MBC_RAM_ENABLE_END 0x1FFF
#define MBC1_ROM_BANK_NUMBER_START 0x2000
#define MBC1_ROM_BANK_NUMBER_END 0x3FFF
#define MBC1_RAM_BANK_NUMBER_START 0x4000
#define MBC1_RAM_BANK_NUMBER_END 0x5FFF
#define MBC1_BANKING_MODE_START 0x6000
#define MBC1_BANKING_MODE_END 0x7FFF

#define MBC5_ROM_BANK_LSB_START 0x2000
#define MBC5_ROM_BANK_LSB_END 0x2FFF
#define MBC5_ROM_BANK_MSB_START 0x3000
#define MBC5_ROM_BANK_MSB_END 0x3FFF
#define MBC5_RAM_BANK_NUMBER_START 0x4000
#define MBC5_RAM_BANK_NUMBER_END 0x5FFF


typedef struct {
    uint8_t ram_enable;
    uint8_t rom_bank_number;
    uint8_t ram_bank_number;
    uint8_t banking_mode;
} mbc1_t;

typedef struct {
    uint8_t ram_enable;
    uint8_t rom_bank_lsb;
    uint8_t rom_bank_msb;
    uint8_t ram_bank_number;
    uint8_t rumble_control;
} mbc5_t;

void mbc1_init(mbc1_t *mbc);
void mbc1_enable_ram(mbc1_t *mbc, uint8_t value);
void mbc1_set_rom_bank(mbc1_t *mbc, uint8_t bank);
void mbc1_set_ram_bank(mbc1_t *mbc, uint8_t bank);
void mbc1_set_banking_mode(mbc1_t *mbc, uint8_t mode);

void mbc5_init(mbc5_t *mbc);
void mbc5_enable_ram(mbc5_t *mbc, uint8_t value);
void mbc5_set_rom_bank(mbc5_t *mbc, uint16_t bank);
void mbc5_set_ram_bank(mbc5_t *mbc, uint8_t bank);
void mbc5_set_rumble(mbc5_t *mbc, bool enable);

#endif 
