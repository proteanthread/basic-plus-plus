// FILENAME: edlin_cmd.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libstandard (edlin_internal.h)
// Implements visual text editor subsystem components for edlin_cmd.
//
// ---- Includes ----

#include "editor/edlin_internal.h"
#include "platform/platform.h"
#include "runtime/format/snprintf.h"
#include "runtime/string/strops.h"
#include "runtime/conv/num_parse.h"

//
// ---- Console I/O Helpers ----

void edlin_print(const char *fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    platform_console_puts(buf);
    platform_console_flush();
}

int edlin_get_page_size(void) {
    int height = platform_console_height();
    if (height > 5) return height - 2;
    return 23;
}

char *edlin_read_line(char *buf, size_t max_len) {
    char *ret = platform_console_gets(buf, max_len);
    if (ret != NULL) {
        if (buf[0] == 3 || buf[0] == 4) return NULL;
    }
    return ret;
}

int edlin_get_int_prompt(const char *prompt) {
    char input[4096];
    edlin_print("%s", prompt);
    if (edlin_read_line(input, sizeof(input)) == NULL) return 0;
    return runtime_atoi(input);
}

void edlin_get_string_prompt(const char *prompt, char *buffer) {
    edlin_print("%s", prompt);
    if (edlin_read_line(buffer, 4096) != NULL) {
        size_t len = runtime_strlen(buffer);
        while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
            buffer[len - 1] = '\0';
            len--;
        }
    } else {
        buffer[0] = '\0';
    }
}

//
// ---- Line Editing Commands ----

void list_edlin_lines(void) {
    for (int i = 0; i < g_edlin_line_count; i++) {
        edlin_print("%d: %s\n", i + 1, g_edlin_buffer[i].text);
    }
}

void insert_edlin_line(void) {
    char input[4096];
    while (1) {
        edlin_print("%d:*", g_edlin_line_count + 1);
        if (edlin_read_line(input, sizeof(input)) == NULL) break;
        size_t len = runtime_strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[len - 1] = '\0';
            len--;
        }
        
        if (runtime_strcmp(input, ".") == 0) break;
        edlin_insert_empty_line_at(g_edlin_line_count);
        edlin_ensure_line_capacity(g_edlin_line_count - 1, (int)len + 1);
        runtime_strcpy(g_edlin_buffer[g_edlin_line_count - 1].text, input);
        g_edlin_buffer[g_edlin_line_count - 1].length = (int)len;
    }
}

