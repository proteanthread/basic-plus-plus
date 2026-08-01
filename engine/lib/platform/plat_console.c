/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
#include "platform/platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <signal.h>
#ifndef STANDALONE_EDITOR
#include "vm/vm.h"
#endif

#if defined(_WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
    #include <conio.h>
    #include <direct.h>
    #include <io.h>
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    #include <unistd.h>
    #include <termios.h>
    #include <sys/select.h>
    #include <sys/ioctl.h>
    #include <sys/stat.h>
    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netdb.h>
    #include <arpa/inet.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <dlfcn.h>
    #include <errno.h>
    #include <pthread.h>
    #ifndef STANDALONE_EDITOR
        #include <ncurses.h>
    #endif
#elif defined(__WATCOMC__) || defined(MSDOS)
    #include <dos.h>
    #include <conio.h>
    #include <direct.h>
#endif

/* Shared input queue for kbhit/getch/mouse unified handling */
static int g_kb_queue[256];
static int g_kb_head = 0;
static int g_kb_tail = 0;

static void push_key(int ch) {
    int next = (g_kb_tail + 1) % 256;
    if (next != g_kb_head) {
        g_kb_queue[g_kb_tail] = ch;
        g_kb_tail = next;
    }
}

#if defined(_WIN32)
static int g_win_mouse_x = 1;
static int g_win_mouse_y = 1;
static int g_win_mouse_btn = 0;
static int g_win_mouse_modifiers = 0;
static bool g_win_mouse_enabled = false;

int platform_get_modifiers(void) {
#ifdef _WIN32
    int mods = 0;
    if (GetKeyState(VK_LSHIFT) & 0x8000) mods |= 1;
    if (GetKeyState(VK_LCONTROL) & 0x8000) mods |= 2;
    if (GetKeyState(VK_LMENU) & 0x8000) mods |= 4;
    if (GetKeyState(VK_LWIN) & 0x8000) mods |= 8;
    if (GetKeyState(VK_RSHIFT) & 0x8000) mods |= 16;
    if (GetKeyState(VK_RCONTROL) & 0x8000) mods |= 32;
    if (GetKeyState(VK_RMENU) & 0x8000) mods |= 64;
    if (GetKeyState(VK_RWIN) & 0x8000) mods |= 128;
    if (GetKeyState(VK_APPS) & 0x8000) mods |= 256;
    return mods;
#else
    return 0;
#endif
}

static void platform_poll_console_events(void) {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return;
    
    DWORD numEvents = 0;
    if (!GetNumberOfConsoleInputEvents(hIn, &numEvents) || numEvents == 0) return;
    
    INPUT_RECORD recs[64];
    DWORD read = 0;
    if (ReadConsoleInputA(hIn, recs, 64, &read) && read > 0) {
        for (DWORD i = 0; i < read; i++) {
            if (recs[i].EventType == KEY_EVENT) {
                KEY_EVENT_RECORD ker = recs[i].Event.KeyEvent;
                if (ker.bKeyDown) {
                    int ch = (unsigned char)ker.uChar.AsciiChar;
                    if (ch != 0) {
                        push_key(ch);
                    } else {
                        int vk = ker.wVirtualKeyCode;
                        if (vk >= VK_F1 && vk <= VK_F12) {
                            int scan = (vk <= VK_F10) ? (59 + (vk - VK_F1)) : (133 + (vk - VK_F11));
                            int mods = platform_get_modifiers();
                            push_key(0);
                            push_key(scan | (mods << 8));
                        } else if (vk == VK_UP) { push_key(224); push_key(72); }
                        else if (vk == VK_DOWN) { push_key(224); push_key(80); }
                        else if (vk == VK_LEFT) { push_key(224); push_key(75); }
                        else if (vk == VK_RIGHT) { push_key(224); push_key(77); }
                        else if (vk == VK_INSERT) { push_key(224); push_key(82); }
                        else if (vk == VK_DELETE) { push_key(224); push_key(83); }
                        else if (vk == VK_HOME) { push_key(224); push_key(71); }
                        else if (vk == VK_END) { push_key(224); push_key(79); }
                        else if (vk == VK_PRIOR) { push_key(224); push_key(73); }
                        else if (vk == VK_NEXT) { push_key(224); push_key(81); }
                    }
                }
            } else if (recs[i].EventType == MOUSE_EVENT) {
                MOUSE_EVENT_RECORD mer = recs[i].Event.MouseEvent;
                g_win_mouse_x = mer.dwMousePosition.X + 1;
                g_win_mouse_y = mer.dwMousePosition.Y + 1;
                
                int btn = 0;
                if (mer.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) btn |= 1;
                if (mer.dwButtonState & RIGHTMOST_BUTTON_PRESSED) btn |= 2;
                if (mer.dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED) btn |= 4;
                g_win_mouse_btn = btn;
                
                int mods = 0;
                if (mer.dwControlKeyState & SHIFT_PRESSED) mods |= 1;
                if (mer.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) mods |= 2;
                if (mer.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) mods |= 4;
                g_win_mouse_modifiers = mods;
            }
        }
    }
}
#elif !defined(__WATCOMC__) && !defined(MSDOS)
static int g_posix_mouse_x = 1;
static int g_posix_mouse_y = 1;
static int g_posix_mouse_btn = 0;
static int g_posix_mouse_modifiers = 0;
static bool g_posix_mouse_enabled = false;

