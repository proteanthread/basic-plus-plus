// FILENAME: vi_cmd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (vi_internal.h)
// Implements visual text editor subsystem components for vi_cmd.
//
// ---- Includes ----

#include "editor/vi_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

#define VI_NUM_BRIGHT_COLORS 6

void vi_handle_normal(int c) {
    static int pending_d = 0;
    
    if (pending_d) {
        pending_d = 0;
        if (c == 'd') {
            if (g_vi_current_lines > 1) {
                vi_free_line(g_vi_cursor_r);
                for (int i = g_vi_cursor_r; i < g_vi_current_lines - 1; i++) {
                    g_vi_text_buffer[i] = g_vi_text_buffer[i + 1];
                }
                g_vi_current_lines--;
                if (g_vi_cursor_r >= g_vi_current_lines) g_vi_cursor_r = g_vi_current_lines - 1;
            } else {
                g_vi_text_buffer[0].text[0] = '\0';
                g_vi_text_buffer[0].length = 0;
                g_vi_cursor_c = 0;
            }
            return;
        }
    }

    switch (c) {
        case 'h': case VI_KEY_LEFT:  g_vi_cursor_c--; break;
        case 'l': case VI_KEY_RIGHT: g_vi_cursor_c++; break;
        case 'j': case VI_KEY_DOWN:  g_vi_cursor_r++; break;
        case 'k': case VI_KEY_UP:    g_vi_cursor_r--; break;
        case VI_KEY_HOME: case '0':  g_vi_cursor_c = 0; break;
        case VI_KEY_END:  case '$':  g_vi_cursor_c = (int)g_vi_text_buffer[g_vi_cursor_r].length; break;
        case VI_KEY_PGUP: g_vi_cursor_r -= (g_vi_screen_rows - 2); break;
        case VI_KEY_PGDN: g_vi_cursor_r += (g_vi_screen_rows - 2); break;
        case VI_KEY_CTRL_HOME: g_vi_cursor_r = 0; g_vi_cursor_c = 0; break;
        case VI_KEY_CTRL_END: g_vi_cursor_r = g_vi_current_lines - 1; g_vi_cursor_c = (int)g_vi_text_buffer[g_vi_cursor_r].length; break;
        
        case 'i': case VI_KEY_INS: g_vi_mode = 1; break;
        case 'a': g_vi_cursor_c++; g_vi_mode = 1; break;
        case 'I': g_vi_cursor_c = 0; g_vi_mode = 1; break;
        case 'A': g_vi_cursor_c = (int)g_vi_text_buffer[g_vi_cursor_r].length; g_vi_mode = 1; break;
        
        case 'x': case VI_KEY_DEL:
            if (g_vi_text_buffer[g_vi_cursor_r].text[g_vi_cursor_c] != '\0') {
                for (int i = g_vi_cursor_c; g_vi_text_buffer[g_vi_cursor_r].text[i]; i++) {
                    g_vi_text_buffer[g_vi_cursor_r].text[i] = g_vi_text_buffer[g_vi_cursor_r].text[i + 1];
                }
                g_vi_text_buffer[g_vi_cursor_r].length--;
            }
            break;
            
        case 'd': pending_d = 1; break;
        case 'o':
            vi_insert_empty_line(g_vi_cursor_r + 1);
            g_vi_cursor_r++;
            g_vi_mode = 1;
            g_vi_cursor_c = 0;
            break;
        case 'O':
            vi_insert_empty_line(g_vi_cursor_r);
            g_vi_mode = 1;
            g_vi_cursor_c = 0;
            break;
            
        case ':':
            g_vi_mode = 2;
            g_vi_cmd_len = 0;
            g_vi_cmd_buffer[0] = '\0';
            break;
    }
}

