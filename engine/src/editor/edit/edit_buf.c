// FILENAME: edit_buf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit_buf.
//
// ---- Includes ----

#include "editor/edit_internal.h"

EditLine *g_edit_text_buffer = NULL;
int g_edit_text_buffer_capacity = 0;
int g_edit_num_lines = 0;
char g_edit_current_filename[4096] = "";
char g_edit_clipboard_line[4096] = "";
char g_edit_search_term[4096] = "";

int g_edit_cx = 0, g_edit_cy = 0;
bool g_edit_sel_active = false;
int g_edit_sel_start_r = 0, g_edit_sel_start_c = 0;
int g_edit_sel_end_r = 0, g_edit_sel_end_c = 0;

int g_edit_target_rx = 0;
int g_edit_row_off = 0, g_edit_col_off = 0;
int g_edit_screen_rows = 24, g_edit_screen_cols = 80;
bool g_edit_exit_editor = false;
int g_edit_menu_mode = 0;
int g_edit_menu_col = 0;
int g_edit_menu_row = 0;

int g_edit_color_index = 0;
bool g_edit_display_gutter = false;

const char *g_edit_bright_colors[] = {
    "\x1b[40;97m", // Black bg, Bright White
    "\x1b[40;96m", // Black bg, Bright Cyan
    "\x1b[40;92m", // Black bg, Bright Green
    "\x1b[40;93m", // Black bg, Bright Yellow
    "\x1b[40;95m", // Black bg, Bright Magenta
    "\x1b[40;91m"  // Black bg, Bright Red
};

const char *g_edit_menu_names[] = { " File ", " Edit ", " Search ", " Display ", " Run ", " Help " };
int g_edit_menu_x[] = { 2, 10, 19, 30, 42, 50 };
const int g_edit_num_menus = 6;

static const char *s_file_menu[] = { " New          ", " Open...      ", " Save         ", " Save As...   ", " Exit         " };
static const char *s_edit_menu[] = { " Cut          ", " Copy         ", " Paste        ", " Clear        " };
static const char *s_search_menu[] = { " Find...      ", " Find Next    " };
static char s_options_menu[2][20] = { " [COLOR MODE]  ", " [LINE GUTTER] " };
static const char *s_options_ptrs[] = { s_options_menu[0], s_options_menu[1] };
static const char *s_run_menu[] = { " Start      F5 ", " Step       F8 ", " Toggle Brk F9 " };
static const char *s_help_menu[] = { " About...     " };

const char **g_edit_dropdowns[] = { s_file_menu, s_edit_menu, s_search_menu, s_options_ptrs, s_run_menu, s_help_menu };
const int g_edit_drop_sizes[] = { 5, 4, 2, 2, 3, 1 };

#ifndef STANDALONE_EDITOR
static bool s_edit_debug_step_mode = false;
#endif

static void edit_oom(void) {
}

void edit_ensure_line_capacity(int row, int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_edit_text_buffer[row].capacity) {
        int new_cap = g_edit_text_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        
        char *new_text = NULL;
        if (hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(g_edit_text_buffer[row].text, (size_t)new_cap);
        } else {
            new_text = (char *)hal->mem.alloc((size_t)new_cap);
            if (new_text && g_edit_text_buffer[row].text) {
                runtime_memcpy(new_text, g_edit_text_buffer[row].text, (size_t)g_edit_text_buffer[row].length);
                new_text[g_edit_text_buffer[row].length] = '\0';
                hal->mem.free(g_edit_text_buffer[row].text);
            }
        }
        if (!new_text) { edit_oom(); return; }
        g_edit_text_buffer[row].text = new_text;
        g_edit_text_buffer[row].capacity = new_cap;
    }
}

void edit_ensure_buffer_capacity(int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > g_edit_text_buffer_capacity) {
        int new_cap = g_edit_text_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        
        EditLine *new_buf = NULL;
        if (hal->mem.realloc) {
            new_buf = (EditLine *)hal->mem.realloc(g_edit_text_buffer, (size_t)new_cap * sizeof(EditLine));
        } else {
            new_buf = (EditLine *)hal->mem.alloc((size_t)new_cap * sizeof(EditLine));
            if (new_buf && g_edit_text_buffer) {
                runtime_memcpy(new_buf, g_edit_text_buffer, (size_t)g_edit_text_buffer_capacity * sizeof(EditLine));
                hal->mem.free(g_edit_text_buffer);
            }
        }
        if (!new_buf) { edit_oom(); return; }
        g_edit_text_buffer = new_buf;
        g_edit_text_buffer_capacity = new_cap;
    }
}

