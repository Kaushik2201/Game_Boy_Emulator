#include "graphics.h"
#include "memory.h"
#include "cpu.h"
#include<stddef.h>
#include<string.h>


inline static void* vram_addr_bank(void *udata, uint16_t addr, uint8_t bank){
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint16_t real_addr = bank*VRAM_BANK_SIZE + (addr - VRAM_START);
    return g->vram + real_addr;
}

inline static void* vram_addr(void *udata, uint16_t addr){
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint8_t bank =IO_PORT_READ(g->mem, IO_PORT_VBK) & 0x1;
    return vram_addr_bank(g, addr, bank); 
}

void gbc_graphic_init(gbc_graphic_t *graphic){
    if ( graphic == NULL){
        return;
    }

    memset(graphic, 0, sizeof(gbc_graphic_t));

}


// Get a tile from VRAM
gbc_tile_t* gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank) {
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    // Ensure the bank is valid (0 or 1)
    if (bank > 1) {
        return NULL;  // Invalid bank
    }

    // Check the tile data addressing mode
    if (type == TILE_TYPE_OBJ || (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_WINDOW_TILE_DATA)) {
        // Mode 2: Unsigned indexing (0x8000–0x8FFF)
        return (gbc_tile_t*)(vram_addr_bank(graphic, 0x8000, bank) + idx * 16);
    } else {
        // Mode 1: Signed indexing (0x8800–0x97FF)
        return (gbc_tile_t*)vram_addr_bank(graphic, 0x9000 + (int8_t)idx * 16, bank);
    }
}

// Get tilemap attributes for the background or window
gbc_tilemap_attr_t* gbc_graphic_get_tilemap_attr(gbc_graphic_t *graphic, uint8_t type) {
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    // Determine the base address of the tilemap attributes in VRAM
    uint16_t attr_addr;
    switch (type) {
        case TILE_TYPE_BG:  // Background tilemap attributes
            attr_addr = (graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_BG_TILE_MAP) ? 0x9C00 : 0x9800;
            break;
        case TILE_TYPE_WIN:  // Window tilemap attributes
            attr_addr = (graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_WINDOW_TILE_MAP) ? 0x9C00 : 0x9800;
            break;
        default:
            return NULL;  // Invalid tilemap type
    }

    // Return a pointer to the tilemap attributes in VRAM
    return (gbc_tilemap_attr_t*)vram_addr(graphic->mem, attr_addr);
}

// Get tilemap data for the background or window
gbc_tilemap_t* gbc_graphic_get_tilemap(gbc_graphic_t *graphic, uint8_t type) {
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    // Determine the base address of the tilemap in VRAM
    uint16_t tilemap_addr;
    switch (type) {
        case TILE_TYPE_BG:  // Background tilemap
            tilemap_addr = (graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_BG_TILE_MAP) ? 0x9C00 : 0x9800;
            break;
        case TILE_TYPE_WIN:  // Window tilemap
            tilemap_addr = (graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_WINDOW_TILE_MAP) ? 0x9C00 : 0x9800;
            break;
        default:
            return NULL;  // Invalid tilemap type
    }

    // Return a pointer to the tilemap in VRAM
    return (gbc_tilemap_t*)vram_addr(graphic->mem, tilemap_addr);
}

