// FILENAME: bgi_text.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (context.c)
// NEEDS: libcore (hal.h, memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libengine (bios.h, bios.c)
// NEEDS: libkernel (bgi_text.h, vcon.h, vcon.c)
// Implements virtual device and graphics rendering logic for bgi_text.
//
// ---- Includes ----

#include "device/bgi_text.h"
#include "bios/bios.h"
#include "device/vcon.h"
#include <stdint.h>
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

// -------------------------------------------------------------------------
// Observer Context
//
// Stores references to the BIOS and VCon subsystems needed by the callback.
// This struct is passed as user_data to the observer callback.
// -------------------------------------------------------------------------
typedef struct {
    BiosContext* bios;
    VConContext*  vcon;
} BgiTextContext;

// Single static instance — only one text-mode observer is active at a time.
static BgiTextContext s_bgi_text_ctx;

// -------------------------------------------------------------------------
// CGA Color Attribute to ANSI Escape Code Mapping
//
// CGA 4-bit color indices (0-15) mapped to ANSI SGR foreground/background
// codes. The CGA palette follows the IRGB model:
// 0=Black, 1=Blue, 2=Green, 3=Cyan, 4=Red, 5=Magenta, 6=Brown, 7=LightGray
// 8=DarkGray, 9=LightBlue, 10=LightGreen, 11=LightCyan, 12=LightRed,
// 13=LightMagenta, 14=Yellow, 15=White
//
// ANSI foreground codes: 30-37 (normal), 90-97 (bright/bold)
// ANSI background codes: 40-47 (normal), 100-107 (bright)
// -------------------------------------------------------------------------
static const int s_fg_ansi_map[16] = {
    30, 34, 32, 36, 31, 35, 33, 37,   // 0-7: normal colors
    90, 94, 92, 96, 91, 95, 93, 97    // 8-15: bright colors
};
static const int s_bg_ansi_map[16] = {
    40, 44, 42, 46, 41, 45, 43, 47,   // 0-7: normal colors
    100, 104, 102, 106, 101, 105, 103, 107 // 8-15: bright colors
};

// -------------------------------------------------------------------------
// render_cga_cell — Render a single CGA text cell to the host terminal
//
// Computes the row, col, character, and attribute from the VRAM offset,
// then emits ANSI escape sequences to:
// 1. Save cursor position
// 2. Move to the target cell
// 3. Set foreground and background colors
// 4. Print the character
// 5. Reset colors
// 6. Restore cursor position
//
// Also updates the VCon internal grid for PEEK consistency.
// -------------------------------------------------------------------------
static void render_cga_cell(BgiTextContext* ctx, uint32_t base_addr, uint32_t offset) {
    // Compute row and column from VRAM offset
    uint32_t cell_index = offset / 2;
    int row = (int)(cell_index / 80);
    int col = (int)(cell_index % 80);

    // Bounds check: CGA text mode is 80x25 = 4000 bytes = 2000 cells
    if (row >= VCON_ROWS || col >= VCON_COLS) return;

    // Read both character and attribute from BIOS backing store
    uint32_t char_addr = base_addr + (cell_index * 2);
    uint32_t attr_addr = char_addr + 1;
    uint8_t ch   = bios_peek(ctx->bios, char_addr);
    uint8_t attr = bios_peek(ctx->bios, attr_addr);

    // Extract CGA foreground (bits 0-3) and background (bits 4-7)
    int fg_idx = attr & 0x0F;
    int bg_idx = (attr >> 4) & 0x0F;

    // Map to ANSI SGR codes
    int fg_ansi = s_fg_ansi_map[fg_idx];
    int bg_ansi = s_bg_ansi_map[bg_idx];

    // Render visible ASCII characters (0x20-0x7E); substitute space for controls
    char display_ch = (ch >= 0x20 && ch <= 0x7E) ? (char)ch : ' ';

    // Update VCon grid for PEEK consistency
    if (ctx->vcon) {
        int active_idx = vcon_get_active_index(ctx->vcon);
        int save_row = 0, save_col = 0;
        vcon_get_cursor(ctx->vcon, active_idx, &save_row, &save_col);
        vcon_locate(ctx->vcon, active_idx, row + 1, col + 1);
        vcon_write_char(ctx->vcon, active_idx, display_ch);
        vcon_locate(ctx->vcon, active_idx, save_row + 1, save_col + 1);
    }

    // Emit ANSI escape sequence to render at the target position.
    char buf[128];
    int n = runtime_snprintf(buf, sizeof(buf), "\033[s\033[%d;%dH\033[%d;%dm%c\033[0m\033[u",
                             row + 1, col + 1, fg_ansi, bg_ansi, display_ch);
    HalContext *hal = hal_get();
    if (n > 0 && hal && hal->io.console_puts) {
        hal->io.console_puts(buf);
    }
    if (hal && hal->io.console_flush) {
        hal->io.console_flush();
    }

}

// -------------------------------------------------------------------------
// vram_write_callback — BIOS VRAM observer callback
//
// Called by bios_poke() whenever a byte is written to a VRAM region.
// Determines whether the write targets CGA or MDA text VRAM and renders
// the affected cell.
// -------------------------------------------------------------------------
static void vram_write_callback(void* user_data, uint32_t phys_addr, uint8_t value) {
    (void)value; // Value is already committed to BIOS backing store
    BgiTextContext* ctx = (BgiTextContext*)user_data;
    if (!ctx || !ctx->bios) return;

    // CGA text VRAM: 0xB8000 - 0xB8F9F (80x25 = 4000 bytes, pages 0-3)
    if (phys_addr >= BIOS_VRAM_CGA_START && phys_addr <= BIOS_VRAM_CGA_END) {
        uint32_t offset = phys_addr - BIOS_VRAM_CGA_START;
        // Only render if within active text page 0 (first 4000 bytes)
        if (offset < 4000) {
            render_cga_cell(ctx, BIOS_VRAM_CGA_START, offset);
        }
        return;
    }

    // MDA monochrome text VRAM: 0xB0000 - 0xB0FFF
    if (phys_addr >= BIOS_VRAM_MDA_START && phys_addr <= BIOS_VRAM_MDA_END) {
        uint32_t offset = phys_addr - BIOS_VRAM_MDA_START;
        if (offset < 4000) {
            render_cga_cell(ctx, BIOS_VRAM_MDA_START, offset);
        }
        return;
    }
}

// -------------------------------------------------------------------------
// Public API
// -------------------------------------------------------------------------

void bgi_text_init(BiosContext* bios, VConContext* vcon) {
    if (!bios) return;

    runtime_memset(&s_bgi_text_ctx, 0, sizeof(s_bgi_text_ctx));
    s_bgi_text_ctx.bios = bios;
    s_bgi_text_ctx.vcon = vcon;

    BiosVRAMObserver observer;
    runtime_memset(&observer, 0, sizeof(observer));
    observer.user_data = &s_bgi_text_ctx;
    observer.on_write  = vram_write_callback;
    observer.on_read   = NULL;

    bios_set_vram_observer(bios, &observer);
}

void bgi_text_shutdown(BiosContext* bios) {
    if (bios) {
        bios_set_vram_observer(bios, NULL);
    }
    runtime_memset(&s_bgi_text_ctx, 0, sizeof(s_bgi_text_ctx));
}
