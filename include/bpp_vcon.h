/**
 * @file bpp_vcon.h
 * @brief Virtual Consoles & Virtual Terminals (VCON/VTERM) interface.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares text grids, cursor tracking, and ANSI/VT100 parsing
 *   for multiple virtual terminal buffers.
 * - Why it exists: Emulates Unix/Linux-style TTY/PTY multiplexing for visual shells.
 * - Why it works this way: It maintains separate framebuffers in memory and maps
 *   them to the active console viewport.
 */

#ifndef BPP_VCON_H
#define BPP_VCON_H

#include <stdbool.h>
#include <stddef.h>
#include "bpp_types.h"

#define VCON_MAX_CONSOLES 8
#define VCON_ROWS 25
#define VCON_COLS 80

typedef struct {
    char     grid[VCON_ROWS][VCON_COLS];
    uint8_t  attribs[VCON_ROWS][VCON_COLS];
    int      cursor_row;
    int      cursor_col;
    uint8_t  current_color;
    bool     ansi_state;
    char     ansi_buf[32];
    int      ansi_len;
} BppVirtualConsole;

typedef struct VConContext VConContext;

VConContext *vcon_init(void);
void         vcon_shutdown(VConContext *ctx);

bool         vcon_select(VConContext *ctx, int index);
int          vcon_get_active_index(VConContext *ctx);
void         vcon_write_char(VConContext *ctx, int index, int c);
void         vcon_write_string(VConContext *ctx, int index, const char *s);
void         vcon_clear(VConContext *ctx, int index);
void         vcon_get_cursor(VConContext *ctx, int index, int *row, int *col);
int          vcon_get_char_at(VConContext *ctx, int index, int row, int col);
int          vcon_get_attr_at(VConContext *ctx, int index, int row, int col);

#endif /* BPP_VCON_H */
