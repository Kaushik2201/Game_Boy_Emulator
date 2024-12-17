#pragma once
#include<stdint.h>

#include "memory.h"
#define VRAM_BANK_SIZE (VRAM_END-VRAM_START+1)


typedef void (*screen_write)(void *udata, uint16_t addr, uint16_t data);


#define VISIBLE_HORIZONTAL_PIXELS 160
#define VISIBLE_VERTICAL_PIXELS 144
#define TILE_SIZE 8
#define TILE_MAP_SIZE 256 
#define MAX_SPRITES 40
#define MAX_SPRITES_PER_LINE 10
#define OBJ_WIDTH 8
#define OBJ_HEIGHT 8
#define OBJ_HEIGHT_2 16

#define TILE_TYPE_OBJ  1
#define TILE_TYPE_BG   2
#define TILE_TYPE_WIN  3

#define TOTAL_SCANLINES 153
#define VISIBLE_SCANLINES 143
#define FRAME_RATE 60
#define DOTS_PER_FRAME 70224
#define DOTS_PER_SCANLINE 456

//PPU Modes
#define PPU_MODE_3 3    //DRAWING
#define PPU_MODE_2 2    //OAM_SCAN
#define PPU_MODE_1 1    //VBLANK
#define PPU_MODE_0 0    //HBLANK

//Dots per PPU Mode
#define PPU_MODE_0_DOTS 100
#define PPU_MODE_1_DOTS DOTS_PER_SCANLINE
#define PPU_MODE_2_DOTS 80
#define PPU_MODE_3_DOTS (DOTS_PER_SCANLINE - PPU_MODE_0_DOTS - PPU_MODE_2_DOTS)

//LCDC Register Bits
#define LCDC_BG_PRIORITY        (1<<0)      // BG & WINDOW MASTER PRIORITY (IN CGB MODE): 0 = OFF ; 1 = ON 
#define LCDC_OBJ_ENABLE         (1<<1)      // OBJ ENABLE : 0 = OFF ; 1 = ON
#define LCDC_OBJ_SIZE           (1<<2)      // OBJ SIZE : 0 = 8*8 ; 1 = 8*16
#define LCDC_BG_TILE_MAP        (1<<3)      // BG TILE MAP AREA : 0 = 9800-9BFF ; 1 = 9C00-9FFF
#define LCDC_BG_TILE            (1<<4)      // BG & WINDOW TILE DATA AREA : 0 = 8800-97FF; 1 = 8000-8FFF 
#define LCDC_WINDOW_ENABLE      (1<<5)      // WINDOW ENABLE : 0 = OFF ; 1 = ON
#define LCDC_WINDOW_TILE_MAP    (1<<6)      // WINDOW TILE MAP AREA : 0 = 9800-9BFF ; 1 = 9C00-9FFF
#define LCDC_PPU_ENABLE         (1<<7)      // LCD & PPU ENABLE : 0 = OFF ; 1 = ON

//STAT Register Bits
//Bits 1 and 0 have no been defined yet
#define STAT_LYC_LY              0x04    
#define STAT_MODE_0_INTERRUPT   (1<<3)      
#define STAT_MODE_1_INTERRUPT   (1<<4)      
#define STAT_MODE_2_INTERRUPT   (1<<5)      
#define STAT_LYC_INTERRUPT      (1<<6)      

//Returning the the bits directly from tile attributes
#define TILE_ATTR_PRIORITY(x) ((x) & 0x80)
#define TILE_ATTR_YFLIP(x) ((x) & 0x40)
#define TILE_ATTR_XFLIP(x) ((x) & 0x20)
#define TILE_ATTR_VRAM_BANK(x) ((x) & 0x08)
#define TILE_ATTR_PALETTE(x) ((x) & 0x07)

//Returning the the bits directly from object attributes
#define OBJECT_ATTR_PRIORITY(x) ((x) & 0x80)
#define OBJECT_ATTR_YFLIP(x) ((x) & 0x40)
#define OBJECT_ATTR_XFLIP(x) ((x) & 0x20)
#define OBJECT_ATTR_VRAM_BANK(x) ((x) & 0x08)
#define OBJECT_ATTR_PALETTE(x) ((x) & 0x07)

//Extracting RGB
#define GBC_COLOR_TO_RGB_R(x) ((x & 0x1F) * 0xFF / 0x1F)
#define GBC_COLOR_TO_RGB_G(x) (((x & 0x03E0) >> 5) * 0xFF / 0x1F)
#define GBC_COLOR_TO_RGB_B(x) (((x & 0x7C00) >> 10) * 0xFF / 0x1F)

#define MAX_OBJ_SCANLINE 10
#define MAX_OBJS ((OAM_END - OAM_BEGIN + 1) / 4)

#define OAM_Y_TO_SCREEN(y) ((y) - 16)
#define OAM_X_TO_SCREEN(x) ((x) - 8)

// each tile is 8x8, top-left is 0,0 
#define TILE_PIXEL_COLORID(td, x, y)  \
    ((td)->data[y * 2] & (1 << (7 - x)) ? 1 : 0) + \
    ((td)->data[y * 2 + 1] & (1 << (7 - x)) ? 2 : 0)


typedef struct {
    uint8_t data[16];       //Because tile has 8x8 pixels of 2bits each
} gbc_tile;

typedef struct{
    uint8_t tile_indices[32][32];           //Tile indices for a map 32*32
} gbc_tilemap;

typedef struct{
    uint8_t data[32][32];      //Tile attributes(for CGB mode)
} gbc_tilemap_attributes;

typedef struct{
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attributes;
} gbc_obj;

typedef struct 
{
    uint32_t dots;   /* dots to next graphic update */
    uint8_t vram[VRAM_BANK_SIZE * 2]; /* 2x8KB */
    uint8_t scanline;
    uint8_t mode;

    void *screen_udata;
    void (*screen_update)(void *udata);
    screen_write screen_write;

    gbc_memory_t *mem;
} gbc_graphic_t;


void gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem);
void gbc_graphic_init(gbc_graphic_t *graphic);
void gbc_graphic_cycle(gbc_graphic_t *graphic);
uint8_t* gbc_graphic_get_tile_attr(gbc_graphic_t *graphic, uint8_t type, uint8_t idx);
gbc_tile* gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank);
