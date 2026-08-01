/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

#ifndef STANDALONE_EDITOR
#include "editor/editor.h"
#endif

#ifdef STANDALONE_EDITOR
#include "standalone_runner.h"
#endif
#include "types/version.h"

#define get_terminal_size edit_get_terminal_size
#define reset_term edit_reset_term
#define init_term edit_init_term
#define get_input edit_get_input
#ifndef STANDALONE_EDITOR
#define main mod_edit_main
#endif
#ifdef _MSC_VER
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif
/*
 *
 * VERSION: 3.1.0
 * LICENSE: MIT License
 * COPYLEFT: BASIC++ Community
 *
 * ed.c - a MS-DOS 5.0 / QBASIC EDIT.COM clone
 * Keyboard-driven terminal text editor. Absolutely no mouse required.
 *
 */

/* Removed redundant POSIX macros */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include "platform/platform.h"

#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    #include <poll.h>
#endif

/* Filter to ensure strictly 7-bit ASCII */
static void sanitize_ascii(char* str) {
    if (!str) return;
    char* p = str;
    while (*str) {
        if ((unsigned char)(*str) < 128) {
            *p++ = *str;
        }
        str++;
    }
    *p = '\0';
}

#if defined(_WIN32) || defined(WIN32)
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <conio.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
    #define GETCH _getch
#elif defined(__MSDOS__) || defined(__DOS__)
    #include <conio.h>
    #define GETCH getch
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    static struct termios orig_termios;
#endif

#define TAB_STOP 4

/* --- ANSI DOS EDIT Themes --- */
#define COL_BG_BW    "\x1b[0m"
#define COL_MENU     "\x1b[30;47m"
#define COL_MENU_SEL "\x1b[37;40m"
#define COL_STATUS   "\x1b[30;46m"
#define COL_SHADOW   "\x1b[40m"

/* --- Key Codes --- */
enum editorKey {
    KEY_ESC = 27,
    KEY_ENTER = 13,
    KEY_BACKSPACE = 8,
    KEY_TAB = 9,
    KEY_TIMEOUT = 99,
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
    INS_KEY,
    KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5, KEY_F8, KEY_F9, KEY_F10,
    ALT_F, ALT_E, ALT_S, ALT_D, ALT_R, ALT_H,
    SHIFT_ARROW_UP, SHIFT_ARROW_DOWN, SHIFT_ARROW_LEFT, SHIFT_ARROW_RIGHT,
    KEY_CTRL_INS, KEY_SHIFT_DEL, KEY_SHIFT_INS,
    CTRL_HOME, CTRL_END
};

/* --- Global State --- */
typedef struct {
    char *text;
    int length;
    int capacity;
    bool breakpoint;
} Line;
static Line *text_buffer = NULL;
static int text_buffer_capacity = 0;
static int num_lines = 0;
static char current_filename[4096] = "";
static char clipboard_line[4096] = "";
static char search_term[4096] = "";

#ifndef STANDALONE_EDITOR
static struct VMContext *current_vm = NULL;
#endif
#ifndef STANDALONE_EDITOR
static bool debug_step_mode = false;
#endif

static void oom(void) {
    fprintf(stderr, "\n\nOut of memory!\n");
    return;
}

static void ensure_line_capacity(int row, int needed) {
    if (needed > text_buffer[row].capacity) {
        int new_cap = text_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        char *new_text = realloc(text_buffer[row].text, new_cap);
        if (!new_text) { oom(); return; }
        text_buffer[row].text = new_text;
        text_buffer[row].capacity = new_cap;
    }
}

static void ensure_buffer_capacity(int needed) {
    if (needed > text_buffer_capacity) {
        int new_cap = text_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        Line *new_buf = realloc(text_buffer, new_cap * sizeof(Line));
        if (!new_buf) { oom(); return; }
        text_buffer = new_buf;
        text_buffer_capacity = new_cap;
    }
}

static void insert_empty_line(int row) {
    ensure_buffer_capacity(num_lines + 1);
    for (int i = num_lines; i > row; i--) {
        text_buffer[i] = text_buffer[i - 1];
    }
    text_buffer[row].text = (char *)calloc(1, 128);
    if (!text_buffer[row].text) { oom(); return; }
    text_buffer[row].text[0] = '\0';
    text_buffer[row].length = 0;
    text_buffer[row].capacity = 128;
    num_lines++;
}

static void free_line(int row) {
    if (text_buffer[row].text) {
        free(text_buffer[row].text);
        text_buffer[row].text = NULL;
    }
}


static int cx = 0, cy = 0;
static bool sel_active = false;
static int sel_start_r = 0, sel_start_c = 0;
static int sel_end_r = 0, sel_end_c = 0;

static void update_sel_end(int r, int c) {
    if (!sel_active) {
        sel_start_r = cy;
        sel_start_c = cx;
        sel_active = true;
    }
    sel_end_r = r;
    sel_end_c = c;
}

static void clear_sel(void) {
    sel_active = false;
}
             
static int target_rx = 0;              
static int row_off = 0, col_off = 0;   
static int screen_rows = 24, screen_cols = 80;
static bool exit_editor = false;
static int menu_mode = 0; /* 0: Editor, 1: Menu Bar, 2: Dropdown */
static int menu_col = 0;
static int menu_row = 0;

/* Toggles */
static const char *bright_colors[] = {
    "\x1b[40;97m", /* Black bg, Bright White */
    "\x1b[40;96m", /* Black bg, Bright Cyan */
    "\x1b[40;92m", /* Black bg, Bright Green */
    "\x1b[40;93m", /* Black bg, Bright Yellow */
    "\x1b[40;95m", /* Black bg, Bright Magenta */
    "\x1b[40;91m"  /* Black bg, Bright Red */
};
#define NUM_BRIGHT_COLORS (sizeof(bright_colors)/sizeof(bright_colors[0]))
static int color_index = 0;
static bool display_gutter = false;