static void platform_poll_console_events(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0) {
        return;
    }
    
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    struct termios orig = raw;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    
    int ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    
    if (ch == 27) {
        struct timeval esc_tv = {0, 20000};
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &esc_tv) > 0) {
            tcsetattr(STDIN_FILENO, TCSANOW, &raw);
            int ch2 = getchar();
            if (ch2 == '[') {
                int ch3 = getchar();
                if (ch3 == '<') {
                    int btn = 0, col = 0, row = 0;
                    char final_char = 0;
                    int c;
                    while ((c = getchar()) != ';') {
                        btn = btn * 10 + (c - '0');
                    }
                    while ((c = getchar()) != ';') {
                        col = col * 10 + (c - '0');
                    }
                    while (1) {
                        c = getchar();
                        if (c == 'M' || c == 'm') {
                            final_char = c;
                            break;
                        }
                        row = row * 10 + (c - '0');
                    }
                    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
                    
                    g_posix_mouse_x = col;
                    g_posix_mouse_y = row;
                    
                    int button_type = btn & 3;
                    bool is_wheel = (btn & 64) != 0;
                    
                    int current_btn = g_posix_mouse_btn;
                    if (final_char == 'm' || button_type == 3) {
                        if (button_type == 0 || button_type == 3) current_btn &= ~1;
                        if (button_type == 2) current_btn &= ~2;
                        if (button_type == 1) current_btn &= ~4;
                    } else if (final_char == 'M') {
                        if (!is_wheel) {
                            if (button_type == 0) current_btn |= 1;
                            if (button_type == 2) current_btn |= 2;
                            if (button_type == 1) current_btn |= 4;
                        }
                    }
                    g_posix_mouse_btn = current_btn;
                    
                    int mods = 0;
                    if (btn & 4) mods |= 1;
                    if (btn & 16) mods |= 2;
                    if (btn & 8) mods |= 4;
                    g_posix_mouse_modifiers = mods;
                    return;
                } else {
                    push_key(ch);
                    push_key(ch2);
                    push_key(ch3);
                    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
                    return;
                }
            } else {
                push_key(ch);
                push_key(ch2);
                tcsetattr(STDIN_FILENO, TCSANOW, &orig);
                return;
            }
        } else {
            push_key(ch);
            tcsetattr(STDIN_FILENO, TCSANOW, &orig);
            return;
        }
    } else {
        push_key(ch);
    }
}
#endif

bool platform_kbhit(void) {
#if defined(_WIN32)
    platform_poll_console_events();
    return g_kb_head != g_kb_tail;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return kbhit() != 0;
#else
    platform_poll_console_events();
    return g_kb_head != g_kb_tail;
#endif
}

int platform_getch(void) {
#if defined(_WIN32)
    while (g_kb_head == g_kb_tail) {
        platform_poll_console_events();
        Sleep(10);
    }
    int ch = g_kb_queue[g_kb_head];
    g_kb_head = (g_kb_head + 1) % 256;
    return ch;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return getch();
#else
    while (g_kb_head == g_kb_tail) {
        platform_poll_console_events();
        usleep(10000);
    }
    int ch = g_kb_queue[g_kb_head];
    g_kb_head = (g_kb_head + 1) % 256;
    return ch;
#endif
}

int platform_console_height(void) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
    return 25; /* Standard fallback */
#elif defined(__WATCOMC__) || defined(MSDOS)
    /* Standard 80x25 screen size on DOS */
    return 25;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_row;
    }
    return 25;
#endif
}

int platform_console_width(void) {
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
#elif defined(__WATCOMC__) || defined(MSDOS)
    return 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return w.ws_col;
    }
    return 80;
