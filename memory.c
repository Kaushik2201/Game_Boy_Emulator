#include "memory.h"
#include <string.h>
#include "common.h"
#include "graphics.h"


void gbc_mem_init(gbc_memory_t *mem) 
{
  memset(mem, 0, sizeof(gbc_memory_t));

  mem->write = mem_write;
  mem->read = mem_read;

  /* WRAM */
  memory_map_entry_t entry;
  entry.id = WRAM_BANK_0_START_ID;
  entry.addr_begin = WRAM_BANK_0_START;
  entry.addr_end = WRAM_BANK_0_END;
  entry.read = mem_raw_read;
  entry.write = mem_raw_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* GBC switchable RAM bank */
  entry.id = WRAM_BANK_SWITCH_START_ID;
  entry.addr_begin = WRAM_BANK_SWITCH_START;
  entry.addr_end = WRAM_BANK_SWITCH_END;
  entry.read = bank_n_read;
  entry.write = bank_n_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* High RAM */
  entry.id = HRAM_START_ID;
  entry.addr_begin = HRAM_START;
  entry.addr_end = HRAM_END;
  entry.read = mem_raw_read;
  entry.write = mem_raw_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* IO Port */
  entry.id = IO_REGISTERS_START_ID;
  entry.addr_begin = IO_REGISTERS_START_1;
  entry.addr_end = IO_REGISTERS_END_1;
  entry.read = io_port_read;
  entry.write = io_port_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* IO Port 2 */
  entry.id = IO_REGISTERS_START_ID_2;
  entry.addr_begin = IO_REGISTERS_START_2;
  entry.addr_end = IO_REGISTERS_END_2;
  entry.read = io_port_read;
  entry.write = io_port_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* 0xFEA0 - 0xFEFF, CGB revision E */
  entry.id = NON_USABLE_START_ID;
  entry.addr_begin = NON_USABLE_START;
  entry.addr_end = NON_USABLE_END;
  entry.read = not_usable_read;
  entry.write = not_usable_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* Echo RAM */
  entry.id = ECHO_RAM_START_ID;
  entry.addr_begin = ECHO_RAM_START;
  entry.addr_end = ECHO_RAM_END;

  entry.read = mem_echo_read;
  entry.write = mem_echo_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* OAM */
  entry.id = OAM_START_ID;
  entry.addr_begin = OAM_START;
  entry.addr_end = OAM_END;
  entry.read = oam_read;
  entry.write = oam_write;
  entry.udata = mem;

  register_memory_map(mem, &entry);

  /* https://gbdev.io/pandocs/Power_Up_Sequence.html */
  IO_PORT_WRITE(mem, IO_PORT_TAC, 0xF8);
  IO_PORT_WRITE(mem, IO_PORT_SC, 0x7F);
  IO_PORT_WRITE(mem, IO_PORT_IF, 0xE1);
  IO_PORT_WRITE(mem, IO_PORT_LCDC, 0x91);
  IO_PORT_WRITE(mem, IO_PORT_BGP, 0xFC);
  IO_PORT_WRITE(mem, IO_PORT_VBK, 0xFE);
  IO_PORT_WRITE(mem, IO_PORT_RP, 0x3E);
  IO_PORT_WRITE(mem, IO_PORT_SVBK, 0xF8);

  /* This one is crucial, otherwise games like Tetris_dx will stuck at the title screen forever, cost me almost two days to identify this */
  IO_PORT_WRITE(mem, IO_PORT_P1, 0xCF);

};



memory_map_entry_t *select_entry(gbc_memory_t *mem, uint16_t addr) 
{
  for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
    if (addr >= mem->map[i].addr_begin && addr <= mem->map[i].addr_end) {
      if (mem->map[i].id == 0) {
        return NULL;
      }
      return &mem->map[i];
    }
  }
  return NULL;
};



static uint8_t mem_write(void *udata, uint16_t addr, uint8_t data) 
{
  LOG_DEBUG("[MEM] Writing to memory at address %x [%x]\n", addr, data);
  gbc_memory_t *mem = (gbc_memory_t*)udata;
  memory_map_entry_t *entry = select_entry(mem, addr);

  if (entry == NULL) {
      LOG_ERROR("[MEM] No memory map entry found for address %x\n", addr);
      abort();
  }

  return entry->write(entry->udata, addr, data);
};



static uint8_t mem_read(void *udata, uint16_t addr) 
{
  memory_map_entry_t *entry = select_entry((gbc_memory_t *)udata, addr);
  if (entry == NULL) {
    LOG_ERROR("[MEM] No memory map entry found for address %x\n", addr);
    abort();
  }
    uint8_t data = entry->read(entry->udata, addr);
    LOG_DEBUG("[MEM] Reading from memory at address %x [%x]\n", addr, data);
    return data;
};


