/*
 * VERSION: 1.2.0
 * LICENSE: MIT License
 * Description: ws.c - Portable WordStar-like Full-Screen Editor (C89 compliant)
 * Fixed: IDE Tab-stop mapping (4 spaces) and Virtual Column Memory for vertical navigation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Terminal Size & Raw Mode Headers --- */
#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#ifndef ENABLE_VIRTUAL_TERMINAL_INPUT
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
#else
#include <sys/ioctl.h>
#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#endif

#define MAX_LINES 1000
#define MAX_LENGTH 255
#define MAX_RENDER_BUF 1024
#define TAB_STOP 4  /* Changed from 8 to 4 to match modern IDE spacing */

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
    INS_KEY
};

/* --- Global State --- */
char text_buffer[MAX_LINES][MAX_LENGTH];
int num_lines = 0;
char current_filename[MAX_LENGTH] = "";

int cx = 0, cy = 0;             /* Physical Cursor coordinates */
int target_rx = 0;              /* Virtual column memory for Up/Down navigation */
int row_off = 0, col_off = 0;   /* Scrolling offsets */
int screen_rows = 24, screen_cols = 80;
int help_active = 0;
int prefix_k = 0;

/* --- Terminal Handling --- */
#ifdef _WIN32
HANDLE hStdin, hStdout;
DWORD fdwSaveOldMode;
DWORD fdwSaveOldOutMode;

void disable_raw_mode(void) {
    printf("\x1b[2J\x1b[H\x1b[?25h");
    fflush(stdout);
    SetConsoleMode(hStdin, fdwSaveOldMode);
    SetConsoleMode(hStdout, fdwSaveOldOutMode);
}

void enable_raw_mode(void) {
    DWORD mode;
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    
    GetConsoleMode(hStdin, &fdwSaveOldMode);
    GetConsoleMode(hStdout, &fdwSaveOldOutMode);
    atexit(disable_raw_mode);
    
    mode = (fdwSaveOldMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT)) | ENABLE_VIRTUAL_TERMINAL_INPUT;
    SetConsoleMode(hStdin, mode);
    
    mode = fdwSaveOldOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hStdout, mode);
}

#else
struct termios orig_termios;

void disable_raw_mode(void) {
    printf("\x1b[2J\x1b[H\x1b[?25h");
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enable_raw_mode(void) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int posix_kbhit(void) {
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 100000;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}
#endif

int read_key(void) {
    char c, seq[3];
    int nread;
    
#ifdef _WIN32
    DWORD read_bytes;
    if (!ReadFile(hStdin, &c, 1, &read_bytes, NULL) || read_bytes != 1) return 0;
#else
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1) exit(1);
    }
#endif

    if (c == '\x1b') {
#ifdef _WIN32
        if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return '\x1b';
        if (!ReadFile(hStdin, &seq[0], 1, &read_bytes, NULL) || read_bytes != 1) return '\x1b';
        if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return '\x1b';
        if (!ReadFile(hStdin, &seq[1], 1, &read_bytes, NULL) || read_bytes != 1) return '\x1b';
#else
        if (!posix_kbhit() || read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (!posix_kbhit() || read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
#endif
        
        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
#ifdef _WIN32
                if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return '\x1b';
                if (!ReadFile(hStdin, &seq[2], 1, &read_bytes, NULL) || read_bytes != 1) return '\x1b';
#else
                if (!posix_kbhit() || read(STDIN_FILENO, &seq[2], 1) != 1) return '\x1b';
#endif
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '2': return INS_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    }
    return c;
}

void get_terminal_size(int *rows, int *cols) {
    *rows = 24; 
    *cols = 80;
    
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        *cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        if (w.ws_row > 0) *rows = w.ws_row;
        if (w.ws_col > 0) *cols = w.ws_col;
    }
#endif
    if (*rows < 2) *rows = 24; 
}

/* --- Core Editor Functions --- */
void load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        num_lines = 0;
        while (num_lines < MAX_LINES && fgets(text_buffer[num_lines], MAX_LENGTH, file)) {
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
    strncpy(current_filename, filename, MAX_LENGTH - 1);
    current_filename[MAX_LENGTH - 1] = '\0';
    
    if (num_lines == 0) {
        num_lines = 1;
        text_buffer[0][0] = '\0';
    }
}

void save_file(void) {
    int i;
    FILE *file = fopen(current_filename, "w");
    if (!file) return;
    for (i = 0; i < num_lines; i++) {
        fprintf(file, "%s\n", text_buffer[i]);
    }
    fclose(file);
}

void insert_char(int c) {
    int len = strlen(text_buffer[cy]);
    if (len >= MAX_LENGTH - 1) return;
    memmove(&text_buffer[cy][cx + 1], &text_buffer[cy][cx], len - cx + 1);
    text_buffer[cy][cx] = (char)c;
    cx++;
}

void insert_newline(void) {
    int i;
    if (num_lines >= MAX_LINES) return;
    for (i = num_lines; i > cy + 1; i--) {
        strcpy(text_buffer[i], text_buffer[i - 1]);
    }
    strcpy(text_buffer[cy + 1], text_buffer[cy] + cx);
    text_buffer[cy][cx] = '\0';
    num_lines++;
    cy++;
    cx = 0;
}

