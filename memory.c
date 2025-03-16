#include "memory.h"
#include <string.h>
void gbc_mem_init(gbc_memory_t *mem) {
  mem->read = mem_read;
  mem->write = mem_write;
  mem->boot_rom_enabled = 1;
  memset(mem->wram, 0, sizeof(mem->wram));
  memset(mem->hram, 0, sizeof(mem->hram));
  memset(mem->io_ports, 0, sizeof(mem->io_ports));
  memset(mem->oam, 0, sizeof(mem->oam));
  memset(mem->bg_palette, 0, sizeof(mem->bg_palette));
  memset(mem->obj_palette, 0, sizeof(mem->obj_palette));
};
memory_map_entry_t *select_entry(gbc_memory_t *mem, uint16_t addr) {
  for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
    if (addr >= mem->map[i].addr_begin && addr <= mem->map[i].addr_end) {
      return &mem->map[i];
    }
  }
  return NULL;
};
static uint8_t mem_write(void *udata, uint16_t addr, uint8_t data) {
  memory_map_entry_t *entry = select_entry((gbc_memory_t *)udata, addr);
  if (entry == NULL) {
    return 0;
  }
  if (entry->write != NULL) {
    return entry->write(entry->udata, addr, data);
  } else {
    return 0;
  }
};
static uint8_t mem_read(void *udata, uint16_t addr) {
  memory_map_entry_t *entry = select_entry((gbc_memory_t *)udata, addr);
  if (entry == NULL) {
    return 0;
  }
  if (entry->read != NULL) {
    return entry->read(entry->udata, addr);
  } else {
    return 0;
  }
};
void register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry) {
  if (entry->addr_begin > entry->addr_end) {
    return;
  }
  if (entry->addr_begin < ROM_BANK_00_START || entry->addr_end > HRAM_END) {
    return;
  }
  for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
    if (mem->map[i].id == 0) {
      mem->map[i] = *entry;
      return;
    }
  }
};

void *connect_io_port(gbc_memory_t *mem, uint16_t port) {
  if (port < IO_REGISTERS_START_1 || port > IO_REGISTERS_END_2) {
    return NULL;
  }
  for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
    if (mem->map[i].addr_begin == port) {
      return mem->map[i].udata;
    }
  }
  return NULL;
};
static uint8_t mem_raw_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  if (addr >= WRAM_BANK_0_START && addr <= WRAM_BANK_0_END) {
    mem->wram[addr - WRAM_BANK_0_START] = data;
    return data;
  } else if (addr >= WRAM_BANK_SWITCH_START && addr <= WRAM_BANK_SWITCH_END) {
    uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
    if (bank == 0)
      bank = 1;
    uint32_t offset = (addr - WRAM_BANK_SWITCH_START) + (bank * WRAM_BANK_SIZE);

    if (offset < sizeof(mem->wram)) {
      mem->wram[offset] = data;
      return data;
    }
  } else if (addr >= HRAM_START && addr <= HRAM_END) {
    mem->hram[addr - HRAM_START] = data;
    return data;
  } else {
    memory_map_entry_t *entry = select_entry(mem, addr);
    if (entry != NULL && entry->write != NULL) {
      return entry->write(entry->udata, addr, data);
    }
  }

  return 0;
}
static uint8_t mem_raw_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  if (addr >= WRAM_BANK_0_START && addr <= WRAM_BANK_0_END) {
    return mem->wram[addr - WRAM_BANK_0_START];
  } else if (addr >= WRAM_BANK_SWITCH_START && addr <= WRAM_BANK_SWITCH_END) {
    uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
    if (bank == 0)
      bank = 1;
    uint32_t offset = (addr - WRAM_BANK_SWITCH_START) + (bank * WRAM_BANK_SIZE);

    if (offset < sizeof(mem->wram)) {
      return mem->wram[offset];
    }
    return 0xFF;
  } else if (addr >= HRAM_START && addr <= HRAM_END) {
    return mem->hram[addr - HRAM_START];
  } else {
    memory_map_entry_t *entry = select_entry(mem, addr);
    if (entry != NULL && entry->read != NULL) {
      return entry->read(entry->udata, addr);
    }
  }

  return 0xFF;
}
static uint8_t mem_echo_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t echo_addr = addr - ECHO_RAM_START + WRAM_BANK_0_START;
  if (echo_addr >= WRAM_BANK_0_START && echo_addr <= WRAM_BANK_0_END) {
    mem->wram[echo_addr - WRAM_BANK_0_START] = data;
  }
  return data;
};
static uint8_t mem_echo_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t echo_addr = addr - ECHO_RAM_START + WRAM_BANK_0_START;
  if (echo_addr >= WRAM_BANK_0_START && echo_addr <= WRAM_BANK_0_END) {
    return mem->wram[echo_addr - WRAM_BANK_0_START];
  }
  return 0;
};
static uint8_t io_port_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint8_t port = addr - IO_PORT_BASE;
  if (port >= IO_REGISTERS_START_1 && port <= IO_REGISTERS_END_2) {
    return mem->io_ports[port];
  }
  return 0;
};
static uint8_t io_port_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint8_t port = addr - IO_PORT_BASE;

  if (port >= IO_REGISTERS_START_1 && port <= IO_REGISTERS_END_2) {
    // Handle special registers
    switch (port) {
    case IO_PORT_DMA:
      // Trigger DMA transfer
      io_dma_transfer(mem, data);
      break;

    case IO_PORT_HDMA5:
      // Handle HDMA transfer
      return hdma_transfer(mem, data);

    case IO_PORT_IF:
      // Writing to IF register ANDs with existing value
      mem->io_ports[port] &= data;
      return data;

    case IO_PORT_DIV:
      // Writing any value to DIV resets it to 0
      mem->io_ports[port] = 0;
      return 0;

    case IO_PORT_IE:
      mem->io_ports[port] |= data;
      return data;

    default:
      mem->io_ports[port] = data;
      break;
    }
  }
  return data;
};
static inline uint8_t oam_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t oam_addr = addr - OAM_START + WRAM_BANK_0_START;
  if (oam_addr >= WRAM_BANK_0_START && oam_addr <= WRAM_BANK_0_END) {
    return mem->oam[oam_addr - WRAM_BANK_0_START];
  }
  return 0;
};
static inline uint8_t oam_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t oam_addr = addr - OAM_START + WRAM_BANK_0_START;
  if (oam_addr >= WRAM_BANK_0_START && oam_addr <= WRAM_BANK_0_END) {
    mem->oam[oam_addr - WRAM_BANK_0_START] = data;
  }
  return data;
};

