// FILENAME: editor_buffer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (editor_core.h, hal.h, memops.h, memops.c)
// Implements visual text editor subsystem components for editor_buffer.
//
// ---- Includes ----

#include "editor/editor_core.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"

// ------------------------------------------------------------------
// Out-of-memory handler
// ------------------------------------------------------------------
void editor_core_oom(void) {
    // Silent in freestanding mode or output via HAL if available
}

// ------------------------------------------------------------------
// Line capacity growth
// ------------------------------------------------------------------
void editor_core_ensure_line_capacity(TextLine *lines, int row, int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > lines[row].capacity) {
        int new_cap = lines[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        
        char *new_text = NULL;
        if (hal->mem.realloc) {
            new_text = (char *)hal->mem.realloc(lines[row].text, (size_t)new_cap);
        } else {
            new_text = (char *)hal->mem.alloc((size_t)new_cap);
            if (new_text && lines[row].text) {
                runtime_memcpy(new_text, lines[row].text, (size_t)lines[row].length);
                new_text[lines[row].length] = '\0';
                hal->mem.free(lines[row].text);
            }
        }
        if (!new_text) { editor_core_oom(); return; }
        lines[row].text = new_text;
        lines[row].capacity = new_cap;
    }
}

// ------------------------------------------------------------------
// Buffer (line array) capacity growth
// ------------------------------------------------------------------
void editor_core_ensure_buffer_capacity(TextLine **lines, int *capacity,
                                        int needed) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc || !hal->mem.free) return;

    if (needed > *capacity) {
        int new_cap = (*capacity) * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        
        TextLine *new_buf = NULL;
        if (hal->mem.realloc) {
            new_buf = (TextLine *)hal->mem.realloc(*lines, (size_t)new_cap * sizeof(TextLine));
        } else {
            new_buf = (TextLine *)hal->mem.alloc((size_t)new_cap * sizeof(TextLine));
            if (new_buf && *lines) {
                runtime_memcpy(new_buf, *lines, (size_t)(*capacity) * sizeof(TextLine));
                hal->mem.free(*lines);
            }
        }
        if (!new_buf) { editor_core_oom(); return; }
        *lines = new_buf;
        *capacity = new_cap;
    }
}

// ------------------------------------------------------------------
// Insert an empty line at a given row
// ------------------------------------------------------------------
void editor_core_insert_empty_line(TextLine **lines, int *capacity,
                                   int *num_lines, int row) {
    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return;

    editor_core_ensure_buffer_capacity(lines, capacity, *num_lines + 1);
    for (int i = *num_lines; i > row; i--) {
        (*lines)[i] = (*lines)[i - 1];
    }
    (*lines)[row].text = (char *)hal->mem.alloc(128);
    if (!(*lines)[row].text) { editor_core_oom(); return; }
    runtime_memset((*lines)[row].text, 0, 128);
    (*lines)[row].length = 0;
    (*lines)[row].capacity = 128;
    (*lines)[row].breakpoint = false;
    (*num_lines)++;
}

// ------------------------------------------------------------------
// Free a single line
// ------------------------------------------------------------------
void editor_core_free_line(TextLine *lines, int row) {
    HalContext *hal = hal_get();
    if (lines[row].text) {
        if (hal && hal->mem.free) {
            hal->mem.free(lines[row].text);
        }
        lines[row].text = NULL;
    }
    lines[row].length = 0;
    lines[row].capacity = 0;
}

// ------------------------------------------------------------------
// 7-bit ASCII sanitization filter
// ------------------------------------------------------------------
void editor_core_sanitize_ascii(char *str) {
    if (!str) return;
    char *p = str;
    const char *src = str;
    while (*src) {
        if ((unsigned char)(*src) < 128) {
            *p++ = *src;
        }
        src++;
    }
    *p = '\0';
}