void vi_handle_insert(int c) {
    if (c == 27) { // Escape
        g_vi_mode = 0;
        if (g_vi_cursor_c > 0) g_vi_cursor_c--;
    } else if (c == VI_KEY_UP) { g_vi_cursor_r--; }
    else if (c == VI_KEY_DOWN) { g_vi_cursor_r++; }
    else if (c == VI_KEY_LEFT) { g_vi_cursor_c--; }
    else if (c == VI_KEY_RIGHT) { g_vi_cursor_c++; }
    else if (c == VI_KEY_HOME) { g_vi_cursor_c = 0; }
    else if (c == VI_KEY_END) { g_vi_cursor_c = (int)g_vi_text_buffer[g_vi_cursor_r].length; }
    else if (c == VI_KEY_PGUP) { g_vi_cursor_r -= (g_vi_screen_rows - 2); }
    else if (c == VI_KEY_PGDN) { g_vi_cursor_r += (g_vi_screen_rows - 2); }
    else if (c == VI_KEY_CTRL_HOME) { g_vi_cursor_r = 0; g_vi_cursor_c = 0; }
    else if (c == VI_KEY_CTRL_END) { g_vi_cursor_r = g_vi_current_lines - 1; g_vi_cursor_c = (int)g_vi_text_buffer[g_vi_cursor_r].length; }
    else if (c == VI_KEY_DEL) {
        if (g_vi_text_buffer[g_vi_cursor_r].text[g_vi_cursor_c] != '\0') {
            for (int i = g_vi_cursor_c; g_vi_text_buffer[g_vi_cursor_r].text[i]; i++) {
                g_vi_text_buffer[g_vi_cursor_r].text[i] = g_vi_text_buffer[g_vi_cursor_r].text[i + 1];
            }
            g_vi_text_buffer[g_vi_cursor_r].length--;
        } else if (g_vi_cursor_r < g_vi_current_lines - 1) {
            int len = g_vi_text_buffer[g_vi_cursor_r].length;
            int next_len = g_vi_text_buffer[g_vi_cursor_r + 1].length;
            vi_ensure_line_capacity(g_vi_cursor_r, len + next_len + 1);
            runtime_memmove(&g_vi_text_buffer[g_vi_cursor_r].text[len], g_vi_text_buffer[g_vi_cursor_r + 1].text, (size_t)(next_len + 1));
            g_vi_text_buffer[g_vi_cursor_r].length += next_len;
            vi_free_line(g_vi_cursor_r + 1);
            for (int i = g_vi_cursor_r + 1; i < g_vi_current_lines - 1; i++) {
                g_vi_text_buffer[i] = g_vi_text_buffer[i + 1];
            }
            g_vi_current_lines--;
        }
    } else if (c == 10 || c == 13) { // Enter
        vi_insert_empty_line(g_vi_cursor_r + 1);
        int rem_len = g_vi_text_buffer[g_vi_cursor_r].length - g_vi_cursor_c;
        vi_ensure_line_capacity(g_vi_cursor_r + 1, rem_len + 1);
        runtime_memmove(g_vi_text_buffer[g_vi_cursor_r + 1].text, &g_vi_text_buffer[g_vi_cursor_r].text[g_vi_cursor_c], (size_t)(rem_len + 1));
        g_vi_text_buffer[g_vi_cursor_r + 1].length = rem_len;
        g_vi_text_buffer[g_vi_cursor_r].text[g_vi_cursor_c] = '\0';
        g_vi_text_buffer[g_vi_cursor_r].length = g_vi_cursor_c;
        g_vi_cursor_r++;
        g_vi_cursor_c = 0;
    } else if (c == 8 || c == 127) { // Backspace
        if (g_vi_cursor_c > 0) {
            int len = g_vi_text_buffer[g_vi_cursor_r].length;
            for (int i = g_vi_cursor_c; i <= len; i++) {
                g_vi_text_buffer[g_vi_cursor_r].text[i - 1] = g_vi_text_buffer[g_vi_cursor_r].text[i];
            }
            g_vi_text_buffer[g_vi_cursor_r].length--;
            g_vi_cursor_c--;
        } else if (g_vi_cursor_r > 0) {
            int prev_len = g_vi_text_buffer[g_vi_cursor_r - 1].length;
            int cur_len = g_vi_text_buffer[g_vi_cursor_r].length;
            vi_ensure_line_capacity(g_vi_cursor_r - 1, prev_len + cur_len + 1);
            runtime_strcat(g_vi_text_buffer[g_vi_cursor_r - 1].text, g_vi_text_buffer[g_vi_cursor_r].text);
            g_vi_text_buffer[g_vi_cursor_r - 1].length += cur_len;
            vi_free_line(g_vi_cursor_r);
            for (int i = g_vi_cursor_r; i < g_vi_current_lines - 1; i++) {
                g_vi_text_buffer[i] = g_vi_text_buffer[i + 1];
            }
            g_vi_current_lines--;
            g_vi_cursor_r--;
            g_vi_cursor_c = prev_len;
        }
    } else if (c >= 32 && c <= 126) { // ASCII Characters
        int len = g_vi_text_buffer[g_vi_cursor_r].length;
        vi_ensure_line_capacity(g_vi_cursor_r, len + 2);
        for (int i = len; i >= g_vi_cursor_c; i--) {
            g_vi_text_buffer[g_vi_cursor_r].text[i + 1] = g_vi_text_buffer[g_vi_cursor_r].text[i];
        }
        g_vi_text_buffer[g_vi_cursor_r].text[g_vi_cursor_c] = (char)c;
        g_vi_text_buffer[g_vi_cursor_r].length++;
        g_vi_cursor_c++;
    } else if (c == VI_KEY_F1) {
        vi_display_help();
    } else if (c == VI_KEY_F2) {
        if (g_vi_current_filename[0]) vi_save_file();
        else {
            g_vi_mode = 2;
            runtime_strcpy(g_vi_cmd_buffer, "w ");
            g_vi_cmd_len = 2;
        }
    } else if (c == VI_KEY_F3) {
        g_vi_mode = 2;
        runtime_strcpy(g_vi_cmd_buffer, "load ");
        g_vi_cmd_len = 5;
    } else if (c == VI_KEY_F4) {
        g_vi_running = false;
    }
}

