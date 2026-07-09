/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: mod_edlin.c
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Portable, self-contained line editor module for BASIC++. Ported
 *    from the standalone C89 edlin.c to strict C17. Operates entirely
 *    on its own static text buffer and communicates with the host
 *    interpreter exclusively through the EdlinCallbacks contract
 *    defined in mod_edlin.h.
 *
 * 2. WHAT TO EXPECT:
 *    The editor presents a '*' prompt and accepts single-letter
 *    commands: l(ist), i(nsert), d(elete), e(nd/save), q(uit),
 *    c(opy), m(ove), p(age), s(earch), r(eplace), t(ransfer),
 *    w(rite), a(ppend), x(execute), h/?. Numeric input edits that
 *    specific line. The 'x' command passes the buffer to the host
 *    for execution and returns to the editor prompt.
 *
 * 3. WHAT CAN BE CHANGED:
 *    - New editor commands can be added to the dispatch switch.
 *    - Buffer sizes are controlled by MAX_EDLIN_LINES/LENGTH in
 *      mod_edlin.h.
 *    - The output mechanism can be overridden via callbacks.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    - This file must NOT include any BASIC++ core headers (parser.h,
 *      exec.h, runtime.h, lexer.h, memory.h, etc.). All interaction
 *      with the interpreter flows through EdlinCallbacks.
 *    - The file must remain fully portable C17 with only standard
 *      library dependencies.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    - If the editor exits immediately: check that callbacks->read_line
 *      is not NULL and the console is connected to a terminal.
 *    - If 'x' does nothing: verify callbacks->execute_buffer is wired.
 *    - If lines are truncated: increase MAX_EDLIN_LENGTH in mod_edlin.h.
 * ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

#include "mod_edlin.h"

/* =====================================================================
 * Internal State
 * =====================================================================
 * The editor maintains its own static text buffer. This buffer is
 * completely independent of the BASIC++ program store. When the user
 * types 'x' (execute), the buffer is passed to the host via callback.
 * ===================================================================== */

static char edlin_buffer[MAX_EDLIN_LINES][MAX_EDLIN_LENGTH];
static int  edlin_line_count = 0;
static int  edlin_page_pos   = 0;
static char edlin_filename[MAX_EDLIN_LENGTH] = "";

/* Cached callbacks pointer (set once per edlin_start invocation). */
static const EdlinCallbacks *edlin_cb = NULL;

/* =====================================================================
 * Output Helpers
 * =====================================================================
 * All output flows through edlin_print() so the host can redirect it
 * to an SDL console window or any other display surface.
 * ===================================================================== */

/* edlin_print - Write a formatted string to the console.
 * Uses the write_str callback if available, otherwise falls back
 * to printf(). Thread-safe with respect to the callback contract. */
static void edlin_print(const char *fmt, ...)
{
    char buf[1024];
    va_list args;

    /* Format the string into a local buffer. */
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    /* Route through callback or fallback to stdout. */
    if (edlin_cb != NULL && edlin_cb->write_str != NULL) {
        edlin_cb->write_str(buf);
    } else {
        printf("%s", buf);
        fflush(stdout);
    }
}

/* get_edlin_page_size - Helper to determine console page size. */
static int get_edlin_page_size(void)
{
    if (edlin_cb != NULL && edlin_cb->get_terminal_height != NULL) {
        int height = edlin_cb->get_terminal_height();
        /* Sane terminal height must be > 5. Paginate with (height - 2)
         * to leave room for the prompt. */
        if (height > 5) {
            return height - 2;
        }
    }
    return 23; /* Fallback default */
}

/* =====================================================================
 * Input Helpers
 * =====================================================================
 * All input flows through edlin_read_line() so the host can supply
 * its own console reader (e.g. gw_console_read_line for SDL builds).
 * ===================================================================== */

/* edlin_read_line - Read a line of input from the console.
 * Returns buf on success, NULL on EOF. */