inline static uint16_t gbc_graphic_render_pixel(gbc_graphic_t *graphic, uint16_t scanline, int16_t col, uint8_t *objs_idx, uint8_t objs_count) {
    if (graphic == NULL || graphic->mem == NULL) {
        return 0x7FFF;  // Return white (default) if there's an issue
    }

    uint16_t bg_color = 0x7FFF;  // Default background color (white)
    uint16_t win_color = 0x7FFF; // Default window color (white)
    uint16_t obj_color = 0x7FFF; // Default sprite color (white)
    bool bg_priority = false;    // Background priority flag
    bool obj_priority = false;   // Sprite priority flag

    // Check if the background is enabled
    if (graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_BG_PRIORITY) {
        // Calculate the background pixel position with scrolling
        uint8_t bg_x = (col + graphic->mem->io_ports[IO_PORT_SCX]) % 256;
        uint8_t bg_y = (scanline + graphic->mem->io_ports[IO_PORT_SCY]) % 256;

        // Get the background tilemap
        gbc_tilemap_t *bg_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_BG);
        gbc_tilemap_attr_t *bg_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_BG);

        if (bg_tilemap != NULL && bg_attr != NULL) {
            // Calculate tile position
            uint8_t tile_x = bg_x / TILE_SIZE;
            uint8_t tile_y = bg_y / TILE_SIZE;

            // Get tile index and attributes
            uint8_t tile_idx = bg_tilemap->tile_indices[tile_y][tile_x];
            uint8_t tile_attr = bg_attr->data[tile_y][tile_x];

            // Get tile data
            gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_BG, tile_idx, TILE_ATTR_VRAM_BANK(tile_attr));

            if (tile != NULL) {
                // Calculate the pixel position within the tile
                uint8_t x_in_tile = bg_x % TILE_SIZE;
                uint8_t y_in_tile = bg_y % TILE_SIZE;

                // Handle flipping
                if (TILE_ATTR_XFLIP(tile_attr)) {
                    x_in_tile = TILE_SIZE - 1 - x_in_tile;
                }
                if (TILE_ATTR_YFLIP(tile_attr)) {
                    y_in_tile = TILE_SIZE - 1 - y_in_tile;
                }

                // Get the color ID from the tile data
                uint8_t color_id = TILE_PIXEL_COLORID(tile, x_in_tile, y_in_tile);

                // If the color ID is not zero (0 is transparent), use the color
                if (color_id) {
                    uint8_t palette_idx = TILE_ATTR_PALETTE(tile_attr);
                    bg_color = graphic->mem->bg_palette[palette_idx].c[color_id];
                    bg_priority = TILE_ATTR_PRIORITY(tile_attr);  // Set background priority
                }
            }
        }
    }

    // Check if the window is enabled and visible
    if ((graphic->mem->io_ports[IO_PORT_LCDC] & LCDC_WINDOW_ENABLE) &&
        (col >= graphic->mem->io_ports[IO_PORT_WX] - 7) &&
        (scanline >= graphic->mem->io_ports[IO_PORT_WY])) {
        // Calculate the window pixel position
        uint8_t win_x = col - (graphic->mem->io_ports[IO_PORT_WX] - 7);
        uint8_t win_y = scanline - graphic->mem->io_ports[IO_PORT_WY];

        // Get the window tilemap
        gbc_tilemap_t *win_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_WIN);
        gbc_tilemap_attr_t *win_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_WIN);

        if (win_tilemap != NULL && win_attr != NULL) {
            // Calculate tile position
            uint8_t tile_x = win_x / TILE_SIZE;
            uint8_t tile_y = win_y / TILE_SIZE;

            // Get tile index and attributes
            uint8_t tile_idx = win_tilemap->tile_indices[tile_y][tile_x];
            uint8_t tile_attr = win_attr->data[tile_y][tile_x];

            // Get tile data
            gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_WIN, tile_idx, TILE_ATTR_VRAM_BANK(tile_attr));

            if (tile != NULL) {
                // Calculate the pixel position within the tile
                uint8_t x_in_tile = win_x % TILE_SIZE;
                uint8_t y_in_tile = win_y % TILE_SIZE;

                // Handle flipping
                if (TILE_ATTR_XFLIP(tile_attr)) {
                    x_in_tile = TILE_SIZE - 1 - x_in_tile;
                }
                if (TILE_ATTR_YFLIP(tile_attr)) {
                    y_in_tile = TILE_SIZE - 1 - y_in_tile;
                }

                // Get the color ID from the tile data
                uint8_t color_id = TILE_PIXEL_COLORID(tile, x_in_tile, y_in_tile);

                // If the color ID is not zero (0 is transparent), use the color
                if (color_id) {
                    uint8_t palette_idx = TILE_ATTR_PALETTE(tile_attr);
                    win_color = graphic->mem->bg_palette[palette_idx].c[color_id];
                }
            }
        }
    }

    // Render sprites (objects)
    for (uint8_t i = 0; i < objs_count; i++) {
        gbc_obj_t *obj = (gbc_obj_t*)(graphic->mem->oam + (objs_idx[i] * 4));
        if (col >= obj->x_pos && col < obj->x_pos + OBJ_WIDTH &&
            scanline >= obj->y_pos && scanline < obj->y_pos + OBJ_HEIGHT) {
            // Calculate the sprite pixel position
            uint8_t obj_x = col - obj->x_pos;
            uint8_t obj_y = scanline - obj->y_pos;

            // Handle flipping
            if (OBJECT_ATTR_XFLIP(obj->attributes)) {
                obj_x = OBJ_WIDTH - 1 - obj_x;
            }
            if (OBJECT_ATTR_YFLIP(obj->attributes)) {
                obj_y = OBJ_HEIGHT - 1 - obj_y;
            }

            // Get tile data
            gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_OBJ, obj->tile_index, OBJECT_ATTR_VRAM_BANK(obj->attributes));

            if (tile != NULL) {
                // Get the color ID from the tile data
                uint8_t color_id = TILE_PIXEL_COLORID(tile, obj_x, obj_y);

                // If the color ID is not zero (0 is transparent), use the color
                if (color_id) {
                    uint8_t palette_idx = OBJECT_ATTR_PALETTE(obj->attributes);
                    obj_color = graphic->mem->obj_palette[palette_idx].c[color_id];
                    obj_priority = OBJECT_ATTR_PRIORITY(obj->attributes);  // Set sprite priority
                }
            }
        }
    }

    // Return the final pixel color based on priority
    if (obj_color != 0x7FFF && (!obj_priority || bg_color == 0x7FFF)){
        return obj_color;  // Sprite has priority
    }
    if (win_color != 0x7FFF) {
        return win_color;  // Window has priority
    }
    if (bg_color != 0x7FFF) {
        return bg_color;  // Background has priority
    }
    return 0x7FFF;  // Default to white
}

