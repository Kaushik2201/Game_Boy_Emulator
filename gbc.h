#pragma once
#include "cartridge.h"
#include "common.h"
#include "cpu.h"
#include "graphics.h"
#include "io.h"
#include "isa.h"
#include "mbc.h"
#include "memory.h"
#include "timers.h"
#include "audio.h"

// A structure to represent the game boy itself, including every part of the
// system.
typedef struct {
  gbc_cpu_t cpu;
  gbc_memory_t mem;
  gbc_graphic_t graphics;
  gbc_mbc_t mbc;
  gbc_timer_t timer;
  gbc_io_t io;
  gbc_audio_t audio;

  uint32_t debug_steps;
  volatile uint8_t running:1;
  volatile uint8_t paused:1;

} gbc_t;

int gbc_init(gbc_t *gbc, const char *game_rom, const char *boot_rom);
void gbc_run(gbc_t *gbc);
void bootload(gbc_t *gbc, const char *boot_rom_path);