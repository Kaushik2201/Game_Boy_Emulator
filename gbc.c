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
  return 0;
};
void gbc_run(gbc_t *gbc) {
  // Connect all components
  gbc_cpu_connect(&gbc->cpu, &gbc->mem);
  gbc_graphic_connect(&gbc->graphics, &gbc->mem);
  gbc_mbc_connect(&gbc->mbc, &gbc->mem);
  timer_connect();
  while (1) {
    uint64_t target_cycles = get_time() + CYCLES_PER_FRAME;
    gbc->cpu.cycles = 0;

    while (get_time() < target_cycles) {
      gbc_cpu_cycle(&gbc->cpu);
      gbc_graphic_cycle(&gbc->graphics);
      timer_update(gbc->cpu.cycles);

      // Handle input (would need to be implemented)
      // process_input(gbc);
    }
  }
}
