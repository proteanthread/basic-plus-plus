// FILENAME: ws_buf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (ws_internal.h)
// Implements visual text editor subsystem components for ws_buf.
//
// ---- Includes ----

#include "editor/ws_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

WsLine *g_ws_text_buffer = NULL;
int g_ws_text_buffer_capacity = 0;
int g_ws_num_lines = 0;
char g_ws_current_filename[4096] = "";

int g_ws_cx = 0, g_ws_cy = 0;
int g_ws_target_rx = 0;
int g_ws_row_off = 0, g_ws_col_off = 0;
int g_ws_screen_rows = 24, g_ws_screen_cols = 80;
int g_ws_color_index = 0;
bool g_ws_running = true;
bool g_ws_help_active = false;
bool g_ws_prefix_k = false;

bool g_ws_sel_active = false;
int g_ws_sel_start_r = 0, g_ws_sel_start_c = 0;
int g_ws_sel_end_r = 0, g_ws_sel_end_c = 0;

static char *s_ws_clipboard = NULL;

#ifndef STANDALONE_EDITOR
static bool s_ws_debug_step_mode = false;
static bool s_ws_trace_mode = false;
#endif

const char *g_ws_bright_colors[] = {
    "\x1b[40;97m", // Black bg, Bright White
    "\x1b[40;96m", // Black bg, Bright Cyan
    "\x1b[40;92m", // Black bg, Bright Green
    "\x1b[40;93m", // Black bg, Bright Yellow
    "\x1b[40;95m", // Black bg, Bright Magenta
    "\x1b[40;91m"  // Black bg, Bright Red
};

static void ws_oom(void) {
}

static char *ws_strdup(const char *s) {
    if (!s) return NULL;
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;

    size_t len = runtime_strlen(s);
    char *dup = (char *)hal->mem.alloc(len + 1);
    if (dup) {
        runtime_strcpy(dup, s);
    }
    return dup;
}

void ws_set_clipboard(const char *text) {
    HalContext *hal = hal_get();
    if (s_ws_clipboard && hal && hal->mem.free) {
        hal->mem.free(s_ws_clipboard);
        s_ws_clipboard = NULL;
    }
    s_ws_clipboard = text ? ws_strdup(text) : NULL;
}

char *ws_get_clipboard(void) {
    return s_ws_clipboard ? ws_strdup(s_ws_clipboard) : NULL;
}

void ws_update_sel_end(int r, int c) {
    if (!g_ws_sel_active) {
        g_ws_sel_start_r = g_ws_cy;
        g_ws_sel_start_c = g_ws_cx;
        g_ws_sel_active = true;
    }
    g_ws_sel_end_r = r;
    g_ws_sel_end_c = c;
}

void ws_clear_sel(void) {
    g_ws_sel_active = false;
}

void ws_get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (!r1 || !c1 || !r2 || !c2) return;
    if (g_ws_sel_start_r < g_ws_sel_end_r || (g_ws_sel_start_r == g_ws_sel_end_r && g_ws_sel_start_c <= g_ws_sel_end_c)) {
        *r1 = g_ws_sel_start_r; *c1 = g_ws_sel_start_c;
        *r2 = g_ws_sel_end_r; *c2 = g_ws_sel_end_c;
    } else {
        *r1 = g_ws_sel_end_r; *c1 = g_ws_sel_end_c;
        *r2 = g_ws_sel_start_r; *c2 = g_ws_sel_start_c;
    }
}

char *ws_get_selected_text(void) {
    if (!g_ws_sel_active) return NULL;
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;

    int r1, c1, r2, c2;
    ws_get_sel_bounds(&r1, &c1, &r2, &c2);
    int buf_size = 65536;
    char *buf = (char *)hal->mem.alloc((size_t)buf_size);
    if (!buf) return NULL;
    runtime_memset(buf, 0, (size_t)buf_size);
    int pos = 0;
    for (int r = r1; r <= r2; r++) {
        if (pos >= buf_size - 1) break;
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : (int)g_ws_text_buffer[r].length;
        for (int i = start; i < end; i++) {
            if (pos >= buf_size - 1) break;
            buf[pos++] = g_ws_text_buffer[r].text[i];
        }
        if (r < r2) {
            if (pos >= buf_size - 1) break;
            buf[pos++] = '\n';
        }
    }
    buf[pos] = '\0';
    return buf;
}

