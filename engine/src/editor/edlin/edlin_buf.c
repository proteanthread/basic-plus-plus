// FILENAME: edlin_buf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edlin_internal.h)
// Implements visual text editor subsystem components for edlin_buf.
//
// ---- Includes ----

#include "editor/edlin_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

//
// ---- Global State Definitions ----

EdlinLine *g_edlin_buffer = NULL;
int g_edlin_buffer_capacity = 0;
int g_edlin_line_count = 0;
int g_edlin_page_pos = 0;
char g_edlin_filename[4096] = "";
int g_edlin_color_index = 0;

const char *g_edlin_bright_colors[] = {
    "\x1b[97m",
    "\x1b[96m",
    "\x1b[92m",
    "\x1b[93m",
    "\x1b[95m",
    "\x1b[91m"
};
const size_t g_edlin_num_bright_colors = sizeof(g_edlin_bright_colors) / sizeof(g_edlin_bright_colors[0]);

#ifndef STANDALONE_EDITOR
struct VMContext *g_edlin_current_vm = NULL;
bool g_edlin_debug_step = false;
bool g_edlin_trace_mode = false;
#endif

//
// ---- Buffer Allocation & Cleanups ----

void edlin_sanitize_ascii(char *str) {
    if (!str) return;
    char *p = str;
    while (*str) {
        if ((unsigned char)(*str) < 128) {
            *p++ = *str;
        }
        str++;
    }
    *p = '\0';
}

void edlin_oom(void) {
}

void edlin_ensure_line_capacity(int row, int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_edlin_buffer[row].capacity) {
        int new_cap = g_edlin_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        
        char *new_text = NULL;
        if (hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(g_edlin_buffer[row].text, (size_t)new_cap);
        } else {
            new_text = (char *)hal->mem.alloc((size_t)new_cap);
            if (new_text && g_edlin_buffer[row].text) {
                runtime_memcpy(new_text, g_edlin_buffer[row].text, (size_t)g_edlin_buffer[row].length);
                new_text[g_edlin_buffer[row].length] = '\0';
                hal->mem.free(g_edlin_buffer[row].text);
            }
        }
        if (!new_text) { edlin_oom(); return; }
        g_edlin_buffer[row].text = new_text;
        g_edlin_buffer[row].capacity = new_cap;
    }
}

void edlin_ensure_buffer_capacity(int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_edlin_buffer_capacity) {
        int new_cap = g_edlin_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        
        EdlinLine *new_buf = NULL;
        if (hal->mem.realloc) {
            new_buf = (EdlinLine *)hal->mem.realloc(g_edlin_buffer, (size_t)new_cap * sizeof(EdlinLine));
        } else {
            new_buf = (EdlinLine *)hal->mem.alloc((size_t)new_cap * sizeof(EdlinLine));
            if (new_buf && g_edlin_buffer) {
                runtime_memcpy(new_buf, g_edlin_buffer, (size_t)g_edlin_buffer_capacity * sizeof(EdlinLine));
                hal->mem.free(g_edlin_buffer);
            }
        }
        if (!new_buf) { edlin_oom(); return; }
        g_edlin_buffer = new_buf;
        g_edlin_buffer_capacity = new_cap;
    }
}

void edlin_free_line(int row) {
    HalContext *hal = hal_get();
    if (g_edlin_buffer[row].text) {
        if (hal && hal->mem.free) {
            hal->mem.free(g_edlin_buffer[row].text);
        }
        g_edlin_buffer[row].text = NULL;
    }
}

void edlin_insert_empty_line_at(int row) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    edlin_ensure_buffer_capacity(g_edlin_line_count + 1);
    for (int i = g_edlin_line_count; i > row; i--) {
        g_edlin_buffer[i] = g_edlin_buffer[i - 1];
    }
    g_edlin_buffer[row].text = (char *)hal->mem.alloc(128);
    if (!g_edlin_buffer[row].text) { edlin_oom(); return; }
    runtime_memset(g_edlin_buffer[row].text, 0, 128);
    g_edlin_buffer[row].length = 0;
    g_edlin_buffer[row].capacity = 128;
    g_edlin_line_count++;
}

void load_edlin_file(const char *filename) {
    HalContext *hal = hal_get();
    if (!hal) return;

    if (g_edlin_buffer) {
        for (int i = 0; i < g_edlin_line_count; i++) edlin_free_line(i);
    }
    g_edlin_line_count = 0;

    IoHandle h = hal->io.file_open(filename, "r");
    if (h != IO_HANDLE_INVALID) {
        char line_buf[4096];
        while (hal_file_read_line(hal, h, line_buf, sizeof(line_buf))) {
            size_t len = runtime_strlen(line_buf);
            if (len > 0 && line_buf[len - 1] == '\n') {
                line_buf[len - 1] = '\0'; len--;
            }
            if (len > 0 && line_buf[len - 1] == '\r') {
                line_buf[len - 1] = '\0'; len--;
            }
            edlin_insert_empty_line_at(g_edlin_line_count);
            edlin_ensure_line_capacity(g_edlin_line_count - 1, (int)len + 1);
            runtime_strcpy(g_edlin_buffer[g_edlin_line_count - 1].text, line_buf);
            g_edlin_buffer[g_edlin_line_count - 1].length = (int)len;
        }
        hal->io.file_close(h);
        edlin_print("End of input file\n");
    } else {
        edlin_print("New file\n");
    }
    runtime_snprintf(g_edlin_filename, sizeof(g_edlin_filename), "%s", filename);
}

void save_edlin_file(void) {
    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_edlin_filename, "w");
    if (h == IO_HANDLE_INVALID) {
        edlin_print("Error: Cannot save file.\n");
        return;
    }
    for (int i = 0; i < g_edlin_line_count; i++) {
        if (g_edlin_buffer[i].text) {
            hal->io.file_write(h, g_edlin_buffer[i].text, 1, (size_t)g_edlin_buffer[i].length);
        }
        hal->io.file_write(h, "\n", 1, 1);
    }
    hal->io.file_close(h);
}

