/*
 * vi.c - Bare minimum vi-style visual text editor
 * Strict C89, compiles cleanly on Linux, Windows 11, and FreeDOS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../mod_vi.h"

/* --- Extended Key Codes --- */
#define KEY_UP    1000
#define KEY_DOWN  1001
#define KEY_LEFT  1002
#define KEY_RIGHT 1003
#define KEY_HOME  1004
#define KEY_END   1005
#define KEY_PGUP  1006
#define KEY_PGDN  1007
#define KEY_INS   1008
#define KEY_DEL   1009
#define SHIFT_ARROW_UP 1100
#define SHIFT_ARROW_DOWN 1101
#define SHIFT_ARROW_LEFT 1102
#define SHIFT_ARROW_RIGHT 1103
#define KEY_CTRL_INS 1104
#define KEY_SHIFT_DEL 1105
#define KEY_SHIFT_INS 1106

#define KEY_F1    1011
#define KEY_F2    1012
#define KEY_F3    1013
#define KEY_F4    1014
#define KEY_F5    1015
#define KEY_F10   1020

char text_buffer[MAX_VI_LINES][MAX_VI_LENGTH];
char current_filename[MAX_VI_LENGTH] = "";
char cmd_buffer[MAX_VI_LENGTH] = "";

int current_lines = 0;
int cursor_r = 0, cursor_c = 0;
int row_offset = 0;
int mode = 0; /* 0: Normal, 1: Insert, 2: Command */
int cmd_len = 0;
int running = 1;
int screen_rows = 24;

static const ViCallbacks *vi_cb = NULL;

static void vi_print(const char *fmt, ...)
{
    char buf[1024];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (vi_cb != NULL && vi_cb->write_str != NULL) {
        vi_cb->write_str(buf);
    } else {
        printf("%s", buf);
        fflush(stdout);
    }
}

void get_terminal_size(void) {
    if (vi_cb && vi_cb->get_terminal_size) {
        int c, r;
        vi_cb->get_terminal_size(&c, &r);
        screen_rows = r;
    } else {
        screen_rows = 24;
    }
    if (screen_rows < 5) screen_rows = 24;
}

#ifndef DEFAULT_COLOR_MODE
#define DEFAULT_COLOR_MODE 1
#endif

static int color_mode = DEFAULT_COLOR_MODE;
static int pushed_char = -1;
static int sel_active = 0;
static int sel_start_r = 0, sel_start_c = 0;
static int sel_end_r = 0, sel_end_c = 0;