void handle_backspace(void) {
    int i, len, prev_len, cur_len;
    if (cx > 0) {
        len = strlen(text_buffer[cy]);
        memmove(&text_buffer[cy][cx - 1], &text_buffer[cy][cx], len - cx + 1);
        cx--;
    } else if (cy > 0) {
        prev_len = strlen(text_buffer[cy - 1]);
        cur_len = strlen(text_buffer[cy]);
        if (prev_len + cur_len < MAX_LENGTH) {
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

/* Calculates visual cursor position based on tab expansion */
int get_render_x(int row, int physical_x) {
    int rx = 0, j;
    for (j = 0; j < physical_x && text_buffer[row][j] != '\0'; j++) {
        if (text_buffer[row][j] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

/* Maps a visual target column back to a physical string index for Up/Down navigation */
int get_physical_x(int row, int target_rx) {
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

void render_row(int row, char *out_buf) {
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

void refresh_screen(void) {
    int y, y_start = 0, print_len, len, file_row, rx;
    char status_bar[120];
    char r_buf[MAX_RENDER_BUF];
    
    get_terminal_size(&screen_rows, &screen_cols);
    
    printf("\x1b[?25l"); /* Hide cursor */
    printf("\x1b[H");    /* Move cursor to top-left */
    
    if (help_active) {
        printf("\x1b[7m"); 
        printf("----------------- Help (^K^H toggles) ----------------------\r\n");
        printf(" ^S = Left | ^D = Right | ^E = Up | ^X = Down               \r\n");
        printf(" ^K^D = Save & Exit  |  ^K^Q = Quit no save                 \r\n");
        printf("\x1b[m"); 
        y_start = 3;
    }
    
    for (y = y_start; y < screen_rows - 1; y++) {
        file_row = row_off + (y - y_start);
        if (file_row < num_lines) {
            render_row(file_row, r_buf);
            len = strlen(r_buf);
            if (len > col_off) {
                print_len = len - col_off;
                if (print_len > screen_cols) print_len = screen_cols;
                printf("%.*s", print_len, r_buf + col_off);
            }
        } else {
            if (!(num_lines == 1 && file_row == 0)) printf("~");
        }
        printf("\x1b[K\r\n"); 
    }
    
    /* Draw status bar */
    printf("\x1b[7m");
    sprintf(status_bar, " %s%s | File: %s | Line: %d ", 
            prefix_k ? "^K " : "", 
            help_active ? "" : "(Press ^K^H for Help)", 
            current_filename[0] ? current_filename : "NEW FILE", 
            cy + 1);
            
    len = strlen(status_bar);
    printf("%s", status_bar);
    for (y = len; y < screen_cols; y++) printf(" "); 
    printf("\x1b[m");
    
    rx = get_render_x(cy, cx);
    printf("\x1b[%d;%dH", (cy - row_off) + y_start + 1, (rx - col_off) + 1);
    printf("\x1b[?25h"); 
    fflush(stdout);
}

/* --- Main Loop --- */
int main(int argc, char *argv[]) {
    int c, len, cur_len, next_len, visible_rows, rx, i;
    int moved_vertically = 0;
    
    if (argc > 1) {
        load_file(argv[1]);
    } else {
        num_lines = 1;
        text_buffer[0][0] = '\0';
    }
    
    enable_raw_mode();
    
    while (1) {
        refresh_screen();
        c = read_key();
        moved_vertically = 0;
        
        if (prefix_k) {
            if (c == 8 || c == 'h' || c == 'H') {         
                help_active = !help_active;
            } else if (c == 4 || c == 'd' || c == 'D') {  
                save_file();
                break;
            } else if (c == 17 || c == 'q' || c == 'Q') { 
                break;
            }
            prefix_k = 0;
            continue;
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
                else if (cy > 0) { cy--; cx = strlen(text_buffer[cy]); }
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
                cx = strlen(text_buffer[cy]);
                break;
            case DEL_KEY:
                len = strlen(text_buffer[cy]);
                if (cx < len) {
                    memmove(&text_buffer[cy][cx], &text_buffer[cy][cx + 1], len - cx);
                } else if (cy < num_lines - 1) {
                    cur_len = strlen(text_buffer[cy]);
                    next_len = strlen(text_buffer[cy + 1]);
                    if (cur_len + next_len < MAX_LENGTH) {
                        strcat(text_buffer[cy], text_buffer[cy + 1]);
                        for (i = cy + 1; i < num_lines - 1; i++) {
                            strcpy(text_buffer[i], text_buffer[i + 1]);
                        }
                        num_lines--;
                    }
                }
                break;
            case INS_KEY:
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
        
        len = strlen(text_buffer[cy]);
        if (moved_vertically) {
            /* Map virtual memory target back to closest physical index */
            cx = get_physical_x(cy, target_rx);
        } else {
            /* Update virtual memory target whenever we type or move horizontally */
            if (cx > len) cx = len;
            target_rx = get_render_x(cy, cx);
        }
        
        visible_rows = screen_rows - (help_active ? 4 : 1);
        if (cy < row_off) row_off = cy;
        if (cy >= row_off + visible_rows) row_off = cy - visible_rows + 1;
        
        rx = get_render_x(cy, cx);
        if (rx < col_off) col_off = rx;
        if (rx >= col_off + screen_cols) col_off = rx - screen_cols + 1;
    }
    
    return 0;
}