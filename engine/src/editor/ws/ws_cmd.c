// FILENAME: ws_cmd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (ws_internal.h)
// Implements visual text editor subsystem components for ws_cmd.
//
// ---- Includes ----

#include "editor/ws_internal.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/memops.h"

#define WS_NUM_BRIGHT_COLORS 6

void ws_insert_char(int c) {
    int len = g_ws_text_buffer[g_ws_cy].length;
    ws_ensure_line_capacity(g_ws_cy, len + 2);
    runtime_memmove(&g_ws_text_buffer[g_ws_cy].text[g_ws_cx + 1], &g_ws_text_buffer[g_ws_cy].text[g_ws_cx], (size_t)(len - g_ws_cx + 1));
    g_ws_text_buffer[g_ws_cy].text[g_ws_cx] = (char)c;
    g_ws_text_buffer[g_ws_cy].length++;
    g_ws_cx++;
}

void ws_insert_newline(void) {
    ws_insert_empty_line(g_ws_cy + 1);
    int remaining_len = g_ws_text_buffer[g_ws_cy].length - g_ws_cx;
    ws_ensure_line_capacity(g_ws_cy + 1, remaining_len + 1);
    runtime_memmove(g_ws_text_buffer[g_ws_cy + 1].text, g_ws_text_buffer[g_ws_cy].text + g_ws_cx, (size_t)(remaining_len + 1));
    g_ws_text_buffer[g_ws_cy + 1].length = remaining_len;
    g_ws_text_buffer[g_ws_cy].text[g_ws_cx] = '\0';
    g_ws_text_buffer[g_ws_cy].length = g_ws_cx;
    g_ws_cy++;
    g_ws_cx = 0;
}

void ws_handle_backspace(void) {
    if (g_ws_cx > 0) {
        int len = g_ws_text_buffer[g_ws_cy].length;
        runtime_memmove(&g_ws_text_buffer[g_ws_cy].text[g_ws_cx - 1], &g_ws_text_buffer[g_ws_cy].text[g_ws_cx], (size_t)(len - g_ws_cx + 1));
        g_ws_text_buffer[g_ws_cy].length--;
        g_ws_cx--;
    } else if (g_ws_cy > 0) {
        int prev_len = g_ws_text_buffer[g_ws_cy - 1].length;
        int cur_len = g_ws_text_buffer[g_ws_cy].length;
        ws_ensure_line_capacity(g_ws_cy - 1, prev_len + cur_len + 1);
        runtime_memmove(&g_ws_text_buffer[g_ws_cy - 1].text[prev_len], g_ws_text_buffer[g_ws_cy].text, (size_t)(cur_len + 1));
        g_ws_text_buffer[g_ws_cy - 1].length += cur_len;
        ws_free_line(g_ws_cy);
        for (int i = g_ws_cy; i < g_ws_num_lines - 1; i++) {
            g_ws_text_buffer[i] = g_ws_text_buffer[i + 1];
        }
        g_ws_num_lines--;
        g_ws_cy--;
        g_ws_cx = prev_len;
    }
}