void register_memory_map(gbc_memory_t *mem, memory_map_entry_t *entry) {
  LOG_DEBUG("[MEM] Registering memory map entry with id %d [0x%x] - [0x%x]\n", entry->id, entry->addr_begin, entry->addr_end);

  if (entry->addr_begin > entry->addr_end) {
    LOG_ERROR("[MEM] Memory map entry id %d has invalid address range [%x] - [%x]\n", entry->id, entry->addr_begin, entry->addr_end);
    abort();
  }

  if (entry->addr_begin < ROM_BANK_00_START || entry->addr_end > HRAM_END) {
    LOG_ERROR("[MEM] Memory map entry id %d is already registered\n", entry->id);
    abort();
  }

  for (int i = 0; i < MEMORY_MAP_ENTRIES; i++) {
    if (mem->map[i].id != NULL && mem->map[i].id == entry->id &&
        mem->map[i].addr_begin <= entry->addr_end &&
        mem->map[i].addr_end >= entry->addr_begin) {
      return;
    }
  }
  mem->map[entry->id - 1] = *entry;
};

void* connect_io_port(gbc_memory_t *mem, uint16_t port)
{
    return (mem->io_ports + port);
}


static uint8_t mem_raw_write(void *udata, uint16_t addr, uint8_t data) 
{
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  if (addr >= WRAM_BANK_0_START && addr <= WRAM_BANK_0_END) {
    mem->wram[addr - WRAM_BANK_0_START] = data;
    return data;
  } else if (addr >= HRAM_START && addr <= HRAM_END) {
    mem->hram[addr - HRAM_START] = data;
    return data;
  }

  LOG_ERROR("[MEM] Invalid write, %x is not a valid WRAM addr \n", addr);
  abort();
}

static uint8_t mem_raw_read(void *udata, uint16_t addr) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  if (IN_RANGE(addr, HRAM_START, HRAM_END)) {
    return mem->hram[addr - HRAM_START];

} else if (IN_RANGE(addr, WRAM_BANK_0_START, WRAM_BANK_0_END)) {
    return mem->wram[addr - WRAM_BANK_0_START];

}

LOG_ERROR("[MEM] Invalid read, %x is not a valid WRAM addr \n", addr);
abort();
}


static uint8_t mem_echo_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  addr = addr - ECHO_RAM_START + WRAM_BANK_0_START;
  return mem->write(mem, addr, data);
};


static uint8_t mem_echo_read(void *udata, uint16_t addr) 
{
  gbc_memory_t *mem = (gbc_memory_t*)udata;
  addr = addr - ECHO_RAM_START + WRAM_BANK_0_START;
  return mem->read(mem, addr);
};



static uint8_t io_port_read(void *udata, uint16_t addr) {
  LOG_DEBUG("[MEM] Reading from IO port at address %x\n", addr);
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint8_t port = IO_ADDR_PORT(addr);
  // return mem->io_ports[port];
  if (port == IO_PORT_BCPD) {
    return *((uint8_t *)(mem->bg_palette) +
              (IO_PORT_READ(mem, IO_PORT_BCPS) & 0x3f));
  } else if (port == IO_PORT_OCPD) {
    return *((uint8_t *)(mem->obj_palette) +
              (IO_PORT_READ(mem, IO_PORT_OCPS) & 0x3f));
  } else if (port == IO_PORT_VBK) {
    return IO_PORT_READ(mem, IO_PORT_VBK) | 0xfe;
  }
  return IO_PORT_READ(mem, port);
};