static char *edlin_read_line(char *buf, size_t max_len)
{
    if (edlin_cb != NULL && edlin_cb->read_line != NULL) {
        return edlin_cb->read_line(buf, max_len);
    }
    return fgets(buf, (int)max_len, stdin);
}

/* edlin_get_int_prompt - Display a prompt and read an integer.
 * Returns the parsed integer value, or 0 on invalid input. */
static int edlin_get_int_prompt(const char *prompt)
{
    char input[MAX_EDLIN_LENGTH];
    edlin_print("%s", prompt);
    if (edlin_read_line(input, MAX_EDLIN_LENGTH) == NULL) {
        return 0;
    }
    /* Use strtol for safe parsing (C17 best practice). */
    char *endptr = NULL;
    long val = strtol(input, &endptr, 10);
    if (endptr == input) {
        return 0;  /* No digits found */
    }
    return (int)val;
}

/* edlin_get_string_prompt - Display a prompt and read a string.
 * The trailing newline is stripped. */
static void edlin_get_string_prompt(const char *prompt, char *buffer)
{
    edlin_print("%s", prompt);
    if (edlin_read_line(buffer, MAX_EDLIN_LENGTH) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

/* =====================================================================
 * Core Editor Functions
 * =====================================================================
 * Each function operates on the static edlin_buffer. They are direct
 * C17 ports of the original C89 edlin.c functions with modernized
 * variable declarations, safe string handling, and removed globals
 * in favor of module-scoped statics.
 * ===================================================================== */

/* display_edlin_help - Print the editor command reference. */
static void display_edlin_help(void)
{
    int page_size = get_edlin_page_size();
    edlin_print("\nedlin - Built-in Line Editor for BASIC++\n");
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
    edlin_print("  s      - Search text\n");
    edlin_print("  t      - Transfer (merge) another file\n");
    edlin_print("  w      - Write lines to disk\n");
    edlin_print("  x      - Execute buffer as BASIC++ program\n\n");
}

/* load_edlin_file - Load a text file into the editor buffer.
 * Clears any existing buffer contents. Sets edlin_filename. */
static void load_edlin_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        edlin_line_count = 0;
        while (edlin_line_count < MAX_EDLIN_LINES &&
               fgets(edlin_buffer[edlin_line_count], MAX_EDLIN_LENGTH, file) != NULL) {
            size_t len = strlen(edlin_buffer[edlin_line_count]);
            if (len > 0 && edlin_buffer[edlin_line_count][len - 1] == '\n') {
                edlin_buffer[edlin_line_count][len - 1] = '\0';
            }
            edlin_line_count++;
        }
        fclose(file);
        edlin_print("End of input file\n");
    } else {
        edlin_print("New file\n");
    }
    snprintf(edlin_filename, MAX_EDLIN_LENGTH, "%s", filename);
}

/* save_edlin_file - Write the entire buffer to edlin_filename. */
static void save_edlin_file(void)
{
    FILE *file = fopen(edlin_filename, "w");
    if (file == NULL) {
        edlin_print("Error: Cannot save file.\n");
        return;
    }
    for (int i = 0; i < edlin_line_count; i++) {
        fprintf(file, "%s\n", edlin_buffer[i]);
    }
    fclose(file);
}

/* list_edlin_lines - Print all lines with their line numbers. */
static void list_edlin_lines(void)
{
    for (int i = 0; i < edlin_line_count; i++) {
        edlin_print("%d: %s\n", i + 1, edlin_buffer[i]);
    }
}

/* insert_edlin_line - Append lines to the end of the buffer.
 * The user types lines one at a time. A line containing only '.'
 * terminates insertion. */
