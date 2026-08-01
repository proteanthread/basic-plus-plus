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

#define get_terminal_size edlin_get_terminal_size
#define reset_term edlin_reset_term
#define init_term edlin_init_term
#define get_input edlin_get_input
#ifndef STANDALONE_EDITOR
#define main mod_edlin_main
#endif
#ifdef _MSC_VER
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#endif
/* 
 *
 * Standalone DOS EDLIN Clone
 * VERSION: 3.1.0
 * LICENSE: MIT License
 * COPYLEFT: BASIC++ Community
 *
 * edlin.c - based on the original MSDOS line editor
 *
 *    The editor presents a '*' prompt and accepts single-letter
 *    commands: l(ist), i(nsert), d(elete), e(nd/save), q(uit),
 *    c(opy), m(ove), p(age), s(earch), r(eplace), t(ransfer),
 *    w(rite), a(ppend), h/?. Numeric input edits that specific line.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>

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
#include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif


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

/* =====================================================================
 * Internal State
 * ===================================================================== */

typedef struct {
    char *text;
    int length;
    int capacity;
} Line;
static Line *edlin_buffer = NULL;
static int edlin_buffer_capacity = 0;
static int  edlin_line_count = 0;
static int  edlin_page_pos   = 0;
static char edlin_filename[4096] = "";

#ifndef STANDALONE_EDITOR
static struct VMContext *current_vm = NULL;
#endif

static void oom(void) {
    fprintf(stderr, "\n\nOut of memory!\n");
    return;
}

static void ensure_line_capacity(int row, int needed) {
    if (needed > edlin_buffer[row].capacity) {
        int new_cap = edlin_buffer[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        char *new_text = realloc(edlin_buffer[row].text, new_cap);
        if (!new_text) { oom(); return; }
        edlin_buffer[row].text = new_text;
        edlin_buffer[row].capacity = new_cap;
    }
}

static void ensure_buffer_capacity(int needed) {
    if (needed > edlin_buffer_capacity) {
        int new_cap = edlin_buffer_capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        Line *new_buf = realloc(edlin_buffer, new_cap * sizeof(Line));
        if (!new_buf) { oom(); return; }
        edlin_buffer = new_buf;
        edlin_buffer_capacity = new_cap;
    }
}

static void free_line(int row) {
    if (edlin_buffer[row].text) {
        free(edlin_buffer[row].text);
        edlin_buffer[row].text = NULL;
    }
}

static void insert_empty_line_at(int row) {
    ensure_buffer_capacity(edlin_line_count + 1);
    for (int i = edlin_line_count; i > row; i--) {
        edlin_buffer[i] = edlin_buffer[i - 1];
    }
    edlin_buffer[row].text = (char *)calloc(1, 128);
    if (!edlin_buffer[row].text) { oom(); return; }
    edlin_buffer[row].text[0] = '\0';
    edlin_buffer[row].length = 0;
    edlin_buffer[row].capacity = 128;
    edlin_line_count++;
}


/* =====================================================================
 * Output Helpers
 * ===================================================================== */

static void edlin_print(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);
}

static int get_edlin_page_size(void)
{
    int height = 23;
#if defined(_WIN32) || defined(WIN32)
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_row > 0) {
        height = w.ws_row;
    }
#endif
    /* Paginate with (height - 2) to leave room for the prompt. */
    if (height > 5) {
        return height - 2;
    }
    return 23; /* Fallback default */
}

/* =====================================================================
 * Input Helpers
 * ===================================================================== */

static char *edlin_read_line(char *buf, size_t max_len)
{
    char *ret = fgets(buf, (int)max_len, stdin);
    if (ret != NULL) {
        if (buf[0] == 3 || buf[0] == 4) return NULL; /* Ctrl+C/D to exit */
    }
    return ret;
}

static int edlin_get_int_prompt(const char *prompt)
{
    char input[4096];
    edlin_print("%s", prompt);
    if (edlin_read_line(input, 4096) == NULL) {
        return 0;
    }
    char *endptr = NULL;
    long val = strtol(input, &endptr, 10);
    if (endptr == input) {
        return 0;  /* No digits found */
    }
    return (int)val;
}