static void gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline){
    if (graphic == NULL || graphic->mem == NULL) {
        return;
    }

    // Iterate over each column in the scanline
    for (int16_t col = 0; col < VISIBLE_HORIZONTAL_PIXELS; col++) {
        // Render the pixel at the current scanline and column
        uint16_t color = gbc_graphic_render_pixel(graphic, scanline, col, NULL, 0);

        // Write the pixel to the screen
        if (graphic->screen_write != NULL) {
            graphic->screen_write(graphic->screen_udata, scanline * VISIBLE_HORIZONTAL_PIXELS + col, color);
        }
    }

    // Call the screen update function if it is set
    if (graphic->screen_update != NULL) {
        graphic->screen_update(graphic->screen_udata);
    }
}

void gbc_graphic_cycle(gbc_graphic_t *graphic) {
    if (graphic == NULL || graphic->mem == NULL) {
        return; // Null check
    }

    // Increment the dots counter
    graphic->dots++;

    // Check the current mode and handle accordingly
    switch (graphic->mode) {
        case PPU_MODE_2: // OAM Scan
            if (graphic->dots >= PPU_MODE_2_DOTS) {
                graphic->dots = 0;
                graphic->mode = PPU_MODE_3; // Switch to Drawing mode
                
                // Mode 2 STAT Interrupt
                if (graphic->mem->io_ports[IO_PORT_STAT] & STAT_MODE_2_INTERRUPT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
            break;

        case PPU_MODE_3: // Drawing
            if (graphic->dots >= PPU_MODE_3_DOTS) {
                graphic->dots = 0;
                graphic->mode = PPU_MODE_0; // Switch to HBLANK mode
                gbc_graphic_draw_line(graphic, graphic->scanline); // Draw the current scanline
            }
            break;

        case PPU_MODE_0: // HBLANK
            if (graphic->dots >= PPU_MODE_0_DOTS) {
                graphic->dots = 0;
                graphic->scanline++;
                if (graphic->scanline >= VISIBLE_SCANLINES) {
                    graphic->mode = PPU_MODE_1; // Switch to VBLANK mode
                    // Trigger VBLANK interrupt in cpu.h
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_VBLANK);
                    
                    // Mode 1 STAT Interruptin cpu.h
                    if (graphic->mem->io_ports[IO_PORT_STAT] & STAT_MODE_1_INTERRUPT) {
                        REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                    }
                } else {
                    graphic->mode = PPU_MODE_2; // Switch to OAM Scan mode
                }
            }
            break;

        case PPU_MODE_1: // VBLANK
            if (graphic->dots >= PPU_MODE_1_DOTS) {
                graphic->dots = 0;
                graphic->scanline++;
                if (graphic->scanline >= TOTAL_SCANLINES) {
                    graphic->scanline = 0;
                    graphic->mode = PPU_MODE_2; // Switch to OAM Scan mode
                }
            }
            break;
    }
}
static uint8_t vram_read(void *udata, uint16_t addr){
    gbc_graphic_t *g = (gbc_graphic_t *)udata;

    // Ensure the address is within the VRAM range
    if (addr >= VRAM_START && addr <= VRAM_END) {
        return g->vram[addr - VRAM_START];
    }

    // Return 0 if the address is out of range
    return 0;
}

static uint8_t vram_write(void *udata, uint16_t addr, uint8_t data){
    gbc_graphic_t *g = (gbc_graphic_t *)udata;

    // Ensure the address is within the VRAM range
    if (addr >= VRAM_START && addr <= VRAM_END) {
        g->vram[addr - VRAM_START] = data;
    }
    return data;
}

void gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem){
    if(graphic == NULL || mem == NULL){
        return;
    }
     graphic->mem=mem;
}