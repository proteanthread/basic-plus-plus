// FILENAME: vcon.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel
// NEEDS: libkernel (types.h)
// Implements virtual device and graphics rendering logic for vcon.
//
// ---- Includes ----

#ifndef DEVICE_VCON_H
#define DEVICE_VCON_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "types/types.h"

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
void         vcon_clear(VConContext *ctx, int index);
void         vcon_write_char(VConContext *ctx, int index, int c);
void         vcon_locate(VConContext *ctx, int index, int row, int col);
void         vcon_get_cursor(VConContext *ctx, int index, int *row, int *col);
int          vcon_get_char_at(VConContext *ctx, int index, int row, int col);
int          vcon_get_attr_at(VConContext *ctx, int index, int row, int col);
void         vcon_set_color(VConContext *ctx, int index, int fg, int bg);
void         vcon_clear_screen(VConContext *ctx, int index, int mode);
void         vcon_set_view_print(VConContext *ctx, int index, int top, int bottom);
void         vcon_set_width(VConContext *ctx, int index, int cols);
void         vcon_set_key_labels_visible(VConContext *ctx, bool visible);
void         vcon_set_key_label(VConContext *ctx, int key_idx, const char *text);
const char  *vcon_get_key_label(VConContext *ctx, int key_idx);
bool         vcon_get_key_labels_visible(VConContext *ctx);

#endif // DEVICE_VCON_H