void edit_insert_empty_line(int row) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    edit_ensure_buffer_capacity(g_edit_num_lines + 1);
    for (int i = g_edit_num_lines; i > row; i--) {
        g_edit_text_buffer[i] = g_edit_text_buffer[i - 1];
    }
    g_edit_text_buffer[row].text = (char *)hal->mem.alloc(128);
    if (!g_edit_text_buffer[row].text) { edit_oom(); return; }
    runtime_memset(g_edit_text_buffer[row].text, 0, 128);
    g_edit_text_buffer[row].length = 0;
    g_edit_text_buffer[row].capacity = 128;
    g_edit_text_buffer[row].breakpoint = false;
    g_edit_num_lines++;
}

void edit_free_line(int row) {
    HalContext *hal = hal_get();
    if (g_edit_text_buffer[row].text) {
        if (hal && hal->mem.free) {
            hal->mem.free(g_edit_text_buffer[row].text);
        }
        g_edit_text_buffer[row].text = NULL;
    }
}

void edit_update_sel_end(int r, int c) {
    if (!g_edit_sel_active) {
        g_edit_sel_start_r = g_edit_cy;
        g_edit_sel_start_c = g_edit_cx;
        g_edit_sel_active = true;
    }
    g_edit_sel_end_r = r;
    g_edit_sel_end_c = c;
}

void edit_clear_sel(void) {
    g_edit_sel_active = false;
}

void edit_get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (!r1 || !c1 || !r2 || !c2) return;
    if (g_edit_sel_start_r < g_edit_sel_end_r || (g_edit_sel_start_r == g_edit_sel_end_r && g_edit_sel_start_c <= g_edit_sel_end_c)) {
        *r1 = g_edit_sel_start_r; *c1 = g_edit_sel_start_c;
        *r2 = g_edit_sel_end_r; *c2 = g_edit_sel_end_c;
    } else {
        *r1 = g_edit_sel_end_r; *c1 = g_edit_sel_end_c;
        *r2 = g_edit_sel_start_r; *c2 = g_edit_sel_start_c;
    }
}

char *edit_get_selected_text(void) {
    if (!g_edit_sel_active) return NULL;
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;

    int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    edit_get_sel_bounds(&r1, &c1, &r2, &c2);
    int buf_size = 65536;
    char *buf = (char *)hal->mem.alloc((size_t)buf_size);
    if (!buf) return NULL;
    runtime_memset(buf, 0, (size_t)buf_size);
    int pos = 0;
    for (int r = r1; r <= r2; r++) {
        if (pos >= buf_size - 1) break;
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : (int)g_edit_text_buffer[r].length;
        for (int i = start; i < end; i++) {
            if (pos >= buf_size - 1) break;
            buf[pos++] = g_edit_text_buffer[r].text[i];
        }
        if (r < r2) {
            if (pos >= buf_size - 1) break;
            buf[pos++] = '\n';
        }
    }
    buf[pos] = '\0';
    return buf;
}

void edit_delete_selection(void) {
    if (!g_edit_sel_active) return;
    int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    edit_get_sel_bounds(&r1, &c1, &r2, &c2);

    if (r1 == r2) {
        int len = g_edit_text_buffer[r1].length;
        int count = c2 - c1;
        runtime_memmove(&g_edit_text_buffer[r1].text[c1], &g_edit_text_buffer[r1].text[c2], (size_t)(len - c2 + 1));
        g_edit_text_buffer[r1].length -= count;
        g_edit_cx = c1;
        g_edit_cy = r1;
    } else {
        int tail_len = g_edit_text_buffer[r2].length - c2;
        edit_ensure_line_capacity(r1, c1 + tail_len + 1);
        runtime_memcpy(&g_edit_text_buffer[r1].text[c1], &g_edit_text_buffer[r2].text[c2], (size_t)(tail_len + 1));
        g_edit_text_buffer[r1].length = c1 + tail_len;

        for (int r = r1 + 1; r <= r2; r++) {
            edit_free_line(r);
        }
        int num_removed = r2 - r1;
        for (int r = r1 + 1; r < g_edit_num_lines - num_removed; r++) {
            g_edit_text_buffer[r] = g_edit_text_buffer[r + num_removed];
        }
        g_edit_num_lines -= num_removed;
        g_edit_cx = c1;
        g_edit_cy = r1;
    }
    edit_clear_sel();
    g_edit_target_rx = edit_get_render_x(g_edit_cy, g_edit_cx);
}