void ws_ensure_line_capacity(int row, int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_ws_text_buffer[row].capacity) {
        int new_cap = g_ws_text_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        
        char *new_text = NULL;
        if (hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(g_ws_text_buffer[row].text, (size_t)new_cap);
        } else {
            new_text = (char *)hal->mem.alloc((size_t)new_cap);
            if (new_text && g_ws_text_buffer[row].text) {
                runtime_memcpy(new_text, g_ws_text_buffer[row].text, (size_t)g_ws_text_buffer[row].length);
                new_text[g_ws_text_buffer[row].length] = '\0';
                hal->mem.free(g_ws_text_buffer[row].text);
            }
        }
        if (!new_text) { ws_oom(); return; }
        g_ws_text_buffer[row].text = new_text;
        g_ws_text_buffer[row].capacity = new_cap;
    }
}

void ws_ensure_buffer_capacity(int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_ws_text_buffer_capacity) {
        int new_cap = g_ws_text_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        
        WsLine *new_buf = NULL;
        if (hal->mem.realloc) {
            new_buf = (WsLine *)hal->mem.realloc(g_ws_text_buffer, (size_t)new_cap * sizeof(WsLine));
        } else {
            new_buf = (WsLine *)hal->mem.alloc((size_t)new_cap * sizeof(WsLine));
            if (new_buf && g_ws_text_buffer) {
                runtime_memcpy(new_buf, g_ws_text_buffer, (size_t)g_ws_text_buffer_capacity * sizeof(WsLine));
                hal->mem.free(g_ws_text_buffer);
            }
        }
        if (!new_buf) { ws_oom(); return; }
        g_ws_text_buffer = new_buf;
        g_ws_text_buffer_capacity = new_cap;
    }
}

void ws_insert_empty_line(int row) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    ws_ensure_buffer_capacity(g_ws_num_lines + 1);
    for (int i = g_ws_num_lines; i > row; i--) {
        g_ws_text_buffer[i] = g_ws_text_buffer[i - 1];
    }
    g_ws_text_buffer[row].text = (char *)hal->mem.alloc(128);
    if (!g_ws_text_buffer[row].text) { ws_oom(); return; }
    runtime_memset(g_ws_text_buffer[row].text, 0, 128);
    g_ws_text_buffer[row].length = 0;
    g_ws_text_buffer[row].capacity = 128;
    g_ws_num_lines++;
}

void ws_free_line(int row) {
    HalContext *hal = hal_get();
    if (g_ws_text_buffer[row].text) {
        if (hal && hal->mem.free) {
            hal->mem.free(g_ws_text_buffer[row].text);
        }
        g_ws_text_buffer[row].text = NULL;
    }
}

void ws_delete_selected_text(void) {
    if (!g_ws_sel_active) return;
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    int r1, c1, r2, c2;
    ws_get_sel_bounds(&r1, &c1, &r2, &c2);
    int rem_len = g_ws_text_buffer[r2].length - c2;
    char *rem = (char *)hal->mem.alloc((size_t)rem_len + 1);
    if (!rem) { ws_oom(); return; }
    runtime_strcpy(rem, g_ws_text_buffer[r2].text + c2);
    
    g_ws_text_buffer[r1].text[c1] = '\0';
    g_ws_text_buffer[r1].length = c1;
    ws_ensure_line_capacity(r1, c1 + rem_len + 1);
    runtime_strcat(g_ws_text_buffer[r1].text, rem);
    g_ws_text_buffer[r1].length += rem_len;
    hal->mem.free(rem);
    
    int lines_to_del = r2 - r1;
    if (lines_to_del > 0) {
        for (int i = r1 + 1; i <= r2; i++) ws_free_line(i);
        for (int i = r1 + 1; i < g_ws_num_lines - lines_to_del; i++) {
            g_ws_text_buffer[i] = g_ws_text_buffer[i + lines_to_del];
        }
        g_ws_num_lines -= lines_to_del;
    }
    g_ws_cy = r1; g_ws_cx = c1;
    g_ws_sel_active = false;
}

