/*
 * ed.c - Strict C89 MS-DOS 5.0 / QBASIC EDIT.COM clone
 * Keyboard-driven terminal text editor. Absolutely no mouse required.
 * 
 * Compilation:
 *   Windows:   gcc ed.c -o ed.exe -O2
 *   Linux/Mac: gcc ed.c -o ed -O2 -std=c89 -pedantic -Wall
 */

#if !defined(_WIN32) && !defined(WIN32) && !defined(__MSDOS__) && !defined(__DOS__)
    #define _DEFAULT_SOURCE
    #define _BSD_SOURCE
    #define _POSIX_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Platform Specific Terminal Handling --- */
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
    #include <sys/time.h>
    #include <sys/types.h>
    #include <unistd.h>
    #include <termios.h>
#endif

/* --- Configuration & Limits --- */
#define MAX_LINES 2000
#define MAX_LENGTH 255
#define MAX_RENDER_BUF 2048
#define TAB_STOP 4

/* --- ANSI DOS EDIT Themes --- */
#define COL_BG       "\x1b[1;37;44m"
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
    F1_KEY,
    ALT_F, ALT_E, ALT_S, ALT_O, ALT_H
};

/* --- Global State --- */
char text_buffer[MAX_LINES][MAX_LENGTH];
int num_lines = 0;
char current_filename[MAX_LENGTH] = "";
char clipboard_line[MAX_LENGTH] = "";
char search_term[MAX_LENGTH] = "";

int cx = 0, cy = 0;             
int target_rx = 0;              
int row_off = 0, col_off = 0;   
int screen_rows = 24, screen_cols = 80;
int exit_editor = 0;
int menu_mode = 0; /* 0: Editor, 1: Menu Bar, 2: Dropdown */
int menu_col = 0;
int menu_row = 0;

/* --- Menus Data Structure --- */
const char* menu_names[] = { " File ", " Edit ", " Search ", " Options ", " Help " };
int menu_x[] = { 2, 10, 19, 30, 42 };
const int num_menus = 5;

const char* file_menu[] = { " New          ", " Open...      ", " Save         ", " Save As...   ", " Exit         " };
const char* edit_menu[] = { " Cut          ", " Copy         ", " Paste        ", " Clear        " };
const char* search_menu[]={" Find...      ", " Find Next    " };
const char* options_menu[]={" Display...   " };
const char* help_menu[] = { " About...     " };

const char** dropdowns[] = { file_menu, edit_menu, search_menu, options_menu, help_menu };
int drop_sizes[] = { 5, 4, 2, 1, 1 };


/* --- Terminal Environment Control --- */
#ifdef _WIN32
HANDLE hStdin, hStdout;
DWORD fdwSaveOldMode;
DWORD fdwSaveOldOutMode;

void disable_raw_mode(void) {
    printf("\x1b[2J\x1b[H\x1b[0m\x1b[?25h");
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
    printf("\x1b[2J\x1b[H\x1b[0m\x1b[?25h");
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
#endif

#ifdef _WIN32
    if (!ReadFile(hStdin, &c, 1, &read_bytes, NULL) || read_bytes != 1) return 0;
#else
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1) exit(1);
    }
