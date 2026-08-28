// FILENAME: editor_selection.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (editor_core.h, hal.h, memops.h, memops.c)
// Implements visual text editor subsystem components for editor_selection.
//
// ---- Includes ----

#include "editor/editor_core.h"
#include "hal/hal.h"
#include "runtime/string/memops.h"

// ------------------------------------------------------------------
// Selection state management
// ------------------------------------------------------------------
void editor_core_update_sel(EditorSelection *sel, int cur_r, int cur_c,
                            int new_r, int new_c) {
    if (!sel) return;
    if (!sel->active) {
        sel->start_r = cur_r;
        sel->start_c = cur_c;
        sel->active = true;
    }
    sel->end_r = new_r;
    sel->end_c = new_c;
}

void editor_core_clear_sel(EditorSelection *sel) {
    if (!sel) return;
    sel->active = false;
    sel->start_r = sel->start_c = 0;
    sel->end_r = sel->end_c = 0;
}

void editor_core_get_sel_bounds(const EditorSelection *sel,
                                int *r1, int *c1, int *r2, int *c2) {
    if (!sel || !r1 || !c1 || !r2 || !c2) return;
    if (sel->start_r < sel->end_r ||
        (sel->start_r == sel->end_r && sel->start_c <= sel->end_c)) {
        *r1 = sel->start_r; *c1 = sel->start_c;
        *r2 = sel->end_r;   *c2 = sel->end_c;
    } else {
        *r1 = sel->end_r;   *c1 = sel->end_c;
        *r2 = sel->start_r; *c2 = sel->start_c;
    }
}

// ------------------------------------------------------------------
// Selection text extraction
// ------------------------------------------------------------------
char *editor_core_get_selected_text(const EditorSelection *sel,
                                    const TextLine *lines, int num_lines) {
    if (!sel || !sel->active || !lines || num_lines <= 0) return NULL;

    HalContext *hal = hal_get();
    if (!hal || !hal->mem.alloc) return NULL;

    int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    editor_core_get_sel_bounds(sel, &r1, &c1, &r2, &c2);

    // Clamp bounds
    if (r1 < 0) r1 = 0;
    if (r2 >= num_lines) r2 = num_lines - 1;

    // Calculate total size needed
    size_t total = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : lines[r].length;
        if (start > lines[r].length) start = lines[r].length;
        if (end > lines[r].length) end = lines[r].length;
        if (end > start) total += (size_t)(end - start);
        if (r < r2) total++; // newline
    }

    char *result = (char *)hal->mem.alloc(total + 1);
    if (!result) return NULL;
    runtime_memset(result, 0, total + 1);

    size_t pos = 0;
    for (int r = r1; r <= r2; r++) {
        int start = (r == r1) ? c1 : 0;
        int end = (r == r2) ? c2 : lines[r].length;
        if (start > lines[r].length) start = lines[r].length;
        if (end > lines[r].length) end = lines[r].length;
        if (end > start) {
            runtime_memcpy(result + pos, lines[r].text + start, (size_t)(end - start));
            pos += (size_t)(end - start);
        }
        if (r < r2) {
            result[pos++] = '\n';
        }
    }
    result[pos] = '\0';
    return result;
}

// ------------------------------------------------------------------
// Selection deletion
// ------------------------------------------------------------------
void editor_core_delete_selected_text(EditorSelection *sel,
                                      TextLine *lines, int *num_lines,
                                      int *cx, int *cy) {
    if (!sel || !sel->active || !lines || !num_lines || *num_lines <= 0) return;

    int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    editor_core_get_sel_bounds(sel, &r1, &c1, &r2, &c2);

    if (r1 < 0) r1 = 0;
    if (r2 >= *num_lines) r2 = *num_lines - 1;

    if (r1 == r2) {
        // Single-line deletion
        int start = c1;
        int end = c2;
        if (start > lines[r1].length) start = lines[r1].length;
        if (end > lines[r1].length) end = lines[r1].length;
        if (end > start) {
            int shift = lines[r1].length - end;
            runtime_memmove(lines[r1].text + start, lines[r1].text + end, (size_t)shift + 1);
            lines[r1].length -= (end - start);
        }
        if (cx) *cx = start;
        if (cy) *cy = r1;
    } else {
        // Multi-line deletion
        int start = c1;
        if (start > lines[r1].length) start = lines[r1].length;

        int end = c2;
        if (end > lines[r2].length) end = lines[r2].length;

        // Truncate line r1 at start, then append tail of line r2 (from end)
        int tail_len = lines[r2].length - end;
        editor_core_ensure_line_capacity(lines, r1, start + tail_len + 1);
        if (tail_len > 0) {
            runtime_memcpy(lines[r1].text + start, lines[r2].text + end, (size_t)tail_len);
        }
        lines[r1].length = start + tail_len;
        lines[r1].text[lines[r1].length] = '\0';

        // Free lines r1+1 through r2
        for (int r = r1 + 1; r <= r2; r++) {
            editor_core_free_line(lines, r);
        }

        // Shift remaining lines up
        int lines_to_remove = r2 - r1;
        for (int r = r1 + 1; r < *num_lines - lines_to_remove; r++) {
            lines[r] = lines[r + lines_to_remove];
        }
        *num_lines -= lines_to_remove;

        if (cx) *cx = start;
        if (cy) *cy = r1;
    }

    editor_core_clear_sel(sel);
}
