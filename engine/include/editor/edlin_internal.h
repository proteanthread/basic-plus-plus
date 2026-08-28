// FILENAME: edlin_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libstandard (edlin.c, edlin_buf.c, edlin_cmd.c, edlin_exec.c)
// NEEDS: libcore (ctype.h, ctype.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (num_parse.h, num_parse.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libengine (vm.h)
// NEEDS: libstandard (editor.h, editor.c, standalone_runner.h)
// Implements visual text editor subsystem components for edlin_internal.
//
// ---- Includes ----

#ifndef EDITOR_EDLIN_INTERNAL_H
#define EDITOR_EDLIN_INTERNAL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "runtime/ctype/ctype.h"
#include "runtime/conv/num_parse.h"


#ifndef STANDALONE_EDITOR
#include "editor/editor.h"
#include "vm/vm.h"
#endif

#ifdef STANDALONE_EDITOR
#include "standalone_runner.h"
#endif

#if defined(_WIN32) || defined(WIN32)
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif

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

//
// ---- Data Structures & Global State Declarations ----

typedef struct {
    char *text;
    int length;
    int capacity;
} EdlinLine;

extern EdlinLine *g_edlin_buffer;
extern int g_edlin_buffer_capacity;
extern int g_edlin_line_count;
extern int g_edlin_page_pos;
extern char g_edlin_filename[4096];
extern int g_edlin_color_index;
extern const char *g_edlin_bright_colors[];
extern const size_t g_edlin_num_bright_colors;

#ifndef STANDALONE_EDITOR
extern struct VMContext *g_edlin_current_vm;
extern bool g_edlin_debug_step;
extern bool g_edlin_trace_mode;
#endif

//
// ---- Internal Functions ----

void edlin_oom(void);
void edlin_ensure_line_capacity(int row, int needed);
void edlin_ensure_buffer_capacity(int needed);
void edlin_free_line(int row);
void edlin_insert_empty_line_at(int row);
void edlin_sanitize_ascii(char *str);
void edlin_print(const char *fmt, ...);
int edlin_get_page_size(void);
char *edlin_read_line(char *buf, size_t max_len);
int edlin_get_int_prompt(const char *prompt);
void edlin_get_string_prompt(const char *prompt, char *buffer);
void load_edlin_file(const char *filename);
void save_edlin_file(void);

void list_edlin_lines(void);
void insert_edlin_line(void);
void delete_edlin_line(void);
void edit_edlin_line(int index);
void copy_edlin_lines(void);
void move_edlin_lines(void);
void page_edlin_display(void);
void search_edlin_text(void);
void replace_edlin_text(void);
void transfer_edlin_file(void);
void write_edlin_lines(void);
void append_edlin_lines(void);

void display_edlin_help(void);
void execute_program_edlin(int exec_mode);

#endif // EDITOR_EDLIN_INTERNAL_H