#endif

    if (c == '\r' || c == '\n') return KEY_ENTER;
    if (c == 8 || c == 127) return KEY_BACKSPACE;

    if (c == '\x1b') {
#ifdef _WIN32
        if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return KEY_ESC;
        if (!ReadFile(hStdin, &seq[0], 1, &read_bytes, NULL) || read_bytes != 1) return KEY_ESC;
#else
        if (!posix_kbhit() || read(STDIN_FILENO, &seq[0], 1) != 1) return KEY_ESC;
#endif

        if (seq[0] == 'f' || seq[0] == 'F') return ALT_F;
        if (seq[0] == 'e' || seq[0] == 'E') return ALT_E;
        if (seq[0] == 's' || seq[0] == 'S') return ALT_S;
        if (seq[0] == 'o' || seq[0] == 'O') return ALT_O;
        if (seq[0] == 'h' || seq[0] == 'H') return ALT_H;

        if (seq[0] == '[') {
#ifdef _WIN32
            if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return KEY_ESC;
            if (!ReadFile(hStdin, &seq[1], 1, &read_bytes, NULL) || read_bytes != 1) return KEY_ESC;
#else
            if (!posix_kbhit() || read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;
#endif
            if (seq[1] >= '0' && seq[1] <= '9') {
#ifdef _WIN32
                if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return KEY_ESC;
                if (!ReadFile(hStdin, &seq[2], 1, &read_bytes, NULL) || read_bytes != 1) return KEY_ESC;
#else
                if (!posix_kbhit() || read(STDIN_FILENO, &seq[2], 1) != 1) return KEY_ESC;
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
#ifdef _WIN32
            if (WaitForSingleObject(hStdin, 100) != WAIT_OBJECT_0) return KEY_ESC;
            if (!ReadFile(hStdin, &seq[1], 1, &read_bytes, NULL) || read_bytes != 1) return KEY_ESC;
#else
            if (!posix_kbhit() || read(STDIN_FILENO, &seq[1], 1) != 1) return KEY_ESC;
#endif
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
                case 'P': return F1_KEY;
            }
        }
        return KEY_ESC;
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
    if (*rows < 5) *rows = 24; 
}

/* --- Editor Core Logistics --- */
void load_file(const char *filename, int is_initial) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        if (!is_initial) {
            /* Handled locally or by show_message */
            return;
        } else {
            strncpy(current_filename, filename, MAX_LENGTH - 1);
            current_filename[MAX_LENGTH - 1] = '\0';
            num_lines = 1; text_buffer[0][0] = '\0';
            cx = 0; cy = 0; row_off = 0; col_off = 0;
            return;
        }
    }
    num_lines = 0;
    while (num_lines < MAX_LINES && fgets(text_buffer[num_lines], MAX_LENGTH, file)) {
        size_t len = strlen(text_buffer[num_lines]);
        while (len > 0 && (text_buffer[num_lines][len - 1] == '\n' || text_buffer[num_lines][len - 1] == '\r')) {
            text_buffer[num_lines][len - 1] = '\0';
            len--;
        }
        num_lines++;
    }
    fclose(file);
    if (num_lines == 0) {
        num_lines = 1; text_buffer[0][0] = '\0';
    }
    strncpy(current_filename, filename, MAX_LENGTH - 1);
    current_filename[MAX_LENGTH - 1] = '\0';
    cx = 0; cy = 0; row_off = 0; col_off = 0;
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
    int len = (int)strlen(text_buffer[cy]);
    if (len >= MAX_LENGTH - 1) return;
    memmove(&text_buffer[cy][cx + 1], &text_buffer[cy][cx], len - cx + 1);
    text_buffer[cy][cx] = (char)c;
    cx++;
}

void insert_newline(void) {
    int i;
    if (num_lines >= MAX_LINES) return;
    for (i = num_lines; i > cy + 1; i--) strcpy(text_buffer[i], text_buffer[i - 1]);
    strcpy(text_buffer[cy + 1], text_buffer[cy] + cx);
    text_buffer[cy][cx] = '\0';
    num_lines++;
    cy++;
    cx = 0;
}

void handle_backspace(void) {
    int i, len, prev_len, cur_len;
    if (cx > 0) {
        len = (int)strlen(text_buffer[cy]);
        memmove(&text_buffer[cy][cx - 1], &text_buffer[cy][cx], len - cx + 1);
        cx--;
    } else if (cy > 0) {
        prev_len = (int)strlen(text_buffer[cy - 1]);
        cur_len = (int)strlen(text_buffer[cy]);
        if (prev_len + cur_len < MAX_LENGTH) {
            strcat(text_buffer[cy - 1], text_buffer[cy]);
            for (i = cy; i < num_lines - 1; i++) strcpy(text_buffer[i], text_buffer[i + 1]);
            num_lines--; cy--; cx = prev_len;
        }
    }
}

