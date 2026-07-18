#ifndef STANDALONE_EDITOR
#include "bpp_editor.h"
#endif
#include "bpp_version.h"

#ifdef STANDALONE_EDITOR
#include "standalone_runner.h"
#endif

#define get_terminal_size vi_get_terminal_size
#define reset_term vi_reset_term
#define init_term vi_init_term
#define get_input vi_get_input
#ifndef STANDALONE_EDITOR
#define main mod_vi_main
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
 * vi.c - Bare minimum vi-style visual text editor
 *
 */

/* Removed redundant POSIX macros */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "bpp_platform.h"

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

/* --- Platform Specific Terminal Handling --- */
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
#define KEY_F8    1018
#define KEY_F10   1020
#define KEY_TIMEOUT 1021
#define KEY_CTRL_HOME 1022
#define KEY_CTRL_END  1023

void fix_cursor(void);
void render_screen(void);

int current_lines = 0;
typedef struct {
    char *text;
    int length;
    int capacity;
} Line;
Line *text_buffer = NULL;
int text_buffer_capacity = 0;
char current_filename[4096] = "";
char cmd_buffer[4096] = "";

#ifndef STANDALONE_EDITOR
static struct VMContext *current_vm = NULL;
#endif

static void oom(void) {
    fprintf(stderr, "\n\nOut of memory!\n");
    exit(1);
}

static void ensure_line_capacity(int row, int needed) {
    if (needed > text_buffer[row].capacity) {
        int new_cap = text_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        char *new_text = realloc(text_buffer[row].text, new_cap);
        if (!new_text) oom();
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
        if (!new_buf) oom();
        text_buffer = new_buf;
        text_buffer_capacity = new_cap;
    }
}

static void insert_empty_line(int row) {
    ensure_buffer_capacity(current_lines + 1);
    for (int i = current_lines; i > row; i--) {
        text_buffer[i] = text_buffer[i - 1];
    }
    text_buffer[row].text = malloc(128);
    if (!text_buffer[row].text) oom();
    text_buffer[row].text[0] = '\0';
    text_buffer[row].length = 0;
    text_buffer[row].capacity = 128;
    current_lines++;
}

static void free_line(int row) {
    if (text_buffer[row].text) {
        free(text_buffer[row].text);
        text_buffer[row].text = NULL;
    }
}

int cursor_r = 0, cursor_c = 0;
int row_offset = 0;
int mode = 0; /* 0: Normal, 1: Insert, 2: Command */
int cmd_len = 0;
bool running = true;
int screen_rows = 24;

static const char *bright_colors[] = {
    "\x1b[97m", /* Bright White */
    "\x1b[96m", /* Bright Cyan */
    "\x1b[92m", /* Bright Green */
    "\x1b[93m", /* Bright Yellow */
    "\x1b[95m", /* Bright Magenta */
    "\x1b[91m"  /* Bright Red */
};
#define NUM_BRIGHT_COLORS (sizeof(bright_colors)/sizeof(bright_colors[0]))
static int color_index = 0;
static int pushed_char = -1;
int screen_cols = 80;

#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
/* Removed duplicate orig_termios */
#else
HANDLE hOut;
DWORD dwMode;
#endif

static bool sel_active = false;
static int sel_start_r = 0, sel_start_c = 0;
static int sel_end_r = 0, sel_end_c = 0;

void get_terminal_size(void) {
#if defined(_WIN32) || defined(WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        screen_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        screen_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        screen_rows = 24; screen_cols = 80;
    }
#elif defined(__MSDOS__) || defined(__DOS__)
    screen_rows = 25; screen_cols = 80; /* DOS text-mode standard */
#else
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) != -1 && w.ws_row > 0 && w.ws_col > 0) {
        screen_rows = w.ws_row;
        screen_cols = w.ws_col;
    } else {
        screen_rows = 24; screen_cols = 80;
    }
#endif
    if (screen_rows < 5) screen_rows = 24;
    if (screen_cols < 20) screen_cols = 80;
}