void ws_insert_text_at_cursor(const char *text) {
    if (g_ws_sel_active) ws_delete_selected_text();
    const char *p = text;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            if (*p == '\r' && *(p+1) == '\n') p++; 
            ws_insert_empty_line(g_ws_cy + 1);
            g_ws_cy++; g_ws_cx = 0; p++;
        } else {
            int len = g_ws_text_buffer[g_ws_cy].length;
            ws_ensure_line_capacity(g_ws_cy, len + 2);
            runtime_memmove(&g_ws_text_buffer[g_ws_cy].text[g_ws_cx + 1], &g_ws_text_buffer[g_ws_cy].text[g_ws_cx], (size_t)(len - g_ws_cx + 1));
            g_ws_text_buffer[g_ws_cy].text[g_ws_cx] = *p;
            g_ws_text_buffer[g_ws_cy].length++;
            g_ws_cx++;
            p++;
        }
    }
}

void ws_load_file(const char *filename) {
    HalContext *hal = hal_get();
    if (!hal) return;

    if (g_ws_text_buffer) {
        for (int i = 0; i < g_ws_num_lines; i++) ws_free_line(i);
    }
    g_ws_num_lines = 0;
    ws_ensure_buffer_capacity(1);

    IoHandle h = hal->io.file_open(filename, "r");
    if (h != IO_HANDLE_INVALID) {
        char line_buf[4096];
        while (hal_file_read_line(hal, h, line_buf, sizeof(line_buf))) {
            ws_sanitize_ascii(line_buf);
            size_t len = runtime_strlen(line_buf);
            while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
                line_buf[len - 1] = '\0';
                len--;
            }
            ws_insert_empty_line(g_ws_num_lines);
            ws_ensure_line_capacity(g_ws_num_lines - 1, (int)len + 1);
            runtime_strcpy(g_ws_text_buffer[g_ws_num_lines - 1].text, line_buf);
            g_ws_text_buffer[g_ws_num_lines - 1].length = (int)len;
        }
        hal->io.file_close(h);
    }
    runtime_strncpy(g_ws_current_filename, filename, sizeof(g_ws_current_filename) - 1);
    g_ws_current_filename[sizeof(g_ws_current_filename) - 1] = '\0';
    if (g_ws_num_lines == 0) ws_insert_empty_line(0);
}

void ws_save_file(void) {
    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_ws_current_filename, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_ws_num_lines; i++) {
            if (g_ws_text_buffer[i].text) {
                hal->io.file_write(h, g_ws_text_buffer[i].text, 1, (size_t)g_ws_text_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
}

#ifndef STANDALONE_EDITOR
static void ws_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    bool should_pause = false;
    if (s_ws_debug_step_mode && runtime_strcmp(event_type, "step") == 0) should_pause = true;
    
    if (should_pause) {
        tui_multiplexer_init();
        ws_init_term();
        char prefix[64];
        runtime_snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < g_ws_num_lines; i++) {
            if (runtime_strncmp(g_ws_text_buffer[i].text, prefix, runtime_strlen(prefix)) == 0) {
                if (g_ws_text_buffer[i].text[runtime_strlen(prefix)] == ' ') {
                    g_ws_cy = i;
                    break;
                }
            }
        }
        g_ws_cx = 0;
        ws_refresh_screen();
        
        int key;
        do {
            key = ws_get_input();
        } while (key != ' ' && key != 13 && key != 27 && key != 10);
        if (key == 13 || key == 10 || key == 27) s_ws_debug_step_mode = false;
        else if (key == ' ') s_ws_debug_step_mode = true;
        ws_reset_term();
        tui_multiplexer_shutdown();
    } else if (s_ws_trace_mode && runtime_strcmp(event_type, "step") == 0) {
        // Trace mode
    }
}
#endif

void ws_execute_program(VMContext *vm, int exec_mode) {
    if (!vm) return;
    HalContext *hal = hal_get();
    if (!hal) return;

    const char *target = g_ws_current_filename[0] ? g_ws_current_filename : "untitled.bas";
    IoHandle h = hal->io.file_open(target, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_ws_num_lines; i++) {
            if (g_ws_text_buffer[i].text) {
                hal->io.file_write(h, g_ws_text_buffer[i].text, 1, (size_t)g_ws_text_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
    ws_reset_term();
    tui_multiplexer_shutdown();
    
    s_ws_debug_step_mode = (exec_mode == 1);
    s_ws_trace_mode = (exec_mode == 2);
    if (exec_mode > 0) {
        vm_set_debug_hook(vm, ws_debug_hook, NULL);
    } else {
        vm_set_debug_hook(vm, NULL, NULL);
    }
    
    vm_load_program_file(vm, target);
    vm_run_program(vm);
    
    tui_multiplexer_init();
    ws_init_term();
}