static void insert_edlin_line(void)
{
    char input[MAX_EDLIN_LENGTH];

    if (edlin_line_count >= MAX_EDLIN_LINES) {
        edlin_print("Error: Buffer is full (%d lines).\n", MAX_EDLIN_LINES);
        return;
    }

    while (edlin_line_count < MAX_EDLIN_LINES) {
        edlin_print("%d:*", edlin_line_count + 1);
        if (edlin_read_line(input, MAX_EDLIN_LENGTH) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (strcmp(input, ".") == 0) {
            break;
        }
        snprintf(edlin_buffer[edlin_line_count], MAX_EDLIN_LENGTH, "%s", input);
        edlin_line_count++;
    }
}

/* delete_edlin_line - Delete a single line by number. */
static void delete_edlin_line(void)
{
    if (edlin_line_count == 0) {
        edlin_print("Error: Buffer is empty.\n");
        return;
    }

    int index = edlin_get_int_prompt("Line to delete: ") - 1;
    if (index >= 0 && index < edlin_line_count) {
        for (int i = index; i < edlin_line_count - 1; i++) {
            snprintf(edlin_buffer[i], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i + 1]);
        }
        edlin_line_count--;
        edlin_print("Line deleted.\n");
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

/* edit_edlin_line - Edit a specific line in-place.
 * Shows the current content, prompts for replacement. If the user
 * enters an empty line, the original content is preserved. */
static void edit_edlin_line(int index)
{
    char input[MAX_EDLIN_LENGTH];

    if (index >= 0 && index < edlin_line_count) {
        edlin_print("%d: %s\n", index + 1, edlin_buffer[index]);
        edlin_print("%d:*", index + 1);
        if (edlin_read_line(input, MAX_EDLIN_LENGTH) != NULL) {
            input[strcspn(input, "\n")] = '\0';
            if (strlen(input) > 0) {
                snprintf(edlin_buffer[index], MAX_EDLIN_LENGTH, "%s", input);
            }
        }
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

/* copy_edlin_lines - Copy a range of lines to a destination position. */
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
    if (edlin_line_count + count > MAX_EDLIN_LINES) {
        edlin_print("Error: Buffer limit reached.\n");
        return;
    }

    /* Shift lines down to make room at destination. */
    for (int i = edlin_line_count - 1; i >= dest; i--) {
        snprintf(edlin_buffer[i + count], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i]);
    }

    /* Copy source lines into the gap. Adjust source offset if the
     * destination was before the source (shift moved them). */
    int src_offset = (dest < start) ? count : 0;
    for (int j = 0; j < count; j++) {
        snprintf(edlin_buffer[dest + j], MAX_EDLIN_LENGTH, "%s",
                 edlin_buffer[start + src_offset + j]);
    }
    edlin_line_count += count;
    edlin_print("%d lines copied.\n", count);
}

/* move_edlin_lines - Move a range of lines to a destination position. */
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

    /* Shift lines down to make room at destination. */
    for (int i = edlin_line_count - 1; i >= dest; i--) {
        snprintf(edlin_buffer[i + count], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i]);
    }

    /* Copy source lines (adjusted for shift). */
    int del_start;
    if (dest < start) {
        for (int j = 0; j < count; j++) {
            snprintf(edlin_buffer[dest + j], MAX_EDLIN_LENGTH, "%s",
                     edlin_buffer[start + count + j]);
        }
        del_start = start + count;
    } else {
        for (int j = 0; j < count; j++) {
            snprintf(edlin_buffer[dest + j], MAX_EDLIN_LENGTH, "%s",
                     edlin_buffer[start + j]);
        }
        del_start = start;
    }

    /* Remove the original source lines. */
    for (int i = del_start; i < edlin_line_count; i++) {
        snprintf(edlin_buffer[i], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i + count]);
    }
    edlin_print("%d lines moved.\n", count);
}

/* page_edlin_display - Show paginated display based on console size. */
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
        edlin_print("%d: %s\n", i + 1, edlin_buffer[i]);
    }
    edlin_page_pos = end;
}

/* search_edlin_text - Search for a substring within a line range. */
static void search_edlin_text(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[MAX_EDLIN_LENGTH];
    int found = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    if (start < 0 || end >= edlin_line_count || start > end ||
        strlen(search_str) == 0) {
        return;
    }

    for (int i = start; i <= end; i++) {
        if (strstr(edlin_buffer[i], search_str) != NULL) {
            edlin_print("%d: %s\n", i + 1, edlin_buffer[i]);
            found++;
        }
    }
    edlin_print("%d matches found.\n", found);
}

