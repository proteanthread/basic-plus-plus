// FILENAME: edit_render.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (calendar.h, calendar.c)
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit_render.
//
// ---- Includes ----

#include "editor/edit_internal.h"
#include "runtime/time/calendar.h"

int edit_get_render_x(int row, int physical_x) {
    int rx = 0;
    for (int j = 0; j < physical_x && g_edit_text_buffer[row].text[j] != '\0'; j++) {
        if (g_edit_text_buffer[row].text[j] == '\t') {
            rx += (EDIT_TAB_STOP - 1) - (rx % EDIT_TAB_STOP);
        }
        rx++;
    }
    return rx;
}

int edit_get_physical_x(int row, int target_x) {
    int rx = 0, j;
    for (j = 0; g_edit_text_buffer[row].text[j] != '\0'; j++) {
        int next_rx = rx;
        if (g_edit_text_buffer[row].text[j] == '\t') {
            next_rx += (EDIT_TAB_STOP - 1) - (rx % EDIT_TAB_STOP);
        }
        next_rx++;
        if (next_rx > target_x) return j;
        rx = next_rx;
    }
    return j;
}

void edit_render_row(int row, char *out_buf) {
    int j = 0, idx = 0;
    while (g_edit_text_buffer[row].text[j] != '\0' && idx < (EDIT_MAX_RENDER_BUF - 1)) {
        if (g_edit_text_buffer[row].text[j] == '\t') {
            out_buf[idx++] = ' ';
            while (idx % EDIT_TAB_STOP != 0 && idx < (EDIT_MAX_RENDER_BUF - 1)) out_buf[idx++] = ' ';
        } else {
            out_buf[idx++] = g_edit_text_buffer[row].text[j];
        }
        j++;
    }
    out_buf[idx] = '\0';
}