void vi_handle_command(VMContext *vm, int c) {
    if (c == 27) {
        g_vi_mode = 0;
    } else if (c == 10 || c == 13) {
        if (runtime_strcmp(g_vi_cmd_buffer, "w") == 0) vi_save_file();
        else if (runtime_strncmp(g_vi_cmd_buffer, "w ", 2) == 0) {
            char new_file[4096];
            runtime_strncpy(new_file, g_vi_cmd_buffer + 2, sizeof(new_file) - 1);
            new_file[sizeof(new_file) - 1] = '\0';
            runtime_strcpy(g_vi_current_filename, new_file);
            vi_save_file();
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "q") == 0 || runtime_strcmp(g_vi_cmd_buffer, "q!") == 0) g_vi_running = false;
        else if (runtime_strcmp(g_vi_cmd_buffer, "bye") == 0) g_vi_running = false;
        else if (runtime_strcmp(g_vi_cmd_buffer, "wq") == 0 || runtime_strcmp(g_vi_cmd_buffer, "x") == 0) {
            vi_save_file();
            g_vi_running = false;
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "run") == 0 || runtime_strcmp(g_vi_cmd_buffer, "r!") == 0) {
            vi_execute_program(vm, 0);
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "debug") == 0) {
            vi_execute_program(vm, 1);
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "trace") == 0) {
            vi_execute_program(vm, 2);
        }
        else if (runtime_strncmp(g_vi_cmd_buffer, "load ", 5) == 0 || runtime_strncmp(g_vi_cmd_buffer, "g ", 2) == 0 || runtime_strncmp(g_vi_cmd_buffer, "get ", 4) == 0) {
            char new_file[4096];
            const char *file_start = g_vi_cmd_buffer + 5;
            if (runtime_strncmp(g_vi_cmd_buffer, "g ", 2) == 0) file_start = g_vi_cmd_buffer + 2;
            else if (runtime_strncmp(g_vi_cmd_buffer, "get ", 4) == 0) file_start = g_vi_cmd_buffer + 4;
            runtime_strncpy(new_file, file_start, sizeof(new_file) - 1);
            new_file[sizeof(new_file) - 1] = '\0';
            runtime_strcpy(g_vi_current_filename, new_file);
            vi_load_file(g_vi_current_filename);
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "color") == 0) {
            g_vi_color_index = (g_vi_color_index + 1) % VI_NUM_BRIGHT_COLORS;
        }
        else if (runtime_strncmp(g_vi_cmd_buffer, "color ", 6) == 0) {
            char new_color[4096];
            runtime_strncpy(new_color, g_vi_cmd_buffer + 6, sizeof(new_color) - 1);
            new_color[sizeof(new_color) - 1] = '\0';
            if (runtime_strcmp(new_color, "white") == 0) g_vi_color_index = 0;
            else if (runtime_strcmp(new_color, "cyan") == 0) g_vi_color_index = 1;
            else if (runtime_strcmp(new_color, "green") == 0) g_vi_color_index = 2;
            else if (runtime_strcmp(new_color, "yellow") == 0) g_vi_color_index = 3;
            else if (runtime_strcmp(new_color, "magenta") == 0) g_vi_color_index = 4;
            else if (runtime_strcmp(new_color, "red") == 0) g_vi_color_index = 5;
        }
        else if (runtime_strcmp(g_vi_cmd_buffer, "?") == 0 || runtime_strcmp(g_vi_cmd_buffer, "h") == 0) vi_display_help();
        g_vi_mode = 0;
    } else if (c == 8 || c == 127) {
        if (g_vi_cmd_len > 0) g_vi_cmd_buffer[--g_vi_cmd_len] = '\0';
        else g_vi_mode = 0;
    } else if (c >= 32 && c <= 126 && g_vi_cmd_len < 4096 - 1) {
        g_vi_cmd_buffer[g_vi_cmd_len++] = (char)c;
        g_vi_cmd_buffer[g_vi_cmd_len] = '\0';
    }
}

void vi_exit_editor(void) {
    platform_console_printf("\x1b[2J\x1b[H\x1b[?25h");
    platform_console_flush();
    vi_reset_term();
}

int vi_main_loop(VMContext *vm, const char *filename) {
    int c;
    if (filename && filename[0]) {
        vi_load_file(filename);
    } else {
        vi_insert_empty_line(0);
    }
    
    vi_init_term();
    platform_console_printf("\x1b[2J\x1b[H"); 
    
    while (g_vi_running) {
        vi_get_terminal_size();
        vi_fix_cursor();
        vi_render_screen();
        
        c = vi_get_input();
        if (c == 0 || c == VI_KEY_TIMEOUT) continue;
        if (c == 3 || c == 4) { g_vi_running = false; continue; }
        
        if (g_vi_mode == 0) vi_handle_normal(c);
        else if (g_vi_mode == 1) vi_handle_insert(c);
        else if (g_vi_mode == 2) vi_handle_command(vm, c);
    }
    
    vi_exit_editor();
    return 0;
}