static uint8_t io_port_write(void *udata, uint16_t addr, uint8_t data) 
{
  LOG_DEBUG("[MEM] Writing to IO port at address %x [%x]\n", addr, data);

  uint8_t port = IO_ADDR_PORT(addr);

  gbc_memory_t *mem = (gbc_memory_t*)udata;

  #if LOGLEVEL == LOG_LEVEL_DEBUG
  if (port == IO_PORT_TAC) {
      LOG_DEBUG("[Timer] Writing to TAC register [%x]\n", data);
  } else if (port == IO_PORT_TMA) {
      LOG_DEBUG("[Timer] Writing to TMA register [%x]\n", data);
  } else if (port == IO_PORT_TIMA) {
      LOG_DEBUG("[Timer] Writing to TIMA register [%x]\n", data);
  } else if (port == IO_PORT_STAT) {
      LOG_DEBUG("[STAT] Writing to STAT register [%x]\n", data);
  } else if (port == IO_PORT_LCDC) {
      LOG_DEBUG("[STAT] Writing to STAT register [%x]\n", data);
  }
  #endif

  if (port == IO_PORT_DIV) {
      /* Writing to DIV resets it */
      data = 0;
  } else if (port == IO_PORT_DISABLE_BOOT_ROM) {
      /* Writing 0x11 to this register disables the boot ROM */
      if (data == 0x11) {
          mem->boot_rom_enabled = 0;
      }
  } else if (port == IO_PORT_P1) {
      /* https://gbdev.io/pandocs/Joypad_Input.html#ff00--p1joyp-joypad */
      if ((data & 0x30) == 0x30) {
          /* all keys released */
          data = data | 0x0f;
      } else {
          /* lower 4 bits are read-only */
          uint8_t v = IO_PORT_READ((gbc_memory_t*)udata, IO_PORT_P1);
          data = (data & 0xf0) | (v & 0x0f);
      }
  } else if (port == IO_PORT_BCPD) {
      uint8_t bcps = IO_PORT_READ(mem, IO_PORT_BCPS);
      ((uint8_t*)mem->bg_palette)[bcps & 0x3f] = data;
      if (bcps & 0x80) {
          /* auto increment */
          bcps = (bcps + 1) & 0x3f | 0x80;
          IO_PORT_WRITE(mem, IO_PORT_BCPS, bcps);
      }
  } else if (port == IO_PORT_OCPD) {
      uint8_t ocps = IO_PORT_READ(mem, IO_PORT_OCPS);
      ((uint8_t*)mem->obj_palette)[ocps & 0x3f] = data;
      if (ocps & 0x80) {
          /* auto increment */
          ocps = (ocps + 1) & 0x3f | 0x80;
          IO_PORT_WRITE(mem, IO_PORT_OCPS, ocps);
      }
  } else if (port == IO_PORT_DMA) {
      io_dma_transer(mem, data);
  } else if (port == IO_PORT_VBK) {
      data &= 0x01;
  } else if (port == IO_PORT_HDMA5) {
      data = hdma_transer(mem, data);
  }

  IO_PORT_WRITE(mem, port, data);
  return data;
};



static inline uint8_t oam_read(void *udata, uint16_t addr) 
{
  return ((gbc_memory_t*)udata)->oam[addr - OAM_START];
};



static inline uint8_t oam_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;
  uint16_t oam_addr = addr - OAM_START;
  mem->oam[oam_addr] = data;
  return data;

};


static uint8_t bank_n_write(void *udata, uint16_t addr, uint8_t data) {
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
  if (bank == 0){
    bank = 1;
  }

  LOG_DEBUG("[MEM] Writing to switchable RAM bank [%x] at address %x [%x]\n", bank, addr, data);

  
  uint16_t bank_base = (bank * WRAM_BANK_SIZE);
  mem->wram[addr - WRAM_BANK_SWITCH_START + bank_base] = data;

  return data;
};



static uint8_t bank_n_read(void *udata, uint16_t addr) 
{
  gbc_memory_t *mem = (gbc_memory_t *)udata;

  uint8_t bank = mem->io_ports[IO_PORT_SVBK] & 0x07;
  if (bank == 0){
    bank = 1;
  }

  LOG_DEBUG("[MEM] Reading from switchable RAM bank [%x] at address %x\n", bank, addr);

    uint16_t bank_base = (bank * WRAM_BANK_SIZE);
    return mem->wram[addr - WRAM_BANK_SWITCH_START + bank_base];
};


static uint8_t not_usable_write(void *udata, uint16_t addr, uint8_t data) {
  LOG_INFO("[MEM] Writing to Not-Usable(Nintendo says) memory at address %x [%x]\n", addr, data);
  return 0;
};

static uint8_t not_usable_read(void *udata, uint16_t addr)
{
    /* It is actually readable, this implementation emulates CGB revision E */
    LOG_INFO("[MEM] Reading from Not-Usable(Nintendo says) memory at address %x\n", addr);

    uint8_t lcdsr = IO_PORT_READ((gbc_memory_t*)udata, IO_PORT_STAT);

    uint8_t mode = lcdsr & PPU_MODE_MASK;
    if (mode == PPU_MODE_2 || mode == PPU_MODE_3) {
        return 0xff;
    }

    addr = addr & 0xf0;
    addr |= addr >> 4;
    return addr;
}


static inline void io_dma_transfer(gbc_memory_t *mem, uint8_t addr) {
  uint16_t src = addr << 8;
  for (uint16_t dst = OAM_START; dst <= OAM_END; dst++, src++) {
      mem->oam[dst-OAM_START] = mem->read(mem, src);
  }
}
static inline uint8_t hdma_transfer(gbc_memory_t *mem, uint8_t data) {
    uint16_t src = (IO_PORT_READ(mem, IO_PORT_HDMA1) << 8) | IO_PORT_READ(mem, IO_PORT_HDMA2);
    uint16_t dst = (IO_PORT_READ(mem, IO_PORT_HDMA3) << 8) | IO_PORT_READ(mem, IO_PORT_HDMA4);
    src &= 0xfff0;
    dst &= 0x1ff0;
    dst += 0x8000;

    uint16_t len = ((data & 0x7f) + 1) * 0x10;

    for (int i = 0; i < len; i++)
        mem->write(mem, dst + i, mem->read(mem, src + i));

    return 0xff;
}