#endif
}

void platform_tui_init(void) {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #ifndef STANDALONE_EDITOR
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
  #endif
#endif
}

void platform_tui_shutdown(void) {
#if defined(_WIN32)
    /* No specific shutdown required for Windows ANSI mode */
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  #ifndef STANDALONE_EDITOR
    endwin();
  #endif
#endif
}

int platform_screen_get_char(int row, int col) {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return -1;
    COORD coord;
    coord.X = (SHORT)col;
    coord.Y = (SHORT)row;
    char ch;
    DWORD read = 0;
    if (ReadConsoleOutputCharacterA(hOut, &ch, 1, coord, &read)) {
        return (unsigned char)ch;
    }
#endif
    return -1;
}

int platform_screen_get_attr(int row, int col) {
#if defined(_WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return -1;
    COORD coord;
    coord.X = (SHORT)col;
    coord.Y = (SHORT)row;
    WORD attr;
    DWORD read = 0;
    if (ReadConsoleOutputAttribute(hOut, &attr, 1, coord, &read)) {
        return attr;
    }
#endif
    return -1;
}

static int g_mouse_cursor_char = 0;
static int g_mouse_cursor_attrib = 7;
static bool g_mouse_cursor_visible = false;

void platform_mouse_enable(bool enable) {
    g_mouse_cursor_visible = enable;
#if defined(_WIN32)
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hIn, &mode)) {
            if (enable) {
                mode |= ENABLE_MOUSE_INPUT;
                mode &= ~ENABLE_QUICK_EDIT_MODE;
                g_win_mouse_enabled = true;
            } else {
                mode &= ~ENABLE_MOUSE_INPUT;
                mode |= ENABLE_QUICK_EDIT_MODE;
                g_win_mouse_enabled = false;
            }
            SetConsoleMode(hIn, mode);
        }
    }
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    if (enable) {
        printf("\033[?1003h\033[?1006h");
        fflush(stdout);
        g_posix_mouse_enabled = true;
    } else {
        printf("\033[?1003l\033[?1006l");
        fflush(stdout);
        g_posix_mouse_enabled = false;
    }
#else
    (void)enable;
#endif
}

void platform_mouse_get_position(int *col, int *row) {
#if defined(_WIN32)
    if (col) *col = g_win_mouse_x;
    if (row) *row = g_win_mouse_y;
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    if (col) *col = g_posix_mouse_x;
    if (row) *row = g_posix_mouse_y;
#else
    if (col) *col = 1;
    if (row) *row = 1;
#endif
}

void platform_mouse_set_position(int col, int row) {
#if defined(_WIN32)
    g_win_mouse_x = col;
    g_win_mouse_y = row;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        COORD coord;
        coord.X = (SHORT)(col - 1);
        coord.Y = (SHORT)(row - 1);
        SetConsoleCursorPosition(hOut, coord);
    }
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    g_posix_mouse_x = col;
    g_posix_mouse_y = row;
    printf("\033[%d;%dH", row, col);
    fflush(stdout);
#else
    (void)col; (void)row;
#endif
}

int platform_mouse_get_button(int btn_idx) {
#if defined(_WIN32)
    if (btn_idx < 0 || btn_idx > 2) return 0;
    int mask = (btn_idx == 0) ? 1 : ((btn_idx == 1) ? 2 : 4);
    return (g_win_mouse_btn & mask) ? -1 : 0;
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    if (btn_idx < 0 || btn_idx > 2) return 0;
    int mask = (btn_idx == 0) ? 1 : ((btn_idx == 1) ? 2 : 4);
    return (g_posix_mouse_btn & mask) ? -1 : 0;
#else
    (void)btn_idx;
    return 0;
#endif
}

int platform_mouse_get_modifiers(void) {
#if defined(_WIN32)
    return g_win_mouse_modifiers;
#elif !defined(__WATCOMC__) && !defined(MSDOS)
    return g_posix_mouse_modifiers;
#else
    return 0;
#endif
}

void platform_mouse_set_cursor(int char_code, int attrib) {
    g_mouse_cursor_char = char_code;
    g_mouse_cursor_attrib = attrib;
}

void platform_mouse_get_cursor(int *char_code, int *attrib) {
    if (char_code) *char_code = g_mouse_cursor_char;
    if (attrib) *attrib = g_mouse_cursor_attrib;
}

bool platform_mouse_is_visible(void) {
    return g_mouse_cursor_visible;
}
