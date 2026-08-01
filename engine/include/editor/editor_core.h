/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/*
 * What it does: Public API for the shared editor TUI library used by all
 *   editor personalities (EDIT, vi, WS, EDLIN).
 * Why it exists: Eliminates ~1500 lines of duplicated text buffer, terminal,
 *   rendering, and selection management code across the four editor modules.
 * Why it works this way: Exposes opaque data structures and function APIs
 *   that each personality calls into, while keeping personality-specific
 *   keybinding dispatch and UI layout in src/editor/mod_*.c.
 * What can be changed: Add new shared helpers, extend TextLine fields.
 * What cannot be changed: TextLine struct layout (binary compat with editors).
 * What to expect: All editors share one copy of buffer, term, and selection code.
 * What to do if something breaks: Check that editors include this header and
 *   link against the bpp_editor_core library target.
 * Assumptions: 7-bit ASCII text lines, single-threaded editor access.
 * Portability concerns: Strict C17. Terminal code uses platform abstraction.
 * Future expansions: Undo/redo stack, syntax highlighting hooks.
 * External extension hooks: New editor personalities include this header.
 */

#ifndef BPP_EDITOR_CORE_H
#define BPP_EDITOR_CORE_H

#include <stddef.h>
#include <stdbool.h>

/* ------------------------------------------------------------------ */
/*  TextLine: shared line representation used by all editor modules    */
/* ------------------------------------------------------------------ */
typedef struct {
    char *text;
    int   length;
    int   capacity;
    bool  breakpoint;
} TextLine;

/* ------------------------------------------------------------------ */
/*  Text Buffer Management (text_buffer.c)                             */
/* ------------------------------------------------------------------ */

/* Report an out-of-memory condition to stderr. */
void editor_core_oom(void);

/* Ensure a single line has at least 'needed' bytes of capacity. */
void editor_core_ensure_line_capacity(TextLine *lines, int row, int needed);

/* Ensure the line array can hold at least 'needed' lines total. */
void editor_core_ensure_buffer_capacity(TextLine **lines, int *capacity,
                                        int needed);

/* Insert a new empty line at 'row', shifting subsequent lines down. */
void editor_core_insert_empty_line(TextLine **lines, int *capacity,
                                   int *num_lines, int row);

/* Free a single line's text allocation. */
void editor_core_free_line(TextLine *lines, int row);

/* Filter a string in-place to contain only 7-bit ASCII characters. */
void editor_core_sanitize_ascii(char *str);

/* ------------------------------------------------------------------ */
/*  Tab Layout (text_layout.c)                                         */
/* ------------------------------------------------------------------ */

/* Convert a physical cursor X to a rendered X accounting for TAB stops. */
int editor_core_get_render_x(const TextLine *line, int cx, int tab_stop);

/* Convert a rendered X back to a physical cursor X. */
int editor_core_get_physical_x(const TextLine *line, int rx, int tab_stop);

/* Expand tabs in a line into spaces, writing to out_buf. Returns length. */
int editor_core_render_row(const TextLine *line, char *out_buf,
                           int out_buf_size, int tab_stop);

/* Format a filename for status bar display, truncating if needed. */
void editor_core_format_filename(char *out_buf, const char *in_filename,
                                 int max_len);

/* ------------------------------------------------------------------ */
/*  ANSI Color Palette (tui_ansi.c)                                    */
/* ------------------------------------------------------------------ */

/* Shared bright color escape sequence table (6 entries). */
extern const char *editor_core_bright_colors[6];

/* ------------------------------------------------------------------ */
/*  Selection Management (tui_selection.c)                             */
/* ------------------------------------------------------------------ */
typedef struct {
    bool active;
    int  start_r, start_c;
    int  end_r, end_c;
} EditorSelection;

/* Begin or update a selection endpoint. */
void editor_core_update_sel(EditorSelection *sel, int cur_r, int cur_c,
                            int new_r, int new_c);

/* Clear (deactivate) a selection. */
void editor_core_clear_sel(EditorSelection *sel);

/* Get normalized (top-to-bottom) selection bounds. */
void editor_core_get_sel_bounds(const EditorSelection *sel,
                                int *r1, int *c1, int *r2, int *c2);

/* Extract selected text from a line buffer as a malloc'd string. */
char *editor_core_get_selected_text(const EditorSelection *sel,
                                    const TextLine *lines, int num_lines);

/* Delete selected text from a line buffer. */
void editor_core_delete_selected_text(EditorSelection *sel,
                                      TextLine *lines, int *num_lines,
                                      int *cx, int *cy);

#endif /* BPP_EDITOR_CORE_H */