static void update_sel_end(int r, int c) {
    if (!sel_active) {
        sel_start_r = cursor_r;
        sel_start_c = cursor_c;
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

static char* get_selected_text_vi(void) {
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

static void delete_selected_text_vi(void) {
    if (!sel_active) return;
    int r1, c1, r2, c2;
    get_sel_bounds(&r1, &c1, &r2, &c2);
    char rem[MAX_VI_LENGTH];
    strcpy(rem, text_buffer[r2] + c2);
    text_buffer[r1][c1] = '\0';
    strncat(text_buffer[r1], rem, MAX_VI_LENGTH - strlen(text_buffer[r1]) - 1);
    int lines_to_del = r2 - r1;
    if (lines_to_del > 0) {
        for (int i = r1 + 1; i < current_lines - lines_to_del; i++) {
            strcpy(text_buffer[i], text_buffer[i + lines_to_del]);
        }
        current_lines -= lines_to_del;
    }
    cursor_r = r1; cursor_c = c1;
    sel_active = 0;
}

static void insert_newline_vi(void) {
    if (current_lines >= MAX_VI_LINES) return;
    for (int i = current_lines; i > cursor_r; i--) {
        strcpy(text_buffer[i], text_buffer[i-1]);
    }
    current_lines++;
    text_buffer[cursor_r + 1][0] = '\0';
}

static void insert_text_at_cursor_vi(const char *text) {
    if (sel_active) delete_selected_text_vi();
    const char *p = text;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            if (*p == '\r' && *(p+1) == '\n') p++; 
            insert_newline_vi();
            cursor_r++;
            cursor_c = 0;
            p++;
        } else {
            if (strlen(text_buffer[cursor_r]) < MAX_VI_LENGTH - 1) {
                memmove(&text_buffer[cursor_r][cursor_c + 1], &text_buffer[cursor_r][cursor_c], strlen(text_buffer[cursor_r]) - cursor_c + 1);
                text_buffer[cursor_r][cursor_c] = *p;
                cursor_c++;
            }
            p++;
        }
    }
}



int get_input(void) {
    int c;
    if (!vi_cb || !vi_cb->read_char) return '\0';

    if (pushed_char != -1) {
        c = pushed_char;
        pushed_char = -1;
        return c;
    }

    c = vi_cb->read_char();
    
    if (c == 27) {
        char seq1, seq2, seq3;
        
        /* If we have a kbhit function we could check it here. For now, 
           since we only have read_char which blocks, we will block.
           If it's not an escape sequence, we push it back. */
        seq1 = vi_cb->read_char();
        if (seq1 == 27) {
            return 27; /* Fallback for double ESC */
        }
        
        if (seq1 == '[' || seq1 == 'O') {
            seq2 = vi_cb->read_char();
            if (seq1 == '[') {
                if (seq2 >= '0' && seq2 <= '9') {
                    seq3 = vi_cb->read_char();
                    if (seq3 == '~') {
                        switch(seq2) {
                            case '1': return KEY_HOME;
                            case '2': return KEY_INS;
                            case '3': return KEY_DEL;
                            case '4': return KEY_END;
                            case '5': return KEY_PGUP;
                            case '6': return KEY_PGDN;
                            case '7': return KEY_HOME;
                            case '8': return KEY_END;
                        }
                    } else if (seq2 == '1') {
                        /* Handle F1-F5: Esc[11~ etc */
                        if (seq3 >= '1' && seq3 <= '5') {
                            char seq4 = vi_cb->read_char();
                            if (seq4 == '~') {
                                switch(seq3) {
                                    case '1': return KEY_F1;
                                    case '2': return KEY_F2;
                                    case '3': return KEY_F3;
                                    case '4': return KEY_F4;
                                    case '5': return KEY_F5;
                                }
                            }
                            if (seq2 == '2' && seq3 == '1' && seq4 == '~') return KEY_F10;
                            if (seq2 == '2' && seq4 == '~') {
                                if (seq3 == '1') return KEY_F10;
                            }
                        }
                    } else if (seq3 == ';') {
                        char seq4 = vi_cb->read_char();
                        char seq5 = vi_cb->read_char();
                        if (seq2 == '1' && seq4 == '2') {
                            if (seq5 == 'A') return SHIFT_ARROW_UP;
                            if (seq5 == 'B') return SHIFT_ARROW_DOWN;
                            if (seq5 == 'C') return SHIFT_ARROW_RIGHT;
                            if (seq5 == 'D') return SHIFT_ARROW_LEFT;
                        }
                        if (seq2 == '2' && seq4 == '5' && seq5 == '~') return KEY_CTRL_INS;
                        if (seq2 == '3' && seq4 == '2' && seq5 == '~') return KEY_SHIFT_DEL;
                        if (seq2 == '2' && seq4 == '2' && seq5 == '~') return KEY_SHIFT_INS;
                    }
                } else {
                    switch(seq2) {
                        case 'A': return KEY_UP;
                        case 'B': return KEY_DOWN;
                        case 'C': return KEY_RIGHT;
                        case 'D': return KEY_LEFT;
                        case 'H': return KEY_HOME;
                        case 'F': return KEY_END;
                    }
                }
            } else if (seq1 == 'O') {
                switch(seq2) {
                    case 'H': return KEY_HOME;
                    case 'F': return KEY_END;
                    case 'P': return KEY_F1;
                    case 'Q': return KEY_F2;
                    case 'R': return KEY_F3;
                    case 'S': return KEY_F4;
                }
            }
        } else {
            pushed_char = seq1;
            return 27;
        }
        return 27;
    }
    return c;
}

void load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    current_lines = 0;
    if (file) {
        while (current_lines < MAX_VI_LINES && fgets(text_buffer[current_lines], MAX_VI_LENGTH, file)) {
            int len = (int)(int)strlen(text_buffer[current_lines]);
            while (len > 0 && (text_buffer[current_lines][len - 1] == '\n' || text_buffer[current_lines][len - 1] == '\r')) {
                text_buffer[current_lines][len - 1] = '\0';
                len--;
            }
            current_lines++;
        }
        fclose(file);
    }
    if (current_lines == 0) {
        text_buffer[0][0] = '\0';
        current_lines = 1;
    }
    strncpy(current_filename, filename, MAX_VI_LENGTH - 1);
    current_filename[MAX_VI_LENGTH - 1] = '\0';
}

