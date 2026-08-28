// FILENAME: edit_term.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edit_internal.h)
// Implements visual text editor subsystem components for edit_term.
//
// ---- Includes ----

#include "editor/edit_internal.h"

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
#endif

static char s_render_buf[65536];
static int s_render_buf_pos = 0;

void edit_print(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    size_t len = runtime_strlen(buf);
    if (s_render_buf_pos + (int)len < (int)sizeof(s_render_buf) - 1) {
        runtime_strcpy(s_render_buf + s_render_buf_pos, buf);
        s_render_buf_pos += (int)len;
    }
}

void edit_flush(void) {
    if (s_render_buf_pos > 0) {
        s_render_buf[s_render_buf_pos] = '\0';
        HalContext *hal = hal_get();
        if (hal && hal->io.file_write) {
            hal->io.file_write(IO_STDOUT_HANDLE, s_render_buf, 1, (size_t)s_render_buf_pos);
            hal->io.file_flush(IO_STDOUT_HANDLE);
        }
        s_render_buf_pos = 0;
    }
}


void edit_sanitize_ascii(char *str) {
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

void edit_reset_term(void) {
#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    tcsetattr(0, TCSANOW, &orig_termios);
#endif
}

void edit_init_term(void) {
#if defined(_WIN32) || defined(WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#elif !defined(__MSDOS__) && !defined(__DOS__)
    struct termios raw;
    tcgetattr(0, &orig_termios);
    atexit(edit_reset_term);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &raw);
#endif
}