int ws_main_loop(VMContext *vm, const char *filename) {
    int c;
    int len, visible_rows, rx;
    bool moved_vertically = false;
    HalContext *hal = hal_get();
    
    if (filename && filename[0]) {
        ws_load_file(filename);
    } else {
        g_ws_num_lines = 0;
        ws_insert_empty_line(0);
    }
    
    ws_init_term();
    
    while (g_ws_running) {
        ws_refresh_screen();
        c = ws_get_input();
        if (c == 0 || c == 3 || c == 4) break; // Ctrl+C/D to exit
        if (c == WS_KEY_TIMEOUT) continue;
        moved_vertically = false;
        
        if (g_ws_prefix_k) {
            if (c == 8 || c == 'h' || c == 'H') {         
                g_ws_help_active = !g_ws_help_active;
            } else if (c == 15 || c == 'o' || c == 'O' || c == 'v' || c == 'V') {
                g_ws_color_index = (g_ws_color_index + 1) % WS_NUM_BRIGHT_COLORS;
            } else if (c == 24 || c == 'x' || c == 'X') { // ^K^X
                if (g_ws_current_filename[0]) ws_save_file();
                break;
            } else if (c == 17 || c == 'q' || c == 'Q') { // ^K^Q
                break;
            } else if (c == 18 || c == 'r' || c == 'R') { // ^K^R
                ws_execute_program(vm, 0);
            } else if (c == 4 || c == 'd' || c == 'D') {  // ^K^D
                ws_execute_program(vm, 1);
            } else if (c == 20 || c == 't' || c == 'T') { // ^K^T
                ws_execute_program(vm, 2);
            }
            g_ws_prefix_k = false;
            continue;
        }
        
        if (c == 27) { g_ws_help_active = false; ws_clear_sel(); }
        else if (c == WS_KEY_F10) { g_ws_color_index = (g_ws_color_index + 1) % WS_NUM_BRIGHT_COLORS; }
        else if (c == WS_SHIFT_ARROW_UP) { ws_update_sel_end(g_ws_cy > 0 ? g_ws_cy - 1 : 0, g_ws_cx); g_ws_cy = g_ws_sel_end_r; }
        else if (c == WS_SHIFT_ARROW_DOWN) { ws_update_sel_end(g_ws_cy < g_ws_num_lines - 1 ? g_ws_cy + 1 : g_ws_num_lines - 1, g_ws_cx); g_ws_cy = g_ws_sel_end_r; }
        else if (c == WS_SHIFT_ARROW_LEFT) { 
            ws_update_sel_end(g_ws_cy, g_ws_cx > 0 ? g_ws_cx - 1 : 0); 
            if (g_ws_cx == 0 && g_ws_cy > 0) ws_update_sel_end(g_ws_cy - 1, (int)g_ws_text_buffer[g_ws_cy - 1].length);
            g_ws_cx = g_ws_sel_end_c; g_ws_cy = g_ws_sel_end_r; 
        }
        else if (c == WS_SHIFT_ARROW_RIGHT) { 
            ws_update_sel_end(g_ws_cy, g_ws_cx < (int)g_ws_text_buffer[g_ws_cy].length ? g_ws_cx + 1 : g_ws_cx); 
            if (g_ws_cx == (int)g_ws_text_buffer[g_ws_cy].length && g_ws_cy < g_ws_num_lines - 1) ws_update_sel_end(g_ws_cy + 1, 0);
            g_ws_cx = g_ws_sel_end_c; g_ws_cy = g_ws_sel_end_r; 
        }
        else if (c == WS_KEY_CTRL_INS) {
            char *txt = ws_get_selected_text();
            if (txt) {
                ws_set_clipboard(txt);
                if (hal && hal->mem.free) hal->mem.free(txt);
                ws_clear_sel();
            }
        }
        else if (c == WS_KEY_SHIFT_DEL) {
            char *txt = ws_get_selected_text();
            if (txt) {
                ws_set_clipboard(txt);
                if (hal && hal->mem.free) hal->mem.free(txt);
                ws_delete_selected_text();
            }
        }
        else if (c == WS_KEY_SHIFT_INS) {
            char *txt = ws_get_clipboard();
            if (txt) {
                ws_insert_text_at_cursor(txt);
                if (hal && hal->mem.free) hal->mem.free(txt);
            }
        }
        else if (c == WS_KEY_UP || c == WS_KEY_DOWN || c == WS_KEY_LEFT || c == WS_KEY_RIGHT || c == WS_KEY_HOME || c == WS_KEY_END || c == WS_KEY_PGUP || c == WS_KEY_PGDN) {
            ws_clear_sel();
        }
        else {
            if ((c >= 32 && c <= 126) || c == 8 || c == 127 || c == 13 || c == WS_KEY_DEL) ws_clear_sel();
        }

        switch (c) {
            case 11: // ^K Prefix
                g_ws_prefix_k = true;
                break;
            case 5: // ^E Up
            case WS_KEY_UP:
                if (g_ws_cy > 0) { g_ws_cy--; moved_vertically = true; }
                break;
            case 24: // ^X Down
            case WS_KEY_DOWN:
                if (g_ws_cy < g_ws_num_lines - 1) { g_ws_cy++; moved_vertically = true; }
                break;
            case 19: // ^S Left
            case WS_KEY_LEFT:
                if (g_ws_cx > 0) g_ws_cx--;
                else if (g_ws_cy > 0) { g_ws_cy--; g_ws_cx = (int)g_ws_text_buffer[g_ws_cy].length; }
                break;
            case 4: // ^D Right
            case WS_KEY_RIGHT:
                if (g_ws_cx < (int)g_ws_text_buffer[g_ws_cy].length) g_ws_cx++;
                else if (g_ws_cy < g_ws_num_lines - 1) { g_ws_cy++; g_ws_cx = 0; }
                break;
            case WS_KEY_PGUP:
                g_ws_cy -= (g_ws_screen_rows - (g_ws_help_active ? 5 : 1));
                if (g_ws_cy < 0) g_ws_cy = 0;
                moved_vertically = true;
                break;
            case WS_KEY_PGDN:
                g_ws_cy += (g_ws_screen_rows - (g_ws_help_active ? 5 : 1));
                if (g_ws_cy >= g_ws_num_lines) g_ws_cy = g_ws_num_lines - 1;
                moved_vertically = true;
                break;
            case WS_KEY_HOME:
                g_ws_cx = 0;
                break;
            case WS_KEY_END:
                g_ws_cx = (int)g_ws_text_buffer[g_ws_cy].length;
                break;
            case WS_KEY_CTRL_HOME:
                g_ws_cy = 0; g_ws_cx = 0; moved_vertically = true;
                break;
            case WS_KEY_CTRL_END:
                g_ws_cy = g_ws_num_lines - 1; g_ws_cx = (int)g_ws_text_buffer[g_ws_cy].length; moved_vertically = true;
                break;
            case WS_KEY_DEL:
                {
                    int curl = g_ws_text_buffer[g_ws_cy].length;
                    if (g_ws_cx < curl) {
                        runtime_memmove(&g_ws_text_buffer[g_ws_cy].text[g_ws_cx], &g_ws_text_buffer[g_ws_cy].text[g_ws_cx + 1], (size_t)(curl - g_ws_cx));
                        g_ws_text_buffer[g_ws_cy].length--;
                    } else if (g_ws_cy < g_ws_num_lines - 1) {
                        int nl = g_ws_text_buffer[g_ws_cy + 1].length;
                        ws_ensure_line_capacity(g_ws_cy, curl + nl + 1);
                        runtime_memmove(&g_ws_text_buffer[g_ws_cy].text[curl], g_ws_text_buffer[g_ws_cy + 1].text, (size_t)(nl + 1));
                        g_ws_text_buffer[g_ws_cy].length += nl;
                        ws_free_line(g_ws_cy + 1);
                        for (int i = g_ws_cy + 1; i < g_ws_num_lines - 1; i++) {
                            g_ws_text_buffer[i] = g_ws_text_buffer[i + 1];
                        }
                        g_ws_num_lines--;
                    }
                }
                break;
            case WS_KEY_INS:
            case WS_KEY_F10:
                break;
            case 10:
            case 13: 
                ws_insert_newline();
                break;
            case 8:
            case 127: 
                ws_handle_backspace();
                break;
            default:
                if ((c >= 32 && c <= 126) || c == '\t') {
                    ws_insert_char(c);
                }
                break;
        }
        
        len = (int)g_ws_text_buffer[g_ws_cy].length;
        if (moved_vertically) {
            g_ws_cx = ws_get_physical_x(g_ws_cy, g_ws_target_rx);
        } else {
            if (g_ws_cx > len) g_ws_cx = len;
            g_ws_target_rx = ws_get_render_x(g_ws_cy, g_ws_cx);
        }
        
        visible_rows = g_ws_screen_rows - (g_ws_help_active ? 6 : 1);
        if (g_ws_cy < g_ws_row_off) g_ws_row_off = g_ws_cy;
        if (g_ws_cy >= g_ws_row_off + visible_rows) g_ws_row_off = g_ws_cy - visible_rows + 1;
        
        rx = ws_get_render_x(g_ws_cy, g_ws_cx);
        if (rx < g_ws_col_off) g_ws_col_off = rx;
        if (rx >= g_ws_col_off + g_ws_screen_cols) g_ws_col_off = rx - g_ws_screen_cols + 1;
    }
    
    ws_print("\x1b[0m\x1b[2J\x1b[H"); 
    return 0;
}