static uint8_t bank_n_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
  if (bank == 0)
    bank = 1;

  uint32_t offset = (addr - WRAM_BANK_SWITCH_START) + (bank * WRAM_BANK_SIZE);

  if (offset < sizeof(mem->wram)) {
    mem->wram[offset] = data;
  }
  return data;
};

static uint8_t bank_n_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
  if (bank == 0)
    bank = 1;

  uint32_t offset = (addr - WRAM_BANK_SWITCH_START) + (bank * WRAM_BANK_SIZE);

  if (offset < sizeof(mem->wram)) {
    return mem->wram[offset];
  }
  return 0xFF;
};
static uint8_t not_usable_write(void *udata, uint16_t addr, uint8_t data) {
  if (addr >= NON_USABLE_START && addr <= NON_USABLE_END) {
    return 0xFF;
  }
  return 0xFF;
};

static uint8_t not_usable_read(void *udata, uint16_t addr) { return 0xFF; };

static inline void io_dma_transfer(gbc_memory_t *mem, uint8_t addr) {
  uint16_t source = (uint16_t)addr << 8;

  uint16_t dest = OAM_START;

  for (uint16_t i = 0; i < 0xA0; i++) {
    uint8_t value = mem_raw_read(mem, source + i);
    mem_raw_write(mem, dest + i, value);
  }
}
static inline uint8_t hdma_transfer(gbc_memory_t *mem, uint8_t data) {
  // Get the source address from HDMA1 and HDMA2 registers
  uint16_t source = ((uint16_t)mem->io_ports[IO_PORT_HDMA1] << 8) |
                    mem->io_ports[IO_PORT_HDMA2];
  // Mask lower 4 bits as they are ignored (treated as 0)
  source &= 0xFFF0;

  // Get the destination address from HDMA3 and HDMA4 registers
  uint16_t dest = ((uint16_t)mem->io_ports[IO_PORT_HDMA3] << 8) |
                  mem->io_ports[IO_PORT_HDMA4];
  // Destination is always in VRAM (0x8000-0x9FF0)
  dest = 0x8000 | (dest & 0x1FF0);

  // Extract transfer mode and length
  uint8_t mode = data >> 7;     // 0 = General Purpose DMA, 1 = HBlank DMA
  uint8_t length = data & 0x7F; // Transfer length (divided by $10, minus 1)
  uint16_t bytes = ((uint16_t)length + 1) * 0x10; // Calculate actual byte count

  // Handle transfer termination for HBlank mode
  if (mode == 0 && (mem->io_ports[IO_PORT_HDMA5] & 0x80)) {
    // If writing 0 to bit 7 while an HBlank transfer is active, terminate it
    mem->io_ports[IO_PORT_HDMA5] =
        0x80 | length; // Set bit 7 and preserve length
    return 0xFF;       // Transfer terminated
  }

  // Perform General Purpose DMA (all at once)
  if (mode == 0) {
    // Copy bytes from source to destination
    for (uint16_t i = 0; i < bytes; i++) {
      uint8_t value = mem_raw_read(mem, source + i);
      mem_raw_write(mem, dest + i, value);
    }

    // Transfer complete, set HDMA5 to 0xFF
    mem->io_ports[IO_PORT_HDMA5] = 0xFF;
  }
  // Set up HBlank DMA
  else {
    // Store transfer details in IO_PORT_HDMA5
    mem->io_ports[IO_PORT_HDMA5] = data;

    // Note: The actual HBlank transfer should be handled by the PPU/LCD
    // controller during HBlank periods. This function just sets up the
    // transfer. Additional code will be needed in the LCD/PPU emulation to
    // handle the transfer of 0x10 bytes during each HBlank.
  }

  return data;
}