void edit_get_terminal_size(int *rows, int *cols) {
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

int edit_read_key(void) {
#if defined(_WIN32) || defined(WIN32)
    DWORD start = GetTickCount();
    while (!_kbhit()) {
        if (GetTickCount() - start > 1000) return EDIT_KEY_TIMEOUT;
        Sleep(50);
    }
    int c = GETCH();
#elif defined(__MSDOS__) || defined(__DOS__)
    int c = GETCH();
#endif
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    if (c < 0) { g_edit_exit_editor = true; return 0; }
    if (c == '\r' || c == '\n') return EDIT_KEY_ENTER;
    if (c == 8 || c == 127) return EDIT_KEY_BACKSPACE;
    if (c == 0 || c == 224) {
        int seq = GETCH();
        switch (seq) {
            case 72: return EDIT_ARROW_UP;
            case 80: return EDIT_ARROW_DOWN;
            case 75: return EDIT_ARROW_LEFT;
            case 77: return EDIT_ARROW_RIGHT;
            case 71: return EDIT_HOME_KEY;
            case 79: return EDIT_END_KEY;
            case 73: return EDIT_PAGE_UP;
            case 81: return EDIT_PAGE_DOWN;
            case 82: return EDIT_INS_KEY;
            case 83: return EDIT_DEL_KEY;
            case 119: return EDIT_CTRL_HOME;
            case 117: return EDIT_CTRL_END;
            case 59: return EDIT_KEY_F1;
            case 60: return EDIT_KEY_F2;
            case 61: return EDIT_KEY_F3;
            case 62: return EDIT_KEY_F4;
            case 63: return EDIT_KEY_F5;
            case 68: return EDIT_KEY_F10;
            case 66: return EDIT_KEY_F8;
            case 67: return EDIT_KEY_F9;
            case 33: return EDIT_ALT_F;
            case 18: return EDIT_ALT_E;
            case 31: return EDIT_ALT_S;
            case 32: return EDIT_ALT_D;
            case 19: return EDIT_ALT_R;
            case 35: return EDIT_ALT_H;
            case 152: return EDIT_SHIFT_ARROW_UP;
            case 160: return EDIT_SHIFT_ARROW_DOWN;
            case 164: return EDIT_KEY_CTRL_INS;
            case 165: return EDIT_KEY_SHIFT_DEL;
            case 166: return EDIT_KEY_SHIFT_INS;
        }
        return EDIT_KEY_ESC;
    }
    return c;
#else
    char c, seq1, seq2, seq3;
    struct pollfd pfd;
    pfd.fd = 0;
    pfd.events = POLLIN;
    int ret = poll(&pfd, 1, 1000);
    if (ret == 0) return EDIT_KEY_TIMEOUT;
    if (ret < 0 || read(0, &c, 1) != 1) { g_edit_exit_editor = true; return 0; }
    if (c == '\r' || c == '\n') return EDIT_KEY_ENTER;
    if (c == 8 || c == 127) return EDIT_KEY_BACKSPACE;

    if (c == 27) {
        struct termios raw;
        tcgetattr(0, &raw);
        raw.c_cc[VMIN] = 0; raw.c_cc[VTIME] = 1;
        tcsetattr(0, TCSANOW, &raw);
        
        if (read(0, &seq1, 1) != 1) {
            raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
            return EDIT_KEY_ESC;
        }

        if (seq1 == 'f' || seq1 == 'F') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_F; }
        if (seq1 == 'e' || seq1 == 'E') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_E; }
        if (seq1 == 's' || seq1 == 'S') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_S; }
        if (seq1 == 'd' || seq1 == 'D') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_D; }
        if (seq1 == 'r' || seq1 == 'R') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_R; }
        if (seq1 == 'h' || seq1 == 'H') { raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw); return EDIT_ALT_H; }

        if (seq1 == '[') {
            if (read(0, &seq2, 1) == 1) {
                if (seq2 >= '0' && seq2 <= '9') {
                    if (read(0, &seq3, 1) == 1) {
                        if (seq3 == '~') {
                            raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                            switch(seq2) {
                                case '1': return EDIT_HOME_KEY;
                                case '2': return EDIT_INS_KEY;
                                case '3': return EDIT_DEL_KEY;
                                case '4': return EDIT_END_KEY;
                                case '5': return EDIT_PAGE_UP;
                                case '6': return EDIT_PAGE_DOWN;
                                case '7': return EDIT_HOME_KEY;
                                case '8': return EDIT_END_KEY;
                            }
                        } else if (seq3 == ';') {
                            char seq4, seq5;
                            if (read(0, &seq4, 1) == 1 && read(0, &seq5, 1) == 1) {
                                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                if (seq2 == '1' && seq4 == '2') {
                                    if (seq5 == 'A') return EDIT_SHIFT_ARROW_UP;
                                    if (seq5 == 'B') return EDIT_SHIFT_ARROW_DOWN;
                                    if (seq5 == 'C') return EDIT_SHIFT_ARROW_RIGHT;
                                    if (seq5 == 'D') return EDIT_SHIFT_ARROW_LEFT;
                                }
                                if (seq2 == '1' && seq4 == '5') {
                                    if (seq5 == 'H') return EDIT_CTRL_HOME;
                                    if (seq5 == 'F') return EDIT_CTRL_END;
                                }
                                if (seq2 == '2' && seq4 == '5' && seq5 == '~') return EDIT_KEY_CTRL_INS;
                                if (seq2 == '3' && seq4 == '2' && seq5 == '~') return EDIT_KEY_SHIFT_DEL;
                                if (seq2 == '2' && seq4 == '2' && seq5 == '~') return EDIT_KEY_SHIFT_INS;
                            }
                        } else {
                            char seq4;
                            if (read(0, &seq4, 1) == 1) {
                                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                                if (seq4 == '~') {
                                    switch(seq3) {
                                        case '1': return EDIT_KEY_F1;
                                        case '2': return EDIT_KEY_F2;
                                        case '3': return EDIT_KEY_F3;
                                        case '4': return EDIT_KEY_F4;
                                        case '5': return EDIT_KEY_F5;
                                    }
                                } else if (seq2 == '1' && seq3 == '9' && seq4 == '~') {
                                    return EDIT_KEY_F8;
                                } else if (seq2 == '2' && seq3 == '0' && seq4 == '~') {
                                    return EDIT_KEY_F9;
                                } else if (seq2 == '2' && seq3 == '1' && seq4 == '~') {
                                    return EDIT_KEY_F10;
                                }
                            }
                        }
                    }
                } else {
                    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                    switch(seq2) {
                        case 'A': return EDIT_ARROW_UP;
                        case 'B': return EDIT_ARROW_DOWN;
                        case 'C': return EDIT_ARROW_RIGHT;
                        case 'D': return EDIT_ARROW_LEFT;
                        case 'H': return EDIT_HOME_KEY;
                        case 'F': return EDIT_END_KEY;
                    }
                }
            }
        } else if (seq1 == 'O') {
            if (read(0, &seq2, 1) == 1) {
                raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
                switch(seq2) {
                    case 'A': return EDIT_ARROW_UP;
                    case 'B': return EDIT_ARROW_DOWN;
                    case 'C': return EDIT_ARROW_RIGHT;
                    case 'D': return EDIT_ARROW_LEFT;
                    case 'H': return EDIT_HOME_KEY;
                    case 'F': return EDIT_END_KEY;
                    case 'P': return EDIT_KEY_F1;
                    case 'Q': return EDIT_KEY_F2;
                    case 'R': return EDIT_KEY_F3;
                    case 'S': return EDIT_KEY_F4;
                    case 't': return EDIT_KEY_F5;
                    case 'w': return EDIT_KEY_F8;
                    case 'x': return EDIT_KEY_F9;
                }
            }
        }
        raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &raw);
        return EDIT_KEY_ESC;
    }
    return c;
#endif
}
