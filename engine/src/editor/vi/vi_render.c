// FILENAME: vi_render.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (vi_internal.h)
// Implements visual text editor subsystem components for vi_render.
//
// ---- Includes ----

#include "editor/vi_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

void vi_get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (!r1 || !c1 || !r2 || !c2) return;
    if (g_vi_sel_start_r < g_vi_sel_end_r || (g_vi_sel_start_r == g_vi_sel_end_r && g_vi_sel_start_c <= g_vi_sel_end_c)) {
        *r1 = g_vi_sel_start_r; *c1 = g_vi_sel_start_c;
        *r2 = g_vi_sel_end_r; *c2 = g_vi_sel_end_c;
    } else {
        *r1 = g_vi_sel_end_r; *c1 = g_vi_sel_end_c;
        *r2 = g_vi_sel_start_r; *c2 = g_vi_sel_start_c;
    }
}

void vi_format_filename_for_status(char *out_buf, const char *in_filename, int max_len) {
    if (!out_buf || max_len <= 0) return;
    if (!in_filename || !in_filename[0]) {
        runtime_strncpy(out_buf, "NEW", (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }
    const char *last_slash = runtime_strrchr(in_filename, '/');
    const char *last_bslash = runtime_strrchr(in_filename, '\\');
    const char *base = in_filename;
    if (last_slash && last_slash >= base) base = last_slash + 1;
    if (last_bslash && last_bslash >= base) base = last_bslash + 1;

    size_t full_len = runtime_strlen(in_filename);
    if (full_len <= (size_t)max_len) {
        runtime_strncpy(out_buf, in_filename, (size_t)max_len);
        out_buf[max_len - 1] = '\0';
    } else {
        size_t base_len = runtime_strlen(base);
        if (base_len + 4 <= (size_t)max_len) {
            size_t lead_avail = (size_t)max_len - base_len - 4;
            if (lead_avail > 0) {
                runtime_strncpy(out_buf, in_filename, lead_avail);
                out_buf[lead_avail] = '\0';
                runtime_strcat(out_buf, "...");
                const char *lead_slash = (base > in_filename) ? (base - 1) : "/";
                char sep_str[2] = { *lead_slash, '\0' };
                runtime_strcat(out_buf, sep_str);
                runtime_strcat(out_buf, base);
            } else {
                runtime_strncpy(out_buf, "...", 4);
                runtime_strncat(out_buf, base, (size_t)max_len - 4);
            }
        } else {
            size_t copy_len = (size_t)max_len - 1;
            if (copy_len > base_len) copy_len = base_len;
            runtime_strncpy(out_buf, base, copy_len);
            out_buf[copy_len] = '\0';
        }
    }
}

void vi_render_screen(void) {
    HalContext *hal = hal_get();
    if (!hal) return;

    platform_console_printf("\x1b[?25l"); // Hide cursor momentarily to prevent flicker
    platform_console_printf("\x1b[H");
    
    platform_console_printf("%s", g_vi_bright_colors[g_vi_color_index]);
    
    int show_splash = 0;
    if (g_vi_current_lines == 1 && g_vi_text_buffer[0].length == 0 && g_vi_current_filename[0] == '\0') {
        show_splash = 1;
    }

    for (int i = 0; i < g_vi_screen_rows - 1; i++) {
        int line_idx = g_vi_row_offset + i;
        if (line_idx < g_vi_current_lines) {
            platform_console_printf("%s", g_vi_bright_colors[g_vi_color_index]);
            for (int j = 0; j < (int)g_vi_text_buffer[line_idx].length; j++) {
                int in_sel = 0;
                if (g_vi_sel_active) {
                    int r1, c1, r2, c2;
                    vi_get_sel_bounds(&r1, &c1, &r2, &c2);
                    if (line_idx > r1 && line_idx < r2) in_sel = 1;
                    else if (line_idx == r1 && line_idx == r2 && j >= c1 && j < c2) in_sel = 1;
                    else if (line_idx == r1 && line_idx < r2 && j >= c1) in_sel = 1;
                    else if (line_idx == r2 && line_idx > r1 && j < c2) in_sel = 1;
                }
                if (in_sel) {
                    platform_console_printf("\x1b[47;30m%c%s", g_vi_text_buffer[line_idx].text[j], g_vi_bright_colors[g_vi_color_index]);
                } else {
                    platform_console_putchar(g_vi_text_buffer[line_idx].text[j]);
                }
            }
            platform_console_printf("\x1b[K\r\n");
        } else {
            if (show_splash && i == (g_vi_screen_rows - 1) / 3) {
                char splash[128];
                runtime_snprintf(splash, sizeof(splash), "BASIC++ Standard v%s", BASIC_VERSION_STRING);
                int pad = (g_vi_screen_cols - (int)runtime_strlen(splash)) / 2;
                if (pad < 2) pad = 2;
                platform_console_printf("\x1b[36m~");
                for (int p = 1; p < pad; p++) platform_console_putchar(' ');
                platform_console_printf("%s\x1b[K\r\n", splash);
            } else {
                platform_console_printf("\x1b[36m~%s\x1b[K\r\n", g_vi_bright_colors[g_vi_color_index]);
            }
        }
    }
    
    // Dynamic Status Line
    platform_console_printf("\x1b[47;30m");
    char left_status[4200];
    if (g_vi_mode == 2) {
        runtime_snprintf(left_status, sizeof(left_status), ":%s", g_vi_cmd_buffer);
    } else if (g_vi_mode == 1) {
        runtime_snprintf(left_status, sizeof(left_status), "-- INSERT --");
    } else {
        char trunc_name[4096];
        vi_format_filename_for_status(trunc_name, g_vi_current_filename, g_vi_screen_cols - 30);
        runtime_snprintf(left_status, sizeof(left_status), "\"%s\" %d:%d", trunc_name, g_vi_cursor_r + 1, g_vi_current_lines);
    }
    
    char time_str[64] = "";
    int ll = (int)runtime_strlen(left_status);
    int tl = (int)runtime_strlen(time_str);
    int pad = g_vi_screen_cols - ll - tl - 2;
    if (pad < 1) pad = 1;
    
    platform_console_printf("\x1b[%d;1H\x1b[47;30m\x1b[K", g_vi_screen_rows);
    platform_console_printf("%s", left_status);
    for (int i = 0; i < pad; i++) platform_console_putchar(' ');
    platform_console_printf("%s", time_str);
    platform_console_printf("\x1b[0m");
    
    // Physical Cursor Position
    if (g_vi_mode == 2) {
        platform_console_printf("\x1b[%d;%dH", g_vi_screen_rows, g_vi_cmd_len + 2);
    } else {
        platform_console_printf("\x1b[%d;%dH", (g_vi_cursor_r - g_vi_row_offset) + 1, g_vi_cursor_c + 1);
    }
    platform_console_printf("\x1b[?25h");
    platform_console_flush();
}

void vi_display_help(void) {
    platform_console_printf("\x1b[2J\x1b[H"); 
    platform_console_printf("--- vi Built-in Help ---\r\n\n");
    platform_console_printf(" NORMAL MODE:\r\n");
    platform_console_printf("   h,j,k,l / Arrows : Move cursor\r\n");
    platform_console_printf("   Home / End       : Jump to start/end of line\r\n");
    platform_console_printf("   PgUp / PgDn      : Page up / Page down\r\n");
    platform_console_printf("   0, $             : Jump to start/end of line\r\n");
    platform_console_printf("   i, a, I, A       : Enter Insert Mode\r\n");
    platform_console_printf("   o, O             : Insert new line / Insert Mode\r\n");
    platform_console_printf("   x, Del           : Delete character under cursor\r\n");
    platform_console_printf("   dd               : Delete current line\r\n");
    platform_console_printf("   :                : Enter Command Mode\r\n\n");
    platform_console_printf(" INSERT MODE:\r\n");
    platform_console_printf("   Esc              : Return to Normal Mode\r\n");
    platform_console_printf("   Enter            : Split line\r\n");
    platform_console_printf("   Backspace        : Delete char or merge lines\r\n\n");
    platform_console_printf(" COMMAND MODE:\r\n");
    platform_console_printf("   :w               : Save file\r\n");
    platform_console_printf("   :w <file>        : Save to new file\r\n");
    platform_console_printf("   :load <file>     : Load a file (aliases: :g, :get)\r\n");
    platform_console_printf("   :run, :r!        : Execute the current program\r\n");
    platform_console_printf("   :debug           : Execute program in step mode (F8 to step, F5 to continue)\r\n");
    platform_console_printf("   :trace           : Execute program and print line trace\r\n");
    platform_console_printf("   :q, :q!          : Quit / Quit without saving\r\n");
    platform_console_printf("   :wq, :x          : Save and Quit\r\n");
    platform_console_printf("   :?, :h           : Show this help screen\r\n\n");

    platform_console_printf("Press any key to return...");
    platform_console_flush();
    int key;
    do {
        key = vi_get_input();
    } while (key == 0 || key == VI_KEY_TIMEOUT);
}
