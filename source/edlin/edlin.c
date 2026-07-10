/*
 * VERSION: 1.3.2
 * LICENSE: MIT License
 * COPYLEFT: BASIC++ Community
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Terminal Size Headers --- */
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#define MAX_LINES 1000
#define MAX_LENGTH 255

char text_buffer[MAX_LINES][MAX_LENGTH];
int current_lines = 0;
int current_page = 0;
char current_filename[MAX_LENGTH] = "";

/* --- Terminal Size Helper --- */
int get_terminal_lines(void) {
    int lines = 24; /* Standard default terminal height */
    
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        lines = csbi.srWindow.Bottom - csbi.srWindow.Top + 1; 
    }
#else
    struct winsize w;
    /* STDOUT_FILENO is usually 1 */
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1) {
        if (w.ws_row > 0) {
            lines = w.ws_row;
        }
    }
#endif

    /* Subtract 1 to leave room for the input prompt, fallback to 23 */
    return (lines > 1) ? (lines - 1) : 23;
}

/* --- Input Helper Functions --- */
int get_int_prompt(const char *prompt) {
    char input[MAX_LENGTH];
    printf("%s", prompt);
    if (!fgets(input, MAX_LENGTH, stdin)) return 0;
    return atoi(input);
}

void get_string_prompt(const char *prompt, char *buffer) {
    printf("%s", prompt);
    if (fgets(buffer, MAX_LENGTH, stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

/* --- Core Editor Functions --- */
void display_help(void) {
    printf("\nedlin - Portable Line Editor (Version 1.3.2)\n");
    printf("Available Commands:\n");
    printf("  [line] - Edit a specific line (enter line number)\n");
    printf("  a     - Append lines from disk into memory\n");
    printf("  c     - Copy lines\n");
    printf("  d     - Delete line(s)\n");
    printf("  e     - End editing (Save and Exit)\n");
    printf("  h, ?  - Display this built-in help message\n");
    printf("  i     - Insert lines at the end of the file\n");
    printf("  l     - List all lines currently in the memory buffer\n");
    printf("  m     - Move lines\n");
    printf("  p     - Page display\n");
    printf("  q     - Quit the editor immediately without saving changes\n");
    printf("  r     - Replace text\n");
    printf("  s     - Search text\n");
    printf("  t     - Transfer (merge) another file\n");
    printf("  w     - Write lines to disk\n\n");
}

void load_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        current_lines = 0;
        while (current_lines < MAX_LINES && fgets(text_buffer[current_lines], MAX_LENGTH, file)) {
            size_t len = strlen(text_buffer[current_lines]);
            if (len > 0 && text_buffer[current_lines][len - 1] == '\n') {
                text_buffer[current_lines][len - 1] = '\0';
            }
            current_lines++;
        }
        fclose(file);
        printf("End of input file\n");
    } else {
        printf("New file\n");
    }
    strncpy(current_filename, filename, MAX_LENGTH - 1);
    current_filename[MAX_LENGTH - 1] = '\0';
}

void save_file(void) {
    int i;
    FILE *file = fopen(current_filename, "w");
    if (!file) {
        printf("Error: Cannot save file.\n");
        return;
    }
    for (i = 0; i < current_lines; i++) {
        fprintf(file, "%s\n", text_buffer[i]);
    }
    fclose(file);
}

void list_lines(void) {
    int i;
    for (i = 0; i < current_lines; i++) {
        printf("%d: %s\n", i + 1, text_buffer[i]);
    }
}

void insert_line(void) {
    char input[MAX_LENGTH];
    
    if (current_lines >= MAX_LINES) {
        printf("Error: Memory limit reached.\n");
        return;
    }
    
    while (current_lines < MAX_LINES) {
        printf("%d:*", current_lines + 1);
        if (!fgets(input, MAX_LENGTH, stdin)) break;
        
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, ".") == 0) break;
        strncpy(text_buffer[current_lines], input, MAX_LENGTH);
        current_lines++;
    }
}

void delete_line(void) {
    int index, i;
    
    if (current_lines == 0) {
        printf("Error: Buffer is empty.\n");
        return;
    }
    
    index = get_int_prompt("Line to delete: ") - 1;
    if (index >= 0 && index < current_lines) {
        for (i = index; i < current_lines - 1; i++) {
            strncpy(text_buffer[i], text_buffer[i + 1], MAX_LENGTH);
        }
        current_lines--;
        printf("Line deleted.\n");
    } else {
        printf("Error: Invalid line number.\n");
    }
}