void save_file(void) {
    FILE *file = fopen(current_filename, "w");
    if (file) {
        int i;
        for (i = 0; i < current_lines; i++) {
            fprintf(file, "%s\n", text_buffer[i]);
        }
        fclose(file);
    }
}

void fix_cursor(void) {
    int len;
    if (cursor_r < 0) cursor_r = 0;
    if (cursor_r >= current_lines) cursor_r = current_lines - 1;

    len = (int)(int)strlen(text_buffer[cursor_r]);
    if (mode == 0) {
        if (cursor_c >= len && len > 0) cursor_c = len - 1;
    } else {
        if (cursor_c > len) cursor_c = len;
    }
    if (cursor_c < 0) cursor_c = 0;

    if (cursor_r < row_offset) row_offset = cursor_r;
    if (cursor_r >= row_offset + screen_rows - 1) row_offset = cursor_r - (screen_rows - 2);
}

void render_screen(void) {
    int i;
    vi_print("\x1b[?25l"); /* Hide cursor momentarily to prevent flicker */
    vi_print("\x1b[H");
    
    if (color_mode) {
        vi_print("\x1b[44;37m"); /* Blue background, white text */
    } else {
        vi_print("\x1b[0m"); /* Default monochrome */
    }
    
    for (i = 0; i < screen_rows - 1; i++) {
        int line_idx = row_offset + i;
        if (line_idx < current_lines) {
            if (!sel_active || color_mode) { /* Standard draw or color draw */
                if (color_mode) vi_print("\x1b[44;37m");
                for (int j = 0; j < (int)strlen(text_buffer[line_idx]); j++) {
                    int in_sel = 0;
                    if (sel_active) {
                        int r1, c1, r2, c2;
                        get_sel_bounds(&r1, &c1, &r2, &c2);
                        if (line_idx > r1 && line_idx < r2) in_sel = 1;
                        else if (line_idx == r1 && line_idx == r2 && j >= c1 && j < c2) in_sel = 1;
                        else if (line_idx == r1 && line_idx < r2 && j >= c1) in_sel = 1;
                        else if (line_idx == r2 && line_idx > r1 && j < c2) in_sel = 1;
                    }
                    if (in_sel) {
                        if (color_mode) vi_print("\x1b[47;30m%c\x1b[44;37m", text_buffer[line_idx][j]);
                        else vi_print("\x1b[7m%c\x1b[27m", text_buffer[line_idx][j]);
                    } else {
                        vi_print("%c", text_buffer[line_idx][j]);
                    }
                }
                if (color_mode) vi_print("\x1b[K\r\n");
                else vi_print("\x1b[K\r\n");
            } else {
                /* Fast path for monochrome no-selection */
                vi_print("%s\x1b[K\r\n", text_buffer[line_idx]);
            }
        } else {
            if (color_mode) vi_print("\x1b[36m~\x1b[37m\x1b[K\r\n");
            else vi_print("~\x1b[K\r\n");
        }
    }
    
    /* Dynamic Status Line */
    if (color_mode) vi_print("\x1b[47;30m");
    else vi_print("\x1b[7m");
    if (mode == 2) {
        vi_print(":%s\x1b[K", cmd_buffer);
    } else if (mode == 1) {
        vi_print("-- INSERT --\x1b[K");
    } else {
        vi_print("\"%s\" %d:%d\x1b[K", current_filename[0] ? current_filename : "NEW", cursor_r + 1, current_lines);
    }
    vi_print("\x1b[0m");
    
    /* Set Physical Cursor Position */
    if (mode == 2) {
        vi_print("\x1b[%d;%dH", screen_rows, cmd_len + 2);
    } else {
        vi_print("\x1b[%d;%dH", (cursor_r - row_offset) + 1, cursor_c + 1);
    }
    vi_print("\x1b[?25h");
    fflush(stdout);
}

