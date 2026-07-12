/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_ws.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Portable, WordStar-like full screen editor module for BASIC++.
 *    Operates entirely on its own static text buffer and communicates
 *    with the host interpreter exclusively through the WsCallbacks
 *    contract defined in mod_ws.h.
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "mod_ws.h"
#include "platform.h"

#define TAB_STOP 4

#define SHIFT_ARROW_UP 1100
#define SHIFT_ARROW_DOWN 1101
#define SHIFT_ARROW_LEFT 1102
#define SHIFT_ARROW_RIGHT 1103
#define KEY_CTRL_INS 1104
#define KEY_SHIFT_DEL 1105
#define KEY_SHIFT_INS 1106

/* --- Key Definitions --- */
enum editorKey {
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    INS_KEY, KEY_F10
};

/* --- Global State --- */
static char text_buffer[MAX_WS_LINES][MAX_WS_LENGTH];
static int num_lines = 0;
static char current_filename[MAX_WS_LENGTH] = "";

static int cx = 0, cy = 0;             
static int target_rx = 0;              
static int row_off = 0, col_off = 0;   
static int screen_rows = 24, screen_cols = 80;
#ifndef DEFAULT_COLOR_MODE
#define DEFAULT_COLOR_MODE 1
#endif
static int color_mode = DEFAULT_COLOR_MODE;

static char *ws_clipboard = NULL;

static void ws_set_clipboard(const char* text) {
    if (ws_clipboard) free(ws_clipboard);
    ws_clipboard = text ? plat_strdup(text) : NULL;
}

static char* ws_get_clipboard(void) {
    return ws_clipboard ? plat_strdup(ws_clipboard) : NULL;
}
static int help_active = 0;
static int sel_active = 0;
static int sel_start_r = 0, sel_start_c = 0;
static int sel_end_r = 0, sel_end_c = 0;

static void update_sel_end(int r, int c) {
    if (!sel_active) {
        sel_start_r = cy;
        sel_start_c = cx;
        sel_active = 1;
    }
    sel_end_r = r;
    sel_end_c = c;
}
static void clear_sel(void) { sel_active = 0; }

static void get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (sel_start_r < sel_end_r || (sel_start_r == sel_end_r && sel_start_c <= sel_end_c)) {
        *r1 = sel_start_r; *c1 = sel_start_c;
        *r2 = sel_end_r; *c2 = sel_end_c;
    } else {
        *r1 = sel_end_r; *c1 = sel_end_c;
        *r2 = sel_start_r; *c2 = sel_start_c;
    }
}

static char* get_selected_text_ws(void) {
    if (!sel_active) return NULL;
    int r1, c1, r2, c2;
    get_sel_bounds(&r1, &c1, &r2, &c2);
    char *buf = malloc(65536);
    if (!buf) return NULL;
    buf[0] = '\0';
    int pos = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : (int)strlen(text_buffer[r]);
        for (int i = start; i < end; i++) {
            buf[pos++] = text_buffer[r][i];
        }
        if (r < r2) buf[pos++] = '\n';
    }
    buf[pos] = '\0';
    return buf;
}

static void delete_selected_text_ws(void) {
    if (!sel_active) return;
    int r1, c1, r2, c2;
    get_sel_bounds(&r1, &c1, &r2, &c2);
    char rem[MAX_WS_LENGTH];
    strcpy(rem, text_buffer[r2] + c2);
    text_buffer[r1][c1] = '\0';
    strncat(text_buffer[r1], rem, MAX_WS_LENGTH - strlen(text_buffer[r1]) - 1);
    int lines_to_del = r2 - r1;
    if (lines_to_del > 0) {
        for (int i = r1 + 1; i < num_lines - lines_to_del; i++) {
            strcpy(text_buffer[i], text_buffer[i + lines_to_del]);
        }
        num_lines -= lines_to_del;
    }
    cy = r1; cx = c1;
    sel_active = 0;
}

static void insert_newline_ws(void) {
    if (num_lines >= MAX_WS_LINES) return;
    for (int i = num_lines; i > cy; i--) {
        strcpy(text_buffer[i], text_buffer[i-1]);
    }
    num_lines++;
    text_buffer[cy + 1][0] = '\0';
}

