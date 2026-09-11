// FILENAME: vi_buf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (vi_internal.h)
// Implements visual text editor subsystem components for vi_buf.
//
// ---- Includes ----

#include "editor/vi_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

ViLine *g_vi_text_buffer = NULL;
int g_vi_current_lines = 0;
int g_vi_text_buffer_capacity = 0;
char g_vi_current_filename[4096] = "";
char g_vi_cmd_buffer[4096] = "";

int g_vi_cursor_r = 0;
int g_vi_cursor_c = 0;
int g_vi_row_offset = 0;
int g_vi_mode = 0; // 0: Normal, 1: Insert, 2: Command
int g_vi_cmd_len = 0;
bool g_vi_running = true;
int g_vi_screen_rows = 24;
int g_vi_screen_cols = 80;
int g_vi_color_index = 0;

bool g_vi_sel_active = false;
int g_vi_sel_start_r = 0, g_vi_sel_start_c = 0;
int g_vi_sel_end_r = 0, g_vi_sel_end_c = 0;

const char *g_vi_bright_colors[] = {
    "\x1b[97m", // Bright White
    "\x1b[96m", // Bright Cyan
    "\x1b[92m", // Bright Green
    "\x1b[93m", // Bright Yellow
    "\x1b[95m", // Bright Magenta
    "\x1b[91m"  // Bright Red
};

#ifndef STANDALONE_EDITOR
static bool s_vi_debug_step_mode = false;
static bool s_vi_trace_mode = false;
#endif

static void vi_oom(void) {
}

void vi_ensure_line_capacity(int row, int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_vi_text_buffer[row].capacity) {
        int new_cap = g_vi_text_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        
        char *new_text = NULL;
        if (hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(g_vi_text_buffer[row].text, (size_t)new_cap);
        } else {
            new_text = (char *)hal->mem.alloc((size_t)new_cap);
            if (new_text && g_vi_text_buffer[row].text) {
                runtime_memcpy(new_text, g_vi_text_buffer[row].text, (size_t)g_vi_text_buffer[row].length);
                new_text[g_vi_text_buffer[row].length] = '\0';
                hal->mem.free(g_vi_text_buffer[row].text);
            }
        }
        if (!new_text) { vi_oom(); return; }
        g_vi_text_buffer[row].text = new_text;
        g_vi_text_buffer[row].capacity = new_cap;
    }
}

void vi_ensure_buffer_capacity(int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_vi_text_buffer_capacity) {
        int new_cap = g_vi_text_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        
        ViLine *new_buf = NULL;
        if (hal->mem.realloc) {
            new_buf = (ViLine *)hal->mem.realloc(g_vi_text_buffer, (size_t)new_cap * sizeof(ViLine));
        } else {
            new_buf = (ViLine *)hal->mem.alloc((size_t)new_cap * sizeof(ViLine));
            if (new_buf && g_vi_text_buffer) {
                runtime_memcpy(new_buf, g_vi_text_buffer, (size_t)g_vi_text_buffer_capacity * sizeof(ViLine));
                hal->mem.free(g_vi_text_buffer);
            }
        }
        if (!new_buf) { vi_oom(); return; }
        g_vi_text_buffer = new_buf;
        g_vi_text_buffer_capacity = new_cap;
    }
}

void vi_insert_empty_line(int row) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    vi_ensure_buffer_capacity(g_vi_current_lines + 1);
    for (int i = g_vi_current_lines; i > row; i--) {
        g_vi_text_buffer[i] = g_vi_text_buffer[i - 1];
    }
    g_vi_text_buffer[row].text = (char *)hal->mem.alloc(128);
    if (!g_vi_text_buffer[row].text) { vi_oom(); return; }
    runtime_memset(g_vi_text_buffer[row].text, 0, 128);
    g_vi_text_buffer[row].length = 0;
    g_vi_text_buffer[row].capacity = 128;
    g_vi_current_lines++;
}

void vi_free_line(int row) {
    HalContext *hal = hal_get();
    if (g_vi_text_buffer[row].text) {
        if (hal && hal->mem.free) {
            hal->mem.free(g_vi_text_buffer[row].text);
        }
        g_vi_text_buffer[row].text = NULL;
    }
}