void reset_term(void) {
#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    tcsetattr(0, TCSANOW, &orig_termios);
#endif
}

void init_term(void) {
#if defined(_WIN32) || defined(WIN32)
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    dwMode = 0;
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

static void get_sel_bounds(int *r1, int *c1, int *r2, int *c2) {
    if (sel_start_r < sel_end_r || (sel_start_r == sel_end_r && sel_start_c <= sel_end_c)) {
        *r1 = sel_start_r; *c1 = sel_start_c;
        *r2 = sel_end_r; *c2 = sel_end_c;
    } else {
        *r1 = sel_end_r; *c1 = sel_end_c;
        *r2 = sel_start_r; *c2 = sel_start_c;
    }
}

int get_input(void) {
    if (pushed_char != -1) {
        int c = pushed_char;
        pushed_char = -1;
        return c;
    }

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
    if (c < 0) {
        running = false; 
        return 0;
    }
    if (c == 0 || c == 224) {
        int seq = GETCH();
        switch (seq) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            case 71: return KEY_HOME;
            case 79: return KEY_END;
            case 73: return KEY_PGUP;
            case 81: return KEY_PGDN;
            case 82: return KEY_INS;
            case 83: return KEY_DEL;
            case 119: return KEY_CTRL_HOME;
            case 117: return KEY_CTRL_END;
            case 63: return KEY_F5;
            case 66: return KEY_F8;
        }
        return 0;
    }
    return c;
#else
    char c, seq1, seq2, seq3;
    struct pollfd pfd;
    pfd.fd = 0;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, 1000);
    if (ret == 0) return KEY_TIMEOUT;
    if (ret < 0 || read(0, &c, 1) != 1) {
        running = false;
        return 0;
    }
    
    if (c == 27) {
        struct termios raw;
        tcgetattr(0, &raw);
        raw.c_cc[VMIN] = 0;
        raw.c_cc[VTIME] = 1;
        tcsetattr(0, TCSANOW, &raw);
        
        if (read(0, &seq1, 1) == 1) {
            if (seq1 == '[' || seq1 == 'O') {
                if (read(0, &seq2, 1) == 1) {
                    if (seq1 == '[' && seq2 >= '0' && seq2 <= '9') {
                        if (read(0, &seq3, 1) == 1) {
                            if (seq3 == '~') {
                                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
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
                            } else if (seq3 == ';') {
                                char seq4, seq5;
                                if (read(0, &seq4, 1) == 1 && read(0, &seq5, 1) == 1) {
                                    if (seq4 == '5') {
                                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                        if (seq5 == 'H') return KEY_CTRL_HOME;
                                        if (seq5 == 'F') return KEY_CTRL_END;
                                    }
                                }
                            } else if (seq2 == '1') {
                                if ((seq3 >= '1' && seq3 <= '5') || seq3 == '9') {
                                    char seq4;
                                    if (read(0, &seq4, 1) == 1 && seq4 == '~') {
                                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                        switch(seq3) {
                                            case '1': return KEY_F1;
                                            case '2': return KEY_F2;
                                            case '3': return KEY_F3;
                                            case '4': return KEY_F4;
                                            case '5': return KEY_F5;
                                            case '9': return KEY_F8;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                        if (seq1 == '[') {
                            switch(seq2) {
                                case 'A': return KEY_UP;
                                case 'B': return KEY_DOWN;
                                case 'C': return KEY_RIGHT;
                                case 'D': return KEY_LEFT;
                                case 'H': return KEY_HOME;
                                case 'F': return KEY_END;
                            }
                        } else if (seq1 == 'O') {
                            switch(seq2) {
                                case 'A': return KEY_UP;
                                case 'B': return KEY_DOWN;
                                case 'C': return KEY_RIGHT;
                                case 'D': return KEY_LEFT;
                                case 'H': return KEY_HOME;
                                case 'F': return KEY_END;
                            }
                        }
                    }
                }
            } else {
                pushed_char = seq1;
                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                return 27;
            }
        }
        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
        return 27;
    }
    return c;
#endif
}

void load_file(const char *filename) {
    if (text_buffer) {
        for (int i = 0; i < current_lines; i++) free_line(i);
    }
    current_lines = 0;
    ensure_buffer_capacity(1);
    FILE *file = fopen(filename, "r");
    if (file) {
        char line_buf[4096];
        while (fgets(line_buf, sizeof(line_buf), file)) {
            sanitize_ascii(line_buf);
            int len = (int)(int)strlen(line_buf);
            while (len > 0 && (line_buf[len - 1] == '\n' || line_buf[len - 1] == '\r')) {
                line_buf[len - 1] = '\0';
                len--;
            }
            insert_empty_line(current_lines);
            ensure_line_capacity(current_lines - 1, len + 1);
            strcpy(text_buffer[current_lines - 1].text, line_buf);
            text_buffer[current_lines - 1].length = len;
        }
        fclose(file);
    }
    if (current_lines == 0) insert_empty_line(0);
    strncpy(current_filename, filename, 4095);
    current_filename[4095] = '\0';
}

void save_file(void);

#ifndef STANDALONE_EDITOR
static bool debug_step_mode = false;
static bool trace_mode = false;
#endif

#ifndef STANDALONE_EDITOR
static void vi_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    bool should_pause = false;
    if (debug_step_mode && strcmp(event_type, "step") == 0) should_pause = true;
    
    if (should_pause) {
        tui_multiplexer_init(); init_term();
        char prefix[32]; sprintf(prefix, "%d", line_num);
        for (int i = 0; i < current_lines; i++) {
            if (strncmp(text_buffer[i].text, prefix, strlen(prefix)) == 0) {
                if (text_buffer[i].text[strlen(prefix)] == ' ') { cursor_r = i; break; }
            }
        }
        fix_cursor();
        render_screen();
        
        int key;
        do { key = get_input(); } while (key != KEY_F5 && key != KEY_F8 && key != 27);
        if (key == KEY_F5) debug_step_mode = false;
        else if (key == KEY_F8) debug_step_mode = true;
        reset_term(); tui_multiplexer_shutdown();
    } else if (trace_mode && strcmp(event_type, "step") == 0) {
        printf("[TRACE] Executing Line %d\r\n", line_num);
    }
}
#endif

void execute_program_vi(int exec_mode) {
#ifdef STANDALONE_EDITOR
    const char *target = current_filename[0] ? current_filename : "untitled.bas";
    save_file();
    reset_term();
    printf("\x1b[2J\x1b[H"); fflush(stdout);
    
    execute_standalone(target, exec_mode);
    
    printf("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
    init_term();
#else
    if (!current_vm) return;
    const char *target = current_filename[0] ? current_filename : "untitled.bas";
    FILE *file = fopen(target, "w");
    if (file) {
        for (int i = 0; i < current_lines; i++) {
            fprintf(file, "%s\n", text_buffer[i].text);
        }
        fclose(file);
    }
    reset_term();
    tui_multiplexer_shutdown();
    printf("\x1b[2J\x1b[H"); fflush(stdout);
    
    debug_step_mode = (exec_mode == 1);
    trace_mode = (exec_mode == 2);
    if (exec_mode > 0) {
        vm_set_debug_hook(current_vm, vi_debug_hook, NULL);
    } else {
        vm_set_debug_hook(current_vm, NULL, NULL);
    }
    
    vm_load_program_file(current_vm, target);
    vm_run_program(current_vm);
    printf("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
    tui_multiplexer_init();
    init_term();
#endif
}

void save_file(void) {
    FILE *file = fopen(current_filename, "w");
    if (file) {
        for (int i = 0; i < current_lines; i++) {
            fprintf(file, "%s\n", text_buffer[i].text);
        }
        fclose(file);
    }
}

void fix_cursor(void) {
    int len;
    if (cursor_r < 0) cursor_r = 0;
    if (cursor_r >= current_lines) cursor_r = current_lines - 1;

    len = text_buffer[cursor_r].length;
    if (mode == 0) {
        if (cursor_c >= len && len > 0) cursor_c = len - 1;
    } else {
        if (cursor_c > len) cursor_c = len;
    }
    if (cursor_c < 0) cursor_c = 0;

    if (cursor_r < row_offset) row_offset = cursor_r;
    if (cursor_r >= row_offset + screen_rows - 1) row_offset = cursor_r - (screen_rows - 2);
}

static void format_filename_for_status(char *out_buf, const char *in_filename, int max_len) {
    if (!in_filename || !in_filename[0]) {
        strcpy(out_buf, "NEW");
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

    int len = (int)(int)strlen(abs_path);
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
    
    int base_len = (int)(int)strlen(base);
    if (base_len >= max_len) {
        /* Extreme edge case: fallback to base filename */
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

void render_screen(void) {
    printf("\x1b[?25l"); /* Hide cursor momentarily to prevent flicker */
    printf("\x1b[H");
    
    printf("%s", bright_colors[color_index]);
    
    int show_splash = 0;
    if (current_lines == 1 && text_buffer[0].length == 0 && current_filename[0] == '\0') {
        show_splash = 1;
    }

    for (int i = 0; i < screen_rows - 1; i++) {
        int line_idx = row_offset + i;
        if (line_idx < current_lines) {
            printf("%s", bright_colors[color_index]);
            for (int j = 0; j < (int)text_buffer[line_idx].length; j++) {
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
                    printf("\x1b[47;30m%c%s", text_buffer[line_idx].text[j], bright_colors[color_index]);
                } else {
                    printf("%c", text_buffer[line_idx].text[j]);
                }
            }
            printf("\x1b[K\r\n");
        } else {
            if (show_splash && i == (screen_rows - 1) / 3) {
                char splash[128];
                sprintf(splash, "BASIC++ Standard v%s", BPP_VERSION_STRING);
                int pad = (screen_cols - (int)strlen(splash)) / 2;
                if (pad < 2) pad = 2;
                printf("\x1b[36m~");
                for(int p=1; p<pad; p++) printf(" ");
                printf("%s\x1b[K\r\n", splash);
            } else {
                printf("\x1b[36m~%s\x1b[K\r\n", bright_colors[color_index]);
            }
        }
    }
    
    /* Dynamic Status Line */
    printf("\x1b[47;30m");
    char left_status[4200];
    if (mode == 2) {
        snprintf(left_status, sizeof(left_status), ":%s", cmd_buffer);
    } else if (mode == 1) {
        snprintf(left_status, sizeof(left_status), "-- INSERT --");
    } else {
        char trunc_name[4096];
        format_filename_for_status(trunc_name, current_filename, screen_cols - 30);
        snprintf(left_status, sizeof(left_status), "\"%s\" %d:%d", trunc_name, cursor_r + 1, current_lines);
    }
    
    time_t rawtime;
    struct tm tm_buf;
    struct tm *timeinfo;
    char time_str[64];
    time(&rawtime);
    timeinfo = platform_localtime(&rawtime, &tm_buf);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    
    int ll = (int)(int)strlen(left_status);
    int tl = (int)(int)strlen(time_str);
    int pad = screen_cols - ll - tl - 2;
    if (pad < 1) pad = 1;
    
    printf("\x1b[%d;1H\x1b[47;30m\x1b[K", screen_rows);
    printf("%s", left_status);
    for (int i = 0; i < pad; i++) printf(" ");
    printf("%s", time_str);
    printf("\x1b[0m");
    
    /* Set Physical Cursor Position */
    if (mode == 2) {
        printf("\x1b[%d;%dH", screen_rows, cmd_len + 2);
    } else {
        printf("\x1b[%d;%dH", (cursor_r - row_offset) + 1, cursor_c + 1);
    }
    printf("\x1b[?25h");
    fflush(stdout);
}

void display_help(void) {
    printf("\x1b[2J\x1b[H"); 
    printf("--- vi Built-in Help ---\r\n\n");
    printf(" NORMAL MODE:\r\n");
    printf("   h,j,k,l / Arrows : Move cursor\r\n");
    printf("   Home / End       : Jump to start/end of line\r\n");
    printf("   PgUp / PgDn      : Page up / Page down\r\n");
    printf("   0, $             : Jump to start/end of line\r\n");
    printf("   i, a, I, A       : Enter Insert Mode\r\n");
    printf("   o, O             : Insert new line / Insert Mode\r\n");
    printf("   x, Del           : Delete character under cursor\r\n");
    printf("   dd               : Delete current line\r\n");
    printf("   :                : Enter Command Mode\r\n\n");
    printf(" INSERT MODE:\r\n");
    printf("   Esc              : Return to Normal Mode\r\n");
    printf("   Enter            : Split line\r\n");
    printf("   Backspace        : Delete char or merge lines\r\n\n");
    printf(" COMMAND MODE:\r\n");
    printf("   :w               : Save file\r\n");
    printf("   :w <file>        : Save to new file\r\n");
    printf("   :load <file>     : Load a file (aliases: :g, :get)\r\n");
    printf("   :run, :r!        : Execute the current program\r\n");
    printf("   :debug           : Execute program in step mode (F8 to step, F5 to continue)\r\n");
    printf("   :trace           : Execute program and print line trace\r\n");
    printf("   :q, :q!          : Quit / Quit without saving\r\n");
    printf("   :wq, :x          : Save and Quit\r\n");
    printf("   :?, :h           : Show this help screen\r\n\n");

    printf("Press any key to return...");
    fflush(stdout);
    int key;
    do {
        key = get_input();
    } while (key == 0 || key == KEY_TIMEOUT);
}

void handle_normal(int c) {
    static int pending_d = 0;
    
    if (pending_d) {
        pending_d = 0;
        if (c == 'd') {
            if (current_lines > 1) {
                free_line(cursor_r);
                for (int i = cursor_r; i < current_lines - 1; i++) {
                    text_buffer[i] = text_buffer[i + 1];
                }
                current_lines--;
                if (cursor_r >= current_lines) cursor_r = current_lines - 1;
            } else {
                text_buffer[0].text[0] = '\0';
                text_buffer[0].length = 0;
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
        case KEY_END:  case '$':  cursor_c = (int)text_buffer[cursor_r].length; break;
        case KEY_PGUP: cursor_r -= (screen_rows - 2); break;
        case KEY_PGDN: cursor_r += (screen_rows - 2); break;
        case KEY_CTRL_HOME: cursor_r = 0; cursor_c = 0; break;
        case KEY_CTRL_END: cursor_r = current_lines - 1; cursor_c = (int)text_buffer[cursor_r].length; break;
        
        case 'i': case KEY_INS: mode = 1; break;
        case 'a': cursor_c++; mode = 1; break;
        case 'I': cursor_c = 0; mode = 1; break;
        case 'A': cursor_c = (int)text_buffer[cursor_r].length; mode = 1; break;
        
        case 'x': case KEY_DEL:
            if (text_buffer[cursor_r].text[cursor_c] != '\0') {
                for (int i = cursor_c; text_buffer[cursor_r].text[i]; i++) {
                    text_buffer[cursor_r].text[i] = text_buffer[cursor_r].text[i + 1];
                }
                text_buffer[cursor_r].length--;
            }
            break;
            
        case 'd': pending_d = 1; break;
        case 'o':
            insert_empty_line(cursor_r + 1);
            cursor_r++;
            mode = 1;
            cursor_c = 0;
            break;
        case 'O':
            insert_empty_line(cursor_r);
            mode = 1;
            cursor_c = 0;
            break;
            
        case ':':
            mode = 2;
            cmd_len = 0;
            cmd_buffer[0] = '\0';
            break;
    }
}

void handle_insert(int c) {
    if (c == 27) { /* Escape */
        mode = 0;
        if (cursor_c > 0) cursor_c--;
    } else if (c == KEY_UP) { cursor_r--; }
    else if (c == KEY_DOWN) { cursor_r++; }
    else if (c == KEY_LEFT) { cursor_c--; }
    else if (c == KEY_RIGHT) { cursor_c++; }
    else if (c == KEY_HOME) { cursor_c = 0; }
    else if (c == KEY_END) { cursor_c = (int)text_buffer[cursor_r].length; }
    else if (c == KEY_PGUP) { cursor_r -= (screen_rows - 2); }
    else if (c == KEY_PGDN) { cursor_r += (screen_rows - 2); }
    else if (c == KEY_CTRL_HOME) { cursor_r = 0; cursor_c = 0; }
    else if (c == KEY_CTRL_END) { cursor_r = current_lines - 1; cursor_c = (int)text_buffer[cursor_r].length; }
    else if (c == KEY_DEL) {
        if (text_buffer[cursor_r].text[cursor_c] != '\0') {
            for (int i = cursor_c; text_buffer[cursor_r].text[i]; i++) {
                text_buffer[cursor_r].text[i] = text_buffer[cursor_r].text[i + 1];
            }
            text_buffer[cursor_r].length--;
        } else if (cursor_r < current_lines - 1) {
            int len = text_buffer[cursor_r].length;
            int next_len = text_buffer[cursor_r + 1].length;
            ensure_line_capacity(cursor_r, len + next_len + 1);
            memmove(&text_buffer[cursor_r].text[len], text_buffer[cursor_r + 1].text, next_len + 1);
            text_buffer[cursor_r].length += next_len;
            free_line(cursor_r + 1);
            for (int i = cursor_r + 1; i < current_lines - 1; i++) {
                text_buffer[i] = text_buffer[i + 1];
            }
            current_lines--;
        }
    } else if (c == 10 || c == 13) { /* Enter */
        insert_empty_line(cursor_r + 1);
        int rem_len = text_buffer[cursor_r].length - cursor_c;
        ensure_line_capacity(cursor_r + 1, rem_len + 1);
        memmove(text_buffer[cursor_r + 1].text, &text_buffer[cursor_r].text[cursor_c], rem_len + 1);
        text_buffer[cursor_r + 1].length = rem_len;
        text_buffer[cursor_r].text[cursor_c] = '\0';
        text_buffer[cursor_r].length = cursor_c;
        cursor_r++;
        cursor_c = 0;
    } else if (c == 8 || c == 127) { /* Backspace */
        if (cursor_c > 0) {
            int len = text_buffer[cursor_r].length;
            for (int i = cursor_c; i <= len; i++) {
                text_buffer[cursor_r].text[i - 1] = text_buffer[cursor_r].text[i];
            }
            text_buffer[cursor_r].length--;
            cursor_c--;
        } else if (cursor_r > 0) {
            int prev_len = text_buffer[cursor_r - 1].length;
            int cur_len = text_buffer[cursor_r].length;
            ensure_line_capacity(cursor_r - 1, prev_len + cur_len + 1);
            strcat(text_buffer[cursor_r - 1].text, text_buffer[cursor_r].text);
            text_buffer[cursor_r - 1].length += cur_len;
            free_line(cursor_r);
            for (int i = cursor_r; i < current_lines - 1; i++) {
                text_buffer[i] = text_buffer[i + 1];
            }
            current_lines--;
            cursor_r--;
            cursor_c = prev_len;
        }
    } else if (c >= 32 && c <= 126) { /* Standard ASCII Characters */
        int len = text_buffer[cursor_r].length;
        ensure_line_capacity(cursor_r, len + 2);
        for (int i = len; i >= cursor_c; i--) {
            text_buffer[cursor_r].text[i + 1] = text_buffer[cursor_r].text[i];
        }
        text_buffer[cursor_r].text[cursor_c] = (char)c;
        text_buffer[cursor_r].length++;
        cursor_c++;
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
        running = false;
    }
}

void handle_command(int c) {
    if (c == 27) {
        mode = 0;
    } else if (c == 10 || c == 13) {
        if (strcmp(cmd_buffer, "w") == 0) save_file();
        else if (strncmp(cmd_buffer, "w ", 2) == 0) {
            char new_file[4096];
            strncpy(new_file, cmd_buffer + 2, 4096 - 1);
            new_file[4096 - 1] = '\0';
            strcpy(current_filename, new_file);
            save_file();
        }
        else if (strcmp(cmd_buffer, "q") == 0 || strcmp(cmd_buffer, "q!") == 0) running = false;
        else if (strcmp(cmd_buffer, "bye") == 0) running = false;
        else if (strcmp(cmd_buffer, "wq") == 0 || strcmp(cmd_buffer, "x") == 0) {
            save_file();
            running = false;
        }
        else if (strcmp(cmd_buffer, "run") == 0 || strcmp(cmd_buffer, "r!") == 0) {
            execute_program_vi(0);
        }
        else if (strcmp(cmd_buffer, "debug") == 0) {
            execute_program_vi(1);
        }
        else if (strcmp(cmd_buffer, "trace") == 0) {
            execute_program_vi(2);
        }
        else if (strncmp(cmd_buffer, "load ", 5) == 0 || strncmp(cmd_buffer, "g ", 2) == 0 || strncmp(cmd_buffer, "get ", 4) == 0) {
            char new_file[4096];
            const char *file_start = cmd_buffer + 5;
            if (strncmp(cmd_buffer, "g ", 2) == 0) file_start = cmd_buffer + 2;
            else if (strncmp(cmd_buffer, "get ", 4) == 0) file_start = cmd_buffer + 4;
            strncpy(new_file, file_start, 4096 - 1);
            new_file[4096 - 1] = '\0';
            strcpy(current_filename, new_file);
            load_file(current_filename);
        }
        else if (strcmp(cmd_buffer, "color") == 0) {
            color_index = (color_index + 1) % NUM_BRIGHT_COLORS;
        }
        else if (strncmp(cmd_buffer, "color ", 6) == 0) {
            char new_color[4096];
            strncpy(new_color, cmd_buffer + 6, 4096 - 1);
            new_color[4096 - 1] = '\0';
            if (strcmp(new_color, "white") == 0) color_index = 0;
            else if (strcmp(new_color, "cyan") == 0) color_index = 1;
            else if (strcmp(new_color, "green") == 0) color_index = 2;
            else if (strcmp(new_color, "yellow") == 0) color_index = 3;
            else if (strcmp(new_color, "magenta") == 0) color_index = 4;
            else if (strcmp(new_color, "red") == 0) color_index = 5;
        }
        else if (strcmp(cmd_buffer, "?") == 0 || strcmp(cmd_buffer, "h") == 0) display_help();
        mode = 0;
    } else if (c == 8 || c == 127) {
        if (cmd_len > 0) cmd_buffer[--cmd_len] = '\0';
        else mode = 0;
    } else if (c >= 32 && c <= 126 && cmd_len < 4096 - 1) {
        cmd_buffer[cmd_len++] = (char)c;
        cmd_buffer[cmd_len] = '\0';
    }
}

void exit_editor(void) {
    printf("\x1b[2J\x1b[H\x1b[?25h"); /* Restore Screen bounds and physical cursor pointer */
    fflush(stdout);
    reset_term();
}

#ifdef STANDALONE_EDITOR
int main(int argc, char **argv) {
    int c;
    const char *filename = (argc > 1) ? argv[1] : "";
    (void)argc;
#else
int main(VMContext *vm, const char *filename) {
    int c;
    current_vm = vm;
#endif

    if ((filename ? 2 : 1) > 1) {
        load_file(filename);
    } else {
        insert_empty_line(0);
    }
    
    init_term();
    printf("\x1b[2J\x1b[H"); 
    
    while (running) {
        get_terminal_size(); /* Dynamically re-read layout per user iteration */
        fix_cursor();
        render_screen();
        
        c = get_input();
        if (c == 0 || c == KEY_TIMEOUT) continue;
        if (c == 3 || c == 4) { running = false; continue; } /* Ctrl+C/D to exit */
        
        if (mode == 0) handle_normal(c);
        else if (mode == 1) handle_insert(c);
        else if (mode == 2) handle_command(c);
    }
    
    exit_editor();
    return 0;
}