void edit_line(int index) {
    char input[MAX_LENGTH];
    if (index >= 0 && index < current_lines) {
        printf("%d: %s\n", index + 1, text_buffer[index]);
        printf("%d:*", index + 1);
        if (fgets(input, MAX_LENGTH, stdin)) {
            input[strcspn(input, "\n")] = '\0';
            if (strlen(input) > 0) {
                strncpy(text_buffer[index], input, MAX_LENGTH);
            }
        }
    } else {
        printf("Error: Invalid line number.\n");
    }
}

/* --- New Extended Commands --- */
void copy_lines(void) {
    int start = get_int_prompt("Start line: ") - 1;
    int end = get_int_prompt("End line: ") - 1;
    int dest = get_int_prompt("Destination line: ") - 1;
    int count, i, j;
    
    if (start < 0 || end >= current_lines || start > end || dest < 0 || dest > current_lines) {
        printf("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        printf("Error: Cannot copy into source range.\n");
        return;
    }
    
    count = end - start + 1;
    if (current_lines + count > MAX_LINES) {
        printf("Error: Memory limit reached.\n");
        return;
    }
    
    for (i = current_lines - 1; i >= dest; i--) {
        strncpy(text_buffer[i + count], text_buffer[i], MAX_LENGTH);
    }
    
    if (dest < start) {
        for (j = 0; j < count; j++) {
            strncpy(text_buffer[dest + j], text_buffer[start + count + j], MAX_LENGTH);
        }
    } else {
        for (j = 0; j < count; j++) {
            strncpy(text_buffer[dest + j], text_buffer[start + j], MAX_LENGTH);
        }
    }
    current_lines += count;
    printf("%d lines copied.\n", count);
}

void move_lines(void) {
    int start = get_int_prompt("Start line: ") - 1;
    int end = get_int_prompt("End line: ") - 1;
    int dest = get_int_prompt("Destination line: ") - 1;
    int count, i, j, del_start;
    
    if (start < 0 || end >= current_lines || start > end || dest < 0 || dest > current_lines) {
        printf("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        printf("Error: Cannot move into source range.\n");
        return;
    }
    
    count = end - start + 1;
    for (i = current_lines - 1; i >= dest; i--) {
        strncpy(text_buffer[i + count], text_buffer[i], MAX_LENGTH);
    }
    
    if (dest < start) {
        for (j = 0; j < count; j++) {
            strncpy(text_buffer[dest + j], text_buffer[start + count + j], MAX_LENGTH);
        }
        del_start = start + count;
    } else {
        for (j = 0; j < count; j++) {
            strncpy(text_buffer[dest + j], text_buffer[start + j], MAX_LENGTH);
        }
        del_start = start;
    }
    
    for (i = del_start; i < current_lines; i++) {
        strncpy(text_buffer[i], text_buffer[i + count], MAX_LENGTH);
    }
    printf("%d lines moved.\n", count);
}

void page_display(void) {
    int i, end;
    
    /* Dynamically request the terminal size */
    int dynamic_lines = get_terminal_lines();
    
    if (current_page >= current_lines) current_page = 0;
    
    end = current_page + dynamic_lines;
    if (end > current_lines) end = current_lines;
    
    for (i = current_page; i < end; i++) {
        printf("%d: %s\n", i + 1, text_buffer[i]);
    }
    current_page = end;
}

void search_text(void) {
    int start = get_int_prompt("Start line: ") - 1;
    int end = get_int_prompt("End line: ") - 1;
    char search_str[MAX_LENGTH];
    int i, found = 0;
    
    get_string_prompt("Search for: ", search_str);
    if (start < 0 || end >= current_lines || start > end || strlen(search_str) == 0) return;
    
    for (i = start; i <= end; i++) {
        if (strstr(text_buffer[i], search_str)) {
            printf("%d: %s\n", i + 1, text_buffer[i]);
            found++;
        }
    }
    printf("%d matches found.\n", found);
}

void replace_text(void) {
    int start = get_int_prompt("Start line: ") - 1;
    int end = get_int_prompt("End line: ") - 1;
    char search_str[MAX_LENGTH];
    char replace_str[MAX_LENGTH];
    char temp[MAX_LENGTH];
    int i, replaced = 0;
    char *pos;
    
    get_string_prompt("Search for: ", search_str);
    get_string_prompt("Replace with: ", replace_str);
    
    if (start < 0 || end >= current_lines || start > end || strlen(search_str) == 0) return;
    
    for (i = start; i <= end; i++) {
        pos = strstr(text_buffer[i], search_str);
        if (pos) {
            int prefix_len = pos - text_buffer[i];
            strncpy(temp, text_buffer[i], prefix_len);
            temp[prefix_len] = '\0';
            strncat(temp, replace_str, MAX_LENGTH - strlen(temp) - 1);
            strncat(temp, pos + strlen(search_str), MAX_LENGTH - strlen(temp) - 1);
            strncpy(text_buffer[i], temp, MAX_LENGTH);
            printf("%d: %s\n", i + 1, text_buffer[i]);
            replaced++;
        }
    }
    printf("%d lines updated.\n", replaced);
}

void transfer_file(void) {
    int dest = get_int_prompt("Insert before line: ") - 1;
    char filename[MAX_LENGTH];
    char input[MAX_LENGTH];
    int i;
    FILE *f;
    
    get_string_prompt("Filename: ", filename);
    if (dest < 0) dest = 0;
    if (dest > current_lines) dest = current_lines;
    
    f = fopen(filename, "r");
    if (!f) {
        printf("Error: Cannot open %s\n", filename);
        return;
    }
    
    while (current_lines < MAX_LINES && fgets(input, MAX_LENGTH, f)) {
        input[strcspn(input, "\n")] = '\0';
        for (i = current_lines; i > dest; i--) {
            strncpy(text_buffer[i], text_buffer[i - 1], MAX_LENGTH);
        }
        strncpy(text_buffer[dest], input, MAX_LENGTH);
        current_lines++;
        dest++;
    }
    fclose(f);
    printf("File transferred.\n");
}

void write_lines(void) {
    int count = get_int_prompt("Number of lines to write: ");
    int i;
    FILE *file;
    
    if (count <= 0 || count > current_lines) return;
    file = fopen(current_filename, "a");
    if (!file) {
        printf("Error: Cannot write to file.\n");
        return;
    }
    for (i = 0; i < count; i++) {
        fprintf(file, "%s\n", text_buffer[i]);
    }
    fclose(file);
    
    for (i = count; i < current_lines; i++) {
        strncpy(text_buffer[i - count], text_buffer[i], MAX_LENGTH);
    }
    current_lines -= count;
    printf("%d lines written to disk and cleared from memory.\n", count);
}

void append_lines(void) {
    char input[MAX_LENGTH];
    FILE *file;
    int skip;
    int appended = 0;
    
    if (current_lines >= MAX_LINES) {
        printf("Error: Memory buffer is full.\n");
        return;
    }
    
    file = fopen(current_filename, "r");
    if (!file) {
        printf("Error: Cannot read file.\n");
        return;
    }
    
    /* Skip lines already in memory */
    skip = current_lines;
    while (skip > 0 && fgets(input, MAX_LENGTH, file)) {
        skip--;
    }
    
    while (current_lines < MAX_LINES && fgets(input, MAX_LENGTH, file)) {
        input[strcspn(input, "\n")] = '\0';
        strncpy(text_buffer[current_lines], input, MAX_LENGTH);
        current_lines++;
        appended++;
    }
    fclose(file);
    printf("%d lines appended from disk.\n", appended);
}

int main(int argc, char *argv[]) {
    char command[MAX_LENGTH];
    
    if (argc > 1) {
        load_file(argv[1]);
        printf("Type '?' or 'h' for a list of available commands.\n");
    } else {
        printf("Error: No file specified.\n");
        printf("Usage: edlin <filename>\n");
        return 1;
    }
    
    while (1) {
        printf("*");
        if (!fgets(command, MAX_LENGTH, stdin)) break;
        command[strcspn(command, "\n")] = '\0';
        
        if (command[0] >= '0' && command[0] <= '9') {
            edit_line(atoi(command) - 1);
        } else if (command[0] == 'a' || command[0] == 'A') {
            append_lines();
        } else if (command[0] == 'c' || command[0] == 'C') {
            copy_lines();
        } else if (command[0] == 'd' || command[0] == 'D') {
            delete_line();
        } else if (command[0] == 'e' || command[0] == 'E') {
            save_file();
            break;
        } else if (command[0] == 'h' || command[0] == 'H' || command[0] == '?') {
            display_help();
        } else if (command[0] == 'i' || command[0] == 'I') {
            insert_line();
        } else if (command[0] == 'l' || command[0] == 'L') {
            list_lines();
        } else if (command[0] == 'm' || command[0] == 'M') {
            move_lines();
        } else if (command[0] == 'p' || command[0] == 'P') {
            page_display();
        } else if (command[0] == 'q' || command[0] == 'Q') {
            break;
        } else if (command[0] == 'r' || command[0] == 'R') {
            replace_text();
        } else if (command[0] == 's' || command[0] == 'S') {
            search_text();
        } else if (command[0] == 't' || command[0] == 'T') {
            transfer_file();
        } else if (command[0] == 'w' || command[0] == 'W') {
            write_lines();
        } else if (strlen(command) > 0) {
            printf("Entry error (type '?' for help)\n");
        }
    }
    return 0;
}