void vi_load_file(const char *filename) {
    HalContext *hal = hal_get();
    if (!hal) return;

    if (g_vi_text_buffer) {
        for (int i = 0; i < g_vi_current_lines; i++) vi_free_line(i);
    }
    g_vi_current_lines = 0;
    vi_ensure_buffer_capacity(1);

    IoHandle h = hal->io.file_open(filename, "r");
    if (h != IO_HANDLE_INVALID) {
        char line_buf[4096];
        while (hal_file_read_line(hal, h, line_buf, sizeof(line_buf))) {
            vi_sanitize_ascii(line_buf);
            size_t len = runtime_strlen(line_buf);
            while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
                line_buf[len - 1] = '\0';
                len--;
            }
            vi_insert_empty_line(g_vi_current_lines);
            vi_ensure_line_capacity(g_vi_current_lines - 1, (int)len + 1);
            runtime_strcpy(g_vi_text_buffer[g_vi_current_lines - 1].text, line_buf);
            g_vi_text_buffer[g_vi_current_lines - 1].length = (int)len;
        }
        hal->io.file_close(h);
    }
    if (g_vi_current_lines == 0) vi_insert_empty_line(0);
    runtime_strncpy(g_vi_current_filename, filename, sizeof(g_vi_current_filename) - 1);
    g_vi_current_filename[sizeof(g_vi_current_filename) - 1] = '\0';
}

void vi_save_file(void) {
    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_vi_current_filename, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_vi_current_lines; i++) {
            if (g_vi_text_buffer[i].text) {
                hal->io.file_write(h, g_vi_text_buffer[i].text, 1, (size_t)g_vi_text_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
}

void vi_fix_cursor(void) {
    int len;
    if (g_vi_cursor_r < 0) g_vi_cursor_r = 0;
    if (g_vi_cursor_r >= g_vi_current_lines) g_vi_cursor_r = g_vi_current_lines - 1;

    len = g_vi_text_buffer[g_vi_cursor_r].length;
    if (g_vi_mode == 0) {
        if (g_vi_cursor_c >= len && len > 0) g_vi_cursor_c = len - 1;
    } else {
        if (g_vi_cursor_c > len) g_vi_cursor_c = len;
    }
    if (g_vi_cursor_c < 0) g_vi_cursor_c = 0;

    if (g_vi_cursor_r < g_vi_row_offset) g_vi_row_offset = g_vi_cursor_r;
    if (g_vi_cursor_r >= g_vi_row_offset + g_vi_screen_rows - 1) {
        g_vi_row_offset = g_vi_cursor_r - (g_vi_screen_rows - 2);
    }
}

#ifndef STANDALONE_EDITOR
static void vi_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    bool should_pause = false;
    if (s_vi_debug_step_mode && runtime_strcmp(event_type, "step") == 0) should_pause = true;
    
    if (should_pause) {
        tui_multiplexer_init();
        vi_init_term();
        char prefix[64];
        runtime_snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < g_vi_current_lines; i++) {
            if (runtime_strncmp(g_vi_text_buffer[i].text, prefix, runtime_strlen(prefix)) == 0) {
                if (g_vi_text_buffer[i].text[runtime_strlen(prefix)] == ' ') {
                    g_vi_cursor_r = i;
                    break;
                }
            }
        }
        vi_fix_cursor();
        vi_render_screen();
        
        int key;
        do {
            key = vi_get_input();
        } while (key != VI_KEY_F5 && key != VI_KEY_F8 && key != 27);
        
        if (key == VI_KEY_F5) s_vi_debug_step_mode = false;
        else if (key == VI_KEY_F8) s_vi_debug_step_mode = true;
        vi_reset_term();
        tui_multiplexer_shutdown();
    } else if (s_vi_trace_mode && runtime_strcmp(event_type, "step") == 0) {
        // Trace mode
    }
}
#endif

void vi_execute_program(VMContext *vm, int exec_mode) {
    if (!vm) return;
    HalContext *hal = hal_get();
    if (!hal) return;

    const char *target = g_vi_current_filename[0] ? g_vi_current_filename : "untitled.bas";
    IoHandle h = hal->io.file_open(target, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_vi_current_lines; i++) {
            if (g_vi_text_buffer[i].text) {
                hal->io.file_write(h, g_vi_text_buffer[i].text, 1, (size_t)g_vi_text_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
    vi_reset_term();
    tui_multiplexer_shutdown();
    
    s_vi_debug_step_mode = (exec_mode == 1);
    s_vi_trace_mode = (exec_mode == 2);
    if (exec_mode > 0) {
        vm_set_debug_hook(vm, vi_debug_hook, NULL);
    } else {
        vm_set_debug_hook(vm, NULL, NULL);
    }
    
    vm_load_program_file(vm, target);
    vm_run_program(vm);
    
    tui_multiplexer_init();
    vi_init_term();
}
