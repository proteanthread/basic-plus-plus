// FILENAME: vi_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (vi.c, vi_buf.c, vi_cmd.c, vi_render.c, vi_term.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libstandard
// Implements visual text editor subsystem components for vi_internal.
//
// ---- Includes ----

#ifndef EDITOR_VI_INTERNAL_H
#define EDITOR_VI_INTERNAL_H

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

//
// ---- Extended Key Codes ----

#define VI_KEY_UP          1000
#define VI_KEY_DOWN        1001
#define VI_KEY_LEFT        1002
#define VI_KEY_RIGHT       1003
#define VI_KEY_HOME        1004
#define VI_KEY_END         1005
#define VI_KEY_PGUP        1006
#define VI_KEY_PGDN        1007
#define VI_KEY_INS         1008
#define VI_KEY_DEL         1009
#define VI_SHIFT_ARROW_UP  1100
#define VI_SHIFT_ARROW_DOWN 1101
#define VI_SHIFT_ARROW_LEFT 1102
#define VI_SHIFT_ARROW_RIGHT 1103
#define VI_KEY_CTRL_INS    1104
#define VI_KEY_SHIFT_DEL   1105
#define VI_KEY_SHIFT_INS   1106

#define VI_KEY_F1          1011
#define VI_KEY_F2          1012
#define VI_KEY_F3          1013
#define VI_KEY_F4          1014
#define VI_KEY_F5          1015
#define VI_KEY_F8          1018
#define VI_KEY_F10         1020
#define VI_KEY_TIMEOUT     1021
#define VI_KEY_CTRL_HOME   1022
#define VI_KEY_CTRL_END    1023

//
// ---- Buffer Structures & Shared State ----

typedef struct {
    char *text;
    int length;
    int capacity;
} ViLine;

extern ViLine *g_vi_text_buffer;
extern int g_vi_current_lines;
extern int g_vi_text_buffer_capacity;
extern char g_vi_current_filename[4096];
extern char g_vi_cmd_buffer[4096];

extern int g_vi_cursor_r;
extern int g_vi_cursor_c;
extern int g_vi_row_offset;
extern int g_vi_mode; // 0: Normal, 1: Insert, 2: Command
extern int g_vi_cmd_len;
extern bool g_vi_running;
extern int g_vi_screen_rows;
extern int g_vi_screen_cols;
extern int g_vi_color_index;

extern bool g_vi_sel_active;
extern int g_vi_sel_start_r, g_vi_sel_start_c;
extern int g_vi_sel_end_r, g_vi_sel_end_c;

extern const char *g_vi_bright_colors[];
#define VI_NUM_BRIGHT_COLORS 6

//
// ---- Terminal Interface ----

void vi_sanitize_ascii(char *str);
void vi_get_terminal_size(void);
void vi_reset_term(void);
void vi_init_term(void);
int vi_get_input(void);

//
// ---- Buffer & Execution Interface ----

void vi_ensure_line_capacity(int row, int needed);
void vi_ensure_buffer_capacity(int needed);
void vi_insert_empty_line(int row);
void vi_free_line(int row);
void vi_load_file(const char *filename);
void vi_save_file(void);
void vi_fix_cursor(void);
void vi_execute_program(VMContext *vm, int exec_mode);

//
// ---- Rendering Interface ----

void vi_get_sel_bounds(int *r1, int *c1, int *r2, int *c2);
void vi_format_filename_for_status(char *out_buf, const char *in_filename, int max_len);
void vi_render_screen(void);
void vi_display_help(void);

//
// ---- Command Interface & Main Loop ----

void vi_handle_normal(int c);
void vi_handle_insert(int c);
void vi_handle_command(VMContext *vm, int c);
void vi_exit_editor(void);
int vi_main_loop(VMContext *vm, const char *filename);

#endif // EDITOR_VI_INTERNAL_H