/* replace_edlin_text - Find and replace text within a line range. */
static void replace_edlin_text(void)
{
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[MAX_EDLIN_LENGTH];
    char replace_str[MAX_EDLIN_LENGTH];
    int replaced = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    edlin_get_string_prompt("Replace with: ", replace_str);

    if (start < 0 || end >= edlin_line_count || start > end ||
        strlen(search_str) == 0) {
        return;
    }

    for (int i = start; i <= end; i++) {
        char *pos = strstr(edlin_buffer[i], search_str);
        if (pos != NULL) {
            char temp[MAX_EDLIN_LENGTH * 2];
            int prefix_len = (int)(pos - edlin_buffer[i]);
            /* Build replacement: prefix + replacement + suffix.
             * Use an oversized temp buffer to avoid gcc
             * -Wformat-truncation warnings. The strncpy below
             * safely truncates to MAX_EDLIN_LENGTH - 1. */
            snprintf(temp, sizeof(temp), "%.*s%s%s",
                     prefix_len, edlin_buffer[i],
                     replace_str,
                     pos + strlen(search_str));
            size_t temp_len = strlen(temp);
            size_t copy_len = temp_len < (size_t)(MAX_EDLIN_LENGTH - 1) ? temp_len : (size_t)(MAX_EDLIN_LENGTH - 1);
            memcpy(edlin_buffer[i], temp, copy_len);
            edlin_buffer[i][copy_len] = '\0';
            edlin_print("%d: %s\n", i + 1, edlin_buffer[i]);
            replaced++;
        }
    }
    edlin_print("%d lines updated.\n", replaced);
}

/* transfer_edlin_file - Merge another file into the buffer at a
 * specified insertion point. */
static void transfer_edlin_file(void)
{
    int dest = edlin_get_int_prompt("Insert before line: ") - 1;
    char filename[MAX_EDLIN_LENGTH];
    char input[MAX_EDLIN_LENGTH];

    edlin_get_string_prompt("Filename: ", filename);
    if (dest < 0) dest = 0;
    if (dest > edlin_line_count) dest = edlin_line_count;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        edlin_print("Error: Cannot open %s\n", filename);
        return;
    }

    while (edlin_line_count < MAX_EDLIN_LINES &&
           fgets(input, MAX_EDLIN_LENGTH, f) != NULL) {
        input[strcspn(input, "\n")] = '\0';
        /* Shift existing lines down by one. */
        for (int i = edlin_line_count; i > dest; i--) {
            snprintf(edlin_buffer[i], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i - 1]);
        }
        snprintf(edlin_buffer[dest], MAX_EDLIN_LENGTH, "%s", input);
        edlin_line_count++;
        dest++;
    }
    fclose(f);
    edlin_print("File transferred.\n");
}

/* write_edlin_lines - Write N lines from the top of the buffer to
 * the current file and remove them from memory. */
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
        fprintf(file, "%s\n", edlin_buffer[i]);
    }
    fclose(file);

    /* Remove written lines from the buffer by shifting. */
    for (int i = count; i < edlin_line_count; i++) {
        snprintf(edlin_buffer[i - count], MAX_EDLIN_LENGTH, "%s", edlin_buffer[i]);
    }
    edlin_line_count -= count;
    edlin_print("%d lines written to disk and cleared from memory.\n", count);
}

/* append_edlin_lines - Read additional lines from the current file
 * on disk, starting after the lines already in memory. */
