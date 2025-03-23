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