void handle_delete(void) {
    int i, cur_len, next_len;
    cur_len = (int)strlen(text_buffer[cy]);
    if (cx < cur_len) {
        memmove(&text_buffer[cy][cx], &text_buffer[cy][cx + 1], cur_len - cx);
    } else if (cy < num_lines - 1) {
        next_len = (int)strlen(text_buffer[cy + 1]);
        if (cur_len + next_len < MAX_LENGTH) {
            strcat(text_buffer[cy], text_buffer[cy + 1]);
            for (i = cy + 1; i < num_lines - 1; i++) {
                strcpy(text_buffer[i], text_buffer[i + 1]);
            }
            num_lines--;
        }
    }
}

void delete_current_line(void) {
    int i;
    if (num_lines > 1) {
        for (i = cy; i < num_lines - 1; i++) strcpy(text_buffer[i], text_buffer[i + 1]);
        num_lines--;
        if (cy >= num_lines) cy = num_lines - 1;
        if (cx > (int)strlen(text_buffer[cy])) cx = (int)strlen(text_buffer[cy]);
    } else {
        text_buffer[0][0] = '\0';
        cx = 0;
    }
}

int get_render_x(int row, int physical_x) {
    int rx = 0, j;
    for (j = 0; j < physical_x && text_buffer[row][j] != '\0'; j++) {
        if (text_buffer[row][j] == '\t') rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        rx++;
    }
    return rx;
}