void display_help(void) {
    vi_print("\x1b[2J\x1b[H"); 
    vi_print("--- vi Built-in Help ---\r\n\n");
    vi_print(" NORMAL MODE:\r\n");
    vi_print("   h,j,k,l / Arrows : Move cursor\r\n");
    vi_print("   Home / End       : Jump to start/end of line\r\n");
    vi_print("   PgUp / PgDn      : Page up / Page down\r\n");
    vi_print("   0, $             : Jump to start/end of line\r\n");
    vi_print("   i, a, I, A       : Enter Insert Mode\r\n");
    vi_print("   o, O             : Insert new line / Insert Mode\r\n");
    vi_print("   x, Del           : Delete character under cursor\r\n");
    vi_print("   dd               : Delete current line\r\n");
    vi_print("   :                : Enter Command Mode\r\n\n");
    vi_print(" INSERT MODE:\r\n");
    vi_print("   Esc              : Return to Normal Mode\r\n");
    vi_print("   Enter            : Split line\r\n");
    vi_print("   Backspace        : Delete char or merge lines\r\n\n");
    vi_print(" COMMAND MODE:\r\n");
    vi_print("   :w               : Save file\r\n");
    vi_print("   :w <file>        : Save to new file\r\n");
    vi_print("   :q, :q!          : Quit / Quit without saving\r\n");
    vi_print("   :wq, :x          : Save and Quit\r\n");
    vi_print("   :?               : Show this help screen\r\n\n");
    vi_print(" F-KEYS:\r\n");
    vi_print("   F1               : Show this help screen\r\n");
    vi_print("   F3               : Load file (Wait for prompt)\r\n");
    vi_print("   F4               : Exit\r\n\n");
    vi_print("Press any key to return...");
    fflush(stdout);
    get_input(); /* Block execution until user returns */
}

void handle_normal(int c) {
    static int pending_d = 0;
    int i;
    
    if (pending_d) {
        pending_d = 0;
        if (c == 'd') {
            if (current_lines > 1) {
                for (i = cursor_r; i < current_lines - 1; i++) {
                    strcpy(text_buffer[i], text_buffer[i + 1]);
                }
                current_lines--;
                if (cursor_r >= current_lines) cursor_r = current_lines - 1;
            } else {
                text_buffer[0][0] = '\0';
                cursor_c = 0;
            }
            return;
        }
    }

    switch (c) {
        case 'h': case KEY_LEFT:  cursor_c--; break;
        case 'l': case KEY_RIGHT: cursor_c++; break;
        case 'j': case KEY_DOWN:  cursor_r++; break;
        case 'k': case KEY_UP:    cursor_r--; break;
        case KEY_HOME: case '0':  cursor_c = 0; break;
        case KEY_END:  case '$':  cursor_c = (int)(int)strlen(text_buffer[cursor_r]); break;
        case KEY_PGUP: cursor_r -= (screen_rows - 2); break;
        case KEY_PGDN: cursor_r += (screen_rows - 2); break;
        
        case 'i': case KEY_INS: mode = 1; break;
        case 'a': cursor_c++; mode = 1; break;
        case 'I': cursor_c = 0; mode = 1; break;
        case 'A': cursor_c = (int)(int)strlen(text_buffer[cursor_r]); mode = 1; break;
        
        case 'x': case KEY_DEL:
            if (text_buffer[cursor_r][cursor_c] != '\0') {
                for (i = cursor_c; text_buffer[cursor_r][i]; i++) {
                    text_buffer[cursor_r][i] = text_buffer[cursor_r][i + 1];
                }
            }
            break;
            
        case 'd': pending_d = 1; break;
        case 'o':
            if (current_lines < MAX_VI_LINES) {
                for (i = current_lines; i > cursor_r + 1; i--) {
                    strcpy(text_buffer[i], text_buffer[i - 1]);
                }
                cursor_r++;
                text_buffer[cursor_r][0] = '\0';
                current_lines++;
                mode = 1;
                cursor_c = 0;
            }
            break;
        case 'O':
            if (current_lines < MAX_VI_LINES) {
                for (i = current_lines; i > cursor_r; i--) {
                    strcpy(text_buffer[i], text_buffer[i - 1]);
                }
                text_buffer[cursor_r][0] = '\0';
                current_lines++;
                mode = 1;
                cursor_c = 0;
            }
            break;
            
        case ':':
            mode = 2;
            cmd_len = 0;
            cmd_buffer[0] = '\0';
            break;
    }
}