void edit_format_filename_for_status(char *out_buf, const char *in_filename, int max_len) {
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

void edit_draw_all(void) {
    int y, i, file_row, len, print_len;
    char r_buf[EDIT_MAX_RENDER_BUF];
    
    edit_print("\x1b[?25l");
    
    // 1. Menu Bar
    edit_print("\x1b[1;1H" COL_MENU "\x1b[K"); 
    for (i = 0; i < g_edit_num_menus; i++) {
        edit_print("\x1b[1;%dH", g_edit_menu_x[i]);
        if (g_edit_menu_mode > 0 && g_edit_menu_col == i) edit_print(COL_MENU_SEL);
        else edit_print(COL_MENU);
        edit_print("%s", g_edit_menu_names[i]);
    }
    
    edit_print(COL_MENU);
    if (g_edit_current_filename[0]) {
        const char *basename = g_edit_current_filename;
        const char *p = g_edit_current_filename;
        while (*p) {
            if (*p == '/' || *p == '\\') basename = p + 1;
            p++;
        }
        edit_print("  [ %s ]", basename);
    }
    
    // 2. Editor Content Area
    for (y = 2; y <= g_edit_screen_rows - 1; y++) {
        file_row = g_edit_row_off + (y - 2);
        edit_print("\x1b[%d;1H%s", y, g_edit_bright_colors[g_edit_color_index]); 
        
        int draw_cols = g_edit_screen_cols;
        int gutter_offset = 0;

        if (g_edit_display_gutter) {
            char gutter[16];
            if (file_row < g_edit_num_lines) runtime_snprintf(gutter, sizeof(gutter), "%5d |", file_row + 1);
            else runtime_snprintf(gutter, sizeof(gutter), "      |");
            edit_print("\x1b[36m%s\x1b[0m%s", gutter, g_edit_bright_colors[g_edit_color_index]);
            gutter_offset = 7;
            draw_cols -= gutter_offset;
        }

        if (file_row < g_edit_num_lines) {
            edit_render_row(file_row, r_buf);
            len = (int)runtime_strlen(r_buf);
            if (len > g_edit_col_off) {
                print_len = len - g_edit_col_off;
                if (print_len > draw_cols) print_len = draw_cols;
                edit_print("%.*s\x1b[K", print_len, r_buf + g_edit_col_off);
            } else {
                edit_print("\x1b[K");
            }
        } else {
            edit_print("\x1b[K");
        }
    }
    
    // 3. Status Bar
    char right_status[128];
    runtime_snprintf(right_status, sizeof(right_status), "LINE:%d:%d - COL:%d", g_edit_cy + 1, g_edit_num_lines, g_edit_cx + 1);
    int rl = (int)runtime_strlen(right_status);
    
    char left_status[4200];
    char trunc_name[2048];
    edit_format_filename_for_status(trunc_name, g_edit_current_filename, g_edit_screen_cols - rl - 15);
    runtime_snprintf(left_status, sizeof(left_status), " ESC=Menu %s", trunc_name);
    int ll = (int)runtime_strlen(left_status);
    
    int pad = g_edit_screen_cols - ll - rl - 3;
    if (pad < 1) pad = 1;
    
    edit_print("\x1b[%d;1H" COL_STATUS "\x1b[K", g_edit_screen_rows);
    edit_print("%s", left_status);
    for (i = 0; i < pad; i++) edit_print(" ");
    edit_print("%s", right_status);
    
    // 4. Active Dropdown Modal
    if (g_edit_menu_mode == 2) {
        int mx = g_edit_menu_x[g_edit_menu_col];
        int mh = g_edit_drop_sizes[g_edit_menu_col];
        const char **items = g_edit_dropdowns[g_edit_menu_col];
        
        for (i = 0; i < mh; i++) {
            edit_print("\x1b[%d;%dH", 2 + i, mx);
            if (g_edit_menu_row == i) edit_print(COL_MENU_SEL);
            else edit_print(COL_MENU);
            edit_print("%s", items[i]);
            edit_print(COL_SHADOW " \x1b[0m");
        }
        edit_print("\x1b[%d;%dH" COL_SHADOW, 2 + mh, mx + 1);
        for (i = 0; i < (int)runtime_strlen(items[0]) + 1; i++) edit_print(" ");
    }
}

int edit_prompt_input(const char *title, char *buf) {
    int w = 40, h = 7;
    int x, y, len, c, i, j;
    len = (int)runtime_strlen(buf);
    
    edit_get_terminal_size(&g_edit_screen_rows, &g_edit_screen_cols);
    edit_draw_all();

    while (true) {
        x = (g_edit_screen_cols - w) / 2;
        y = (g_edit_screen_rows - h) / 2;
        
        edit_print(COL_SHADOW);
        for (j = 1; j <= h; j++) edit_print("\x1b[%d;%dH  ", y + j, x + w);
        edit_print("\x1b[%d;%dH", y + h, x + 2);
        for (i = 0; i < w; i++) edit_print(" ");
        
        edit_print(COL_MENU);
        for (j = 0; j < h; j++) {
            edit_print("\x1b[%d;%dH", y + j, x);
            if (j == 0 || j == h - 1) {
                edit_print("+"); for (i = 1; i < w - 1; i++) edit_print("-"); edit_print("+");
            } else {
                edit_print("|"); for (i = 1; i < w - 1; i++) edit_print(" "); edit_print("|");
            }
        }
        
        edit_print("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)runtime_strlen(title) - 2) / 2, title);
        edit_print("\x1b[%d;%dH" COL_MENU_SEL, y + 3, x + 4);
        for (i = 0; i < w - 8; i++) {
            if (i < len) edit_print("%c", buf[i]);
            else edit_print(" ");
        }
        
        edit_print("\x1b[%d;%dH" COL_MENU " < OK > ", y + 5, x + w / 2 - 4);
        edit_print("\x1b[%d;%dH\x1b[?25h", y + 3, x + 4 + len);
        edit_flush();
        c = edit_read_key();
        if (c == EDIT_KEY_TIMEOUT) { edit_draw_all(); continue; }
        if (c == EDIT_KEY_ESC) return 0;
        if (c == EDIT_KEY_ENTER) return 1;
        if (c == EDIT_KEY_BACKSPACE) {
            if (len > 0) buf[--len] = '\0';
        } else if (c >= 32 && c <= 126 && len < w - 10) {
            buf[len++] = (char)c;
            buf[len] = '\0';
        }
    }
}

void edit_show_message(const char *title, const char *msg) {
    int w = 50, h = 8;
    int x, y, c, i, j;
    
    edit_get_terminal_size(&g_edit_screen_rows, &g_edit_screen_cols);
    edit_draw_all();

    while (true) {
        x = (g_edit_screen_cols - w) / 2;
        y = (g_edit_screen_rows - h) / 2;
        
        edit_print(COL_SHADOW);
        for (j = 1; j <= h; j++) edit_print("\x1b[%d;%dH  ", y + j, x + w);
        edit_print("\x1b[%d;%dH", y + h, x + 2);
        for (i = 0; i < w; i++) edit_print(" ");
        
        edit_print(COL_MENU);
        for (j = 0; j < h; j++) {
            edit_print("\x1b[%d;%dH", y + j, x);
            if (j == 0 || j == h - 1) {
                edit_print("+"); for (i = 1; i < w - 1; i++) edit_print("-"); edit_print("+");
            } else {
                edit_print("|"); for (i = 1; i < w - 1; i++) edit_print(" "); edit_print("|");
            }
        }
        
        edit_print("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)runtime_strlen(title) - 2) / 2, title);
        edit_print("\x1b[%d;%dH%s", y + 3, x + 2, msg);
        edit_print("\x1b[%d;%dH" COL_MENU " < OK > ", y + 6, x + w / 2 - 4);
        edit_flush();
        c = edit_read_key();
        if (c == EDIT_KEY_TIMEOUT) { edit_draw_all(); continue; }
        if (c == EDIT_KEY_ESC || c == EDIT_KEY_ENTER || c == 32) return;
    }
}
