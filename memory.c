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
static uint8_t mem_raw_read(void *udata, uint16_t addr) {
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
static uint8_mem_echo_write(void *udata, uint16_t addr, uint8_t data) {
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
    mem->io_ports[port] = data;
    if (port == IO_PORT_IE) {
      mem->io_ports[IO_PORT_IF] |= data;
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

  uint16_t bank_addr = addr - WRAM_BANK_SWITCH_START + WRAM_BANK_0_START;
  if (bank_addr >= WRAM_BANK_0_START && bank_addr <= WRAM_BANK_0_END) {
    mem->wram[bank_addr - WRAM_BANK_0_START] = data;
  }
  return data;
};
static uint8_t bank_n_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t bank_addr = addr - WRAM_BANK_SWITCH_START + WRAM_BANK_0_START;
  if (bank_addr >= WRAM_BANK_0_START && bank_addr <= WRAM_BANK_0_END) {
    return mem->wram[bank_addr - WRAM_BANK_0_START];
  }
  return 0;
};
static uint8_t not_usable_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t not_usable_addr = addr - NON_USABLE_START + NON_USABLE_END;
  if (not_usable_addr >= NON_USABLE_START &&
      not_usable_addr <= NON_USABLE_END) {
    mem->wram[not_usable_addr - NON_USABLE_START] = data;
  }
  return data;
};
static uint8_t not_usable_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t not_usable_addr = addr - NON_USABLE_START + NON_USABLE_END;
  if (not_usable_addr >= NON_USABLE_START &&
      not_usable_addr <= NON_USABLE_END) {
    return mem->wram[not_usable_addr - NON_USABLE_START];
  }
  return 0;
};

static inline void io_dma_transfer(gbc_memory_t *mem, uint8_t addr) {}
static inline uint8_t hdma_transfer(gbc_memory_t *mem, uint8_t data) {

};