static void insert_text_at_cursor_ws(const char *text) {
    if (sel_active) delete_selected_text_ws();
    const char *p = text;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            if (*p == '\r' && *(p+1) == '\n') p++; 
            insert_newline_ws();
            cy++; cx = 0; p++;
        } else {
            if (strlen(text_buffer[cy]) < MAX_WS_LENGTH - 1) {
                memmove(&text_buffer[cy][cx + 1], &text_buffer[cy][cx], strlen(text_buffer[cy]) - cx + 1);
                text_buffer[cy][cx] = *p;
                cx++;
            }
            p++;
        }
    }
}

static int prefix_k = 0;

static const WsCallbacks *ws_cb = NULL;

#define MAX_RENDER_BUF 1024

/* --- Output Helpers --- */
static void ws_print(const char *fmt, ...)
{
    char buf[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (ws_cb != NULL && ws_cb->write_str != NULL) {
        ws_cb->write_str(buf);
    } else {
        printf("%s", buf);
        fflush(stdout);
    }
}

static void get_terminal_size(int *rows, int *cols) {
    *rows = 24; 
    *cols = 80;
    if (ws_cb != NULL && ws_cb->get_terminal_size != NULL) {
        ws_cb->get_terminal_size(cols, rows);
    }
    if (*rows < 2) *rows = 24; 
}

/* --- Core Editor Functions --- */
static void load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        num_lines = 0;
        while (num_lines < MAX_WS_LINES && fgets(text_buffer[num_lines], MAX_WS_LENGTH, file)) {
            size_t len = strlen(text_buffer[num_lines]);
            if (len > 0 && (text_buffer[num_lines][len - 1] == '\n' || text_buffer[num_lines][len - 1] == '\r')) {
                text_buffer[num_lines][len - 1] = '\0';
                if (len > 1 && text_buffer[num_lines][len - 2] == '\r') {
                    text_buffer[num_lines][len - 2] = '\0';
                }
            }
            num_lines++;
        }
        fclose(file);
    }
    strncpy(current_filename, filename, MAX_WS_LENGTH - 1);
    current_filename[MAX_WS_LENGTH - 1] = '\0';
    
    if (num_lines == 0) {
        num_lines = 1;
        text_buffer[0][0] = '\0';
    }
}

static void save_file(void) {
    int i;
    FILE *file = fopen(current_filename, "w");
    if (!file) return;
    for (i = 0; i < num_lines; i++) {
        fprintf(file, "%s\n", text_buffer[i]);
    }
    fclose(file);
}

static void insert_char(int c) {
    int len = (int)strlen(text_buffer[cy]);
    if (len >= MAX_WS_LENGTH - 1) return;
    memmove(&text_buffer[cy][cx + 1], &text_buffer[cy][cx], len - cx + 1);
    text_buffer[cy][cx] = (char)c;
    cx++;
}

static void insert_newline(void) {
    int i;
    if (num_lines >= MAX_WS_LINES) return;
    for (i = num_lines; i > cy + 1; i--) {
        strcpy(text_buffer[i], text_buffer[i - 1]);
    }
    strcpy(text_buffer[cy + 1], text_buffer[cy] + cx);
    text_buffer[cy][cx] = '\0';
    num_lines++;
    cy++;
    cx = 0;
}

static void handle_backspace(void) {
    int i, len, prev_len, cur_len;
    if (cx > 0) {
        len = (int)strlen(text_buffer[cy]);
        memmove(&text_buffer[cy][cx - 1], &text_buffer[cy][cx], len - cx + 1);
        cx--;
    } else if (cy > 0) {
        prev_len = (int)strlen(text_buffer[cy - 1]);
        cur_len = (int)strlen(text_buffer[cy]);
        if (prev_len + cur_len < MAX_WS_LENGTH) {
            strcat(text_buffer[cy - 1], text_buffer[cy]);
            for (i = cy; i < num_lines - 1; i++) {
                strcpy(text_buffer[i], text_buffer[i + 1]);
            }
            num_lines--;
            cy--;
            cx = prev_len;
        }
    }
}

