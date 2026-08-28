// FILENAME: ws_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (ws.c, ws_buf.c, ws_cmd.c, ws_render.c, ws_term.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libstandard
// Implements visual text editor subsystem components for ws_internal.
//
// ---- Includes ----

#ifndef EDITOR_WS_INTERNAL_H
#define EDITOR_WS_INTERNAL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/ctype/ctype.h"
#include "platform/platform.h"
#include "types/version.h"
#include "vm/vm.h"

static inline bool hal_file_read_line(HalContext *hal, IoHandle h, char *buf, size_t max_len) {
    if (!hal || !hal->io.file_read || h == IO_HANDLE_INVALID || !buf || max_len == 0) return false;
    size_t pos = 0;
    char c = 0;
    while (pos + 1 < max_len) {
        if (hal->io.file_read(h, &c, 1, 1) != 1) {
            if (pos == 0) return false;
            break;
        }
        buf[pos++] = c;
        if (c == '\n') break;
    }
    buf[pos] = '\0';
    return true;
}

#ifndef STANDALONE_EDITOR
#include "editor/editor.h"
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif

#define WS_TAB_STOP 4

//
// ---- Key Definitions ----

#define WS_KEY_UP          1000
#define WS_KEY_DOWN        1001
#define WS_KEY_LEFT        1002
#define WS_KEY_RIGHT       1003
#define WS_KEY_HOME        1004
#define WS_KEY_END         1005
#define WS_KEY_PGUP        1006
#define WS_KEY_PGDN        1007
#define WS_KEY_INS         1008
#define WS_KEY_DEL         1009
#define WS_KEY_F10         1020
#define WS_KEY_TIMEOUT     1021
#define WS_KEY_CTRL_HOME   1022
#define WS_KEY_CTRL_END    1023

#define WS_SHIFT_ARROW_UP   1100
#define WS_SHIFT_ARROW_DOWN 1101
#define WS_SHIFT_ARROW_LEFT 1102
#define WS_SHIFT_ARROW_RIGHT 1103
#define WS_KEY_CTRL_INS     1104
#define WS_KEY_SHIFT_DEL    1105
#define WS_KEY_SHIFT_INS    1106

#define WS_MAX_RENDER_BUF 1024

//
// ---- Buffer Structures & Shared State ----

typedef struct {
    char *text;
    int length;
    int capacity;
} WsLine;

extern WsLine *g_ws_text_buffer;
extern int g_ws_text_buffer_capacity;
extern int g_ws_num_lines;
extern char g_ws_current_filename[4096];

extern int g_ws_cx, g_ws_cy;
extern int g_ws_target_rx;
extern int g_ws_row_off, g_ws_col_off;
extern int g_ws_screen_rows, g_ws_screen_cols;
extern int g_ws_color_index;
extern bool g_ws_running;
extern bool g_ws_help_active;
extern bool g_ws_prefix_k;

extern bool g_ws_sel_active;
extern int g_ws_sel_start_r, g_ws_sel_start_c;
extern int g_ws_sel_end_r, g_ws_sel_end_c;

extern const char *g_ws_bright_colors[];
#define WS_NUM_BRIGHT_COLORS 6

//
// ---- Terminal Interface ----

void ws_sanitize_ascii(char *str);
void ws_get_terminal_size(void);
void ws_reset_term(void);
void ws_init_term(void);
int ws_get_input(void);
void ws_print(const char *fmt, ...);

//
// ---- Buffer & Execution Interface ----

void ws_ensure_line_capacity(int row, int needed);
void ws_ensure_buffer_capacity(int needed);
void ws_insert_empty_line(int row);
void ws_free_line(int row);
void ws_load_file(const char *filename);
void ws_save_file(void);

void ws_set_clipboard(const char *text);
char *ws_get_clipboard(void);
void ws_update_sel_end(int r, int c);
void ws_clear_sel(void);
void ws_get_sel_bounds(int *r1, int *c1, int *r2, int *c2);
char *ws_get_selected_text(void);
void ws_delete_selected_text(void);
void ws_insert_text_at_cursor(const char *text);
void ws_execute_program(VMContext *vm, int exec_mode);

//
// ---- Rendering Interface ----

int ws_get_render_x(int row, int physical_x);
int ws_get_physical_x(int row, int target_x);
void ws_render_row(int row, char *out_buf);
void ws_format_filename_for_status(char *out_buf, const char *in_filename, int max_len);
void ws_refresh_screen(void);

//
// ---- Editing Commands & Main Loop ----

void ws_insert_char(int c);
void ws_insert_newline(void);
void ws_handle_backspace(void);
int ws_main_loop(VMContext *vm, const char *filename);

#endif // EDITOR_WS_INTERNAL_H