void handle_insert(int c) {
    int i, len;
    
    if (c == 27) { /* Escape */
        mode = 0;
        if (cursor_c > 0) cursor_c--;
    } else if (c == KEY_UP) { cursor_r--; }
    else if (c == KEY_DOWN) { cursor_r++; }
    else if (c == KEY_LEFT) { cursor_c--; }
    else if (c == KEY_RIGHT) { cursor_c++; }
    else if (c == KEY_HOME) { cursor_c = 0; }
    else if (c == KEY_END) { cursor_c = (int)(int)strlen(text_buffer[cursor_r]); }
    else if (c == KEY_PGUP) { cursor_r -= (screen_rows - 2); }
    else if (c == KEY_PGDN) { cursor_r += (screen_rows - 2); }
    else if (c == KEY_DEL) {
        if (text_buffer[cursor_r][cursor_c] != '\0') {
            for (i = cursor_c; text_buffer[cursor_r][i]; i++) {
                text_buffer[cursor_r][i] = text_buffer[cursor_r][i + 1];
            }
        } else if (cursor_r < current_lines - 1) {
            len = (int)(int)strlen(text_buffer[cursor_r]);
            if (len + (int)strlen(text_buffer[cursor_r + 1]) < MAX_VI_LENGTH) {
                memmove(&text_buffer[cursor_r][len], text_buffer[cursor_r + 1], strlen(text_buffer[cursor_r + 1]) + 1);
                for (i = cursor_r + 1; i < current_lines - 1; i++) {
                    memmove(text_buffer[i], text_buffer[i + 1], strlen(text_buffer[i + 1]) + 1);
                }
                current_lines--;
            }
        }
    } else if (c == 10 || c == 13) { /* Enter */
        if (current_lines < MAX_VI_LINES) {
            for (i = current_lines; i > cursor_r; i--) {
                memmove(text_buffer[i], text_buffer[i - 1], strlen(text_buffer[i - 1]) + 1);
            }
            memmove(text_buffer[cursor_r + 1], &text_buffer[cursor_r][cursor_c], strlen(&text_buffer[cursor_r][cursor_c]) + 1);
            text_buffer[cursor_r][cursor_c] = '\0';
            current_lines++;
            cursor_r++;
            cursor_c = 0;
        }
    } else if (c == 8 || c == 127) { /* Backspace */
        if (cursor_c > 0) {
            len = (int)(int)strlen(text_buffer[cursor_r]);
            for (i = cursor_c; i <= len; i++) {
                text_buffer[cursor_r][i - 1] = text_buffer[cursor_r][i];
            }
            cursor_c--;
        } else if (cursor_r > 0) {
            int prev_len = (int)(int)strlen(text_buffer[cursor_r - 1]);
            if (prev_len + (int)(int)strlen(text_buffer[cursor_r]) < MAX_VI_LENGTH) {
                strcat(text_buffer[cursor_r - 1], text_buffer[cursor_r]);
                for (i = cursor_r; i < current_lines - 1; i++) {
                    strcpy(text_buffer[i], text_buffer[i + 1]);
                }
                current_lines--;
                cursor_r--;
                cursor_c = prev_len;
            }
        }
    } else if (c >= 32 && c <= 126) { /* Standard ASCII Characters */
        len = (int)(int)strlen(text_buffer[cursor_r]);
        if (len < MAX_VI_LENGTH - 1) {
            for (i = len; i >= cursor_c; i--) {
                text_buffer[cursor_r][i + 1] = text_buffer[cursor_r][i];
            }
            text_buffer[cursor_r][cursor_c] = (char)c;
            cursor_c++;
        }
    } else if (c == KEY_LEFT) {
        if (cursor_c > 0) cursor_c--;
    } else if (c == KEY_RIGHT) {
        cursor_c++;
    } else if (c == KEY_UP) {
        if (cursor_r > 0) cursor_r--;
    } else if (c == KEY_DOWN) {
        if (cursor_r < current_lines - 1) cursor_r++;
    } else if (c == KEY_HOME) {
        cursor_c = 0;
    } else if (c == KEY_END) {
        cursor_c = (int)strlen(text_buffer[cursor_r]);
    } else if (c == KEY_PGUP) {
        cursor_r -= screen_rows - 2;
        if (cursor_r < 0) cursor_r = 0;
    } else if (c == KEY_PGDN) {
        cursor_r += screen_rows - 2;
        if (cursor_r >= current_lines) cursor_r = current_lines - 1;
    } else if (c == KEY_DEL) {
        int len = (int)strlen(text_buffer[cursor_r]);
        if (cursor_c < len) {
            memmove(&text_buffer[cursor_r][cursor_c], &text_buffer[cursor_r][cursor_c + 1], len - cursor_c);
        }
    } else if (c == KEY_F1) {
        display_help();
    } else if (c == KEY_F2) {
        if (current_filename[0]) save_file();
        else {
            mode = 2;
            strcpy(cmd_buffer, "w ");
            cmd_len = 2;
        }
    } else if (c == KEY_F3) {
        mode = 2;
        strcpy(cmd_buffer, "load ");
        cmd_len = 5;
    } else if (c == KEY_F4) {
        running = 0;
    } else if (c == KEY_F5) {
        const char *line_ptrs[MAX_VI_LINES];
        int i;
        if (current_filename[0]) {
            save_file();
            for (i = 0; i < current_lines; i++) {
                line_ptrs[i] = text_buffer[i];
            }
            vi_cb->execute_buffer(line_ptrs, current_lines, vi_cb->ctx);
            vi_print("\r\n[Press ANY KEY to return to vi]\r\n");
            vi_cb->read_char();
        } else {
            mode = 2;
            strcpy(cmd_buffer, "w ");
            cmd_len = 2;
        }
    }
}