static int get_render_x(int row, int physical_x) {
    int rx = 0, j;
    for (j = 0; j < physical_x && text_buffer[row][j] != '\0'; j++) {
        if (text_buffer[row][j] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

static int get_physical_x(int row, int target_rx) {
    int rx = 0, j;
    for (j = 0; text_buffer[row][j] != '\0'; j++) {
        int next_rx = rx;
        if (text_buffer[row][j] == '\t') next_rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        next_rx++;
        if (next_rx > target_rx) return j;
        rx = next_rx;
    }
    return j;
}

static void render_row(int row, char *out_buf) {
    int j = 0, idx = 0;
    while (text_buffer[row][j] != '\0' && idx < (MAX_RENDER_BUF - 1)) {
        if (text_buffer[row][j] == '\t') {
            out_buf[idx++] = ' ';
            while (idx % TAB_STOP != 0 && idx < (MAX_RENDER_BUF - 1)) out_buf[idx++] = ' ';
        } else {
            out_buf[idx++] = text_buffer[row][j];
        }
        j++;
    }
    out_buf[idx] = '\0';
}

static void refresh_screen(void) {
    int y, y_start = 0, print_len, len, file_row, rx;
    char status_bar[120];
    char r_buf[MAX_RENDER_BUF];
    
    get_terminal_size(&screen_rows, &screen_cols);
    
    ws_print("\x1b[?25l"); 
    ws_print("\x1b[H");    
    
    if (color_mode) ws_print("\x1b[44;37m");
    else ws_print("\x1b[0m");
    
    if (help_active) {
        if (color_mode) ws_print("\x1b[47;30m");
        else ws_print("\x1b[7m"); 
        ws_print("----------------- Help (^K^H toggles) ----------------------\r\n");
        ws_print(" ^S = Left | ^D = Right | ^E = Up | ^X = Down               \r\n");
        ws_print(" ^K^D = Save & Exit | ^K^X = Save & Execute | ^K^Q = Quit \r\n");
        if (color_mode) ws_print("\x1b[44;37m");
        else ws_print("\x1b[m"); 
        y_start = 4;
    }
    
    for (y = y_start; y < screen_rows - 1; y++) {
        file_row = row_off + (y - y_start);
        if (file_row < num_lines) {
            render_row(file_row, r_buf);
            len = (int)strlen(r_buf);
            if (len > col_off) {
                print_len = len - col_off;
                if (print_len > screen_cols) print_len = screen_cols;
                for (int i = col_off; i < col_off + print_len; i++) {
                    int in_sel = 0;
                    if (sel_active) {
                        int r1, c1, r2, c2;
                        get_sel_bounds(&r1, &c1, &r2, &c2);
                        if (file_row > r1 && file_row < r2) in_sel = 1;
                        else if (file_row == r1 && file_row == r2 && i >= c1 && i < c2) in_sel = 1;
                        else if (file_row == r1 && file_row < r2 && i >= c1) in_sel = 1;
                        else if (file_row == r2 && file_row > r1 && i < c2) in_sel = 1;
                    }
                    if (in_sel) {
                        if (color_mode) ws_print("\x1b[47;30m%c\x1b[44;37m", r_buf[i]);
                        else ws_print("\x1b[7m%c\x1b[27m", r_buf[i]);
                    }
                    else ws_print("%c", r_buf[i]);
                }
            }
        } else {
        }
        ws_print("\x1b[K\r\n"); 
    }
    
    if (color_mode) ws_print("\x1b[47;30m");
    else ws_print("\x1b[7m");
    sprintf(status_bar, " %s%s | File: %s | %d:%d ", 
            prefix_k ? "^K " : "", 
            help_active ? "" : "(Press ^K^H for Help)", 
            current_filename[0] ? current_filename : "NEW FILE", 
            cy + 1, num_lines);
            
    len = (int)strlen(status_bar);
    ws_print("%s", status_bar);
    for (y = len; y < screen_cols; y++) ws_print(" "); 
    if (color_mode) ws_print("\x1b[44;37m");
    else ws_print("\x1b[m");
    
    rx = get_render_x(cy, cx);
    ws_print("\x1b[%d;%dH", (cy - row_off) + y_start + 1, (rx - col_off) + 1);
    ws_print("\x1b[?25h"); 
}

static int internal_read_key(void) {
    if (!ws_cb || !ws_cb->read_char) return '\0';

    char c = ws_cb->read_char();
    if (c == '\x1b') {
        char seq1 = 0;
        if (ws_cb->read_char_nb) {
            int timeout = 0;
            while ((seq1 = ws_cb->read_char_nb()) == 0 && timeout < 10) {
                platform_sleep_ms(5);
                timeout++;
            }
            if (seq1 == 0) return 27;
        } else {
            seq1 = ws_cb->read_char();
        }
        char seq2 = ws_cb->read_char();
        
        if (seq1 == '[') {
            if (seq2 >= '0' && seq2 <= '9') {
                char seq3 = ws_cb->read_char();
                if (seq3 == '~') {
                    switch (seq2) {
                        case '1': return HOME_KEY;
                        case '2': return INS_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                } else if (seq2 == '2' && seq3 == '1') {
                    char seq4 = ws_cb->read_char();
                    if (seq4 == '~') return KEY_F10;
                }
            } else {
                switch (seq2) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq1 == 'O') {
            switch (seq2) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    }
    return c;
}

int ws_start(const char *filename, const WsCallbacks *callbacks) {
    int c, len, cur_len, next_len, visible_rows, rx, i;
    int moved_vertically = 0;
    
    if (!callbacks || !callbacks->read_char || !callbacks->execute_buffer) {
        return 1;
    }
    ws_cb = callbacks;
    
    if (filename && filename[0] != '\0') {
        load_file(filename);
    } else {
        num_lines = 1;
        text_buffer[0][0] = '\0';
    }
    
    while (1) {
        refresh_screen();
        c = internal_read_key();
        moved_vertically = 0;
        
        if (prefix_k) {
            if (c == 8 || c == 'h' || c == 'H') {         
                help_active = !help_active;
            } else if (c == 'v' || c == 'V') {
                color_mode = !color_mode;
            } else if (c == 4 || c == 'd' || c == 'D') {  
                if (current_filename[0]) save_file();
                break;
            } else if (c == 24 || c == 'x' || c == 'X') {
                if (current_filename[0]) save_file();
                const char *line_ptrs[MAX_WS_LINES];
                for (i = 0; i < num_lines; i++) {
                    line_ptrs[i] = text_buffer[i];
                }
                
                ws_print("\x1b[2J\x1b[H"); 
                
                callbacks->execute_buffer(line_ptrs, num_lines, callbacks->ctx);
                
                ws_print("\r\n[Press ANY KEY to return to WordStar]\r\n");
                callbacks->read_char();
                /* Reset cursor and offsets because execution might have changed screen size or state */
                cx = 0; cy = 0; row_off = 0; col_off = 0;
                continue;
            } else if (c == 17 || c == 'q' || c == 'Q') { 
                break;
            }
            prefix_k = 0;
            continue;
        }
        
        
        if (c == KEY_F10) { color_mode = !color_mode; }
        else if (c == SHIFT_ARROW_UP) { update_sel_end(cy > 0 ? cy - 1 : 0, cx); cy = sel_end_r; }
        else if (c == SHIFT_ARROW_DOWN) { update_sel_end(cy < num_lines - 1 ? cy + 1 : num_lines - 1, cx); cy = sel_end_r; }
        else if (c == SHIFT_ARROW_LEFT) { 
            update_sel_end(cy, cx > 0 ? cx - 1 : 0); 
            if (cx == 0 && cy > 0) update_sel_end(cy - 1, (int)strlen(text_buffer[cy-1]));
            cx = sel_end_c; cy = sel_end_r; 
        }
        else if (c == SHIFT_ARROW_RIGHT) { 
            update_sel_end(cy, cx < (int)strlen(text_buffer[cy]) ? cx + 1 : cx); 
            if (cx == (int)strlen(text_buffer[cy]) && cy < num_lines - 1) update_sel_end(cy + 1, 0);
            cx = sel_end_c; cy = sel_end_r; 
        }
        else if (c == KEY_CTRL_INS) {
            char *txt = get_selected_text_ws();
            if (txt) { ws_set_clipboard(txt); free(txt); clear_sel(); }
        }
        else if (c == KEY_SHIFT_DEL) {
            char *txt = get_selected_text_ws();
            if (txt) { ws_set_clipboard(txt); free(txt); delete_selected_text_ws(); }
        }
        else if (c == KEY_SHIFT_INS) {
            char *txt = ws_get_clipboard();
            if (txt) { insert_text_at_cursor_ws(txt); free(txt); }
        }
        else if (c == ARROW_UP || c == ARROW_DOWN || c == ARROW_LEFT || c == ARROW_RIGHT || c == HOME_KEY || c == END_KEY || c == PAGE_UP || c == PAGE_DOWN) {
            clear_sel();
        }
        else {
            if ((c >= 32 && c <= 126) || c == 8 || c == 127 || c == 13 || c == DEL_KEY) clear_sel();
        }

        switch (c) {
            case 11: /* ^K Prefix */
                prefix_k = 1;
                break;
            case 5: /* ^E Up */
            case ARROW_UP:
                if (cy > 0) { cy--; moved_vertically = 1; }
                break;
            case 24: /* ^X Down */
            case ARROW_DOWN:
                if (cy < num_lines - 1) { cy++; moved_vertically = 1; }
                break;
            case 19: /* ^S Left */
            case ARROW_LEFT:
                if (cx > 0) cx--;
                else if (cy > 0) { cy--; cx = (int)strlen(text_buffer[cy]); }
                break;
            case 4: /* ^D Right */
            case ARROW_RIGHT:
                if (cx < (int)strlen(text_buffer[cy])) cx++;
                else if (cy < num_lines - 1) { cy++; cx = 0; }
                break;
            case PAGE_UP:
                cy -= (screen_rows - (help_active ? 4 : 1));
                if (cy < 0) cy = 0;
                moved_vertically = 1;
                break;
            case PAGE_DOWN:
                cy += (screen_rows - (help_active ? 4 : 1));
                if (cy >= num_lines) cy = num_lines - 1;
                moved_vertically = 1;
                break;
            case HOME_KEY:
                cx = 0;
                break;
            case END_KEY:
                cx = (int)strlen(text_buffer[cy]);
                break;
            case DEL_KEY:
                len = (int)strlen(text_buffer[cy]);
                if (cx < len) {
                    memmove(&text_buffer[cy][cx], &text_buffer[cy][cx + 1], len - cx);
                } else if (cy < num_lines - 1) {
                    cur_len = (int)strlen(text_buffer[cy]);
                    next_len = (int)strlen(text_buffer[cy + 1]);
                    if (cur_len + next_len < MAX_WS_LENGTH) {
                        strcat(text_buffer[cy], text_buffer[cy + 1]);
                        for (i = cy + 1; i < num_lines - 1; i++) {
                            strcpy(text_buffer[i], text_buffer[i + 1]);
                        }
                        num_lines--;
                    }
                }
                break;
            case INS_KEY:
            case KEY_F10:
                break;
            case 10:
            case 13: 
                insert_newline();
                break;
            case 8:
            case 127: 
                handle_backspace();
                break;
            default:
                if ((c >= 32 && c <= 126) || c == '\t') {
                    insert_char(c);
                }
                break;
        }
        
        len = (int)strlen(text_buffer[cy]);
        if (moved_vertically) {
            cx = get_physical_x(cy, target_rx);
        } else {
            if (cx > len) cx = len;
            target_rx = get_render_x(cy, cx);
        }
        
        visible_rows = screen_rows - (help_active ? 5 : 1);
        if (cy < row_off) row_off = cy;
        if (cy >= row_off + visible_rows) row_off = cy - visible_rows + 1;
        
        rx = get_render_x(cy, cx);
        if (rx < col_off) col_off = rx;
        if (rx >= col_off + screen_cols) col_off = rx - screen_cols + 1;
    }
    
    ws_print("\x1b[0m\x1b[2J\x1b[H"); 
    return 0;
}