#define MAX_RENDER_BUF 2048

/* --- Menus Data Structure --- */
static const char* menu_names[] = { " File ", " Edit ", " Search ", " Display ", " Run ", " Help " };
static int menu_x[] = { 2, 10, 19, 30, 42, 50 };
static const int num_menus = 6;

static const char* file_menu[] = { " New          ", " Open...      ", " Save         ", " Save As...   ", " Exit         " };
static const char* edit_menu[] = { " Cut          ", " Copy         ", " Paste        ", " Clear        " };
static const char* search_menu[]={" Find...      ", " Find Next    " };
static char options_menu[2][20] = { " [COLOR MODE]  ", " [LINE GUTTER] " };
static const char* options_ptrs[] = { options_menu[0], options_menu[1] };
static const char* run_menu[] = { " Start      F5 ", " Step       F8 ", " Toggle Brk F9 " };
static const char* help_menu[] = { " About...     " };

static const char** dropdowns[] = { file_menu, edit_menu, search_menu, options_ptrs, run_menu, help_menu };
static const int drop_sizes[] = { 5, 4, 2, 2, 3, 1 };

static char render_buf[65536];
static int render_buf_pos = 0;

/* --- Terminal I/O --- */
void reset_term(void) {
#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    tcsetattr(0, TCSANOW, &orig_termios);
#endif
}

void init_term(void) {
#if defined(_WIN32) || defined(WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#elif !defined(__MSDOS__) && !defined(__DOS__)
    struct termios raw;
    tcgetattr(0, &orig_termios);
    atexit(reset_term);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &raw);
#endif
}

void get_terminal_size_edit(int *rows, int *cols) {
#if defined(_WIN32) || defined(WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        *rows = 24; *cols = 80;
    }
#elif defined(__MSDOS__) || defined(__DOS__)
    *rows = 25; *cols = 80;
#else
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) != -1 && w.ws_row > 0) {
        *rows = w.ws_row;
        *cols = w.ws_col;
    } else {
        *rows = 24; *cols = 80;
    }
#endif
    if (*rows < 5) *rows = 24;
    if (*cols < 10) *cols = 80;
}

static int read_key_edit(void) {
#if defined(_WIN32) || defined(WIN32)
    DWORD start = GetTickCount();
    while (!_kbhit()) {
        if (GetTickCount() - start > 1000) return KEY_TIMEOUT;
        Sleep(50);
    }
    int c = GETCH();
#elif defined(__MSDOS__) || defined(__DOS__)
    int c = GETCH();
#endif
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    if (c < 0) { exit_editor = true; return 0; }
    if (c == '\r' || c == '\n') return KEY_ENTER;
    if (c == 8 || c == 127) return KEY_BACKSPACE;
    if (c == 0 || c == 224) {
        int seq = GETCH();
        switch (seq) {
            case 72: return ARROW_UP;
            case 80: return ARROW_DOWN;
            case 75: return ARROW_LEFT;
            case 77: return ARROW_RIGHT;
            case 71: return HOME_KEY;
            case 79: return END_KEY;
            case 73: return PAGE_UP;
            case 81: return PAGE_DOWN;
            case 82: return INS_KEY;
            case 83: return DEL_KEY;
            case 119: return CTRL_HOME;
            case 117: return CTRL_END;
            case 59: return KEY_F1;
            case 60: return KEY_F2;
            case 61: return KEY_F3;
            case 62: return KEY_F4;
            case 63: return KEY_F5;
            case 68: return KEY_F10;
            case 66: return KEY_F8;
            case 67: return KEY_F9;
            case 33: return ALT_F;
            case 18: return ALT_E;
            case 31: return ALT_S;
            case 32: return ALT_D;
            case 19: return ALT_R;
            case 35: return ALT_H;
            case 152: return SHIFT_ARROW_UP;
            case 160: return SHIFT_ARROW_DOWN;
            case 164: return KEY_CTRL_INS;
            case 165: return KEY_SHIFT_DEL;
            case 166: return KEY_SHIFT_INS;
        }
        return KEY_ESC;
    }
    return c;
#else
    char c, seq1, seq2, seq3;
    struct pollfd pfd;
    pfd.fd = 0;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, 1000);
    if (ret == 0) return KEY_TIMEOUT;
    if (ret < 0 || read(0, &c, 1) != 1) { exit_editor = true; return 0; }
    if (c == '\r' || c == '\n') return KEY_ENTER;
    if (c == 8 || c == 127) return KEY_BACKSPACE;

    if (c == 27) {
        struct termios raw;
        tcgetattr(0, &raw);
        raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 1;
        tcsetattr(0, TCSANOW, &raw);
        
        if (read(0, &seq1, 1) != 1) {
            raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
            return KEY_ESC;
        }

        if (seq1 == 'f' || seq1 == 'F') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_F; }
        if (seq1 == 'e' || seq1 == 'E') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_E; }
        if (seq1 == 's' || seq1 == 'S') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_S; }
        if (seq1 == 'd' || seq1 == 'D') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_D; }
        if (seq1 == 'r' || seq1 == 'R') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_R; }
        if (seq1 == 'h' || seq1 == 'H') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return ALT_H; }

        if (seq1 == '[') {
            if (read(0, &seq2, 1) == 1) {
                if (seq2 >= '0' && seq2 <= '9') {
                    if (read(0, &seq3, 1) == 1) {
                        if (seq3 == '~') {
                            raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                            switch(seq2) {
                                case '1': return HOME_KEY;
                                case '2': return INS_KEY;
                                case '3': return DEL_KEY;
                                case '4': return END_KEY;
                                case '5': return PAGE_UP;
                                case '6': return PAGE_DOWN;
                                case '7': return HOME_KEY;
                                case '8': return END_KEY;
                            }
                        } else if (seq3 == ';') {
                            char seq4, seq5;
                            if (read(0, &seq4, 1) == 1 && read(0, &seq5, 1) == 1) {
                                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                if (seq2 == '1' && seq4 == '2') {
                                    if (seq5 == 'A') return SHIFT_ARROW_UP;
                                    if (seq5 == 'B') return SHIFT_ARROW_DOWN;
                                    if (seq5 == 'C') return SHIFT_ARROW_RIGHT;
                                    if (seq5 == 'D') return SHIFT_ARROW_LEFT;
                                }
                                if (seq2 == '1' && seq4 == '5') {
                                    if (seq5 == 'H') return CTRL_HOME;
                                    if (seq5 == 'F') return CTRL_END;
                                }
                                if (seq2 == '2' && seq4 == '5' && seq5 == '~') return KEY_CTRL_INS;
                                if (seq2 == '3' && seq4 == '2' && seq5 == '~') return KEY_SHIFT_DEL;
                                if (seq2 == '2' && seq4 == '2' && seq5 == '~') return KEY_SHIFT_INS;
                            }
                        } else {
                            char seq4;
                            if (read(0, &seq4, 1) == 1) {
                                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                if (seq4 == '~') {
                                    switch(seq3) {
                                        case '1': return KEY_F1;
                                        case '2': return KEY_F2;
                                        case '3': return KEY_F3;
                                        case '4': return KEY_F4;
                                        case '5': return KEY_F5;
                                    }
                                } else if (seq2 == '1' && seq3 == '9' && seq4 == '~') {
                                    return KEY_F8;
                                } else if (seq2 == '2' && seq3 == '0' && seq4 == '~') {
                                    return KEY_F9;
                                } else if (seq2 == '2' && seq3 == '1' && seq4 == '~') {
                                    return KEY_F10;
                                }
                            }
                        }
                    }
                } else {
                    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                    switch(seq2) {
                        case 'A': return ARROW_UP;
                        case 'B': return ARROW_DOWN;
                        case 'C': return ARROW_RIGHT;
                        case 'D': return ARROW_LEFT;
                        case 'H': return HOME_KEY;
                        case 'F': return END_KEY;
                    }
                }
            }
        } else if (seq1 == 'O') {
            if (read(0, &seq2, 1) == 1) {
                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                switch(seq2) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                    case 'P': return KEY_F1;
                    case 'Q': return KEY_F2;
                    case 'R': return KEY_F3;
                    case 'S': return KEY_F4;
                    case 't': return KEY_F5;
                    case 'w': return KEY_F8;
                    case 'x': return KEY_F9;
                }
            }
        }
        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
        return KEY_ESC;
    }
    return c;
