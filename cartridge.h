#ifndef cartridge_h
#define cartridge_h

#include <stdint.h>
#include <stdbool.h>

// Cartridge types
#define CART_TYPE_ROM_ONLY 0x00
#define CART_TYPE_MBC1 0x01
#define CART_TYPE_MBC1_RAM 0x02
#define CART_TYPE_MBC1_RAM_BATTERY 0x03
#define CART_TYPE_MBC2 0x05
#define CART_TYPE_MBC2_BATTERY 0x06
#define CART_TYPE_ROM_RAM 0x08
#define CART_TYPE_ROM_RAM_BATTERY 0x09
#define CART_TYPE_MMM01 0x0B
#define CART_TYPE_MMM01_RAM 0x0C
#define CART_TYPE_MMM01_RAM_BATTERY 0x0D
#define CART_TYPE_MBC3_TIMER_BATTERY 0x0F
#define CART_TYPE_MBC3_TIMER_RAM_BATTERY 0x10
#define CART_TYPE_MBC3 0x11
#define CART_TYPE_MBC3_RAM 0x12
#define CART_TYPE_MBC3_RAM_BATTERY 0x13
#define CART_TYPE_MBC5 0x19
#define CART_TYPE_MBC5_RAM 0x1A
#define CART_TYPE_MBC5_RAM_BATTERY 0x1B
#define CART_TYPE_MBC5_RUMBLE 0x1C
#define CART_TYPE_MBC5_RUMBLE_RAM 0x1D
#define CART_TYPE_MBC5_RUMBLE_RAM_BATTERY 0x1E
#define CART_TYPE_MBC6 0x20
#define CART_TYPE_MBC7_SENSOR_RUMBLE_RAM_BATTERY 0x22
#define CART_TYPE_POCKET_CAMERA 0xFC
#define CART_TYPE_BANDAI_TAMA5 0xFD
#define CART_TYPE_HUC3 0xFE
#define CART_TYPE_HUC1_RAM_BATTERY 0xFF


typedef struct {
    uint8_t entry_point[4];
    uint8_t logo[0x30];
    char title[16];
    char manufacturer_code[4];
    uint8_t cgb_flag;
    char new_licensee_code[2];
    uint8_t sgb_flag;
    uint8_t cartridge_type;
    uint8_t rom_size;
    uint8_t ram_size;
    uint8_t destination_code;
    uint8_t old_licensee_code;
    uint8_t mask_rom_version;
    uint8_t header_checksum;
    uint16_t global_checksum;
} cartridge_header_t;

typedef struct {
    uint8_t *rom;
    uint8_t *ram;
    uint8_t rom_bank_count;
    uint8_t ram_bank_count;
    uint8_t current_rom_bank;
    uint8_t current_ram_bank;
    cartridge_header_t header;
} cartridge_t;


bool cartridge_init(cartridge_t *cartridge, const char *filename);
void cartridge_free(cartridge_t *cartridge);
uint8_t cartridge_read8(cartridge_t *cartridge, uint16_t address);
void cartridge_write8(cartridge_t *cartridge, uint16_t address, uint8_t value);
void cartridge_set_rom_bank(cartridge_t *cartridge, uint8_t bank);
void cartridge_set_ram_bank(cartridge_t *cartridge, uint8_t bank);
bool cartridge_verify_header(cartridge_t *cartridge);

#endif 