int get_physical_x(int row, int target_x) {
    int rx = 0, j;
    for (j = 0; text_buffer[row][j] != '\0'; j++) {
        int next_rx = rx;
        if (text_buffer[row][j] == '\t') next_rx += (TAB_STOP - 1) - (rx % TAB_STOP);
        next_rx++;
        if (next_rx > target_x) return j;
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

/* --- UI Rendering Primitives --- */
void draw_all(void) {
    int y, i, file_row, len, print_len, pad;
    char r_buf[MAX_RENDER_BUF];
    char status[256];
    char title_str[MAX_LENGTH + 64];
    
    printf("\x1b[?25l");
    
    /* 1. Menu Bar */
    printf("\x1b[1;1H" COL_MENU);
    for (i = 0; i < screen_cols; i++) printf(" ");
    for (i = 0; i < num_menus; i++) {
        printf("\x1b[1;%dH", menu_x[i]);
        if (menu_mode > 0 && menu_col == i) printf(COL_MENU_SEL);
        else printf(COL_MENU);
        printf("%s", menu_names[i]);
    }
    
    /* 2. Editor Header */
    printf("\x1b[2;1H" COL_BG);
    for(i=0; i<screen_cols; i++) printf(" ");
    sprintf(title_str, "[ %s ]", current_filename[0] ? current_filename : "Untitled");
    pad = (screen_cols - (int)strlen(title_str)) / 2;
    if (pad < 0) pad = 0;
    printf("\x1b[2;%dH%s", pad + 1, title_str);
    
    /* 3. Editor Content Area */
    for (y = 3; y <= screen_rows - 1; y++) {
        file_row = row_off + (y - 3);
        printf("\x1b[%d;1H" COL_BG, y); 
        
        if (file_row < num_lines) {
            render_row(file_row, r_buf);
            len = (int)strlen(r_buf);
            if (len > col_off) {
                print_len = len - col_off;
                if (print_len > screen_cols - 1) print_len = screen_cols - 1;
                printf("%.*s", print_len, r_buf + col_off);
                for (i = print_len; i < screen_cols - 1; i++) printf(" ");
            } else {
                for (i = 0; i < screen_cols - 1; i++) printf(" ");
            }
        } else {
            for (i = 0; i < screen_cols - 1; i++) printf(" ");
        }
        
        /* 4. Scrollbar Visual Element */
        printf("\x1b[%d;%dH" COL_STATUS, y, screen_cols);
        if (y == 3) printf("^");
        else if (y == screen_rows - 1) printf("v");
        else {
            int track_h = screen_rows - 5;
            int thumb = 0;
            if (num_lines > 1) thumb = (cy * (track_h - 1)) / (num_lines - 1);
            if (track_h > 0 && y - 4 == thumb) printf("\x1b[37;46m#\x1b[30;46m"); 
            else printf("|"); 
        }
    }
    
    /* 5. Status Bar */
    sprintf(status, " F1=Help  ESC=Menu                            Line:%-4d Col:%-4d", cy + 1, cx + 1);
    len = (int)strlen(status);
    if (len > screen_cols - 1) len = screen_cols - 1;
    printf("\x1b[%d;1H" COL_STATUS, screen_rows);
    printf("%.*s", len, status);
    for (i = len; i < screen_cols - 1; i++) printf(" ");
    
    /* 6. Active Dropdown Modal */
    if (menu_mode == 2) {
        int mx = menu_x[menu_col];
        int mh = drop_sizes[menu_col];
        const char **items = dropdowns[menu_col];
        
        for (i = 0; i < mh; i++) {
            printf("\x1b[%d;%dH", 2 + i, mx);
            if (menu_row == i) printf(COL_MENU_SEL);
            else printf(COL_MENU);
            printf("%s", items[i]);
            printf(COL_SHADOW " \x1b[0m");
        }
        printf("\x1b[%d;%dH" COL_SHADOW, 2 + mh, mx + 1);
        for (i = 0; i < (int)strlen(items[0]) + 1; i++) printf(" ");
    }
}

/* --- Dialog Elements --- */
int prompt_input(const char *title, char *buf) {
    int w = 40, h = 7;
    int x, y, len, c, i, j;
    len = (int)strlen(buf);
    
    while(1) {
        get_terminal_size(&screen_rows, &screen_cols);
        draw_all();
        
        x = (screen_cols - w) / 2;
        y = (screen_rows - h) / 2;
        
        printf(COL_SHADOW);
        for (j = 1; j <= h; j++) printf("\x1b[%d;%dH  ", y + j, x + w);
        printf("\x1b[%d;%dH", y + h, x + 2);
        for (i = 0; i < w; i++) printf(" ");
        
        printf(COL_MENU);
        for (j = 0; j < h; j++) {
            printf("\x1b[%d;%dH", y + j, x);
            if (j == 0 || j == h - 1) {
                printf("+"); for (i = 1; i < w - 1; i++) printf("-"); printf("+");
            } else {
                printf("|"); for (i = 1; i < w - 1; i++) printf(" "); printf("|");
            }
        }
        
        printf("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)strlen(title) - 2) / 2, title);
        
        printf("\x1b[%d;%dH" COL_MENU_SEL, y + 3, x + 4);
        for (i = 0; i < w - 8; i++) {
            if (i < len) printf("%c", buf[i]);
            else printf(" ");
        }
        
        printf("\x1b[%d;%dH" COL_MENU " < OK > ", y + 5, x + w / 2 - 4);
        
        printf("\x1b[%d;%dH\x1b[?25h", y + 3, x + 4 + len);
        fflush(stdout);
        
        c = read_key();
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

void show_message(const char *title, const char *msg) {
    int w = 40, h = 7;
    int x, y, c, i, j;
    
    while(1) {
        get_terminal_size(&screen_rows, &screen_cols);
        draw_all();
        
        x = (screen_cols - w) / 2;
        y = (screen_rows - h) / 2;
        
        printf(COL_SHADOW);
        for (j = 1; j <= h; j++) printf("\x1b[%d;%dH  ", y + j, x + w);
        printf("\x1b[%d;%dH", y + h, x + 2);
        for (i = 0; i < w; i++) printf(" ");
        
        printf(COL_MENU);
        for (j = 0; j < h; j++) {
            printf("\x1b[%d;%dH", y + j, x);
            if (j == 0 || j == h - 1) {
                printf("+"); for (i = 1; i < w - 1; i++) printf("-"); printf("+");
            } else {
                printf("|"); for (i = 1; i < w - 1; i++) printf(" "); printf("|");
            }
        }
        
        printf("\x1b[%d;%dH" COL_MENU " %s ", y, x + (w - (int)strlen(title) - 2) / 2, title);
        printf("\x1b[%d;%dH%s", y + 3, x + (w - (int)strlen(msg)) / 2, msg);
        printf("\x1b[%d;%dH" COL_MENU " < OK > ", y + 5, x + w / 2 - 4);
        fflush(stdout);
        
        c = read_key();
        if (c == KEY_ESC || c == KEY_ENTER || c == 32) return;
    }
}

void do_find(void) {
    int r, start_x;
    char *p;
    for (r = cy; r < num_lines; r++) {
        start_x = (r == cy) ? cx + 1 : 0;
        if (start_x < (int)strlen(text_buffer[r])) {
            p = strstr(text_buffer[r] + start_x, search_term);
            if (p) {
                cy = r;
                cx = p - text_buffer[r];
                target_rx = get_render_x(cy, cx);
                return;
            }
        }
    }
    show_message(" Find ", "Search term not found.");
}

void execute_menu(void) {
    char buf[MAX_LENGTH];
    menu_mode = 0;
    
    if (menu_col == 0) {
        if (menu_row == 0) {
            num_lines = 1; text_buffer[0][0] = '\0'; cy=0; cx=0; current_filename[0]='\0';
            row_off = 0; col_off = 0;
        } else if (menu_row == 1) {
            buf[0] = '\0';
            if (prompt_input(" Open ", buf) && buf[0] != '\0') {
                load_file(buf, 0);
            }
        } else if (menu_row == 2) {
            if (current_filename[0] == '\0') {
                buf[0] = '\0';
                if (prompt_input(" Save As ", buf) && buf[0] != '\0') {
                    strcpy(current_filename, buf);
                    save_file();
                }
            } else {
                save_file();
            }
        } else if (menu_row == 3) {
            strcpy(buf, current_filename);
            if (prompt_input(" Save As ", buf) && buf[0] != '\0') {
                strcpy(current_filename, buf);
                save_file();
            }
        } else if (menu_row == 4) {
            exit_editor = 1;
        }
    } else if (menu_col == 1) {
        if (menu_row == 0) {
            strcpy(clipboard_line, text_buffer[cy]);
            delete_current_line();
        } else if (menu_row == 1) {
            strcpy(clipboard_line, text_buffer[cy]);
        } else if (menu_row == 2) {
            if (num_lines < MAX_LINES) {
                int i;
                for (i = num_lines; i > cy + 1; i--) strcpy(text_buffer[i], text_buffer[i - 1]);
                strcpy(text_buffer[cy + 1], clipboard_line);
                num_lines++; cy++; cx = 0;
            }
        } else if (menu_row == 3) {
            delete_current_line();
        }
    } else if (menu_col == 2) {
        if (menu_row == 0) {
            buf[0] = '\0';
            if (prompt_input(" Find ", buf) && buf[0] != '\0') {
                strcpy(search_term, buf);
                do_find();
            }
        } else if (menu_row == 1) {
            if (search_term[0] != '\0') do_find();
        }
    } else if (menu_col == 3) {
        if (menu_row == 0) show_message(" Display ", "Blue ANSI Background Active");
    } else if (menu_col == 4) {
        if (menu_row == 0) show_message(" About ", "ed.exe (Strict C89 MS-DOS Editor)");
    }
}

int main(int argc, char *argv[]) {
    int c, rx, visible_rows, visible_cols, max_rows;
    int moved_vertically = 0;
    
    if (argc > 1) load_file(argv[1], 1);
    else { num_lines = 1; text_buffer[0][0] = '\0'; }
    
    enable_raw_mode();
    
    while (!exit_editor) {
        get_terminal_size(&screen_rows, &screen_cols);
        
        visible_rows = screen_rows - 3; 
        visible_cols = screen_cols - 1;
        
        if (cy < row_off) row_off = cy;
        if (cy >= row_off + visible_rows) row_off = cy - visible_rows + 1;
        rx = get_render_x(cy, cx);
        if (rx < col_off) col_off = rx;
        if (rx >= col_off + visible_cols) col_off = rx - visible_cols + 1;
        
        draw_all();
        
        if (menu_mode == 0) {
            rx = get_render_x(cy, cx);
            printf("\x1b[%d;%dH\x1b[?25h", (cy - row_off) + 3, (rx - col_off) + 1);
        } else {
            printf("\x1b[?25l");
        }
        fflush(stdout);
        
        c = read_key();
        moved_vertically = 0;
        
        if (c == KEY_ESC) {
            if (menu_mode == 0) { menu_mode = 1; menu_col = 0; }
            else if (menu_mode == 2) { menu_mode = 1; }
            else { menu_mode = 0; }
            continue;
        }
        
        if (c == ALT_F) { menu_mode = 2; menu_col = 0; menu_row = 0; continue; }
        if (c == ALT_E) { menu_mode = 2; menu_col = 1; menu_row = 0; continue; }
        if (c == ALT_S) { menu_mode = 2; menu_col = 2; menu_row = 0; continue; }
        if (c == ALT_O) { menu_mode = 2; menu_col = 3; menu_row = 0; continue; }
        if (c == ALT_H) { menu_mode = 2; menu_col = 4; menu_row = 0; continue; }
        
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
            else if (c == KEY_ENTER) { execute_menu(); }
        } else {
            switch (c) {
                case ARROW_UP: if (cy > 0) { cy--; moved_vertically = 1; } break;
                case ARROW_DOWN: if (cy < num_lines - 1) { cy++; moved_vertically = 1; } break;
                case ARROW_LEFT: 
                    if (cx > 0) cx--; 
                    else if (cy > 0) { cy--; cx = (int)strlen(text_buffer[cy]); }
                    break;
                case ARROW_RIGHT:
                    if (cx < (int)strlen(text_buffer[cy])) cx++;
                    else if (cy < num_lines - 1) { cy++; cx = 0; }
                    break;
                case PAGE_UP: cy -= (screen_rows - 3); if (cy < 0) cy = 0; moved_vertically = 1; break;
                case PAGE_DOWN: cy += (screen_rows - 3); if (cy >= num_lines) cy = num_lines - 1; moved_vertically = 1; break;
                case HOME_KEY: cx = 0; break;
                case END_KEY: cx = (int)strlen(text_buffer[cy]); break;
                case DEL_KEY: handle_delete(); break;
                case KEY_ENTER: insert_newline(); break;
                case KEY_BACKSPACE: handle_backspace(); break;
                case F1_KEY: show_message(" Help ", "ed.exe (MS-DOS Style)"); break;
                default: 
                    if ((c >= 32 && c <= 126) || c == KEY_TAB) {
                        insert_char(c);
                    }
                    break;
            }
            if (moved_vertically) cx = get_physical_x(cy, target_rx);
            else {
                if (cx > (int)strlen(text_buffer[cy])) cx = (int)strlen(text_buffer[cy]);
                target_rx = get_render_x(cy, cx);
            }
        }
    }
    
    printf("\x1b[2J\x1b[H\x1b[0m\x1b[?25h");
    return 0;
}