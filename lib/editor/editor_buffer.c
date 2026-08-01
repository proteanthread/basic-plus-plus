/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/*
 * What it does: Implements shared text buffer operations for all interactive
 *   editors (EDIT, vi, WS, EDLIN). Manages dynamic TextLine arrays with
 *   automatic capacity growth, line insertion/deletion, and ASCII sanitization.
 * Why it exists: Eliminates ~60% duplicated buffer management code across the
 *   four editor personalities. Every editor had identical copies of oom(),
 *   ensure_line_capacity(), ensure_buffer_capacity(), insert_empty_line(),
 *   free_line(), and sanitize_ascii().
 * Why it works this way: Uses caller-owned TextLine arrays rather than opaque
 *   handles so editors can maintain their existing global state patterns while
 *   sharing the core logic. Growth uses doubling strategy with minimum floors.
 * What can be changed: Minimum capacity floors (128 bytes, 256 lines), growth
 *   factors. Add undo/redo recording calls.
 * What cannot be changed: TextLine struct layout (must match bpp_editor_core.h).
 *   The calloc zero-initialization requirement for new lines.
 * What to expect: Fast O(n) line insertions, safe capacity growth with OOM
 *   error reporting via stderr.
 * What to do if something breaks: Check that callers pass valid pointers and
 *   that row indices are within [0, num_lines].
 * Assumptions: Single-threaded access. Lines are 7-bit ASCII strings.
 * Portability concerns: Strict C17. No platform-specific calls.
 * Future expansions: Add undo/redo stack recording per mutation.
 * External extension hooks: New editors include bpp_editor_core.h and call
 *   these functions directly.
 */

#include "bpp_editor_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Out-of-memory handler                                              */
/* ------------------------------------------------------------------ */
void editor_core_oom(void) {
    fprintf(stderr, "\n\nOut of memory!\n");
}

/* ------------------------------------------------------------------ */
/*  Line capacity growth                                               */
/* ------------------------------------------------------------------ */
void editor_core_ensure_line_capacity(TextLine *lines, int row, int needed) {
    if (needed > lines[row].capacity) {
        int new_cap = lines[row].capacity * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 128) new_cap = 128;
        char *new_text = realloc(lines[row].text, (size_t)new_cap);
        if (!new_text) { editor_core_oom(); return; }
        lines[row].text = new_text;
        lines[row].capacity = new_cap;
    }
}

/* ------------------------------------------------------------------ */
/*  Buffer (line array) capacity growth                                */
/* ------------------------------------------------------------------ */
void editor_core_ensure_buffer_capacity(TextLine **lines, int *capacity,
                                        int needed) {
    if (needed > *capacity) {
        int new_cap = (*capacity) * 2;
        if (new_cap < needed) new_cap = needed;
        if (new_cap < 256) new_cap = 256;
        TextLine *new_buf = realloc(*lines, (size_t)new_cap * sizeof(TextLine));
        if (!new_buf) { editor_core_oom(); return; }
        *lines = new_buf;
        *capacity = new_cap;
    }
}

/* ------------------------------------------------------------------ */
/*  Insert an empty line at a given row                                */
/* ------------------------------------------------------------------ */
void editor_core_insert_empty_line(TextLine **lines, int *capacity,
                                   int *num_lines, int row) {
    editor_core_ensure_buffer_capacity(lines, capacity, *num_lines + 1);
    for (int i = *num_lines; i > row; i--) {
        (*lines)[i] = (*lines)[i - 1];
    }
    (*lines)[row].text = (char *)calloc(1, 128);
    if (!(*lines)[row].text) { editor_core_oom(); return; }
    (*lines)[row].text[0] = '\0';
    (*lines)[row].length = 0;
    (*lines)[row].capacity = 128;
    (*lines)[row].breakpoint = false;
    (*num_lines)++;
}

/* ------------------------------------------------------------------ */
/*  Free a single line                                                 */
/* ------------------------------------------------------------------ */
void editor_core_free_line(TextLine *lines, int row) {
    if (lines[row].text) {
        free(lines[row].text);
        lines[row].text = NULL;
    }
    lines[row].length = 0;
    lines[row].capacity = 0;
}

/* ------------------------------------------------------------------ */
/*  7-bit ASCII sanitization filter                                    */
/* ------------------------------------------------------------------ */
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
