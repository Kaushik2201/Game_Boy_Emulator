#include "graphics.h"
#include "memory.h"
#include "cpu.h"
#include<stddef.h>
#include<string.h>


inline static void* 
vram_addr_bank(void *udata, uint16_t addr, uint8_t bank)
{
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint16_t real_addr = (bank * VRAM_BANK_SIZE) + (addr - VRAM_START);
    return g->vram + real_addr;
}

inline static void* 
vram_addr(void *udata, uint16_t addr){
    gbc_graphic_t* g = (gbc_graphic_t*)udata;
    uint8_t bank = IO_PORT_READ(g->mem, IO_PORT_VBK) & 0x1;
    return vram_addr_bank(udata, addr, bank); 
}


// Get a tile from VRAM
gbc_tile_t* 
gbc_graphic_get_tile(gbc_graphic_t *graphic, uint8_t type, uint8_t idx, uint8_t bank) {

    if (type == TILE_TYPE_OBJ || (IO_PORT_READ(graphic->mem, IO_PORT_LCDC) & LCDC_BG_WINDOW_TILE_DATA)) {
        // Mode 2: Unsigned indexing (0x8000–0x8FFF)
        return (gbc_tile_t*)(vram_addr_bank(graphic, 0x8000, bank) + idx * 16);
    }

    // Mode 1: Signed indexing (0x8800–0x97FF)
    return (gbc_tile_t*)vram_addr_bank(graphic, 0x9000 + (int8_t)idx * 16, bank);
}

// Get tilemap attributes for the background or window
gbc_tilemap_attr_t* 
gbc_graphic_get_tilemap_attr(gbc_graphic_t *graphic, uint8_t type) 
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t addr = 0;

    if (type == TILE_TYPE_BG) {
        addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
    } else if (type == TILE_TYPE_WIN) {
        addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
    } else {
        LOG_ERROR("Invalid tile type\n");
        assert(0);
    }
    /* BG attr tilemap  is always in bank 1 */
    return (gbc_tilemap_attr_t*)vram_addr_bank(graphic, addr, 1);

}

// Get tilemap data for the background or window
gbc_tilemap_t* 
gbc_graphic_get_tilemap(gbc_graphic_t *graphic, uint8_t type) 
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t addr = 0;

    if (type == TILE_TYPE_BG) {
        addr = lcdc & LCDC_BG_TILE_MAP ? 0x9C00 : 0x9800;
    } else if (type == TILE_TYPE_WIN) {
        addr = lcdc & LCDC_WINDOW_TILE_MAP ? 0x9C00 : 0x9800;
    } else {
        LOG_ERROR("Invalid tile type\n");
        assert(0);
    }

    /* BG tilemap is always in bank 0 */
    return (gbc_tilemap_t*)vram_addr_bank(graphic, addr, 0);
}