#endif
}

/* --- Output Helpers --- */
static void edit_print(const char *fmt, ...)
{
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    int len = (int)strlen(buf);
    if (render_buf_pos + len < (int)sizeof(render_buf) - 1) {
        strcpy(render_buf + render_buf_pos, buf);
        render_buf_pos += len;
    }
}

static void edit_flush(void) {
    if (render_buf_pos > 0) {
        render_buf[render_buf_pos] = '\0';
        printf("%s", render_buf);
        fflush(stdout);
        render_buf_pos = 0;
    }
}

/* --- Editor Core Logistics --- */
static void load_file_edit(const char *filename, bool is_initial) {
    if (text_buffer) {
        for (int i = 0; i < num_lines; i++) free_line(i);
    }
    num_lines = 0;
    ensure_buffer_capacity(1);
    FILE *file = fopen(filename, "r");
    if (!file) {
        if (!is_initial) {
            insert_empty_line(0);
            return;
        } else {
            strncpy(current_filename, filename, 4095);
            current_filename[4095] = '\0';
            insert_empty_line(0);
            cx = 0; cy = 0; row_off = 0; col_off = 0;
            return;
        }
    }
    char line_buf[4096];
    while (fgets(line_buf, sizeof(line_buf), file)) {
        sanitize_ascii(line_buf);
        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
            line_buf[len - 1] = '\0';
            len--;
        }
        insert_empty_line(num_lines);
        ensure_line_capacity(num_lines - 1, len + 1);
        strcpy(text_buffer[num_lines - 1].text, line_buf);
        text_buffer[num_lines - 1].length = len;
    }
    fclose(file);
    if (num_lines == 0) {
        insert_empty_line(0);
    }
    strncpy(current_filename, filename, 4095);
    current_filename[4095] = '\0';
    cx = 0; cy = 0; row_off = 0; col_off = 0;
}

static void save_file_edit(void) {
    if (current_filename[0] == '\0') return;
    FILE *file = fopen(current_filename, "w");
    if (!file) return;
    for (int i = 0; i < num_lines; i++) {
        fprintf(file, "%s\n", text_buffer[i].text);
    }
    fclose(file);
}

static void insert_char(int c) {
    int len = text_buffer[cy].length;
    ensure_line_capacity(cy, len + 2);
    memmove(&text_buffer[cy].text[cx + 1], &text_buffer[cy].text[cx], len - cx + 1);
    text_buffer[cy].text[cx] = (char)c;
    text_buffer[cy].length++;
    cx++;
}

static void insert_newline(void) {
    insert_empty_line(cy + 1);
    int remaining_len = text_buffer[cy].length - cx;
    ensure_line_capacity(cy + 1, remaining_len + 1);
    memmove(text_buffer[cy + 1].text, text_buffer[cy].text + cx, remaining_len + 1);
    text_buffer[cy + 1].length = remaining_len;
    text_buffer[cy].text[cx] = '\0';
    text_buffer[cy].length = cx;
    cy++;
    cx = 0;
}