static void edlin_get_string_prompt(const char *prompt, char *buffer)
{
    edlin_print("%s", prompt);
    if (edlin_read_line(buffer, 4096) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

/* =====================================================================
 * Core Editor Functions
 * ===================================================================== */

static void display_edlin_help(void)
{
    int page_size = get_edlin_page_size();
    edlin_print("\nedlin - Built-in Line Editor\n");
    edlin_print("Available Commands:\n");
    edlin_print("  [line] - Edit a specific line (enter line number)\n");
    edlin_print("  a      - Append lines from disk into memory\n");
    edlin_print("  c      - Copy lines\n");
    edlin_print("  d      - Delete line(s)\n");
    edlin_print("  e      - End editing (Save and Exit)\n");
    edlin_print("  h, ?   - Display this help message\n");
    edlin_print("  i      - Insert lines at the end of the buffer\n");
    edlin_print("  l      - List all lines currently in the buffer\n");
    edlin_print("  m      - Move lines\n");
    edlin_print("  p      - Page display (%d lines at a time)\n", page_size);
    edlin_print("  q      - Quit without saving\n");
    edlin_print("  r      - Replace text\n");
    edlin_print("  run    - Run program\n");
    edlin_print("  s      - Search text\n");
    edlin_print("  t      - Transfer (merge) another file\n");
    edlin_print("  w      - Write lines to disk\n");
    edlin_print("  x      - Execute program\n\n");
}

static void load_edlin_file(const char *filename)
{
    if (edlin_buffer) {
        for (int i = 0; i < edlin_line_count; i++) free_line(i);
    }
    edlin_line_count = 0;
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        char line_buf[4096];
        while (fgets(line_buf, sizeof(line_buf), file) != NULL) {
            size_t len = (int)strlen(line_buf);
            if (len > 0 && line_buf[len - 1] == '\n') {
                line_buf[len - 1] = '\0'; len--;
            }
            if (len > 0 && line_buf[len - 1] == '\r') {
                line_buf[len - 1] = '\0'; len--;
            }
            insert_empty_line_at(edlin_line_count);
            ensure_line_capacity(edlin_line_count - 1, len + 1);
            strcpy(edlin_buffer[edlin_line_count - 1].text, line_buf);
            edlin_buffer[edlin_line_count - 1].length = len;
        }
        fclose(file);
        edlin_print("End of input file\n");
    } else {
        edlin_print("New file\n");
    }
    snprintf(edlin_filename, 4096, "%s", filename);
}

static void save_edlin_file(void);

#ifndef STANDALONE_EDITOR
static bool edlin_debug_step = false;
static bool edlin_trace_mode = false;
#endif

#ifndef STANDALONE_EDITOR
static void edlin_debug_hook(struct VMContext *vm, const char *event_type, int line_num, const char *symbol, void *user_data) {
    (void)vm; (void)symbol; (void)user_data;
    if (strcmp(event_type, "step") == 0) {
        if (edlin_trace_mode) {
            printf("[TRACE] Executing Line %d\n", line_num);
        } else if (edlin_debug_step) {
            printf("[DEBUG] Line %d. Press Space to step, Enter to continue...\n", line_num);
            int key;
            do {
                key = getchar();
            } while (key != ' ' && key != '\n' && key != '\r');
            if (key == '\n' || key == '\r') {
                edlin_debug_step = false;
            }
        }
    }
}
#endif

static void execute_program_edlin(int exec_mode) {
#ifdef STANDALONE_EDITOR
    const char *target = edlin_filename[0] ? edlin_filename : "untitled.bas";
    FILE *file = fopen(target, "w");
    if (file) {
        for (int i = 0; i < edlin_line_count; i++) {
            fprintf(file, "%s\n", edlin_buffer[i].text);
        }
        fclose(file);
    }
    
    edlin_print("\x1b[2J\x1b[H"); fflush(stdout);
    execute_standalone(target, exec_mode);
    
    edlin_print("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
#else
    if (!current_vm) return;
    const char *target = edlin_filename[0] ? edlin_filename : "untitled.bas";
    FILE *file = fopen(target, "w");
    if (file) {
        for (int i = 0; i < edlin_line_count; i++) {
            fprintf(file, "%s\n", edlin_buffer[i].text);
        }
        fclose(file);
    }
    
    edlin_print("\x1b[2J\x1b[H"); fflush(stdout);
    
    edlin_debug_step = (exec_mode == 1);
    edlin_trace_mode = (exec_mode == 2);
    if (exec_mode > 0) {
        vm_set_debug_hook(current_vm, edlin_debug_hook, NULL);
    } else {
        vm_set_debug_hook(current_vm, NULL, NULL);
    }
    
    vm_load_program_file(current_vm, target);
    vm_run_program(current_vm);
    edlin_print("\n[Press Enter to return to editor...]\n"); fflush(stdout);
    getchar();
#endif
}

static void save_edlin_file(void)
{
    FILE *file = fopen(edlin_filename, "w");
    if (file == NULL) {
        edlin_print("Error: Cannot save file.\n");
        return;
    }
    for (int i = 0; i < edlin_line_count; i++) {
        fprintf(file, "%s\n", edlin_buffer[i].text);
    }
    fclose(file);
}

static void list_edlin_lines(void)
{
    for (int i = 0; i < edlin_line_count; i++) {
        edlin_print("%d: %s\n", i + 1, edlin_buffer[i].text);
    }
}

static void insert_edlin_line(void)
{
    char input[4096];

    while (1) {
        edlin_print("%d:*", edlin_line_count + 1);
        if (edlin_read_line(input, 4096) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (input[strcspn(input, "\r")] == '\r') input[strcspn(input, "\r")] = '\0';
        
        if (strcmp(input, ".") == 0) {
            break;
        }
        insert_empty_line_at(edlin_line_count);
        size_t len = (int)strlen(input);
        ensure_line_capacity(edlin_line_count - 1, len + 1);
        strcpy(edlin_buffer[edlin_line_count - 1].text, input);
        edlin_buffer[edlin_line_count - 1].length = len;
    }
}

static void delete_edlin_line(void)
{
    if (edlin_line_count == 0) {
        edlin_print("Error: Buffer is empty.\n");
        return;
    }

    int index = edlin_get_int_prompt("Line to delete: ") - 1;
    if (index >= 0 && index < edlin_line_count) {
        free_line(index);
        for (int i = index; i < edlin_line_count - 1; i++) {
            edlin_buffer[i] = edlin_buffer[i + 1];
        }
        edlin_line_count--;
        edlin_print("Line deleted.\n");
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

static void edit_edlin_line(int index)
{
    char input[4096];

    if (index >= 0 && index < edlin_line_count) {
        edlin_print("%d: %s\n", index + 1, edlin_buffer[index].text);
        edlin_print("%d:*", index + 1);
        if (edlin_read_line(input, 4096) != NULL) {
            input[strcspn(input, "\n")] = '\0';
            if (input[strcspn(input, "\r")] == '\r') input[strcspn(input, "\r")] = '\0';
            
            if ((int)strlen(input) > 0) {
                size_t len = (int)strlen(input);
                ensure_line_capacity(index, len + 1);
                strcpy(edlin_buffer[index].text, input);
                edlin_buffer[index].length = len;
            }
        }
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

static void copy_edlin_lines(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    int dest  = edlin_get_int_prompt("Destination line: ") - 1;

    if (start < 0 || end >= edlin_line_count || start > end ||
        dest < 0 || dest > edlin_line_count) {
        edlin_print("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        edlin_print("Error: Cannot copy into source range.\n");
        return;
    }

    int count = end - start + 1;
    ensure_buffer_capacity(edlin_line_count + count);

    for (int i = edlin_line_count - 1; i >= dest; i--) {
        edlin_buffer[i + count] = edlin_buffer[i];
    }

    int src_offset = (dest < start) ? count : 0;
    for (int j = 0; j < count; j++) {
        size_t len = edlin_buffer[start + src_offset + j].length;
        edlin_buffer[dest + j].text = (char *)calloc(1, len + 1);
        if (!edlin_buffer[dest + j].text) { oom(); return; }
        strcpy(edlin_buffer[dest + j].text, edlin_buffer[start + src_offset + j].text);
        edlin_buffer[dest + j].length = len;
        edlin_buffer[dest + j].capacity = len + 1;
    }
    edlin_line_count += count;
    edlin_print("%d lines copied.\n", count);
}

static void move_edlin_lines(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    int dest  = edlin_get_int_prompt("Destination line: ") - 1;

    if (start < 0 || end >= edlin_line_count || start > end ||
        dest < 0 || dest > edlin_line_count) {
        edlin_print("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        edlin_print("Error: Cannot move into source range.\n");
        return;
    }

    int count = end - start + 1;
    ensure_buffer_capacity(edlin_line_count + count);

    for (int i = edlin_line_count - 1; i >= dest; i--) {
        edlin_buffer[i + count] = edlin_buffer[i];
    }

    int del_start;
    if (dest < start) {
        for (int j = 0; j < count; j++) {
            edlin_buffer[dest + j] = edlin_buffer[start + count + j];
        }
        del_start = start + count;
    } else {
        for (int j = 0; j < count; j++) {
            edlin_buffer[dest + j] = edlin_buffer[start + j];
        }
        del_start = start;
    }

    for (int i = del_start; i < edlin_line_count; i++) {
        edlin_buffer[i] = edlin_buffer[i + count];
    }
    edlin_print("%d lines moved.\n", count);
}

static void page_edlin_display(void)
{
    if (edlin_page_pos >= edlin_line_count) {
        edlin_page_pos = 0;
    }
    int page_size = get_edlin_page_size();
    int end = edlin_page_pos + page_size;
    if (end > edlin_line_count) {
        end = edlin_line_count;
    }

    for (int i = edlin_page_pos; i < end; i++) {
        edlin_print("%d: %s\n", i + 1, edlin_buffer[i].text);
    }
    edlin_page_pos = end;
}

static void search_edlin_text(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[4096];
    int found = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    if (start < 0 || end >= edlin_line_count || start > end ||
        (int)strlen(search_str) == 0) {
        return;
    }

    for (int i = start; i <= end; i++) {
        if (strstr(edlin_buffer[i].text, search_str) != NULL) {
            edlin_print("%d: %s\n", i + 1, edlin_buffer[i].text);
            found++;
        }
    }
    edlin_print("%d matches found.\n", found);
}

static void replace_edlin_text(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[4096];
    char replace_str[4096];
    int replaced = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    edlin_get_string_prompt("Replace with: ", replace_str);

    if (start < 0 || end >= edlin_line_count || start > end ||
        (int)strlen(search_str) == 0) {
        return;
    }

    for (int i = start; i <= end; i++) {
        char *pos = strstr(edlin_buffer[i].text, search_str);
        if (pos != NULL) {
            char temp[8192];
            int prefix_len = (int)(pos - edlin_buffer[i].text);
            snprintf(temp, sizeof(temp), "%.*s%s%s",
                     prefix_len, edlin_buffer[i].text,
                     replace_str,
                     pos + (int)strlen(search_str));
            size_t temp_len = (int)strlen(temp);
            ensure_line_capacity(i, temp_len + 1);
            strcpy(edlin_buffer[i].text, temp);
            edlin_buffer[i].length = temp_len;
            edlin_print("%d: %s\n", i + 1, edlin_buffer[i].text);
            replaced++;
        }
    }
    edlin_print("%d lines updated.\n", replaced);
}

static void transfer_edlin_file(void)
{
    int dest = edlin_get_int_prompt("Insert before line: ") - 1;
    char filename[4096];
    char input[4096];

    edlin_get_string_prompt("Filename: ", filename);
    if (dest < 0) dest = 0;
    if (dest > edlin_line_count) dest = edlin_line_count;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        edlin_print("Error: Cannot open %s\n", filename);
        return;
    }

    while (fgets(input, sizeof(input), f) != NULL) {
        input[strcspn(input, "\n")] = '\0';
        if (input[strcspn(input, "\r")] == '\r') input[strcspn(input, "\r")] = '\0';
        
        insert_empty_line_at(dest);
        size_t len = (int)strlen(input);
        ensure_line_capacity(dest, len + 1);
        strcpy(edlin_buffer[dest].text, input);
        edlin_buffer[dest].length = len;
        dest++;
    }
    fclose(f);
    edlin_print("File transferred.\n");
}

static void write_edlin_lines(void)
{
    int count = edlin_get_int_prompt("Number of lines to write: ");
    if (count <= 0 || count > edlin_line_count) {
        return;
    }

    FILE *file = fopen(edlin_filename, "a");
    if (file == NULL) {
        edlin_print("Error: Cannot write to file.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        fprintf(file, "%s\n", edlin_buffer[i].text);
    }
    fclose(file);

    for (int i = 0; i < count; i++) {
        free_line(i);
    }
    for (int i = count; i < edlin_line_count; i++) {
        edlin_buffer[i - count] = edlin_buffer[i];
    }
    edlin_line_count -= count;
    edlin_print("%d lines written to disk and cleared from memory.\n", count);
}

static void append_edlin_lines(void)
{
    char input[4096];
    int appended = 0;

    FILE *file = fopen(edlin_filename, "r");
    if (file == NULL) {
        edlin_print("Error: Cannot read file.\n");
        return;
    }

    int skip = edlin_line_count;
    while (skip > 0 && fgets(input, sizeof(input), file) != NULL) {
        sanitize_ascii(input);
        skip--;
    }

    while (fgets(input, sizeof(input), file) != NULL) {
        input[strcspn(input, "\n")] = '\0';
        if (input[strcspn(input, "\r")] == '\r') input[strcspn(input, "\r")] = '\0';
        
        insert_empty_line_at(edlin_line_count);
        size_t len = (int)strlen(input);
        ensure_line_capacity(edlin_line_count - 1, len + 1);
        strcpy(edlin_buffer[edlin_line_count - 1].text, input);
        edlin_buffer[edlin_line_count - 1].length = len;
        appended++;
    }
    fclose(file);
    edlin_print("%d lines appended from disk.\n", appended);
}

/* =====================================================================
 * Public API
 * ===================================================================== */

#ifdef STANDALONE_EDITOR
int main(int argc, char **argv) {
    const char *filename = (argc > 1) ? argv[1] : "";
    (void)argc;
#else
int main(VMContext *vm, const char *filename)
{
    current_vm = vm;
#endif
#if defined(_WIN32) || defined(WIN32)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    edlin_print("%s", bright_colors[color_index]);

    char command[4096];

    edlin_line_count = 0;
    edlin_page_pos   = 0;
    memset(edlin_filename, 0, sizeof(edlin_filename));
    /* memset(edlin_buffer, 0, sizeof(edlin_buffer)); */

    if ((filename ? 2 : 1) > 1) {
        load_edlin_file(filename);
    }

    edlin_print("Type '?' or 'h' for a list of available commands.\n");

    for (;;) {
        edlin_print("*");
        if (edlin_read_line(command, 4096) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = '\0';
        if (command[strcspn(command, "\r")] == '\r') command[strcspn(command, "\r")] = '\0';

        if (command[0] == '\0') {
            continue;
        }

        if (isdigit((unsigned char)command[0])) {
            char *endptr = NULL;
            long line_val = strtol(command, &endptr, 10);
            edit_edlin_line((int)line_val - 1);
            continue;
        }
        if (strcmp(command, "run") == 0 || strcmp(command, "r!") == 0) {
            execute_program_edlin(0);
            continue;
        }
        if (strcmp(command, "/debug") == 0) {
            execute_program_edlin(1);
            continue;
        }
        if (strcmp(command, "/trace") == 0) {
            execute_program_edlin(2);
            continue;
        }

        if (strcmp(command, "bye") == 0) {
            goto edlin_exit;
        }

        switch (tolower((unsigned char)command[0])) {
        case 'a':
            append_edlin_lines();
            break;
        case 'c':
            copy_edlin_lines();
            break;
        case 'd':
            delete_edlin_line();
            break;
        case 'e':
            save_edlin_file();
            goto edlin_exit;
        case 'h':
        case '?':
            display_edlin_help();
            break;
        case 'i':
            insert_edlin_line();
            break;
        case 'l':
            list_edlin_lines();
            break;
        case 'm':
            move_edlin_lines();
            break;
        case 'o':
            color_index = (color_index + 1) % NUM_BRIGHT_COLORS;
            edlin_print("%s\x1b[2J\x1b[H[Color Changed]\r\n", bright_colors[color_index]);
            break;
        case 'p':
            page_edlin_display();
            break;
        case 'q':
            goto edlin_exit;
        case 'r':
            replace_edlin_text();
            break;
        case 's':
            search_edlin_text();
            break;
        case 't':
            transfer_edlin_file();
            break;
        case 'w':
            write_edlin_lines();
            break;
        case 'x':
            execute_program_edlin(0);
            break;
        default:
            edlin_print("Entry error (type '?' for help)\n");
            break;
        }
    }

edlin_exit:
    edlin_print("\x1b[0m\x1b[2J\x1b[H");
    return 0;
}