inline static uint16_t 
gbc_graphic_render_pixel(gbc_graphic_t *graphic, uint16_t scanline, int16_t col, uint8_t *objs_idx, uint8_t objs_count) 
{
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint16_t bg_color, obj_color;
    uint16_t tile_x, tile_y, x, y, tile_x_offset, tile_y_offset;
    uint8_t attr, bg_color_id;

    gbc_palette_t *palette;
    gbc_tile_t *tile;
    uint8_t bgwin_priority = 0;
    uint8_t lcdc_bit0 = lcdc & LCDC_BG_ENABLE;

    bg_color = obj_color = 0;
    bg_color_id = 0;

    // Check if the background is enabled
    if (lcdc_bit0) {
        uint16_t scroll_x = IO_PORT_READ(graphic->mem, IO_PORT_SCX);
        uint16_t scroll_y = IO_PORT_READ(graphic->mem, IO_PORT_SCY);
        gbc_tilemap_t *bg_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_BG);
        gbc_tilemap_attr_t *bg_tilemap_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_BG);

        x = (scroll_x + col) % TILE_MAP_SIZE;
        y = (scroll_y + scanline) % TILE_MAP_SIZE;

        tile_x = x / TILE_SIZE;
        tile_y = y / TILE_SIZE;

        tile_x_offset = x % TILE_SIZE;
        tile_y_offset = y % TILE_SIZE;

        attr = bg_tilemap_attr->data[tile_y][tile_x];
        tile = gbc_graphic_get_tile(graphic, TILE_TYPE_BG, bg_tilemap->data[tile_y][tile_x],
                TILE_ATTR_VRAM_BANK(attr) ? 1 : 0);

        if (TILE_ATTR_XFLIP(attr)) {
            tile_x_offset = TILE_SIZE - tile_x_offset - 1;
        }
        if (TILE_ATTR_YFLIP(attr)) {
            tile_y_offset = TILE_SIZE - tile_y_offset - 1;
        }

        bg_color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);
        palette = BG_PALETTE_READ(graphic->mem, TILE_ATTR_PALETTE(attr));

        bg_color = palette->c[bg_color_id];

        if (TILE_ATTR_PRIORITY(attr))
            bgwin_priority |= 1;
    }

    // Check if the window is enabled and visible
    if (lcdc & LCDC_WINDOW_ENABLE) {
        // Calculate the window pixel position
        uint8_t win_x = col - (graphic->mem->io_ports[IO_PORT_WX] - 7);
        uint8_t win_y = scanline - graphic->mem->io_ports[IO_PORT_WY];

        // Get the window tilemap
        gbc_tilemap_t *win_tilemap = gbc_graphic_get_tilemap(graphic, TILE_TYPE_WIN);
        gbc_tilemap_attr_t *win_attr = gbc_graphic_get_tilemap_attr(graphic, TILE_TYPE_WIN);

        if (scanline >= win_y && col >= win_x) {
            // Calculate tile position
            x = col - win_x;
            y = scanline - win_y;

            tile_x = win_x / TILE_SIZE;
            tile_y = win_y / TILE_SIZE;

            tile_x_offset = x % TILE_SIZE;
            tile_y_offset = y % TILE_SIZE;

            attr = win_attr->data[tile_y][tile_x];
            tile = gbc_graphic_get_tile(graphic, TILE_TYPE_WIN, win_tilemap->data[tile_y][tile_x],
                    TILE_ATTR_VRAM_BANK(attr) ? 1 : 0);

            if (TILE_ATTR_XFLIP(attr)) {
                tile_x_offset = TILE_SIZE - tile_x_offset - 1;
            }

            if (TILE_ATTR_YFLIP(attr)) {
                tile_y_offset = TILE_SIZE - tile_y_offset - 1;
            }

            bg_color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);
            palette = BG_PALETTE_READ(graphic->mem, TILE_ATTR_PALETTE(attr));

            /* window has higher priority than background */
            bg_color = palette->c[bg_color_id];

            if (TILE_ATTR_PRIORITY(attr))
                bgwin_priority |= 1;
        }
    }

    if (lcdc_bit0 && bg_color_id && bgwin_priority) {
        return bg_color;
    }

    uint8_t obj_found = 0;

    if(lcdc & LCDC_OBJ_ENABLE){
        // Render sprites (objects)
        for (uint8_t i = 0; i < objs_count; i++) {
            gbc_obj_t *obj = (gbc_obj_t*)OAM_ADDR(graphic->mem);
            obj += objs_idx[i];

            int16_t obj_y = OAM_Y_TO_SCREEN(obj->y);
            int16_t obj_x = OAM_X_TO_SCREEN(obj->x);

            if (col < obj_x || col >= obj_x + OBJ_WIDTH) {
                continue;
            }

            uint8_t tile_idx = obj->tile;
            uint8_t tile_x_offset = col - obj_x;
            uint8_t tile_y_offset = scanline - obj_y;

            if (lcdc & LCDC_OBJ_SIZE) {
                /* 8x16 */
                if (scanline >= obj_y + OBJ_HEIGHT) {
                    /* bottom tile */
                    tile_y_offset -= TILE_SIZE;
                    tile_idx = OBJ_ATTR_YFLIP(obj->attr) ? (tile_idx & 0xFE) : (tile_idx | 0x01);
                } else {
                    /* top tile */
                    tile_idx = OBJ_ATTR_YFLIP(obj->attr) ? (tile_idx | 0x01) : (tile_idx & 0xFE);
                }
            }

            uint8_t attr = obj->attr;
            gbc_tile_t *tile = gbc_graphic_get_tile(graphic, TILE_TYPE_OBJ, tile_idx,
                OBJ_ATTR_VRAM_BANK(attr) ? 1 : 0);

            if (OBJ_ATTR_XFLIP(attr)) {
                tile_x_offset = TILE_SIZE - tile_x_offset - 1;
            }

            if (OBJ_ATTR_YFLIP(attr)) {
                tile_y_offset = TILE_SIZE - tile_y_offset - 1;
            }

            assert(tile_x_offset >= 0 && tile_x_offset < TILE_SIZE);

            uint16_t color_id = TILE_PIXEL_COLORID(tile, tile_x_offset, tile_y_offset);

            if (color_id == 0) {
                continue;
            }

            gbc_palette_t *palette = OBJ_PALETTE_READ(graphic->mem, OBJ_ATTR_PALETTE(attr));
            obj_color = palette->c[color_id];
            if (OBJ_ATTR_BG_PRIORITY(attr))
                bgwin_priority |= 1;

            obj_found = 1;
            break;
        }
    }

    if (obj_found && (!bgwin_priority || !bg_color_id || !lcdc_bit0)) {
        return obj_color;
    }

    return bg_color;
}