void edit_insert_text_at_cursor(const char *text) {
    if (!text) return;
    const char *p = text;
    while (*p) {
        if (*p == '\n') {
            int cur_len = g_edit_text_buffer[g_edit_cy].length;
            int right_len = cur_len - g_edit_cx;

            edit_insert_empty_line(g_edit_cy + 1);
            edit_ensure_line_capacity(g_edit_cy + 1, right_len + 1);

            if (right_len > 0) {
                runtime_memcpy(g_edit_text_buffer[g_edit_cy + 1].text, &g_edit_text_buffer[g_edit_cy].text[g_edit_cx], (size_t)right_len);
            }
            g_edit_text_buffer[g_edit_cy + 1].text[right_len] = '\0';
            g_edit_text_buffer[g_edit_cy + 1].length = right_len;

            g_edit_text_buffer[g_edit_cy].text[g_edit_cx] = '\0';
            g_edit_text_buffer[g_edit_cy].length = g_edit_cx;

            g_edit_cy++;
            g_edit_cx = 0;
            p++;
        } else if (*p == '\r') {
            p++;
        } else {
            int len = g_edit_text_buffer[g_edit_cy].length;
            edit_ensure_line_capacity(g_edit_cy, len + 2);
            runtime_memmove(&g_edit_text_buffer[g_edit_cy].text[g_edit_cx + 1], &g_edit_text_buffer[g_edit_cy].text[g_edit_cx], (size_t)(len - g_edit_cx + 1));
            g_edit_text_buffer[g_edit_cy].text[g_edit_cx] = *p;
            g_edit_text_buffer[g_edit_cy].length++;
            g_edit_cx++;
            p++;
        }
    }
}

void edit_delete_current_line(void) {
    if (g_edit_num_lines > 1) {
        edit_free_line(g_edit_cy);
        for (int i = g_edit_cy; i < g_edit_num_lines - 1; i++) {
            g_edit_text_buffer[i] = g_edit_text_buffer[i + 1];
        }
        g_edit_num_lines--;
        if (g_edit_cy >= g_edit_num_lines) g_edit_cy = g_edit_num_lines - 1;
        if (g_edit_cx > (int)g_edit_text_buffer[g_edit_cy].length) g_edit_cx = g_edit_text_buffer[g_edit_cy].length;
    } else {
        g_edit_text_buffer[0].text[0] = '\0';
        g_edit_text_buffer[0].length = 0;
        g_edit_cx = 0;
    }
}

void edit_load_file(const char *filename, bool is_initial) {
    HalContext *hal = hal_get();
    if (!hal) return;

    if (g_edit_text_buffer) {
        for (int i = 0; i < g_edit_num_lines; i++) edit_free_line(i);
    }
    g_edit_num_lines = 0;
    edit_ensure_buffer_capacity(1);

    IoHandle h = hal->io.file_open(filename, "r");
    if (h == IO_HANDLE_INVALID) {
        if (!is_initial) {
            edit_insert_empty_line(0);
            return;
        } else {
            runtime_strncpy(g_edit_current_filename, filename, sizeof(g_edit_current_filename) - 1);
            g_edit_current_filename[sizeof(g_edit_current_filename) - 1] = '\0';
            edit_insert_empty_line(0);
            g_edit_cx = 0; g_edit_cy = 0; g_edit_row_off = 0; g_edit_col_off = 0;
            return;
        }
    }

    char line_buf[4096];
    while (hal_file_read_line(hal, h, line_buf, sizeof(line_buf))) {
        edit_sanitize_ascii(line_buf);
        size_t len = runtime_strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
            line_buf[len - 1] = '\0';
            len--;
        }
        edit_insert_empty_line(g_edit_num_lines);
        edit_ensure_line_capacity(g_edit_num_lines - 1, (int)len + 1);
        runtime_strcpy(g_edit_text_buffer[g_edit_num_lines - 1].text, line_buf);
        g_edit_text_buffer[g_edit_num_lines - 1].length = (int)len;
    }
    hal->io.file_close(h);

    if (g_edit_num_lines == 0) {
        edit_insert_empty_line(0);
    }
    runtime_strncpy(g_edit_current_filename, filename, sizeof(g_edit_current_filename) - 1);
    g_edit_current_filename[sizeof(g_edit_current_filename) - 1] = '\0';
    g_edit_cx = 0; g_edit_cy = 0; g_edit_row_off = 0; g_edit_col_off = 0;
}

