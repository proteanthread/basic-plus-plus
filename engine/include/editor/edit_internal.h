// FILENAME: edit_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (edit.c, edit_buf.c, edit_cmd.c, edit_menu.c)
// NEEDED BY: libstandard (edit_render.c, edit_term.c)
// NEEDS: libcore, libengine, libkernel, libplatform, libstandard
// Implements visual text editor subsystem components for edit_internal.
//
// ---- Includes ----

#ifndef EDITOR_EDIT_INTERNAL_H
#define EDITOR_EDIT_INTERNAL_H

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

void edit_delete_selection(void);

#define EDIT_TAB_STOP 4

//
// ---- ANSI DOS EDIT Themes ----

#define COL_BG_BW    "\x1b[0m"
#define COL_MENU     "\x1b[30;47m"
#define COL_MENU_SEL "\x1b[37;40m"
#define COL_STATUS   "\x1b[30;46m"
#define COL_SHADOW   "\x1b[40m"

//
// ---- Key Codes ----

#define EDIT_KEY_ESC        27
#define EDIT_KEY_ENTER      13
#define EDIT_KEY_BACKSPACE  8
#define EDIT_KEY_TAB        9
#define EDIT_KEY_TIMEOUT    99
#define EDIT_ARROW_LEFT     1000
#define EDIT_ARROW_RIGHT    1001
#define EDIT_ARROW_UP       1002
#define EDIT_ARROW_DOWN     1003
#define EDIT_DEL_KEY        1004
#define EDIT_HOME_KEY       1005
#define EDIT_END_KEY        1006
#define EDIT_PAGE_UP        1007
#define EDIT_PAGE_DOWN      1008
#define EDIT_INS_KEY        1009
#define EDIT_KEY_F1         1010
#define EDIT_KEY_F2         1011
#define EDIT_KEY_F3         1012
#define EDIT_KEY_F4         1013
#define EDIT_KEY_F5         1014
#define EDIT_KEY_F8         1015
#define EDIT_KEY_F9         1016
#define EDIT_KEY_F10        1017
#define EDIT_ALT_F          1018
#define EDIT_ALT_E          1019
#define EDIT_ALT_S          1020
#define EDIT_ALT_D          1021
#define EDIT_ALT_R          1022
#define EDIT_ALT_H          1023
#define EDIT_SHIFT_ARROW_UP   1024
#define EDIT_SHIFT_ARROW_DOWN 1025
#define EDIT_SHIFT_ARROW_LEFT 1026
#define EDIT_SHIFT_ARROW_RIGHT 1027
#define EDIT_KEY_CTRL_INS   1028
#define EDIT_KEY_SHIFT_DEL  1029
#define EDIT_KEY_SHIFT_INS  1030
#define EDIT_CTRL_HOME      1031
#define EDIT_CTRL_END       1032

#define EDIT_MAX_RENDER_BUF 2048

//
// ---- Line Buffer Structures & Shared State ----

typedef struct {
    char *text;
    int length;
    int capacity;
    bool breakpoint;
} EditLine;

extern EditLine *g_edit_text_buffer;
extern int g_edit_text_buffer_capacity;
extern int g_edit_num_lines;
extern char g_edit_current_filename[4096];
extern char g_edit_clipboard_line[4096];
extern char g_edit_search_term[4096];

extern int g_edit_cx, g_edit_cy;
extern bool g_edit_sel_active;
extern int g_edit_sel_start_r, g_edit_sel_start_c;
extern int g_edit_sel_end_r, g_edit_sel_end_c;

extern int g_edit_target_rx;
extern int g_edit_row_off, g_edit_col_off;
extern int g_edit_screen_rows, g_edit_screen_cols;
extern bool g_edit_exit_editor;
extern int g_edit_menu_mode; // 0: Editor, 1: Menu Bar, 2: Dropdown
extern int g_edit_menu_col;
extern int g_edit_menu_row;

extern const char *g_edit_bright_colors[];
#define EDIT_NUM_BRIGHT_COLORS 6
extern int g_edit_color_index;
extern bool g_edit_display_gutter;

extern const char *g_edit_menu_names[];
extern int g_edit_menu_x[];
extern const int g_edit_num_menus;
extern const char **g_edit_dropdowns[];
extern const int g_edit_drop_sizes[];

//
// ---- Terminal Interface ----

void edit_sanitize_ascii(char *str);
void edit_reset_term(void);
void edit_init_term(void);
void edit_get_terminal_size(int *rows, int *cols);
int edit_read_key(void);
void edit_print(const char *fmt, ...);
void edit_flush(void);

//
// ---- Buffer & Execution Interface ----

void edit_ensure_line_capacity(int row, int needed);
void edit_ensure_buffer_capacity(int needed);
void edit_insert_empty_line(int row);
void edit_free_line(int row);
void edit_load_file(const char *filename, bool is_initial);
void edit_save_file(void);

void edit_update_sel_end(int r, int c);
void edit_clear_sel(void);
void edit_get_sel_bounds(int *r1, int *c1, int *r2, int *c2);
char *edit_get_selected_text(void);
void edit_delete_selected_text(void);
void edit_insert_text_at_cursor(const char *text);
void edit_delete_current_line(void);
void edit_do_find(void);
void edit_execute_program(VMContext *vm, int step);

//
// ---- Rendering & Dialog Interface ----

int edit_get_render_x(int row, int physical_x);
int edit_get_physical_x(int row, int target_x);
void edit_render_row(int row, char *out_buf);
void edit_format_filename_for_status(char *out_buf, const char *in_filename, int max_len);
void edit_draw_all(void);
int edit_prompt_input(const char *title, char *buf);
void edit_show_message(const char *title, const char *msg);

//
// ---- Menu & Commands Interface ----

void edit_execute_menu(VMContext *vm);
void edit_insert_char(int c);
void edit_insert_newline(void);
void edit_handle_backspace(void);
void edit_handle_delete(void);
int edit_main_loop(VMContext *vm, const char *filename);

#endif // EDITOR_EDIT_INTERNAL_H
