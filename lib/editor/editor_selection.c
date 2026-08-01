/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/*
 * What it does: Implements shared text selection management for TUI editors.
 *   Tracks visual selection state (start/end coordinates), normalizes bounds,
 *   extracts selected text, and performs multi-line selection deletion.
 * Why it exists: Selection logic was 100% duplicated between mod_edit.c,
 *   mod_vi.c, and mod_ws.c.
 * Why it works this way: Uses an EditorSelection struct owned by each editor,
 *   operating on caller-owned TextLine arrays. No global state.
 * What can be changed: Selection extraction format, deletion strategy.
 * What cannot be changed: EditorSelection and TextLine struct layouts.
 * What to expect: Pure in-memory operations, no I/O.
 * What to do if something breaks: Check that selection bounds are within
 *   valid line/column ranges before calling extraction/deletion.
 * Assumptions: Single-threaded access. Lines are null-terminated.
 * Portability concerns: Strict C17. No platform calls.
 * Future expansions: Rectangular (column) selection mode.
 * External extension hooks: Called via bpp_editor_core.h API.
 */

#include "bpp_editor_core.h"
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Selection state management                                         */
/* ------------------------------------------------------------------ */
void editor_core_update_sel(EditorSelection *sel, int cur_r, int cur_c,
                            int new_r, int new_c) {
    if (!sel->active) {
        sel->start_r = cur_r;
        sel->start_c = cur_c;
        sel->active = true;
    }
    sel->end_r = new_r;
    sel->end_c = new_c;
}

void editor_core_clear_sel(EditorSelection *sel) {
    sel->active = false;
    sel->start_r = sel->start_c = 0;
    sel->end_r = sel->end_c = 0;
}

void editor_core_get_sel_bounds(const EditorSelection *sel,
                                int *r1, int *c1, int *r2, int *c2) {
    if (sel->start_r < sel->end_r ||
        (sel->start_r == sel->end_r && sel->start_c <= sel->end_c)) {
        *r1 = sel->start_r; *c1 = sel->start_c;
        *r2 = sel->end_r;   *c2 = sel->end_c;
    } else {
        *r1 = sel->end_r;   *c1 = sel->end_c;
        *r2 = sel->start_r; *c2 = sel->start_c;
    }
}

/* ------------------------------------------------------------------ */
/*  Selection text extraction                                          */
/* ------------------------------------------------------------------ */
char *editor_core_get_selected_text(const EditorSelection *sel,
                                    const TextLine *lines, int num_lines) {
    if (!sel->active) return NULL;

    int r1, c1, r2, c2;
    editor_core_get_sel_bounds(sel, &r1, &c1, &r2, &c2);

    /* Clamp bounds */
    if (r1 < 0) r1 = 0;
    if (r2 >= num_lines) r2 = num_lines - 1;

    /* Calculate total size needed */
    size_t total = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : lines[r].length;
        if (start > lines[r].length) start = lines[r].length;
        if (end > lines[r].length) end = lines[r].length;
        if (end > start) total += (size_t)(end - start);
        if (r < r2) total++; /* newline */
    }

    char *result = (char *)calloc(1, total + 1);
    if (!result) return NULL;

    size_t pos = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : lines[r].length;
        if (start > lines[r].length) start = lines[r].length;
        if (end > lines[r].length) end = lines[r].length;
        if (end > start) {
            memcpy(result + pos, lines[r].text + start, (size_t)(end - start));
            pos += (size_t)(end - start);
        }
        if (r < r2) {
            result[pos++] = '\n';
        }
    }
    result[pos] = '\0';
    return result;
}

/* ------------------------------------------------------------------ */
/*  Selection deletion                                                 */
/* ------------------------------------------------------------------ */
void editor_core_delete_selected_text(EditorSelection *sel,
                                      TextLine *lines, int *num_lines,
                                      int *cx, int *cy) {
    if (!sel->active) return;

    int r1, c1, r2, c2;
    editor_core_get_sel_bounds(sel, &r1, &c1, &r2, &c2);

    /* Clamp */
    if (r1 < 0) r1 = 0;
    if (r2 >= *num_lines) r2 = *num_lines - 1;
    if (c1 > lines[r1].length) c1 = lines[r1].length;
    if (c2 > lines[r2].length) c2 = lines[r2].length;

    if (r1 == r2) {
        /* Single-line deletion */
        memmove(lines[r1].text + c1, lines[r1].text + c2,
                (size_t)(lines[r1].length - c2 + 1));
        lines[r1].length -= (c2 - c1);
    } else {
        /* Multi-line deletion: merge first and last lines */
        int tail_len = lines[r2].length - c2;
        int new_len = c1 + tail_len;
        editor_core_ensure_line_capacity(lines, r1, new_len + 1);
        memcpy(lines[r1].text + c1, lines[r2].text + c2, (size_t)tail_len);
        lines[r1].text[new_len] = '\0';
        lines[r1].length = new_len;

        /* Free and shift lines */
        for (int r = r1 + 1; r <= r2; r++) {
            editor_core_free_line(lines, r);
        }
        int removed = r2 - r1;
        for (int r = r1 + 1; r < *num_lines - removed; r++) {
            lines[r] = lines[r + removed];
        }
        *num_lines -= removed;
    }

    *cx = c1;
    *cy = r1;
    editor_core_clear_sel(sel);
}
