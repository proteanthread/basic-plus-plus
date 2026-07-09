/*
 * vi.c - Bare minimum vi-style visual text editor
 * Strict C89, compiles cleanly on Linux, Windows 11, and FreeDOS.
 */

#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    #define _POSIX_SOURCE /* Exposes POSIX unbuffered terminal I/O in strict C89 mode on Linux */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Platform Specific Terminal Handling --- */
#if defined(_WIN32) || defined(WIN32)
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
    struct termios orig_termios;
#endif

#define MAX_LINES 1000
#define MAX_LENGTH 255
#define SCREEN_ROWS 24

char text_buffer[MAX_LINES][MAX_LENGTH];
char current_filename[MAX_LENGTH] = "";
char cmd_buffer[MAX_LENGTH] = "";

int current_lines = 0;
int cursor_r = 0, cursor_c = 0;
int row_offset = 0;
int mode = 0; /* 0: Normal, 1: Insert, 2: Command */
int cmd_len = 0;
int running = 1;

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

int get_input(void) {
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    int c = GETCH();
    if (c == 0 || c == 224) { /* Safely swallow physical arrow keys scan codes on Windows/DOS */
        GETCH();
        return 0;
    }
    return c;
#else
    char c;
    if (read(0, &c, 1) == 1) return c;
    return 0;
#endif
}

void load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    current_lines = 0;
    if (file) {
        while (current_lines < MAX_LINES && fgets(text_buffer[current_lines], MAX_LENGTH, file)) {
            int len = (int)strlen(text_buffer[current_lines]);
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
    strncpy(current_filename, filename, MAX_LENGTH - 1);
    current_filename[MAX_LENGTH - 1] = '\0';
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

    len = (int)strlen(text_buffer[cursor_r]);
    if (mode == 0) {
        if (cursor_c >= len && len > 0) cursor_c = len - 1;
    } else {
        if (cursor_c > len) cursor_c = len;
    }
    if (cursor_c < 0) cursor_c = 0;

    if (cursor_r < row_offset) row_offset = cursor_r;
    if (cursor_r >= row_offset + SCREEN_ROWS - 1) row_offset = cursor_r - (SCREEN_ROWS - 2);
}

void render_screen(void) {
    int i;
    printf("\x1b[?25l"); /* Hide cursor momentarily to prevent flicker */
    printf("\x1b[H");
    
    for (i = 0; i < SCREEN_ROWS - 1; i++) {
        int line_idx = row_offset + i;
        if (line_idx < current_lines) {
            printf("%s\x1b[K\r\n", text_buffer[line_idx]);
        } else {
            printf("~\x1b[K\r\n");
        }
    }
    
    /* Dynamic Status Line */
    printf("\x1b[7m");
    if (mode == 2) {
        printf(":%s\x1b[K", cmd_buffer);
    } else if (mode == 1) {
        printf("-- INSERT --\x1b[K");
    } else {
        printf("\"%s\" %d lines\x1b[K", current_filename, current_lines);
    }
    printf("\x1b[0m");
    
    /* Set Physical Cursor Position */
    if (mode == 2) {
        printf("\x1b[%d;%dH", SCREEN_ROWS, cmd_len + 2);
    } else {
        printf("\x1b[%d;%dH", (cursor_r - row_offset) + 1, cursor_c + 1);
    }
    printf("\x1b[?25h");
    fflush(stdout);
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
        /* Fall back and process char if it wasn't a double 'dd' execution */
    }

    switch (c) {
        case 'h': cursor_c--; break;
        case 'l': cursor_c++; break;
        case 'j': cursor_r++; break;
        case 'k': cursor_r--; break;
        case 'i': mode = 1; break;
        case 'a': cursor_c++; mode = 1; break;
        case 'I': cursor_c = 0; mode = 1; break;
        case 'A': cursor_c = (int)strlen(text_buffer[cursor_r]); mode = 1; break;
        case '0': cursor_c = 0; break;
        case '$': cursor_c = (int)strlen(text_buffer[cursor_r]); break;
        case 'x':
            if (text_buffer[cursor_r][cursor_c] != '\0') {
                for (i = cursor_c; text_buffer[cursor_r][i]; i++) {
                    text_buffer[cursor_r][i] = text_buffer[cursor_r][i + 1];
                }
            }
            break;
        case 'd': pending_d = 1; break;
        case 'o':
            if (current_lines < MAX_LINES) {
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
            if (current_lines < MAX_LINES) {
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
    } else if (c == 10 || c == 13) { /* Enter */
        if (current_lines < MAX_LINES) {
            for (i = current_lines; i > cursor_r; i--) {
                strcpy(text_buffer[i], text_buffer[i - 1]);
            }
            strcpy(text_buffer[cursor_r + 1], &text_buffer[cursor_r][cursor_c]);
            text_buffer[cursor_r][cursor_c] = '\0';
            current_lines++;
            cursor_r++;
            cursor_c = 0;
        }
    } else if (c == 8 || c == 127) { /* Backspace */
        if (cursor_c > 0) {
            len = (int)strlen(text_buffer[cursor_r]);
            for (i = cursor_c; i <= len; i++) {
                text_buffer[cursor_r][i - 1] = text_buffer[cursor_r][i];
            }
            cursor_c--;
        } else if (cursor_r > 0) {
            int prev_len = (int)strlen(text_buffer[cursor_r - 1]);
            if (prev_len + (int)strlen(text_buffer[cursor_r]) < MAX_LENGTH) {
                strcat(text_buffer[cursor_r - 1], text_buffer[cursor_r]);
                for (i = cursor_r; i < current_lines - 1; i++) {
                    strcpy(text_buffer[i], text_buffer[i + 1]);
                }
                current_lines--;
                cursor_r--;
                cursor_c = prev_len;
            }
        }
    } else if (c >= 32 && c <= 126) { /* Standard Characters */
        len = (int)strlen(text_buffer[cursor_r]);
        if (len < MAX_LENGTH - 1) {
            for (i = len; i >= cursor_c; i--) {
                text_buffer[cursor_r][i + 1] = text_buffer[cursor_r][i];
            }
            text_buffer[cursor_r][cursor_c] = (char)c;
            cursor_c++;
        }
    }
}

void handle_command(int c) {
    if (c == 27) {
        mode = 0;
    } else if (c == 10 || c == 13) {
        if (strcmp(cmd_buffer, "w") == 0) save_file();
        else if (strcmp(cmd_buffer, "q") == 0 || strcmp(cmd_buffer, "q!") == 0) running = 0;
        else if (strcmp(cmd_buffer, "wq") == 0 || strcmp(cmd_buffer, "x") == 0) {
            save_file();
            running = 0;
        }
        mode = 0;
    } else if (c == 8 || c == 127) {
        if (cmd_len > 0) cmd_buffer[--cmd_len] = '\0';
        else mode = 0;
    } else if (c >= 32 && c <= 126 && cmd_len < MAX_LENGTH - 1) {
        cmd_buffer[cmd_len++] = (char)c;
        cmd_buffer[cmd_len] = '\0';
    }
}

void exit_editor(void) {
    printf("\x1b[2J\x1b[H\x1b[?25h"); /* Restore Screen bounds and physical cursor pointer */
    fflush(stdout);
    reset_term();
    exit(0);
}

int main(int argc, char *argv[]) {
    int c;
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        printf("Usage: vi <filename>\n");
        return 1;
    }
    
    init_term();
    printf("\x1b[2J\x1b[H"); 
    
    while (running) {
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