void delete_edlin_line(void) {
    if (g_edlin_line_count == 0) {
        edlin_print("Error: Buffer is empty.\n");
        return;
    }
    int index = edlin_get_int_prompt("Line to delete: ") - 1;
    if (index >= 0 && index < g_edlin_line_count) {
        edlin_free_line(index);
        for (int i = index; i < g_edlin_line_count - 1; i++) {
            g_edlin_buffer[i] = g_edlin_buffer[i + 1];
        }
        g_edlin_line_count--;
        edlin_print("Line deleted.\n");
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

void edit_edlin_line(int index) {
    char input[4096];
    if (index >= 0 && index < g_edlin_line_count) {
        edlin_print("%d: %s\n", index + 1, g_edlin_buffer[index].text);
        edlin_print("%d:*", index + 1);
        if (edlin_read_line(input, sizeof(input)) != NULL) {
            size_t len = runtime_strlen(input);
            while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
                input[len - 1] = '\0';
                len--;
            }
            
            if (len > 0) {
                edlin_ensure_line_capacity(index, (int)len + 1);
                runtime_strcpy(g_edlin_buffer[index].text, input);
                g_edlin_buffer[index].length = (int)len;
            }
        }
    } else {
        edlin_print("Error: Invalid line number.\n");
    }
}

void copy_edlin_lines(void) {
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    int dest  = edlin_get_int_prompt("Destination line: ") - 1;

    if (start < 0 || end >= g_edlin_line_count || start > end || dest < 0 || dest > g_edlin_line_count) {
        edlin_print("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        edlin_print("Error: Cannot copy into source range.\n");
        return;
    }

    int count = end - start + 1;
    edlin_ensure_buffer_capacity(g_edlin_line_count + count);

    for (int i = g_edlin_line_count - 1; i >= dest; i--) {
        g_edlin_buffer[i + count] = g_edlin_buffer[i];
    }

    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    int src_offset = (dest < start) ? count : 0;
    for (int j = 0; j < count; j++) {
        size_t len = (size_t)g_edlin_buffer[start + src_offset + j].length;
        g_edlin_buffer[dest + j].text = (char *)hal->mem.alloc(len + 1);
        if (!g_edlin_buffer[dest + j].text) { edlin_oom(); return; }
        runtime_strcpy(g_edlin_buffer[dest + j].text, g_edlin_buffer[start + src_offset + j].text);
        g_edlin_buffer[dest + j].length = (int)len;
        g_edlin_buffer[dest + j].capacity = (int)len + 1;
    }
    g_edlin_line_count += count;
    edlin_print("%d lines copied.\n", count);
}

void move_edlin_lines(void) {
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    int dest  = edlin_get_int_prompt("Destination line: ") - 1;

    if (start < 0 || end >= g_edlin_line_count || start > end || dest < 0 || dest > g_edlin_line_count) {
        edlin_print("Error: Invalid range.\n");
        return;
    }
    if (dest >= start && dest <= end) {
        edlin_print("Error: Cannot move into source range.\n");
        return;
    }

    int count = end - start + 1;
    edlin_ensure_buffer_capacity(g_edlin_line_count + count);

    for (int i = g_edlin_line_count - 1; i >= dest; i--) {
        g_edlin_buffer[i + count] = g_edlin_buffer[i];
    }

    int del_start;
    if (dest < start) {
        for (int j = 0; j < count; j++) {
            g_edlin_buffer[dest + j] = g_edlin_buffer[start + count + j];
        }
        del_start = start + count;
    } else {
        for (int j = 0; j < count; j++) {
            g_edlin_buffer[dest + j] = g_edlin_buffer[start + j];
        }
        del_start = start;
    }

    for (int i = del_start; i < g_edlin_line_count; i++) {
        g_edlin_buffer[i] = g_edlin_buffer[i + count];
    }
    edlin_print("%d lines moved.\n", count);
}

void page_edlin_display(void) {
    if (g_edlin_page_pos >= g_edlin_line_count) {
        g_edlin_page_pos = 0;
    }
    int page_size = edlin_get_page_size();
    int end = g_edlin_page_pos + page_size;
    if (end > g_edlin_line_count) {
        end = g_edlin_line_count;
    }

    for (int i = g_edlin_page_pos; i < end; i++) {
        edlin_print("%d: %s\n", i + 1, g_edlin_buffer[i].text);
    }
    g_edlin_page_pos = end;
}

void search_edlin_text(void) {
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[4096];
    int found = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    if (start < 0 || end >= g_edlin_line_count || start > end || runtime_strlen(search_str) == 0) return;

    for (int i = start; i <= end; i++) {
        if (runtime_strstr(g_edlin_buffer[i].text, search_str) != NULL) {
            edlin_print("%d: %s\n", i + 1, g_edlin_buffer[i].text);
            found++;
        }
    }
    edlin_print("%d matches found.\n", found);
}

void replace_edlin_text(void) {
    int start = edlin_get_int_prompt("Start line: ") - 1;
    int end   = edlin_get_int_prompt("End line: ") - 1;
    char search_str[4096];
    char replace_str[4096];
    int replaced = 0;

    edlin_get_string_prompt("Search for: ", search_str);
    edlin_get_string_prompt("Replace with: ", replace_str);

    if (start < 0 || end >= g_edlin_line_count || start > end || runtime_strlen(search_str) == 0) return;

    for (int i = start; i <= end; i++) {
        char *pos = runtime_strstr(g_edlin_buffer[i].text, search_str);
        if (pos != NULL) {
            char temp[8192];
            int prefix_len = (int)(pos - g_edlin_buffer[i].text);
            runtime_snprintf(temp, sizeof(temp), "%.*s%s%s",
                             prefix_len, g_edlin_buffer[i].text,
                             replace_str,
                             pos + (int)runtime_strlen(search_str));
            size_t temp_len = runtime_strlen(temp);
            edlin_ensure_line_capacity(i, (int)temp_len + 1);
            runtime_strcpy(g_edlin_buffer[i].text, temp);
            g_edlin_buffer[i].length = (int)temp_len;
            edlin_print("%d: %s\n", i + 1, g_edlin_buffer[i].text);
            replaced++;
        }
    }
    edlin_print("%d lines updated.\n", replaced);
}

void transfer_edlin_file(void) {
    int dest = edlin_get_int_prompt("Insert before line: ") - 1;
    char filename[4096];
    char input[4096];

    edlin_get_string_prompt("Filename: ", filename);
    if (dest < 0) dest = 0;
    if (dest > g_edlin_line_count) dest = g_edlin_line_count;

    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(filename, "r");
    if (h == IO_HANDLE_INVALID) {
        edlin_print("Error: Cannot open %s\n", filename);
        return;
    }

    while (hal_file_read_line(hal, h, input, sizeof(input))) {
        size_t len = runtime_strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[len - 1] = '\0';
            len--;
        }
        
        edlin_insert_empty_line_at(dest);
        edlin_ensure_line_capacity(dest, (int)len + 1);
        runtime_strcpy(g_edlin_buffer[dest].text, input);
        g_edlin_buffer[dest].length = (int)len;
        dest++;
    }
    hal->io.file_close(h);
    edlin_print("File transferred.\n");
}

void write_edlin_lines(void) {
    int count = edlin_get_int_prompt("Number of lines to write: ");
    if (count <= 0 || count > g_edlin_line_count) return;

    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_edlin_filename, "a");
    if (h == IO_HANDLE_INVALID) {
        edlin_print("Error: Cannot write to file.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        if (g_edlin_buffer[i].text) {
            hal->io.file_write(h, g_edlin_buffer[i].text, 1, (size_t)g_edlin_buffer[i].length);
        }
        hal->io.file_write(h, "\n", 1, 1);
    }
    hal->io.file_close(h);

    for (int i = 0; i < count; i++) {
        edlin_free_line(i);
    }
    for (int i = count; i < g_edlin_line_count; i++) {
        g_edlin_buffer[i - count] = g_edlin_buffer[i];
    }
    g_edlin_line_count -= count;
    edlin_print("%d lines written to disk and cleared from memory.\n", count);
}

void append_edlin_lines(void) {
    char input[4096];
    int appended = 0;

    HalContext *hal = hal_get();
    if (!hal) return;

    IoHandle h = hal->io.file_open(g_edlin_filename, "r");
    if (h == IO_HANDLE_INVALID) {
        edlin_print("Error: Cannot read file.\n");
        return;
    }

    int skip = g_edlin_line_count;
    while (skip > 0 && hal_file_read_line(hal, h, input, sizeof(input))) {
        edlin_sanitize_ascii(input);
        skip--;
    }

    while (hal_file_read_line(hal, h, input, sizeof(input))) {
        size_t len = runtime_strlen(input);
        while (len > 0 && (input[len - 1] == '\n' || input[len - 1] == '\r')) {
            input[len - 1] = '\0';
            len--;
        }
        
        edlin_insert_empty_line_at(g_edlin_line_count);
        edlin_ensure_line_capacity(g_edlin_line_count - 1, (int)len + 1);
        runtime_strcpy(g_edlin_buffer[g_edlin_line_count - 1].text, input);
        g_edlin_buffer[g_edlin_line_count - 1].length = (int)len;
        appended++;
    }
    hal->io.file_close(h);
    edlin_print("%d lines appended from disk.\n", appended);
}