void handle_command(int c) {
    if (c == 27) {
        mode = 0;
    } else if (c == 10 || c == 13) {
        if (strcmp(cmd_buffer, "w") == 0) save_file();
        else if (strncmp(cmd_buffer, "w ", 2) == 0) {
            char new_file[MAX_VI_LENGTH];
            strncpy(new_file, cmd_buffer + 2, MAX_VI_LENGTH - 1);
            new_file[MAX_VI_LENGTH - 1] = '\0';
            strcpy(current_filename, new_file);
            save_file();
        }
        else if (strcmp(cmd_buffer, "q") == 0 || strcmp(cmd_buffer, "q!") == 0) running = 0;
        else if (strcmp(cmd_buffer, "wq") == 0 || strcmp(cmd_buffer, "x") == 0) {
            save_file();
            running = 0;
        }
        else if (strcmp(cmd_buffer, "run!") == 0) {
            const char *line_ptrs[MAX_VI_LINES];
            int i;
            for (i = 0; i < current_lines; i++) {
                line_ptrs[i] = text_buffer[i];
            }
            vi_cb->execute_buffer(line_ptrs, current_lines, vi_cb->ctx);
            vi_print("\r\n[Press ANY KEY to return to vi]\r\n");
            vi_cb->read_char();
        }
        else if (strcmp(cmd_buffer, "run") == 0) {
            const char *line_ptrs[MAX_VI_LINES];
            int i;
            if (current_filename[0]) save_file();
            for (i = 0; i < current_lines; i++) {
                line_ptrs[i] = text_buffer[i];
            }
            vi_cb->execute_buffer(line_ptrs, current_lines, vi_cb->ctx);
            vi_print("\r\n[Press ANY KEY to return to vi]\r\n");
            vi_cb->read_char();
        }
        else if (strncmp(cmd_buffer, "load ", 5) == 0) {
            char new_file[MAX_VI_LENGTH];
            strncpy(new_file, cmd_buffer + 5, MAX_VI_LENGTH - 1);
            new_file[MAX_VI_LENGTH - 1] = '\0';
            strcpy(current_filename, new_file);
            load_file(current_filename);
        }
        else if (strcmp(cmd_buffer, "?") == 0) display_help();
        mode = 0;
    } else if (c == 8 || c == 127) {
        if (cmd_len > 0) cmd_buffer[--cmd_len] = '\0';
        else mode = 0;
    } else if (c >= 32 && c <= 126 && cmd_len < MAX_VI_LENGTH - 1) {
        cmd_buffer[cmd_len++] = (char)c;
        cmd_buffer[cmd_len] = '\0';
    }
}

void exit_editor(void) {
    vi_print("\x1b[2J\x1b[H\x1b[?25h"); /* Restore Screen bounds and physical cursor pointer */
    fflush(stdout);
}

int vi_start(const char *filename, const ViCallbacks *callbacks) {
    int c;
    running = 1;
    if (!callbacks || !callbacks->read_char || !callbacks->execute_buffer) {
        return 1;
    }
    vi_cb = callbacks;

    if (filename && filename[0] != '\0') {
        load_file(filename);
    } else {
        current_lines = 1;
        text_buffer[0][0] = '\0';
    }
    
    vi_print("\x1b[2J\x1b[H"); 
    
    while (running) {
        get_terminal_size(); /* Dynamically re-read layout per user iteration */
        fix_cursor();
        render_screen();
        
        c = get_input();
        if (c == 0) continue;
        
        if (mode == 0) handle_normal(c);
        else if (mode == 1) handle_insert(c);
        else if (mode == 2) handle_command(c);
    }
    
    exit_editor();
    return 0;
}
