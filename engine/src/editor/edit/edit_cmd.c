// FILENAME: edit_cmd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit_cmd.
//
// ---- Includes ----

#include "editor/edit_internal.h"

#define EDIT_NUM_BRIGHT_COLORS 6

void edit_insert_char(int c) {
    int len = g_edit_text_buffer[g_edit_cy].length;
    edit_ensure_line_capacity(g_edit_cy, len + 2);
    runtime_memmove(&g_edit_text_buffer[g_edit_cy].text[g_edit_cx + 1], &g_edit_text_buffer[g_edit_cy].text[g_edit_cx], (size_t)(len - g_edit_cx + 1));
    g_edit_text_buffer[g_edit_cy].text[g_edit_cx] = (char)c;
    g_edit_text_buffer[g_edit_cy].length++;
    g_edit_cx++;
}

void edit_insert_newline(void) {
    edit_insert_empty_line(g_edit_cy + 1);
    int remaining_len = g_edit_text_buffer[g_edit_cy].length - g_edit_cx;
    edit_ensure_line_capacity(g_edit_cy + 1, remaining_len + 1);
    runtime_memmove(g_edit_text_buffer[g_edit_cy + 1].text, g_edit_text_buffer[g_edit_cy].text + g_edit_cx, (size_t)(remaining_len + 1));
    g_edit_text_buffer[g_edit_cy + 1].length = remaining_len;
    g_edit_text_buffer[g_edit_cy].text[g_edit_cx] = '\0';
    g_edit_text_buffer[g_edit_cy].length = g_edit_cx;
    g_edit_cy++;
    g_edit_cx = 0;
}

void edit_handle_backspace(void) {
    if (g_edit_cx > 0) {
        int len = g_edit_text_buffer[g_edit_cy].length;
        runtime_memmove(&g_edit_text_buffer[g_edit_cy].text[g_edit_cx - 1], &g_edit_text_buffer[g_edit_cy].text[g_edit_cx], (size_t)(len - g_edit_cx + 1));
        g_edit_text_buffer[g_edit_cy].length--;
        g_edit_cx--;
    } else if (g_edit_cy > 0) {
        int prev_len = g_edit_text_buffer[g_edit_cy - 1].length;
        int cur_len = g_edit_text_buffer[g_edit_cy].length;
        edit_ensure_line_capacity(g_edit_cy - 1, prev_len + cur_len + 1);
        runtime_memmove(&g_edit_text_buffer[g_edit_cy - 1].text[prev_len], g_edit_text_buffer[g_edit_cy].text, (size_t)(cur_len + 1));
        g_edit_text_buffer[g_edit_cy - 1].length += cur_len;
        edit_free_line(g_edit_cy);
        for (int i = g_edit_cy; i < g_edit_num_lines - 1; i++) {
            g_edit_text_buffer[i] = g_edit_text_buffer[i + 1];
        }
        g_edit_num_lines--;
        g_edit_cy--;
        g_edit_cx = prev_len;
    }
}

void edit_handle_delete(void) {
    int cur_len = g_edit_text_buffer[g_edit_cy].length;
    if (g_edit_cx < cur_len) {
        runtime_memmove(&g_edit_text_buffer[g_edit_cy].text[g_edit_cx], &g_edit_text_buffer[g_edit_cy].text[g_edit_cx + 1], (size_t)(cur_len - g_edit_cx));
        g_edit_text_buffer[g_edit_cy].length--;
    } else if (g_edit_cy < g_edit_num_lines - 1) {
        int next_len = g_edit_text_buffer[g_edit_cy + 1].length;
        edit_ensure_line_capacity(g_edit_cy, cur_len + next_len + 1);
        runtime_memmove(&g_edit_text_buffer[g_edit_cy].text[cur_len], g_edit_text_buffer[g_edit_cy + 1].text, (size_t)(next_len + 1));
        g_edit_text_buffer[g_edit_cy].length += next_len;
        edit_free_line(g_edit_cy + 1);
        for (int i = g_edit_cy + 1; i < g_edit_num_lines - 1; i++) {
            g_edit_text_buffer[i] = g_edit_text_buffer[i + 1];
        }
        g_edit_num_lines--;
    }
}

