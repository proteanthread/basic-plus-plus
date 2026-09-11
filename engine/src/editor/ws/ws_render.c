// FILENAME: ws_render.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (ws_internal.h)
// Implements visual text editor subsystem components for ws_render.
//
// ---- Includes ----

#include "editor/ws_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

int ws_get_render_x(int row, int physical_x) {
    int rx = 0;
    for (int j = 0; j < physical_x && g_ws_text_buffer[row].text[j] != '\0'; j++) {
        if (g_ws_text_buffer[row].text[j] == '\t') {
            rx += (WS_TAB_STOP - 1) - (rx % WS_TAB_STOP);
        }
        rx++;
    }
    return rx;
}

int ws_get_physical_x(int row, int target_x) {
    int rx = 0, j;
    for (j = 0; g_ws_text_buffer[row].text[j] != '\0'; j++) {
        int next_rx = rx;
        if (g_ws_text_buffer[row].text[j] == '\t') {
            next_rx += (WS_TAB_STOP - 1) - (rx % WS_TAB_STOP);
        }
        next_rx++;
        if (next_rx > target_x) return j;
        rx = next_rx;
    }
    return j;
}

void ws_render_row(int row, char *out_buf) {
    int j = 0, idx = 0;
    while (g_ws_text_buffer[row].text[j] != '\0' && idx < (WS_MAX_RENDER_BUF - 1)) {
        if (g_ws_text_buffer[row].text[j] == '\t') {
            out_buf[idx++] = ' ';
            while (idx % WS_TAB_STOP != 0 && idx < (WS_MAX_RENDER_BUF - 1)) out_buf[idx++] = ' ';
        } else {
            out_buf[idx++] = g_ws_text_buffer[row].text[j];
        }
        j++;
    }
    out_buf[idx] = '\0';
}

void ws_format_filename_for_status(char *out_buf, const char *in_filename, int max_len) {
    if (!out_buf || max_len <= 0) return;
    if (!in_filename || !in_filename[0]) {
        runtime_strncpy(out_buf, "NEW FILE", (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }

    size_t len = runtime_strlen(in_filename);
    if ((int)len <= max_len) {
        runtime_strncpy(out_buf, in_filename, (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }
    
    const char *base = in_filename;
    for (int i = (int)len - 1; i >= 0; i--) {
        if (in_filename[i] == '/' || in_filename[i] == '\\') {
            base = &in_filename[i + 1];
            break;
        }
    }
    
    int base_len = (int)runtime_strlen(base);
    if (base_len >= max_len) {
        runtime_strncpy(out_buf, base, (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }
    
    const char *prefix = ".../";
    int prefix_len = (int)runtime_strlen(prefix);
    if (prefix_len + base_len < max_len) {
        runtime_strcpy(out_buf, prefix);
        runtime_strcat(out_buf, base);
    } else {
        int copy_len = base_len;
        if (copy_len > max_len - 1) copy_len = max_len - 1;
        runtime_memcpy(out_buf, base, (size_t)copy_len);
        out_buf[copy_len] = '\0';
    }
}

void ws_refresh_screen(void) {
    int y, y_start = 0, print_len, len, file_row, rx;
    char status_bar[4200];
    char r_buf[WS_MAX_RENDER_BUF];
    
    ws_get_terminal_size();
    
    ws_print("\x1b[?25l"); 
    ws_print("\x1b[H");    
    
    ws_print("%s", g_ws_bright_colors[g_ws_color_index]);
    
    if (g_ws_help_active) {
        ws_print("\x1b[47;30m");
        ws_print("----------------- Help (^K^H toggles) ----------------------\r\n");
        ws_print(" ^S = Left | ^D = Right | ^E = Up | ^X = Down               \r\n");
        ws_print(" ^K^X = Save & Exit | ^K^Q = Quit                           \r\n");
        ws_print(" ^K^R = Run | ^K^D = Debug | ^K^T = Trace                   \r\n");
        ws_print("%s", g_ws_bright_colors[g_ws_color_index]);
        y_start = 5;
    }
    
    for (y = y_start; y < g_ws_screen_rows - 1; y++) {
        file_row = g_ws_row_off + (y - y_start);
        if (file_row < g_ws_num_lines) {
            ws_render_row(file_row, r_buf);
            len = (int)runtime_strlen(r_buf);
            if (len > g_ws_col_off) {
                print_len = len - g_ws_col_off;
                if (print_len > g_ws_screen_cols) print_len = g_ws_screen_cols;
                for (int i = g_ws_col_off; i < g_ws_col_off + print_len; i++) {
                    int in_sel = 0;
                    if (g_ws_sel_active) {
                        int r1, c1, r2, c2;
                        ws_get_sel_bounds(&r1, &c1, &r2, &c2);
                        if (file_row > r1 && file_row < r2) in_sel = 1;
                        else if (file_row == r1 && file_row == r2 && i >= c1 && i < c2) in_sel = 1;
                        else if (file_row == r1 && file_row < r2 && i >= c1) in_sel = 1;
                        else if (file_row == r2 && file_row > r1 && i < c2) in_sel = 1;
                    }
                    if (in_sel) {
                        ws_print("\x1b[47;30m%c%s", r_buf[i], g_ws_bright_colors[g_ws_color_index]);
                    } else {
                        ws_print("%c", r_buf[i]);
                    }
                }
            }
        }
        ws_print("\x1b[K\r\n"); 
    }
    
    ws_print("\x1b[%d;1H\x1b[47;30m\x1b[K", g_ws_screen_rows);
    char trunc_name[2048];
    ws_format_filename_for_status(trunc_name, g_ws_current_filename, g_ws_screen_cols - 40);

    runtime_snprintf(status_bar, sizeof(status_bar), " %s%s | File: %s | %d:%d ", 
            g_ws_prefix_k ? "^K " : "", 
            g_ws_help_active ? "" : "(Press ^K^H for Help)", 
            trunc_name, 
            g_ws_cy + 1, g_ws_num_lines);
            
    char time_str[64] = "";
    int ll = (int)runtime_strlen(status_bar);
    int tl = (int)runtime_strlen(time_str);
    int pad = g_ws_screen_cols - ll - tl - 2;
    if (pad < 1) pad = 1;
    
    ws_print("%s", status_bar);
    for (int i = 0; i < pad; i++) ws_print(" ");
    ws_print("%s", time_str);
    ws_print("%s", g_ws_bright_colors[g_ws_color_index]);
    
    rx = ws_get_render_x(g_ws_cy, g_ws_cx);
    ws_print("\x1b[%d;%dH", (g_ws_cy - g_ws_row_off) + y_start + 1, (rx - g_ws_col_off) + 1);
    ws_print("\x1b[?25h"); 
    platform_console_flush();
}
