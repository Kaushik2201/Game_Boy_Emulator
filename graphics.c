#include "graphics.h"
#include "memory.h"
#include "cpu.h"
#include<stddef.h>
#include<string.h>



inline static void* vram_addr_bank(void *udata, uint16_t addr, uint8_t bank)
{
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint16_t real_addr = (bank *VRAM_BANK_SIZE) + (addr - VRAM_START);
    return g->vram + real_addr;
}

inline static void* vram_addr(void *udata, uint16_t addr)
{
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint8_t bank = IO_PORT_READ(g->mem, IO_PORT_VBK) & 0x1;
    return vram_addr_bank(udata, addr, bank); 
}

void gbc_graphic_init(gbc_graphic_t *graphic)
{
    memset(graphic, 0, sizeof(gbc_graphic_t));
}


// Get a tile from VRAM
gbc_tile_t* gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank) 
{
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    // Ensure the bank is valid (0 or 1)
    if (bank > 1) {
        return NULL;  // Invalid bank
    }

    // Check the tile data addressing mode
    if (type == TILE_TYPE_OBJ || (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_TILE)) {
        // Mode 2: Unsigned indexing (0x8000–0x8FFF)
        return (gbc_tile_t*)(vram_addr_bank(graphic, 0x8000, bank) + idx * 16);
    } else {
        // Mode 1: Signed indexing (0x8800–0x97FF)
        return (gbc_tile_t*)vram_addr_bank(graphic, 0x9000 + (int8_t)idx * 16, bank);
    }
}

// Get tilemap attributes for the background or window
gbc_tilemap_attr_t* gbc_graphic_get_tilemap_attr(gbc_graphic_t *graphic, uint8_t type) 
{
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    // Determine the base address of the tilemap attributes in VRAM
    uint16_t attr_addr = 0;
    switch (type) {
        case TILE_TYPE_BG:  // Background tilemap attributes
            attr_addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
            break;
        case TILE_TYPE_WIN:  // Window tilemap attributes
            attr_addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
            break;
        default:
            return NULL;  // Invalid tilemap type
    }

    // Return a pointer to the tilemap attributes in VRAM
    return (gbc_tilemap_attr_t*)vram_addr_bank(graphic, attr_addr, 1);
}


// Get tilemap data for the background or window
gbc_tilemap_t* gbc_graphic_get_tilemap(gbc_graphic_t *graphic, uint8_t type) 
{
    if (graphic == NULL || graphic->mem == NULL) {
        return NULL;  // Null check
    }

    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    // Determine the base address of the tilemap in VRAM
    uint16_t tilemap_addr = 0;
    switch (type) {
        case TILE_TYPE_BG:  // Background tilemap
            tilemap_addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
            break;
        case TILE_TYPE_WIN:  // Window tilemap
            tilemap_addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
            break;
        default:
            return NULL;  // Invalid tilemap type
    }

    // Return a pointer to the tilemap in VRAM
    return (gbc_tilemap_t*)vram_addr_bank(graphic, tilemap_addr, 0);
}