void edit_save_file(void) {
    if (g_edit_current_filename[0] == '\0') return;
    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_edit_current_filename, "w");
    if (h == IO_HANDLE_INVALID) return;

    for (int i = 0; i < g_edit_num_lines; i++) {
        if (g_edit_text_buffer[i].text) {
            hal->io.file_write(h, g_edit_text_buffer[i].text, 1, (size_t)g_edit_text_buffer[i].length);
        }
        hal->io.file_write(h, "\n", 1, 1);
    }
    hal->io.file_close(h);
}

void edit_do_find(void) {
    char *p;
    for (int r = g_edit_cy; r < g_edit_num_lines; r++) {
        int start_x = (r == g_edit_cy) ? g_edit_cx + 1 : 0;
        if (start_x < (int)g_edit_text_buffer[r].length) {
            p = runtime_strstr(g_edit_text_buffer[r].text + start_x, g_edit_search_term);
            if (p) {
                g_edit_cy = r;
                g_edit_cx = (int)(p - g_edit_text_buffer[r].text);
                g_edit_target_rx = edit_get_render_x(g_edit_cy, g_edit_cx);
                return;
            }
        }
    }
    edit_show_message(" Find ", "Search term not found.");
}

#ifndef STANDALONE_EDITOR
static void edit_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    bool should_pause = false;
    if (s_edit_debug_step_mode && runtime_strcmp(event_type, "step") == 0) should_pause = true;
    if (runtime_strcmp(event_type, "breakpoint") == 0) {
        char prefix[64];
        runtime_snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < g_edit_num_lines; i++) {
            if (runtime_strncmp(g_edit_text_buffer[i].text, prefix, runtime_strlen(prefix)) == 0) {
                if (g_edit_text_buffer[i].text[runtime_strlen(prefix)] == ' ' && g_edit_text_buffer[i].breakpoint) {
                    should_pause = true;
                    break;
                }
            }
        }
    }
    if (should_pause) {
        tui_multiplexer_init();
        edit_init_term();
        char prefix[64];
        runtime_snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < g_edit_num_lines; i++) {
            if (runtime_strncmp(g_edit_text_buffer[i].text, prefix, runtime_strlen(prefix)) == 0) {
                if (g_edit_text_buffer[i].text[runtime_strlen(prefix)] == ' ') {
                    g_edit_cy = i;
                    break;
                }
            }
        }
        edit_draw_all();
        edit_flush();
        int key;
        do {
            key = edit_read_key();
        } while (key != EDIT_KEY_F5 && key != EDIT_KEY_F8 && key != EDIT_KEY_ESC);
        if (key == EDIT_KEY_F5) s_edit_debug_step_mode = false;
        else if (key == EDIT_KEY_F8) s_edit_debug_step_mode = true;
        edit_reset_term();
        tui_multiplexer_shutdown();
    }
}
#endif

void edit_execute_program(VMContext *vm, int step) {
    if (!vm) return;
    HalContext *hal = hal_get();
    if (!hal) return;

    const char *target = g_edit_current_filename[0] ? g_edit_current_filename : "untitled.bas";
    IoHandle h = hal->io.file_open(target, "w");
    if (h != IO_HANDLE_INVALID) {
        for (int i = 0; i < g_edit_num_lines; i++) {
            if (g_edit_text_buffer[i].text) {
                hal->io.file_write(h, g_edit_text_buffer[i].text, 1, (size_t)g_edit_text_buffer[i].length);
            }
            hal->io.file_write(h, "\n", 1, 1);
        }
        hal->io.file_close(h);
    }
    edit_reset_term();
    tui_multiplexer_shutdown();
    
    s_edit_debug_step_mode = (step != 0);
    vm_set_debug_hook(vm, edit_debug_hook, NULL);
    vm_load_program_file(vm, target);
    vm_run_program(vm);
    
    tui_multiplexer_init();
    edit_init_term();
}