int edit_main_loop(VMContext *vm, const char *filename) {
    int c, rx, visible_rows, visible_cols, max_rows;
    bool moved_vertically = false;
    HalContext *hal = hal_get();
    
    if (filename && filename[0]) edit_load_file(filename, true);
    else { g_edit_num_lines = 0; edit_insert_empty_line(0); g_edit_current_filename[0] = '\0'; }

    edit_init_term();
    
    while (!g_edit_exit_editor) {
        edit_get_terminal_size(&g_edit_screen_rows, &g_edit_screen_cols);
        
        visible_rows = g_edit_screen_rows - 3; 
        visible_cols = g_edit_screen_cols - 1 - (g_edit_display_gutter ? 7 : 0);
        
        if (g_edit_cy < g_edit_row_off) g_edit_row_off = g_edit_cy;
        if (g_edit_cy >= g_edit_row_off + visible_rows) g_edit_row_off = g_edit_cy - visible_rows + 1;
        rx = edit_get_render_x(g_edit_cy, g_edit_cx);
        if (rx < g_edit_col_off) g_edit_col_off = rx;
        if (rx >= g_edit_col_off + visible_cols) g_edit_col_off = rx - visible_cols + 1;
        
        edit_draw_all();
        
        if (g_edit_menu_mode == 0) {
            rx = edit_get_render_x(g_edit_cy, g_edit_cx);
            edit_print("\x1b[%d;%dH\x1b[?25h", (g_edit_cy - g_edit_row_off) + 3, (rx - g_edit_col_off) + 1 + (g_edit_display_gutter ? 7 : 0));
        } else {
            edit_print("\x1b[?25l");
        }
        
        edit_flush();
        c = edit_read_key();
        moved_vertically = false;
        
        if (c == EDIT_KEY_TIMEOUT) continue;
        if (c == 3 || c == 4) { g_edit_exit_editor = true; continue; } // Ctrl+C/D to exit
        if (c == EDIT_KEY_ESC) {
            if (g_edit_menu_mode == 0) { g_edit_menu_mode = 1; g_edit_menu_col = 0; }
            else if (g_edit_menu_mode == 2) { g_edit_menu_mode = 1; }
            else { g_edit_menu_mode = 0; }
            continue;
        }

        if (c == EDIT_KEY_F10) { g_edit_color_index = (g_edit_color_index + 1) % EDIT_NUM_BRIGHT_COLORS; continue; }
        if (c == EDIT_KEY_F1) { edit_show_message(" Help ", "Standalone C17 Editor"); continue; }
        if (c == EDIT_KEY_F2) {
            if (g_edit_current_filename[0] == '\0') {
                char buf[4096]; buf[0] = '\0';
                if (edit_prompt_input(" Save As ", buf) && buf[0] != '\0') {
                    runtime_strncpy(g_edit_current_filename, buf, sizeof(g_edit_current_filename) - 1);
                    g_edit_current_filename[sizeof(g_edit_current_filename) - 1] = '\0';
                    edit_save_file();
                }
            } else edit_save_file();
            continue;
        }
        if (c == EDIT_KEY_F3) {
            char buf[4096]; buf[0] = '\0';
            if (edit_prompt_input(" Open ", buf) && buf[0] != '\0') edit_load_file(buf, false);
            continue;
        }
        if (c == EDIT_KEY_F4) { g_edit_exit_editor = true; continue; }
        if (c == EDIT_KEY_F5) { edit_execute_program(vm, 0); continue; }
        if (c == EDIT_KEY_F8) { edit_execute_program(vm, 1); continue; }
        if (c == EDIT_KEY_F9) { g_edit_text_buffer[g_edit_cy].breakpoint = !g_edit_text_buffer[g_edit_cy].breakpoint; continue; }
        
        if (c == EDIT_ALT_F) { g_edit_menu_mode = 2; g_edit_menu_col = 0; g_edit_menu_row = 0; continue; }
        if (c == EDIT_ALT_E) { g_edit_menu_mode = 2; g_edit_menu_col = 1; g_edit_menu_row = 0; continue; }
        if (c == EDIT_ALT_S) { g_edit_menu_mode = 2; g_edit_menu_col = 2; g_edit_menu_row = 0; continue; }
        if (c == EDIT_ALT_D) { g_edit_menu_mode = 2; g_edit_menu_col = 3; g_edit_menu_row = 0; continue; }
        if (c == EDIT_ALT_R) { g_edit_menu_mode = 2; g_edit_menu_col = 4; g_edit_menu_row = 0; continue; }
        if (c == EDIT_ALT_H) { g_edit_menu_mode = 2; g_edit_menu_col = 5; g_edit_menu_row = 0; continue; }
        
        if (g_edit_menu_mode == 1) {
            if (c == EDIT_ARROW_LEFT) { g_edit_menu_col = (g_edit_menu_col + g_edit_num_menus - 1) % g_edit_num_menus; }
            else if (c == EDIT_ARROW_RIGHT) { g_edit_menu_col = (g_edit_menu_col + 1) % g_edit_num_menus; }
            else if (c == EDIT_ARROW_DOWN || c == EDIT_KEY_ENTER) { g_edit_menu_mode = 2; g_edit_menu_row = 0; }
            else if (c == EDIT_ARROW_UP) { g_edit_menu_mode = 2; g_edit_menu_row = g_edit_drop_sizes[g_edit_menu_col] - 1; }
        } else if (g_edit_menu_mode == 2) {
            max_rows = g_edit_drop_sizes[g_edit_menu_col];
            if (c == EDIT_ARROW_UP) { g_edit_menu_row = (g_edit_menu_row + max_rows - 1) % max_rows; }
            else if (c == EDIT_ARROW_DOWN) { g_edit_menu_row = (g_edit_menu_row + 1) % max_rows; }
            else if (c == EDIT_ARROW_LEFT) { g_edit_menu_col = (g_edit_menu_col + g_edit_num_menus - 1) % g_edit_num_menus; g_edit_menu_row = 0; }
            else if (c == EDIT_ARROW_RIGHT) { g_edit_menu_col = (g_edit_menu_col + 1) % g_edit_num_menus; g_edit_menu_row = 0; }
            else if (c == EDIT_KEY_ENTER) { edit_execute_menu(vm); }
        } else {
            if (c == EDIT_SHIFT_ARROW_UP) { edit_update_sel_end(g_edit_cy > 0 ? g_edit_cy - 1 : 0, g_edit_cx); g_edit_cy = g_edit_sel_end_r; }
            else if (c == EDIT_SHIFT_ARROW_DOWN) { edit_update_sel_end(g_edit_cy < g_edit_num_lines - 1 ? g_edit_cy + 1 : g_edit_num_lines - 1, g_edit_cx); g_edit_cy = g_edit_sel_end_r; }
            else if (c == EDIT_SHIFT_ARROW_LEFT) { 
                edit_update_sel_end(g_edit_cy, g_edit_cx > 0 ? g_edit_cx - 1 : 0); 
                if (g_edit_cx == 0 && g_edit_cy > 0) edit_update_sel_end(g_edit_cy - 1, (int)g_edit_text_buffer[g_edit_cy-1].length);
                g_edit_cx = g_edit_sel_end_c; g_edit_cy = g_edit_sel_end_r; 
            }
            else if (c == EDIT_SHIFT_ARROW_RIGHT) { 
                edit_update_sel_end(g_edit_cy, g_edit_cx < (int)g_edit_text_buffer[g_edit_cy].length ? g_edit_cx + 1 : g_edit_cx); 
                if (g_edit_cx == (int)g_edit_text_buffer[g_edit_cy].length && g_edit_cy < g_edit_num_lines - 1) edit_update_sel_end(g_edit_cy + 1, 0);
                g_edit_cx = g_edit_sel_end_c; g_edit_cy = g_edit_sel_end_r; 
            }
            else if (c == EDIT_KEY_CTRL_INS) {
                char *txt = edit_get_selected_text();
                if (txt) {
                    runtime_strncpy(g_edit_clipboard_line, txt, sizeof(g_edit_clipboard_line) - 1);
                    g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
                    if (hal && hal->mem.free) hal->mem.free(txt);
                    edit_clear_sel();
                }
            }
            else if (c == EDIT_KEY_SHIFT_DEL) {
                char *txt = edit_get_selected_text();
                if (txt) {
                    runtime_strncpy(g_edit_clipboard_line, txt, sizeof(g_edit_clipboard_line) - 1);
                    g_edit_clipboard_line[sizeof(g_edit_clipboard_line) - 1] = '\0';
                    if (hal && hal->mem.free) hal->mem.free(txt);
                    edit_delete_selection();
                }
            }
            else if (c == EDIT_KEY_SHIFT_INS) {
                edit_insert_text_at_cursor(g_edit_clipboard_line);
            }
            else if (c == EDIT_ARROW_UP || c == EDIT_ARROW_DOWN || c == EDIT_ARROW_LEFT || c == EDIT_ARROW_RIGHT || c == EDIT_HOME_KEY || c == EDIT_END_KEY || c == EDIT_PAGE_UP || c == EDIT_PAGE_DOWN || c == EDIT_CTRL_HOME || c == EDIT_CTRL_END) {
                edit_clear_sel();
            }
            else {
                if ((c >= 32 && c <= 126) || c == EDIT_KEY_BACKSPACE || c == EDIT_KEY_ENTER || c == EDIT_DEL_KEY) edit_clear_sel();
            }

            switch (c) {
                case EDIT_ARROW_UP: if (g_edit_cy > 0) { g_edit_cy--; moved_vertically = true; } break;
                case EDIT_ARROW_DOWN: if (g_edit_cy < g_edit_num_lines - 1) { g_edit_cy++; moved_vertically = true; } break;
                case EDIT_ARROW_LEFT: 
                    if (g_edit_cx > 0) g_edit_cx--; 
                    else if (g_edit_cy > 0) { g_edit_cy--; g_edit_cx = (int)g_edit_text_buffer[g_edit_cy].length; }
                    break;
                case EDIT_ARROW_RIGHT: 
                    if (g_edit_cx < (int)g_edit_text_buffer[g_edit_cy].length) g_edit_cx++; 
                    else if (g_edit_cy < g_edit_num_lines - 1) { g_edit_cy++; g_edit_cx = 0; }
                    break;
                case EDIT_PAGE_UP: g_edit_cy -= (g_edit_screen_rows - 3); if (g_edit_cy < 0) g_edit_cy = 0; moved_vertically = true; break;
                case EDIT_PAGE_DOWN: g_edit_cy += (g_edit_screen_rows - 3); if (g_edit_cy >= g_edit_num_lines) g_edit_cy = g_edit_num_lines - 1; moved_vertically = true; break;
                case EDIT_HOME_KEY: g_edit_cx = 0; break;
                case EDIT_END_KEY: g_edit_cx = (int)g_edit_text_buffer[g_edit_cy].length; break;
                case EDIT_CTRL_HOME: g_edit_cy = 0; g_edit_cx = 0; moved_vertically = true; break;
                case EDIT_CTRL_END: g_edit_cy = g_edit_num_lines - 1; g_edit_cx = (int)g_edit_text_buffer[g_edit_cy].length; moved_vertically = true; break;
                case EDIT_DEL_KEY: edit_handle_delete(); break;
                case EDIT_KEY_ENTER: edit_insert_newline(); break;
                case EDIT_KEY_BACKSPACE: edit_handle_backspace(); break;
                default: 
                    if ((c >= 32 && c <= 126) || c == EDIT_KEY_TAB) {
                        edit_insert_char(c);
                    }
                    break;
            }
            if (moved_vertically) g_edit_cx = edit_get_physical_x(g_edit_cy, g_edit_target_rx);
            else {
                if (g_edit_cx > (int)g_edit_text_buffer[g_edit_cy].length) g_edit_cx = (int)g_edit_text_buffer[g_edit_cy].length;
                g_edit_target_rx = edit_get_render_x(g_edit_cy, g_edit_cx);
            }
        }
    }
    
    edit_print("\x1b[0m\x1b[2J\x1b[H\x1b[?25h");
    edit_flush();
    return 0;
}
