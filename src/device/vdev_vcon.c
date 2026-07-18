/**
 * @file vdev_vcon.c
 * @brief Virtual Consoles & Virtual Terminals (VCON/VTERM) implementation.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements character formatting, screen scrolling, and basic
 *   ANSI escape parsing on virtual consoles.
 * - Why it exists: Provides independent visual console states for multi-screen displays.
 * - Why it works this way: It buffers chars, handles wraps, and updates virtual cursor grids.
 */

#include "bpp_vcon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct VConContext {
    BppVirtualConsole consoles[VCON_MAX_CONSOLES];
    int               active_index;
};

static void vcon_init_single(BppVirtualConsole *vc) {
    vc->cursor_row = 0;
    vc->cursor_col = 0;
    vc->current_color = 7; /* Standard grey */
    vc->ansi_state = false;
    vc->ansi_len = 0;
    memset(vc->grid, ' ', sizeof(vc->grid));
    memset(vc->attribs, 7, sizeof(vc->attribs));
}

VConContext *g_vcon_context = NULL;

VConContext *vcon_init(void) {
    VConContext *ctx = (VConContext *)malloc(sizeof(VConContext));
    if (!ctx) return NULL;
    ctx->active_index = 0;
    for (int i = 0; i < VCON_MAX_CONSOLES; ++i) {
        vcon_init_single(&ctx->consoles[i]);
    }
    g_vcon_context = ctx;
    return ctx;
}

void vcon_shutdown(VConContext *ctx) {
    if (ctx) {
        if (g_vcon_context == ctx) {
            g_vcon_context = NULL;
        }
        free(ctx);
    }
}

bool vcon_select(VConContext *ctx, int index) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return false;
    ctx->active_index = index;
    return true;
}

int vcon_get_active_index(VConContext *ctx) {
    return ctx ? ctx->active_index : 0;
}

void vcon_clear(VConContext *ctx, int index) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return;
    vcon_init_single(&ctx->consoles[index]);
}

void vcon_get_cursor(VConContext *ctx, int index, int *row, int *col) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return;
    if (row) *row = ctx->consoles[index].cursor_row;
    if (col) *col = ctx->consoles[index].cursor_col;
}

static void vcon_scroll(BppVirtualConsole *vc) {
    /* Shift rows up */
    for (int r = 0; r < VCON_ROWS - 1; ++r) {
        memcpy(vc->grid[r], vc->grid[r + 1], VCON_COLS);
        memcpy(vc->attribs[r], vc->attribs[r + 1], VCON_COLS);
    }
    /* Clear bottom row */
    memset(vc->grid[VCON_ROWS - 1], ' ', VCON_COLS);
    memset(vc->attribs[VCON_ROWS - 1], vc->current_color, VCON_COLS);
    vc->cursor_row = VCON_ROWS - 1;
}

static void parse_ansi_cmd(BppVirtualConsole *vc, char cmd, int *params, int param_count) {
    switch (cmd) {
        case 'J': /* Clear Screen */
            if (param_count > 0 && params[0] == 2) {
                memset(vc->grid, ' ', sizeof(vc->grid));
                memset(vc->attribs, vc->current_color, sizeof(vc->attribs));
                vc->cursor_row = 0;
                vc->cursor_col = 0;
            }
            break;
        case 'H':
        case 'f': /* Cursor position */
            {
                int r = (param_count > 0) ? params[0] - 1 : 0;
                int c = (param_count > 1) ? params[1] - 1 : 0;
                if (r < 0) { r = 0; }
                if (r >= VCON_ROWS) { r = VCON_ROWS - 1; }
                if (c < 0) { c = 0; }
                if (c >= VCON_COLS) { c = VCON_COLS - 1; }
                vc->cursor_row = r;
                vc->cursor_col = c;
            }
            break;
        case 'm':
            for (int i = 0; i < param_count; ++i) {
                int p = params[i];
                if (p == 0) {
                    vc->current_color = 7;
                } else if (p >= 30 && p <= 37) {
                    uint8_t fg = (uint8_t)(p - 30);
                    vc->current_color = (vc->current_color & 0xF0) | fg;
                } else if (p >= 90 && p <= 97) {
                    uint8_t fg = (uint8_t)(p - 90 + 8);
                    vc->current_color = (vc->current_color & 0xF0) | fg;
                } else if (p >= 40 && p <= 47) {
                    uint8_t bg = (uint8_t)(p - 40);
                    vc->current_color = (vc->current_color & 0x0F) | (bg << 4);
                } else if (p >= 100 && p <= 107) {
                    uint8_t bg = (uint8_t)(p - 100 + 8);
                    vc->current_color = (vc->current_color & 0x0F) | (bg << 4);
                }
            }
            break;
        default:
            break;
    }
}

static void handle_ansi(BppVirtualConsole *vc, char c) {
    if (vc->ansi_len < (int)sizeof(vc->ansi_buf) - 1) {
        vc->ansi_buf[vc->ansi_len++] = c;
        vc->ansi_buf[vc->ansi_len] = '\0';
    }

    if (isalpha((unsigned char)c)) {
        /* End of sequence. Parse parameters */
        vc->ansi_state = false;
        int params[8];
        int param_count = 0;
        char *p = vc->ansi_buf;
        if (*p == '[') p++; /* skip bracket */

        while (*p && param_count < 8) {
            if (isdigit((unsigned char)*p)) {
                params[param_count++] = (int)strtol(p, NULL, 10);
                while (*p && isdigit((unsigned char)*p)) p++;
            } else {
                p++;
            }
        }
        parse_ansi_cmd(vc, c, params, param_count);
    }
}

void vcon_write_char(VConContext *ctx, int index, int c) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return;
    BppVirtualConsole *vc = &ctx->consoles[index];

    if (vc->ansi_state) {
        handle_ansi(vc, (char)c);
        return;
    }

    if (c == 27) { /* ESC */
        vc->ansi_state = true;
        vc->ansi_len = 0;
        vc->ansi_buf[0] = '\0';
        return;
    }

    if (c == '\n') {
        vc->cursor_col = 0;
        vc->cursor_row++;
        if (vc->cursor_row >= VCON_ROWS) {
            vcon_scroll(vc);
        }
        return;
    }

    if (c == '\r') {
        vc->cursor_col = 0;
        return;
    }

    if (c == '\b') {
        if (vc->cursor_col > 0) {
            vc->cursor_col--;
        }
        return;
    }

    /* Print character */
    vc->grid[vc->cursor_row][vc->cursor_col] = (char)c;
    vc->attribs[vc->cursor_row][vc->cursor_col] = vc->current_color;
    vc->cursor_col++;

    if (vc->cursor_col >= VCON_COLS) {
        vc->cursor_col = 0;
        vc->cursor_row++;
        if (vc->cursor_row >= VCON_ROWS) {
            vcon_scroll(vc);
        }
    }
}

void vcon_write_string(VConContext *ctx, int index, const char *s) {
    if (s) {
        while (*s) {
            vcon_write_char(ctx, index, (unsigned char)*s++);
        }
    }
}

int vcon_get_char_at(VConContext *ctx, int index, int row, int col) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return 32;
    if (row < 0 || row >= VCON_ROWS || col < 0 || col >= VCON_COLS) return 32;
    return (unsigned char)ctx->consoles[index].grid[row][col];
}

int vcon_get_attr_at(VConContext *ctx, int index, int row, int col) {
    if (!ctx || index < 0 || index >= VCON_MAX_CONSOLES) return 7;
    if (row < 0 || row >= VCON_ROWS || col < 0 || col >= VCON_COLS) return 7;
    return ctx->consoles[index].attribs[row][col];
}