static void append_edlin_lines(void)
{
    char input[MAX_EDLIN_LENGTH];
    int appended = 0;

    if (edlin_line_count >= MAX_EDLIN_LINES) {
        edlin_print("Error: Buffer is full.\n");
        return;
    }

    FILE *file = fopen(edlin_filename, "r");
    if (file == NULL) {
        edlin_print("Error: Cannot read file.\n");
        return;
    }

    /* Skip lines already in memory. */
    int skip = edlin_line_count;
    while (skip > 0 && fgets(input, MAX_EDLIN_LENGTH, file) != NULL) {
        skip--;
    }

    /* Append remaining lines. */
    while (edlin_line_count < MAX_EDLIN_LINES &&
           fgets(input, MAX_EDLIN_LENGTH, file) != NULL) {
        input[strcspn(input, "\n")] = '\0';
        snprintf(edlin_buffer[edlin_line_count], MAX_EDLIN_LENGTH, "%s", input);
        edlin_line_count++;
        appended++;
    }
    fclose(file);
    edlin_print("%d lines appended from disk.\n", appended);
}

/* execute_edlin_buffer - Invoke the host interpreter callback to
 * execute the current buffer contents as a BASIC++ program.
 * The text is passed exactly as the user typed it. */
static void execute_edlin_buffer(void)
{
    if (edlin_cb == NULL || edlin_cb->execute_buffer == NULL) {
        edlin_print("Error: No execution engine connected.\n");
        return;
    }
    if (edlin_line_count == 0) {
        edlin_print("Error: Buffer is empty.\n");
        return;
    }

    /* Build a const char* array pointing into the static buffer. */
    const char *line_ptrs[MAX_EDLIN_LINES];
    for (int i = 0; i < edlin_line_count; i++) {
        line_ptrs[i] = edlin_buffer[i];
    }

    edlin_print("\n--- Executing BASIC++ program ---\n\n");
    edlin_cb->execute_buffer(line_ptrs, edlin_line_count, edlin_cb->ctx);
    edlin_print("\n--- Execution complete ---\n\n");
}

/* =====================================================================
 * Public API
 * ===================================================================== */

/* edlin_start - Main entry point for the interactive editor.
 *
 * This function runs a blocking command loop. The user interacts
 * through single-letter commands at the '*' prompt. The loop exits
 * when the user types 'e' (save and exit) or 'q' (quit without save).
 *
 * Memory management: All buffers are module-scoped statics. No
 * dynamic allocation (malloc/free) is used anywhere in this module.
 *
 * Portability: This module uses only standard C17 library functions.
 * It does not include any BASIC++ core headers. It can be compiled
 * independently and linked into any C17 project. */
int edlin_start(const char *filename, const EdlinCallbacks *callbacks)
{
    char command[MAX_EDLIN_LENGTH];

    if (callbacks == NULL) {
        fprintf(stderr, "Error: EdlinCallbacks is NULL.\n");
        return 1;
    }

    /* Cache the callbacks for use by all internal functions. */
    edlin_cb = callbacks;

    /* Reset editor state for a fresh session. */
    edlin_line_count = 0;
    edlin_page_pos   = 0;
    memset(edlin_filename, 0, sizeof(edlin_filename));
    memset(edlin_buffer, 0, sizeof(edlin_buffer));

    /* Load file if specified. */
    if (filename != NULL && filename[0] != '\0') {
        load_edlin_file(filename);
    }

    edlin_print("Type '?' or 'h' for a list of available commands.\n");

    /* Main command loop. */
    for (;;) {
        edlin_print("*");
        if (edlin_read_line(command, MAX_EDLIN_LENGTH) == NULL) {
            break;  /* EOF */
        }
        command[strcspn(command, "\n")] = '\0';

        /* Skip empty input. */
        if (command[0] == '\0') {
            continue;
        }

        /* Numeric input: edit that line number. */
        if (isdigit((unsigned char)command[0])) {
            char *endptr = NULL;
            long line_val = strtol(command, &endptr, 10);
            edit_edlin_line((int)line_val - 1);
            continue;
        }

        /* Single-character command dispatch. */
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
            execute_edlin_buffer();
            break;
        default:
            edlin_print("Entry error (type '?' for help)\n");
            break;
        }
    }

edlin_exit:
    edlin_cb = NULL;  /* Clear cached callbacks */
    return 0;
}