static void handle_backspace(void) {
    if (cx > 0) {
        int len = text_buffer[cy].length;
        memmove(&text_buffer[cy].text[cx - 1], &text_buffer[cy].text[cx], len - cx + 1);
        text_buffer[cy].length--;
        cx--;
    } else if (cy > 0) {
        int prev_len = text_buffer[cy - 1].length;
        int cur_len = text_buffer[cy].length;
        ensure_line_capacity(cy - 1, prev_len + cur_len + 1);
        memmove(&text_buffer[cy - 1].text[prev_len], text_buffer[cy].text, cur_len + 1);
        text_buffer[cy - 1].length += cur_len;
        free_line(cy);
        for (int i = cy; i < num_lines - 1; i++) {
            text_buffer[i] = text_buffer[i + 1];
        }
        num_lines--; cy--; cx = prev_len;
    }
}

static void handle_delete(void) {
    int cur_len = text_buffer[cy].length;
    if (cx < cur_len) {
        memmove(&text_buffer[cy].text[cx], &text_buffer[cy].text[cx + 1], cur_len - cx);
        text_buffer[cy].length--;
    } else if (cy < num_lines - 1) {
        int next_len = text_buffer[cy + 1].length;
        ensure_line_capacity(cy, cur_len + next_len + 1);
        memmove(&text_buffer[cy].text[cur_len], text_buffer[cy + 1].text, next_len + 1);
        text_buffer[cy].length += next_len;
        free_line(cy + 1);
        for (int i = cy + 1; i < num_lines - 1; i++) {
            text_buffer[i] = text_buffer[i + 1];
        }
        num_lines--;
    }
}

static void delete_current_line(void) {
    if (num_lines > 1) {
        free_line(cy);
        for (int i = cy; i < num_lines - 1; i++) text_buffer[i] = text_buffer[i + 1];
        num_lines--;
        if (cy >= num_lines) cy = num_lines - 1;
        if (cx > (int)text_buffer[cy].length) cx = text_buffer[cy].length;
    } else {
        text_buffer[0].text[0] = '\0';
        text_buffer[0].length = 0;
        cx = 0;
    }
}

