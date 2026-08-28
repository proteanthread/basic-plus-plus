// FILENAME: editor_core.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (editor_buffer.c, editor_render.c, editor_selection.c)
// NEEDED BY: libcore (editor_term.c)
// NEEDS: platform, memory
// Implements visual text editor subsystem components for editor_core.
//
// ---- Includes ----

#ifndef EDITOR_CORE_H
#define EDITOR_CORE_H

#include <stddef.h>
#include <stdbool.h>

// ------------------------------------------------------------------
// TextLine: shared line representation used by all editor modules
// ------------------------------------------------------------------
typedef struct {
    char *text;
    int   length;
    int   capacity;
    bool  breakpoint;
} TextLine;

// ------------------------------------------------------------------
// Text Buffer Management (text_buffer.c)
// ------------------------------------------------------------------

// Report an out-of-memory condition to stderr.
void editor_core_oom(void);

// Ensure a single line has at least 'needed' bytes of capacity.
void editor_core_ensure_line_capacity(TextLine *lines, int row, int needed);

// Ensure the line array can hold at least 'needed' lines total.
void editor_core_ensure_buffer_capacity(TextLine **lines, int *capacity,
                                        int needed);

// Insert a new empty line at 'row', shifting subsequent lines down.
void editor_core_insert_empty_line(TextLine **lines, int *capacity,
                                   int *num_lines, int row);

// Free a single line's text allocation.
void editor_core_free_line(TextLine *lines, int row);

// Filter a string in-place to contain only 7-bit ASCII characters.
void editor_core_sanitize_ascii(char *str);

// ------------------------------------------------------------------
// Tab Layout (text_layout.c)
// ------------------------------------------------------------------

// Convert a physical cursor X to a rendered X accounting for TAB stops.
int editor_core_get_render_x(const TextLine *line, int cx, int tab_stop);

// Convert a rendered X back to a physical cursor X.
int editor_core_get_physical_x(const TextLine *line, int rx, int tab_stop);

// Expand tabs in a line into spaces, writing to out_buf. Returns length.
int editor_core_render_row(const TextLine *line, char *out_buf,
                           int out_buf_size, int tab_stop);

// Format a filename for status bar display, truncating if needed.
void editor_core_format_filename(char *out_buf, const char *in_filename,
                                 int max_len);

// ------------------------------------------------------------------
// ANSI Color Palette (tui_ansi.c)
// ------------------------------------------------------------------

// Shared bright color escape sequence table (6 entries).
extern const char *editor_core_bright_colors[6];

// ------------------------------------------------------------------
// Selection Management (tui_selection.c)
// ------------------------------------------------------------------
typedef struct {
    bool active;
    int  start_r, start_c;
    int  end_r, end_c;
} EditorSelection;

// Begin or update a selection endpoint.
void editor_core_update_sel(EditorSelection *sel, int cur_r, int cur_c,
                            int new_r, int new_c);

// Clear (deactivate) a selection.
void editor_core_clear_sel(EditorSelection *sel);

// Get normalized (top-to-bottom) selection bounds.
void editor_core_get_sel_bounds(const EditorSelection *sel,
                                int *r1, int *c1, int *r2, int *c2);

// Extract selected text from a line buffer as a malloc'd string.
char *editor_core_get_selected_text(const EditorSelection *sel,
                                    const TextLine *lines, int num_lines);

// Delete selected text from a line buffer.
void editor_core_delete_selected_text(EditorSelection *sel,
                                      TextLine *lines, int *num_lines,
                                      int *cx, int *cy);

#endif // EDITOR_CORE_H
