#pragma once
#include<stdint.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 144
#define TILE_SIZE 8
#define MAX_SPRITES 40
#define MAX_SPRITES_PER_LINE 10
#define OBJ_WIDTH 8
#define OBJ_HEIGHT 8
#define OBJ_HEIGHT_2 16

#define TOTAL_SCANLINES 154
#define VISIBLE_SCANLINES 144
#define FRAME_RATE 60
#define DOTS_PER_FRAME 70224
#define DOTS_PER_SCANLINES 456

//PPU Modes
#define PPU_MODE_3 3    //DRAWING
#define PPU_MODE_2 2    //OAM_SCAN
#define PPU_MODE_1 1    //VBLANK
#define PPU_MODE_0 0    //HBLANK

//Dots per PPU Mode
#define PPU_MODE_0_DOTS 100
#define PPU_MODE_1_DOTS 456
#define PPU_MODE_2_DOTS 80
#define PPU_MODE_3_DOTS 276

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

typedef struct {
    uint8_t data[16];       //Because tile has 8x8 pixels of 2bits each
} tile;

typedef struct{
    uint8_t priority : 1;
    uint8_t Y_flip : 1;
    uint8_t X_flip : 1;
    uint8_t vram_bank : 1;
    uint8_t palette : 3;
} tileattributes;

typedef struct{
    uint8_t tile_indices[32][32];           //Tile indices for a map 32*32
} tilemap;

typedef struct{
    tileattributes attributes[32][32];      //Tile attributes(for CGB mode)
} tilemap_attributes;

typedef struct{
    uint8_t y_pos;
    uint8_t x_pos;
    uint8_t tile_index;
    uint8_t attributes;
} object;

typedef struct{
    tile tiles[384];
    object objects[40];
    tilemap background_map;
    tilemap window_map; 
} graphics;

//Returning the the bits directly from tile attributes
#define TILE_ATTR_PRIORITY(x)((x) & 0x80)
#define TILE_ATTR_YFLIP(x)((x) & 0x40)
#define TILE_ATTR_XFLIP(x)((x) & 0x20)
#define TILE_ATTR_VRAM_BANK(x)((x) & 0x08)
#define TILE_ATTR_PALETTE(x)((x) & 0x07)

//Returning the the bits directly from object attributes
#define OBJECT_ATTR_PRIORITY(x)((x) & 0x80)
#define OBJECT_ATTR_YFLIP(x)((x) & 0x40)
#define OBJECT_ATTR_XFLIP(x)((x) & 0x20)
#define OBJECT_ATTR_VRAM_BANK(x)((x) & 0x08)
#define OBJECT_ATTR_PALETTE(x)((x) & 0x07)

//Extracting RGB
#define GBC_COLOR_TO_RGB_R(x)(((x>>11) & 0x1F) * 0xFF / 0x1F)   //Red Color
#define GBC_COLOR_TO_RGB_G(x)(((x>>5) & 0x3F) * 0xFF / 0x3F)    //Green color
#define GBC_COLOR_TO_RGB_B(x)(((x)& 0x1F) * 0xFF / 0x1F)        //Blue Color
