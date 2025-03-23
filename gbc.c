#include "gbc.h"
#include "utils.h"
int gbc_init(gbc_t *gbc, const char *game_rom, const char *boot_rom) {
  gbc_cpu_init(&gbc->cpu);
  mem_init(&gbc->mem);
  gbc_graphic_init(&gbc->graphics);
  init_instruction_set(&gbc->isa);
  gbc_mbc_init(&gbc->mbc);
  timer_init();
  gbc_io_init(&gbc->io);

  // Connect all components
  gbc_cpu_connect(&gbc->cpu, &gbc->mem);
  gbc_graphic_connect(&gbc->graphics, &gbc->mem);
  gbc_mbc_connect(&gbc->mbc, &gbc->mem);
  timer_connect();

  FILE *game_rom_file = fopen(game_rom, "rb");
  if (!game_rom_file) {
    LOG_ERROR("Failed to open game ROM file: %s\n", game_rom);
    return -1;
  }

  // Get the file size of the game ROM
  fseek(game_rom_file, 0, SEEK_END);
  long rom_size = ftell(game_rom_file);
  fseek(game_rom_file, 0, SEEK_SET);

  // Allocate memory and read the ROM data
  uint8_t *rom_data = (uint8_t *)malloc(rom_size);
  if (!rom_data) {
    LOG_ERROR("Failed to allocate memory for game ROM\n");
    fclose(game_rom_file);
    return -1;
  }
  fread(rom_data, 1, rom_size, game_rom_file);
  fclose(game_rom_file);

  // Load the ROM data into the cartridge
  cartridge_t *cart = cartridge_load(rom_data);
  gbc_mbc_init_with_cart(&gbc->mbc, cart);
  WRITE_R16(&gbc->cpu.reg, REG_PC, 0x100);
  if (boot_rom) {
    bootload(gbc, boot_rom);
    WRITE_R16(&gbc->cpu.reg, REG_PC, 0x000);
  } else {
    LOG_ERROR("No boot ROM provided, skipping boot ROM loading\n");
  }

  return 0;
};
void gbc_run(gbc_t *gbc) {
  uint64_t prev_frame_time = get_time();
  uint64_t current_frame_time = 0;
  while (1) {
    current_frame_time = get_time();
    gbc->cpu.cycles = 0;
    if (current_frame_time - prev_frame_time < FRAME_INTERVAL) {
      continue;
    }
    prev_frame_time = current_frame_time;
    uint16_t cycles = 0;
    while (cycles < CYCLES_PER_FRAME) {
      gbc_cpu_cycle(&gbc->cpu);
      gbc_graphic_cycle(&gbc->graphics);
      timer_update(gbc->cpu.cycles);
      // Handle input (would need to be implemented)
      // process_input(gbc);
      cycles++;
    }
  }
}

void bootload(gbc_t *gbc, const char *boot_rom_path) {
  // Load the boot ROM into memory
  FILE *boot_rom_file = fopen(boot_rom_path, "rb");
  if (!boot_rom_file) {
    LOG_ERROR("Failed to open boot ROM file: %s\n", boot_rom_path);
    return;
  }
  fread(gbc->mem.boot_rom, 1, GBC_BOOT_ROM_SIZE, boot_rom_file);
  fclose(boot_rom_file);
  gbc->mem.boot_rom_enabled = 1;
}