static int get_render_x(int row, int physical_x) {
    int rx = 0;
    for (int j = 0; j < physical_x && text_buffer[row].text[j] != '\0'; j++) {
        if (text_buffer[row].text[j] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

static int get_physical_x(int row, int target_x) {
    int rx = 0, j;
    for (j = 0; text_buffer[row].text[j] != '\0'; j++) {
        int next_rx = rx;
        if (text_buffer[row].text[j] == '\t') next_rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        next_rx++;
        if (next_rx > target_x) return j;
        rx = next_rx;
    }
    return j;
}

static void render_row(int row, char *out_buf) {
    int j = 0, idx = 0;
    while (text_buffer[row].text[j] != '\0' && idx < (MAX_RENDER_BUF - 1)) {
        if (text_buffer[row].text[j] == '\t') {
            out_buf[idx++] = ' ';
            while (idx % TAB_STOP != 0 && idx < (MAX_RENDER_BUF - 1)) out_buf[idx++] = ' ';
        } else {
            out_buf[idx++] = text_buffer[row].text[j];
        }
        j++;
    }
    out_buf[idx] = '\0';
}

static void format_filename_for_status(char *out_buf, const char *in_filename, int max_len) {
    if (!in_filename || !in_filename[0]) {
        strcpy(out_buf, "NEW FILE");
        return;
    }
    char abs_path[4096];
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    if (_fullpath(abs_path, in_filename, 4096) == NULL) {
        strcpy(abs_path, in_filename);
    }
#else
    if (realpath(in_filename, abs_path) == NULL) {
        strcpy(abs_path, in_filename);
    }
#endif

    int len = (int)strlen(abs_path);
    if (len <= max_len) {
        strcpy(out_buf, abs_path);
        return;
    }
    
    /* Find base filename */
    const char *base = abs_path;
    for (int i = len - 1; i >= 0; i--) {
        if (abs_path[i] == '/' || abs_path[i] == '\\') {
            base = &abs_path[i + 1];
            break;
        }
    }
    
    int base_len = (int)strlen(base);
    if (base_len >= max_len) {
        strcpy(out_buf, base);
        return;
    }
    
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    const char *prefix = "C:\\...\\";
#else
    const char *prefix = "/.../";
#endif

    int prefix_len = (int)strlen(prefix);
    if (prefix_len + base_len < max_len) {
        strcpy(out_buf, prefix);
        strcat(out_buf, base);
    } else {
        int copy_len = base_len;
        if (copy_len > max_len - 1) copy_len = max_len - 1;
        memcpy(out_buf, base, copy_len);
        out_buf[copy_len] = '\0';
    }
}

/* --- UI Rendering Primitives --- */
static void draw_all(void) {
    int y, i, file_row, len, print_len;
    char r_buf[MAX_RENDER_BUF];
    
    edit_print("\x1b[?25l");
    
    /* 1. Menu Bar */
    edit_print("\x1b[1;1H" COL_MENU "\x1b[K"); 
    for (i = 0; i < num_menus; i++) {
        edit_print("\x1b[1;%dH", menu_x[i]);
        if (menu_mode > 0 && menu_col == i) edit_print(COL_MENU_SEL);
        else edit_print(COL_MENU);
        edit_print("%s", menu_names[i]);
    }
    
    edit_print(COL_MENU);
    if (current_filename[0]) {
        const char *basename = current_filename;
        const char *p = current_filename;
        while (*p) {
            if (*p == '/' || *p == '\\') basename = p + 1;
            p++;
        }
        edit_print("  [ %s ]", basename);
    }
    
    time_t rawtime;
    struct tm tm_buf;
    struct tm *timeinfo;
    char time_str[64];
    time(&rawtime);
    timeinfo = platform_localtime(&rawtime, &tm_buf);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    int time_len = (int)(int)strlen(time_str);
    edit_print("\x1b[1;%dH%s", screen_cols - time_len - 1, time_str);
    
    /* 3. Editor Content Area */
    for (y = 2; y <= screen_rows - 1; y++) {
        file_row = row_off + (y - 2);
        edit_print("\x1b[%d;1H%s", y, bright_colors[color_index]); 
        
        int draw_cols = screen_cols;
        int gutter_offset = 0;

        if (display_gutter) {
            char gutter[16];
            if (file_row < num_lines) snprintf(gutter, sizeof(gutter), "%5d |", file_row + 1);
            else snprintf(gutter, sizeof(gutter), "      |");
            edit_print("\x1b[36m%s\x1b[0m%s", gutter, bright_colors[color_index]);
            gutter_offset = 7;
            draw_cols -= gutter_offset;
        }

        if (file_row < num_lines) {
            render_row(file_row, r_buf);
            len = (int)(int)strlen(r_buf);
            if (len > col_off) {
                print_len = len - col_off;
                if (print_len > draw_cols) print_len = draw_cols;
                edit_print("%.*s\x1b[K", print_len, r_buf + col_off);
            } else {
                edit_print("\x1b[K");
            }
        } else {
            edit_print("\x1b[K");
        }
    }
    
    /* 5. Status Bar */
    char right_status[128];
    snprintf(right_status, sizeof(right_status), "LINE:%d:%d - COL:%d", cy + 1, num_lines, cx + 1);
    int rl = (int)(int)strlen(right_status);
    
    char left_status[4200];
    char trunc_name[2048];
    format_filename_for_status(trunc_name, current_filename, screen_cols - rl - 15);
    snprintf(left_status, sizeof(left_status), " ESC=Menu %s", trunc_name);
    int ll = (int)(int)strlen(left_status);
    
    int pad = screen_cols - ll - rl - 3;
    if (pad < 1) pad = 1; /* Just ensure we have at least 1 space */
    
    edit_print("\x1b[%d;1H" COL_STATUS "\x1b[K", screen_rows);
    edit_print("%s", left_status);
    for (i = 0; i < pad; i++) edit_print(" ");
    edit_print("%s", right_status);
    
    /* 6. Active Dropdown Modal */
    if (menu_mode == 2) {
        int mx = menu_x[menu_col];
        int mh = drop_sizes[menu_col];
        const char **items = dropdowns[menu_col];
        
        for (i = 0; i < mh; i++) {
            edit_print("\x1b[%d;%dH", 2 + i, mx);
            if (menu_row == i) edit_print(COL_MENU_SEL);
            else edit_print(COL_MENU);
            edit_print("%s", items[i]);
            edit_print(COL_SHADOW " \x1b[0m");
        }
        edit_print("\x1b[%d;%dH" COL_SHADOW, 2 + mh, mx + 1);
        for (i = 0; i < (int)(int)strlen(items[0]) + 1; i++) edit_print(" ");
    }
}

/* --- Dialog Elements --- */
static int prompt_input_edit(const char *title, char *buf) {
    int w = 40, h = 7;
    int x, y, len, c, i, j;
    len = (int)(int)strlen(buf);
    
    get_terminal_size_edit(&screen_rows, &screen_cols);
    draw_all();

    while(true) {
        x = (screen_cols - w) / 2;
        y = (screen_rows - h) / 2;
        
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
        
        edit_print("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)(int)strlen(title) - 2) / 2, title);
        
        edit_print("\x1b[%d;%dH" COL_MENU_SEL, y + 3, x + 4);
        for (i = 0; i < w - 8; i++) {
            if (i < len) edit_print("%c", buf[i]);
            else edit_print(" ");
        }
        
        edit_print("\x1b[%d;%dH" COL_MENU " < OK > ", y + 5, x + w / 2 - 4);
        
        edit_print("\x1b[%d;%dH\x1b[?25h", y + 3, x + 4 + len);
        edit_flush();
        c = read_key_edit();
        if (c == KEY_TIMEOUT) { draw_all(); continue; }
        if (c == KEY_ESC) return 0;
        if (c == KEY_ENTER) return 1;
        if (c == KEY_BACKSPACE) {
            if (len > 0) buf[--len] = '\0';
        } else if (c >= 32 && c <= 126 && len < w - 10) {
            buf[len++] = (char)c;
            buf[len] = '\0';
        }
    }
}

static void show_message(const char *title, const char *msg) {
    int w = 50, h = 8;
    int x, y, c, i, j;
    
    get_terminal_size_edit(&screen_rows, &screen_cols);
    draw_all();

    while(true) {
        x = (screen_cols - w) / 2;
        y = (screen_rows - h) / 2;
        
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
        
        edit_print("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)(int)strlen(title) - 2) / 2, title);
        edit_print("\x1b[%d;%dH%s", y + 3, x + 2, msg);
        edit_print("\x1b[%d;%dH" COL_MENU " < OK > ", y + 6, x + w / 2 - 4);
        edit_flush();
        c = read_key_edit();
        if (c == KEY_TIMEOUT) { draw_all(); continue; }
        if (c == KEY_ESC || c == KEY_ENTER || c == 32) return;
    }
}

static void do_find(void) {
    char *p;
    for (int r = cy; r < num_lines; r++) {
        int start_x = (r == cy) ? cx + 1 : 0;
        if (start_x < (int)text_buffer[r].length) {
            p = strstr(text_buffer[r].text + start_x, search_term);
            if (p) {
                cy = r;
                cx = (int)(p - text_buffer[r].text);
                target_rx = get_render_x(cy, cx);
                return;
            }
        }
    }
    show_message(" Find ", "Search term not found.");
}

static void get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (sel_start_r < sel_end_r || (sel_start_r == sel_end_r && sel_start_c <= sel_end_c)) {
        *r1 = sel_start_r; *c1 = sel_start_c;
        *r2 = sel_end_r; *c2 = sel_end_c;
    } else {
        *r1 = sel_end_r; *c1 = sel_end_c;
        *r2 = sel_start_r; *c2 = sel_start_c;
    }
}

static char* get_selected_text_edit(void) {
    if (!sel_active) return NULL;
    int r1, c1, r2, c2;
    get_sel_bounds(&r1, &c1, &r2, &c2);
    char *buf = (char *)calloc(1, 65536);
    if (!buf) return NULL;
    buf[0] = '\0';
    int pos = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : (int)text_buffer[r].length;
        for (int i = start; i < end; i++) {
            buf[pos++] = text_buffer[r].text[i];
        }
        if (r < r2) {
            buf[pos++] = '\n';
        }
    }
    buf[pos] = '\0';
    return buf;
}

static void delete_selected_text_edit(void) {
    if (!sel_active) return;
    int r1, c1, r2, c2;
    get_sel_bounds(&r1, &c1, &r2, &c2);
    
    int rem_len = text_buffer[r2].length - c2;
    char *rem = (char *)calloc(1, rem_len + 1);
    if (!rem) { oom(); return; }
    strcpy(rem, text_buffer[r2].text + c2);
    
    text_buffer[r1].text[c1] = '\0';
    text_buffer[r1].length = c1;
    ensure_line_capacity(r1, c1 + rem_len + 1);
    strcat(text_buffer[r1].text, rem);
    text_buffer[r1].length += rem_len;
    free(rem);
    
    int lines_to_del = r2 - r1;
    if (lines_to_del > 0) {
        for (int i = r1 + 1; i <= r2; i++) free_line(i);
        for (int i = r1 + 1; i < num_lines - lines_to_del; i++) {
            text_buffer[i] = text_buffer[i + lines_to_del];
        }
        num_lines -= lines_to_del;
    }
    cy = r1; cx = c1;
    sel_active = false;
}

static void insert_text_at_cursor_edit(const char *text) {
    if (sel_active) delete_selected_text_edit();
    const char *p = text;
    while (*p) {
        if (*p == '\n' || *p == '\r') {
            if (*p == '\r' && *(p+1) == '\n') p++; 
            insert_newline();
            p++;
        } else {
            int len = text_buffer[cy].length;
            ensure_line_capacity(cy, len + 2);
            memmove(&text_buffer[cy].text[cx + 1], &text_buffer[cy].text[cx], len - cx + 1);
            text_buffer[cy].text[cx] = *p;
            text_buffer[cy].length++;
            cx++;
            p++;
        }
    }
}

#ifndef STANDALONE_EDITOR
static void edit_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    bool should_pause = false;
    if (debug_step_mode && strcmp(event_type, "step") == 0) should_pause = true;
    if (strcmp(event_type, "breakpoint") == 0) {
        char prefix[64]; snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < num_lines; i++) {
            if (strncmp(text_buffer[i].text, prefix, strlen(prefix)) == 0) {
                if (text_buffer[i].text[strlen(prefix)] == ' ' && text_buffer[i].breakpoint) {
                    should_pause = true; break;
                }
            }
        }
    }
    if (should_pause) {
        tui_multiplexer_init(); init_term();
        char prefix[64]; snprintf(prefix, sizeof(prefix), "%d", line_num);
        for (int i = 0; i < num_lines; i++) {
            if (strncmp(text_buffer[i].text, prefix, strlen(prefix)) == 0) {
                if (text_buffer[i].text[strlen(prefix)] == ' ') { cy = i; break; }
            }
        }
        draw_all(); edit_flush();
        int key;
        do { key = read_key_edit(); } while (key != KEY_F5 && key != KEY_F8 && key != KEY_ESC);
        if (key == KEY_F5) debug_step_mode = false;
        else if (key == KEY_F8) debug_step_mode = true;
        reset_term(); tui_multiplexer_shutdown();
    }
}
#endif

static void execute_program_edit(int step) {
#ifdef STANDALONE_EDITOR
    const char *target = current_filename[0] ? current_filename : "untitled.bas";
    FILE *file = fopen(target, "w");
    if (file) {
        for (int i = 0; i < num_lines; i++) {
            fprintf(file, "%s\n", text_buffer[i].text);
        }
        fclose(file);
    }
    reset_term();
    printf("\x1b[2J\x1b[H"); fflush(stdout);
    execute_standalone(target, step ? 1 : 0);
    printf("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
    init_term();
#else
    if (!current_vm) return;
    const char *target = current_filename[0] ? current_filename : "untitled.bas";
    FILE *file = fopen(target, "w");
    if (file) {
        for (int i = 0; i < num_lines; i++) {
            fprintf(file, "%s\n", text_buffer[i].text);
        }
        fclose(file);
    }
    reset_term(); tui_multiplexer_shutdown();
    printf("\x1b[2J\x1b[H"); fflush(stdout);
    debug_step_mode = (step != 0);
    vm_set_debug_hook(current_vm, edit_debug_hook, NULL);
    vm_load_program_file(current_vm, target);
    vm_run_program(current_vm);
    printf("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
    tui_multiplexer_init(); init_term();
#endif
}

static void execute_menu_edit(void) {
    char buf[4096];
    menu_mode = 0;
    
    if (menu_col == 0) {
        if (menu_row == 0) {
            if (text_buffer) { for (int i = 0; i < num_lines; i++) free_line(i); }
            num_lines = 0; insert_empty_line(0); cy=0; cx=0; current_filename[0]='\0';
            row_off = 0; col_off = 0;
        } else if (menu_row == 1) {
            buf[0] = '\0';
            if (prompt_input_edit(" Open ", buf) && buf[0] != '\0') {
                load_file_edit(buf, false);
            }
        } else if (menu_row == 2) {
            if (current_filename[0] == '\0') {
                buf[0] = '\0';
                if (prompt_input_edit(" Save As ", buf) && buf[0] != '\0') {
                    strcpy(current_filename, buf);
                    save_file_edit();
                }
            } else {
                save_file_edit();
            }
        } else if (menu_row == 3) {
            strcpy(buf, current_filename);
            if (prompt_input_edit(" Save As ", buf) && buf[0] != '\0') {
                strcpy(current_filename, buf);
                save_file_edit();
            }
        } else if (menu_row == 4) {
            exit_editor = true;
        }
    } else if (menu_col == 1) {
        if (menu_row == 0) {
            char *sel = get_selected_text_edit();
            if (sel) { strcpy(clipboard_line, sel); free(sel); delete_selected_text_edit(); }
            else { strcpy(clipboard_line, text_buffer[cy].text); delete_current_line(); }
        } else if (menu_row == 1) {
            char *sel = get_selected_text_edit();
            if (sel) { strcpy(clipboard_line, sel); free(sel); }
            else { strcpy(clipboard_line, text_buffer[cy].text); }
        } else if (menu_row == 2) {
            insert_text_at_cursor_edit(clipboard_line);
        } else if (menu_row == 3) {
            if (sel_active) delete_selected_text_edit();
            else delete_current_line();
        }
    } else if (menu_col == 2) {
        if (menu_row == 0) {
            buf[0] = '\0';
            if (prompt_input_edit(" Find ", buf) && buf[0] != '\0') {
                strcpy(search_term, buf);
                do_find();
            }
        } else if (menu_row == 1) {
            if (search_term[0] != '\0') do_find();
        }
    } else if (menu_col == 3) {
        if (menu_row == 0) {
            color_index = (color_index + 1) % NUM_BRIGHT_COLORS;
        } else if (menu_row == 1) {
            display_gutter = !display_gutter;
        }
    } else if (menu_col == 4) {
        if (menu_row == 0) execute_program_edit(0);
        else if (menu_row == 1) execute_program_edit(1);
        else if (menu_row == 2) text_buffer[cy].breakpoint = !text_buffer[cy].breakpoint;
    } else if (menu_col == 5) {
        if (menu_row == 0) {
            char about_msg[128];
            snprintf(about_msg, sizeof(about_msg), "BASIC++ Standard %s", BPP_VERSION_STRING);
            show_message(" About ", about_msg);
        }
    }
}

#ifdef STANDALONE_EDITOR
int main(int argc, char **argv) {
    int c, rx, visible_rows, visible_cols, max_rows;
    bool moved_vertically = false;
    const char *filename = (argc > 1) ? argv[1] : "";
    (void)argc;
#else
int main(VMContext *vm, const char *filename) {
    int c, rx, visible_rows, visible_cols, max_rows;
    bool moved_vertically = false;
    current_vm = vm;
#endif
    
    if ((filename ? 2 : 1) > 1) load_file_edit(filename, true);
    else { num_lines = 0; insert_empty_line(0); current_filename[0] = '\0'; }

    init_term();
    
    while (!exit_editor) {
        get_terminal_size_edit(&screen_rows, &screen_cols);
        
        visible_rows = screen_rows - 3; 
        visible_cols = screen_cols - 1 - (display_gutter ? 7 : 0);
        
        if (cy < row_off) row_off = cy;
        if (cy >= row_off + visible_rows) row_off = cy - visible_rows + 1;
        rx = get_render_x(cy, cx);
        if (rx < col_off) col_off = rx;
        if (rx >= col_off + visible_cols) col_off = rx - visible_cols + 1;
        
        draw_all();
        
        if (menu_mode == 0) {
            rx = get_render_x(cy, cx);
            edit_print("\x1b[%d;%dH\x1b[?25h", (cy - row_off) + 3, (rx - col_off) + 1 + (display_gutter ? 7 : 0));
        } else {
            edit_print("\x1b[?25l");
        }
        
        edit_flush();
        c = read_key_edit();
        moved_vertically = false;
        
        if (c == KEY_TIMEOUT) continue;
        if (c == 3 || c == 4) { exit_editor = true; continue; } /* Ctrl+C/D to exit */
        if (c == KEY_ESC) {
            if (menu_mode == 0) { menu_mode = 1; menu_col = 0; }
            else if (menu_mode == 2) { menu_mode = 1; }
            else { menu_mode = 0; }
            continue;
        }

        /* Invisible F-Keys */
        if (c == KEY_F10) { color_index = (color_index + 1) % NUM_BRIGHT_COLORS; continue; }
        if (c == KEY_F1) { show_message(" Help ", "Standalone C17 Editor"); continue; }
        if (c == KEY_F2) {
            if (current_filename[0] == '\0') {
                char buf[4096]; buf[0] = '\0';
                if (prompt_input_edit(" Save As ", buf) && buf[0] != '\0') {
                    strcpy(current_filename, buf); save_file_edit();
                }
            } else save_file_edit();
            continue;
        }
        if (c == KEY_F3) {
            char buf[4096]; buf[0] = '\0';
            if (prompt_input_edit(" Open ", buf) && buf[0] != '\0') load_file_edit(buf, false);
            continue;
        }
        if (c == KEY_F4) { exit_editor = true; continue; }
        if (c == KEY_F5) { execute_program_edit(0); continue; }
        if (c == KEY_F8) { execute_program_edit(1); continue; }
        if (c == KEY_F9) { text_buffer[cy].breakpoint = !text_buffer[cy].breakpoint; continue; }
        
        if (c == ALT_F) { menu_mode = 2; menu_col = 0; menu_row = 0; continue; }
        if (c == ALT_E) { menu_mode = 2; menu_col = 1; menu_row = 0; continue; }
        if (c == ALT_S) { menu_mode = 2; menu_col = 2; menu_row = 0; continue; }
        if (c == ALT_D) { menu_mode = 2; menu_col = 3; menu_row = 0; continue; }
        if (c == ALT_R) { menu_mode = 2; menu_col = 4; menu_row = 0; continue; }
        if (c == ALT_H) { menu_mode = 2; menu_col = 5; menu_row = 0; continue; }
        
        if (menu_mode == 1) {
            if (c == ARROW_LEFT) { menu_col = (menu_col + num_menus - 1) % num_menus; }
            else if (c == ARROW_RIGHT) { menu_col = (menu_col + 1) % num_menus; }
            else if (c == ARROW_DOWN || c == KEY_ENTER) { menu_mode = 2; menu_row = 0; }
            else if (c == ARROW_UP) { menu_mode = 2; menu_row = drop_sizes[menu_col] - 1; }
        } else if (menu_mode == 2) {
            max_rows = drop_sizes[menu_col];
            if (c == ARROW_UP) { menu_row = (menu_row + max_rows - 1) % max_rows; }
            else if (c == ARROW_DOWN) { menu_row = (menu_row + 1) % max_rows; }
            else if (c == ARROW_LEFT) { menu_col = (menu_col + num_menus - 1) % num_menus; menu_row = 0; }
            else if (c == ARROW_RIGHT) { menu_col = (menu_col + 1) % num_menus; menu_row = 0; }
            else if (c == KEY_ENTER) { execute_menu_edit(); }
        } else {
            if (c == SHIFT_ARROW_UP) { update_sel_end(cy > 0 ? cy - 1 : 0, cx); cy = sel_end_r; }
            else if (c == SHIFT_ARROW_DOWN) { update_sel_end(cy < num_lines - 1 ? cy + 1 : num_lines - 1, cx); cy = sel_end_r; }
            else if (c == SHIFT_ARROW_LEFT) { 
                update_sel_end(cy, cx > 0 ? cx - 1 : 0); 
                if (cx == 0 && cy > 0) update_sel_end(cy - 1, (int)text_buffer[cy-1].length);
                cx = sel_end_c; cy = sel_end_r; 
            }
            else if (c == SHIFT_ARROW_RIGHT) { 
                update_sel_end(cy, cx < (int)text_buffer[cy].length ? cx + 1 : cx); 
                if (cx == (int)text_buffer[cy].length && cy < num_lines - 1) update_sel_end(cy + 1, 0);
                cx = sel_end_c; cy = sel_end_r; 
            }
            else if (c == KEY_CTRL_INS) {
                char *txt = get_selected_text_edit();
                if (txt) { strcpy(clipboard_line, txt); free(txt); clear_sel(); }
            }
            else if (c == KEY_SHIFT_DEL) {
                char *txt = get_selected_text_edit();
                if (txt) { strcpy(clipboard_line, txt); free(txt); delete_selected_text_edit(); }
            }
            else if (c == KEY_SHIFT_INS) {
                insert_text_at_cursor_edit(clipboard_line);
            }
            else if (c == ARROW_UP || c == ARROW_DOWN || c == ARROW_LEFT || c == ARROW_RIGHT || c == HOME_KEY || c == END_KEY || c == PAGE_UP || c == PAGE_DOWN || c == CTRL_HOME || c == CTRL_END) {
                clear_sel();
            }
            else {
                if ((c >= 32 && c <= 126) || c == KEY_BACKSPACE || c == KEY_ENTER || c == DEL_KEY) clear_sel();
            }

            switch (c) {
                case ARROW_UP: if (cy > 0) { cy--; moved_vertically = true; } break;
                case ARROW_DOWN: if (cy < num_lines - 1) { cy++; moved_vertically = true; } break;
                case ARROW_LEFT: 
                    if (cx > 0) cx--; 
                    else if (cy > 0) { cy--; cx = (int)text_buffer[cy].length; }
                    break;
                case ARROW_RIGHT:
                    if (cx < (int)text_buffer[cy].length) cx++;
                    else if (cy < num_lines - 1) { cy++; cx = 0; }
                    break;
                case PAGE_UP: cy -= (screen_rows - 3); if (cy < 0) cy = 0; moved_vertically = true; break;
                case PAGE_DOWN: cy += (screen_rows - 3); if (cy >= num_lines) cy = num_lines - 1; moved_vertically = true; break;
                case HOME_KEY: cx = 0; break;
                case END_KEY: cx = (int)text_buffer[cy].length; break;
                case CTRL_HOME: cy = 0; cx = 0; moved_vertically = true; break;
                case CTRL_END: cy = num_lines - 1; cx = (int)text_buffer[cy].length; moved_vertically = true; break;
                case DEL_KEY: handle_delete(); break;
                case KEY_ENTER: insert_newline(); break;
                case KEY_BACKSPACE: handle_backspace(); break;
                default: 
                    if ((c >= 32 && c <= 126) || c == KEY_TAB) {
                        insert_char(c);
                    }
                    break;
            }
            if (moved_vertically) cx = get_physical_x(cy, target_rx);
            else {
                if (cx > (int)text_buffer[cy].length) cx = (int)text_buffer[cy].length;
                target_rx = get_render_x(cy, cx);
            }
        }
    }
    
    edit_print("\x1b[0m\x1b[2J\x1b[H\x1b[?25h");
    edit_flush();
    return 0;
}
