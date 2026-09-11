// FILENAME: vi_term.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (vi_internal.h)
// Implements visual text editor subsystem components for vi_term.
//
// ---- Includes ----

#include "editor/vi_internal.h"

#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
#include <poll.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
static struct termios orig_termios;
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#define GETCH _getch
static HANDLE hOut;
static DWORD dwMode;
#endif

static int pushed_char = -1;

void vi_sanitize_ascii(char *str) {
    if (!str) return;
    char *p = str;
    while (*str) {
        if ((unsigned char)(*str) < 128) {
            *p++ = *str;
        }
        str++;
    }
    *p = '\0';
}

void vi_get_terminal_size(void) {
#if defined(_WIN32) || defined(WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        g_vi_screen_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        g_vi_screen_cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        g_vi_screen_rows = 24; g_vi_screen_cols = 80;
    }
#elif defined(__MSDOS__) || defined(__DOS__)
    g_vi_screen_rows = 25; g_vi_screen_cols = 80;
#else
    struct winsize w;
    if (ioctl(1, TIOCGWINSZ, &w) != -1 && w.ws_row > 0 && w.ws_col > 0) {
        g_vi_screen_rows = w.ws_row;
        g_vi_screen_cols = w.ws_col;
    } else {
        g_vi_screen_rows = 24; g_vi_screen_cols = 80;
    }
#endif
    if (g_vi_screen_rows < 5) g_vi_screen_rows = 24;
    if (g_vi_screen_cols < 20) g_vi_screen_cols = 80;
}

void vi_reset_term(void) {
#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    tcsetattr(0, TCSANOW, &orig_termios);
#endif
}

void vi_init_term(void) {
#if defined(_WIN32) || defined(WIN32)
    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#elif !defined(__MSDOS__) && !defined(__DOS__)
    struct termios raw;
    tcgetattr(0, &orig_termios);
    atexit(vi_reset_term);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &raw);
#endif
}

int vi_get_input(void) {
    if (pushed_char != -1) {
        int c = pushed_char;
        pushed_char = -1;
        return c;
    }

#if defined(_WIN32) || defined(WIN32)
    DWORD start = GetTickCount();
    while (!_kbhit()) {
        if (GetTickCount() - start > 1000) return VI_KEY_TIMEOUT;
        Sleep(50);
    }
    int c = GETCH();
#elif defined(__MSDOS__) || defined(__DOS__)
    int c = GETCH();
#endif
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    if (c < 0) {
        g_vi_running = false; 
        return 0;
    }
    if (c == 0 || c == 224) {
        int seq = GETCH();
        switch (seq) {
            case 72: return VI_KEY_UP;
            case 80: return VI_KEY_DOWN;
            case 75: return VI_KEY_LEFT;
            case 77: return VI_KEY_RIGHT;
            case 71: return VI_KEY_HOME;
            case 79: return VI_KEY_END;
            case 73: return VI_KEY_PGUP;
            case 81: return VI_KEY_PGDN;
            case 82: return VI_KEY_INS;
            case 83: return VI_KEY_DEL;
            case 119: return VI_KEY_CTRL_HOME;
            case 117: return VI_KEY_CTRL_END;
            case 63: return VI_KEY_F5;
            case 66: return VI_KEY_F8;
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
    if (ret == 0) return VI_KEY_TIMEOUT;
    if (ret < 0 || read(0, &c, 1) != 1) {
        g_vi_running = false;
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
                                    case '1': return VI_KEY_HOME;
                                    case '2': return VI_KEY_INS;
                                    case '3': return VI_KEY_DEL;
                                    case '4': return VI_KEY_END;
                                    case '5': return VI_KEY_PGUP;
                                    case '6': return VI_KEY_PGDN;
                                    case '7': return VI_KEY_HOME;
                                    case '8': return VI_KEY_END;
                                }
                            } else if (seq3 == ';') {
                                char seq4, seq5;
                                if (read(0, &seq4, 1) == 1 && read(0, &seq5, 1) == 1) {
                                    if (seq4 == '5') {
                                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                        if (seq5 == 'H') return VI_KEY_CTRL_HOME;
                                        if (seq5 == 'F') return VI_KEY_CTRL_END;
                                    }
                                }
                            } else if (seq2 == '1') {
                                if ((seq3 >= '1' && seq3 <= '5') || seq3 == '9') {
                                    char seq4;
                                    if (read(0, &seq4, 1) == 1 && seq4 == '~') {
                                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                        switch(seq3) {
                                            case '1': return VI_KEY_F1;
                                            case '2': return VI_KEY_F2;
                                            case '3': return VI_KEY_F3;
                                            case '4': return VI_KEY_F4;
                                            case '5': return VI_KEY_F5;
                                            case '9': return VI_KEY_F8;
                                        }
                                    }
                                }
                            }
                        }
                    } else {
                        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                        if (seq1 == '[') {
                            switch(seq2) {
                                case 'A': return VI_KEY_UP;
                                case 'B': return VI_KEY_DOWN;
                                case 'C': return VI_KEY_RIGHT;
                                case 'D': return VI_KEY_LEFT;
                                case 'H': return VI_KEY_HOME;
                                case 'F': return VI_KEY_END;
                            }
                        } else if (seq1 == 'O') {
                            switch(seq2) {
                                case 'A': return VI_KEY_UP;
                                case 'B': return VI_KEY_DOWN;
                                case 'C': return VI_KEY_RIGHT;
                                case 'D': return VI_KEY_LEFT;
                                case 'H': return VI_KEY_HOME;
                                case 'F': return VI_KEY_END;
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