static void 
gbc_graphic_draw_line(gbc_graphic_t *graphic, uint16_t scanline)
{
    int16_t scanline_base = scanline * VISIBLE_HORIZONTAL_PIXELS;

    /* scan objs */
    uint8_t objs = 0;
    uint8_t lcdc = IO_PORT_READ(graphic->mem, IO_PORT_LCDC);
    uint8_t obj_height = lcdc & LCDC_OBJ_SIZE ? OBJ_HEIGHT_2 : OBJ_HEIGHT;

    gbc_obj_t *obj = (gbc_obj_t*)OAM_ADDR(graphic->mem);
    uint8_t objs_idx[MAX_OBJ_SCANLINE];

    int16_t signed_scanline = (int16_t)scanline;

    for (int i = 0; i < MAX_OBJS; i++, obj++) {
        int16_t y = OAM_Y_TO_SCREEN(obj->y);

        if (signed_scanline >= y && signed_scanline < y + obj_height) {
            objs_idx[objs++] = i;
            if (objs == MAX_OBJ_SCANLINE) {
                break;
            }
        }
    }

    for (int16_t i = 0; i < VISIBLE_HORIZONTAL_PIXELS; i++) {
        uint16_t color = gbc_graphic_render_pixel(graphic, scanline, i, objs_idx, objs);
        graphic->screen_write(graphic->screen_udata, scanline_base + i, color);
    }
}


void 
gbc_graphic_cycle(gbc_graphic_t *graphic) 
{
    if (graphic == NULL || graphic->mem == NULL) {
        return; // Null check
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
                if (io_stat & STAT_MODE_0_INT) {
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
                if (io_stat & STAT_MODE_2_INT) {
                    REQUEST_INTERRUPT(graphic->mem, INTERRUPT_LCD_STAT);
                }
            }
        } else {
            /* V-BLANK */
            if (graphic->mode != PPU_MODE_1) {
                if (io_stat & STAT_MODE_1_INT) {
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
                if (io_stat & STAT_LYC_INT) {
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

static uint8_t
vram_read(void *udata, uint16_t addr)
{
    return *(uint8_t*)vram_addr(udata, addr);

}

static uint8_t 
vram_write(void *udata, uint16_t addr, uint8_t data)
{
    gbc_graphic_t *g = (gbc_graphic_t *)udata;

    // Ensure the address is within the VRAM range
    uint8_t bank = IO_PORT_READ(g->mem, IO_PORT_VBK) & 0x01;
    *(uint8_t*)vram_addr(udata, addr) = data;
    return data;
}

void
gbc_graphic_connect(gbc_graphic_t *graphic, gbc_memory_t *mem)
{
    graphic->mem = mem;

    memory_map_entry_t entry;
    entry.id = VRAM_ID;
    entry.addr_begin = VRAM_BEGIN;
    entry.addr_end = VRAM_END;
    entry.read = vram_read;
    entry.write = vram_write;
    entry.udata = graphic;

    register_memory_map(mem, &entry);
}