inline static uint16_t gbc_graphic_render_pixel(gbc_graphic_t *graphic, uint16_t scanline, int16_t col, uint8_t *objs_idx, uint8_t objs_count) 
 {
    if (graphic == NULL || graphic->mem == NULL) {
        return 0x7FFF;  // Return white (default) if there's an issue
    }

    uint16_t bg_color = 0x7FFF;  // Default background color (white)
    uint16_t win_color = 0x7FFF; // Default window color (white)
    uint16_t obj_color = 0x7FFF; // Default sprite color (white)
    bool bg_priority = false;    // Background priority flag
    bool obj_priority = false;   // Sprite priority flag
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);

    // Check if the background is enabled
    if (lcdc & LCDC_BG_PRIORITY) {
        // Calculate the background pixel position with scrolling
        uint16_t bg_x = (col + graphic->mem->io_ports[IO_PORT_SCX]);
        uint16_t bg_y = (scanline + graphic->mem->io_ports[IO_PORT_SCY]);

        // Get the background tilemap
        gbc_tilemap_t *bg_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_BG);
        gbc_tilemap_attr_t *bg_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_BG);

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
                x_in_tile = TILE_SIZE - x_in_tile - 1;
            }
            if (TILE_ATTR_YFLIP(tile_attr)) {
                y_in_tile = TILE_SIZE - y_in_tile - 1;
            }

            // Get the color ID from the tile data
            uint8_t color_id = TILE_PIXEL_COLORID(tile, x_in_tile, y_in_tile);

            // If the color ID is not zero (0 is transparent), use the color
            uint8_t palette_idx = TILE_ATTR_PALETTE(tile_attr);
            bg_color = graphic->mem->bg_palette[palette_idx].c[color_id];
            if (TILE_ATTR_PRIORITY(tile_attr))
                bg_priority |= 1;
        }
    }


    // Check if the window is enabled and visible
    if ((lcdc & LCDC_WINDOW_ENABLE) &&
        (col >= graphic->mem->io_ports[IO_PORT_WX] - 7) &&
        (scanline >= graphic->mem->io_ports[IO_PORT_WY])) {
        // Calculate the window pixel position
        uint8_t win_x = col - (graphic->mem->io_ports[IO_PORT_WX] - 7);
        uint8_t win_y = scanline - graphic->mem->io_ports[IO_PORT_WY];

        // Get the window tilemap
        gbc_tilemap_t *win_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_WIN);
        gbc_tilemap_attr_t *win_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_WIN);

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
                x_in_tile = TILE_SIZE - x_in_tile -1;
            }
            if (TILE_ATTR_YFLIP(tile_attr)) {
                y_in_tile = TILE_SIZE - y_in_tile - 1;
            }

            // Get the color ID from the tile data
            uint8_t color_id = TILE_PIXEL_COLORID(tile, x_in_tile, y_in_tile);

            // If the color ID is not zero (0 is transparent), use the color
            uint8_t palette_idx = TILE_ATTR_PALETTE(tile_attr);
            win_color = graphic->mem->bg_palette[palette_idx].c[color_id];
            if (TILE_ATTR_PRIORITY(tile_attr))
                win_color |= 1;
        }
    }

    if(lcdc & LCDC_OBJ_ENABLE){
            // Render sprites (objects)
        for (uint8_t i = 0; i < objs_count; i++) {
            gbc_obj_t *obj = (gbc_obj_t*)(graphic->mem->oam + (objs_idx[i] * 4));
            if (col >= obj->x_pos && col < obj->x_pos + OBJ_WIDTH &&
                scanline >= obj->y_pos && scanline < obj->y_pos + OBJ_HEIGHT) {
                // Calculate the sprite pixel position
                uint8_t obj_x = obj->x_pos;
                uint8_t obj_y = obj->y_pos;
                uint8_t tile_x_offset = col - obj_x;
                uint8_t tile_y_offset = scanline - obj_y;
                uint8_t tile_idx = obj->tile_index;

                // Handle flipping
                if (lcdc & LCDC_OBJ_SIZE) {
                    /* 8x16 */
                    if (scanline >= obj_y + OBJ_HEIGHT) {
                        /* bottom tile */
                        tile_y_offset -= TILE_SIZE;
                        tile_idx = OBJ_ATTR_YFLIP(obj->attributes) ? (tile_idx & 0xFE) : (tile_idx | 0x01);
                    } else {
                        /* top tile */
                        tile_idx = OBJ_ATTR_YFLIP(obj->attributes) ? (tile_idx | 0x01) : (tile_idx & 0xFE);
                    }
                }

                uint8_t attr = obj->attributes;
                // Get tile data
                gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_OBJ, obj->tile_index, OBJECT_ATTR_VRAM_BANK(obj->attributes));


                if (OBJ_ATTR_XFLIP(attr)) {
                    tile_x_offset = TILE_SIZE - tile_x_offset - 1;
                }
    
                if (OBJ_ATTR_YFLIP(attr)) {
                    tile_y_offset = TILE_SIZE - tile_y_offset - 1;
                }

                if (tile != NULL) {
                    // Get the color ID from the tile data
                    uint8_t color_id = TILE_PIXEL_COLORID(tile, obj_x, obj_y);

                    // If the color ID is not zero (0 is transparent), use the color
                    if (color_id) {
                        uint8_t palette_idx = OBJECT_ATTR_PALETTE(obj->attributes);
                        obj_color = graphic->mem->obj_palette[palette_idx].c[color_id];
                        if (OBJ_ATTR_BG_PRIORITY(attr))
                            obj_priority |= 1;
                    }
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

static void gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline)
{
    if (graphic == NULL || graphic->mem == NULL) {
        return;
    }

    int16_t scanline_base = scanline * VISIBLE_HORIZONTAL_PIXELS;
    uint8_t objs = 0;
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint8_t obj_height = lcdc & LCDC_OBJ_SIZE ? OBJ_HEIGHT_2 : OBJ_HEIGHT;

    gbc_obj_t *obj = (gbc_obj_t*)OAM_ADDR(graphic->mem);
    uint8_t objs_idx[MAX_OBJ_SCANLINE];

    int16_t signed_scanline = (int16_t)scanline;

    // Iterate over each column in the scanline
    for (int i = 0; i < MAX_OBJS; i++, obj++) {
        int16_t y = OAM_Y_TO_SCREEN(obj->y_pos);

        if (signed_scanline >= y && signed_scanline < y + obj_height) {
            objs_idx[objs++] = i;
            if (objs == MAX_OBJ_SCANLINE) {
                break;
            }
        }
    }

    // Call the screen update function if it is set
    for (int16_t i = 0; i < VISIBLE_HORIZONTAL_PIXELS; i++) {
        uint16_t color = gbc_graphic_render_pixel(graphic, scanline, i, objs_idx, objs);
        graphic->screen_write(graphic->screen_udata, scanline_base + i, color);
    }
}

void gbc_graphic_cycle(gbc_graphic_t *graphic) 
{
if (graphic->dots--) {
        return;
    }

    uint8_t io_lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);

    if (io_lcdc & LCDC_PPU_ENABLE) {
        uint8_t io_stat = IO_PORT_READ(graphic->mem, IO_PORT_STAT);
        uint8_t scanline = graphic->scanline;

        if (scanline <= VISIBLE_SCANLINES) {
            if (graphic->mode == PPU_MODE_3) {
                /* HORIZONTAL BLANK */
                graphic->dots = PPU_MODE_0_DOTS;
                graphic->mode = PPU_MODE_0;
                if (io_stat & STAT_MODE_0_INTERRUPT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }

            } else if (graphic->mode == PPU_MODE_2) {
                /* DRAWING */
                graphic->dots = PPU_MODE_3_DOTS;
                graphic->mode = PPU_MODE_3;
                gbc_graphic_draw_line(graphic, scanline);
            } else if (graphic->mode == PPU_MODE_0 || graphic->mode == PPU_MODE_1) {
                if (graphic->mode != PPU_MODE_1)
                    scanline++;
                /* OAM SCAN */
                /* The real gameboy scans obj here but we scan then in MODE3, see above */
                graphic->dots = PPU_MODE_2_DOTS;
                graphic->mode = PPU_MODE_2;
                if (io_stat & STAT_MODE_2_INTERRUPT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
        } else {
            /* V-BLANK */
            if (graphic->mode != PPU_MODE_1) {
                if (io_stat & STAT_MODE_1_INTERRUPT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
                REQUEST_INTERRUPT(graphic->mem, INTERRUPT_VBLANK);
                graphic->mode = PPU_MODE_1;
            }

            graphic->dots = PPU_MODE_1_DOTS;
            scanline++;
        }

        if (scanline > TOTAL_SCANLINES)
            scanline = 0;

        io_stat &= ~PPU_MODE_MASK;
        io_stat |= graphic->mode & PPU_MODE_MASK;

        if (graphic->scanline != scanline) {
            graphic->scanline = scanline;
            IO_PORT_WRITE(graphic->mem, IO_PORT_LY, scanline);
            uint8_t lyc = IO_PORT_READ(graphic->mem, IO_PORT_LYC);
            io_stat &= ~STAT_LYC_LY;
            if (lyc == scanline) {
                io_stat |= STAT_LYC_LY;
                if (io_stat & STAT_LYC_INTERRUPT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
        }

        IO_PORT_WRITE(graphic->mem, IO_PORT_STAT, io_stat);
    } else {
        IO_PORT_WRITE(graphic->mem, IO_PORT_LY, 0);
        graphic->dots = DOTS_PER_SCANLINE * TOTAL_SCANLINES;
        LOG_DEBUG("[GRAPHIC] PPU DISABLED\n");
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