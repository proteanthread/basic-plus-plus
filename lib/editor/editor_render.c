/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/*
 * What it does: Implements shared tab expansion, cursor coordinate
 *   translation, row rendering, and filename formatting for all editors.
 * Why it exists: get_render_x(), get_physical_x(), render_row(), and
 *   format_filename_for_status() were 100% duplicated across mod_edit.c,
 *   mod_vi.c, and mod_ws.c.
 * Why it works this way: Tab stop expansion uses a simple modular arithmetic
 *   approach. Filename formatting truncates long paths to fit status bars.
 * What can be changed: Default tab stop width, path truncation strategy.
 * What cannot be changed: TextLine struct layout, render_x/physical_x contract.
 * What to expect: Pure computation, no I/O or side effects.
 * What to do if something breaks: Verify tab_stop > 0, check buffer sizes.
 * Assumptions: Lines are null-terminated 7-bit ASCII.
 * Portability concerns: Strict C17. Uses _fullpath (Win) vs realpath (POSIX).
 * Future expansions: Syntax highlighting token boundary tracking.
 * External extension hooks: Called via bpp_editor_core.h API.
 */

#include "bpp_editor_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/*  Tab stop coordinate translation                                    */
/* ------------------------------------------------------------------ */
int editor_core_get_render_x(const TextLine *line, int cx, int tab_stop) {
    int rx = 0;
    for (int j = 0; j < cx && line->text[j] != '\0'; j++) {
        if (line->text[j] == '\t') {
            rx += (tab_stop - 1) - (rx % tab_stop);
        }
        rx++;
    }
    return rx;
}

int editor_core_get_physical_x(const TextLine *line, int rx_target,
                                int tab_stop) {
    int rx = 0, j;
    for (j = 0; line->text[j] != '\0'; j++) {
        int next_rx = rx;
        if (line->text[j] == '\t') {
            next_rx += (tab_stop - 1) - (rx % tab_stop);
        }
        next_rx++;
        if (next_rx > rx_target) return j;
        rx = next_rx;
    }
    return j;
}

int editor_core_render_row(const TextLine *line, char *out_buf,
                           int out_buf_size, int tab_stop) {
    int j = 0, idx = 0;
    while (line->text[j] != '\0' && idx < (out_buf_size - 1)) {
        if (line->text[j] == '\t') {
            out_buf[idx++] = ' ';
            while (idx % tab_stop != 0 && idx < (out_buf_size - 1)) {
                out_buf[idx++] = ' ';
            }
        } else {
            out_buf[idx++] = line->text[j];
        }
        j++;
    }
    out_buf[idx] = '\0';
    return idx;
}

/* ------------------------------------------------------------------ */
/*  Filename formatting for status bars                                */
/* ------------------------------------------------------------------ */
void editor_core_format_filename(char *out_buf, const char *in_filename,
                                 int max_len) {
    if (!in_filename || !in_filename[0]) {
        strcpy(out_buf, "NEW FILE");
        return;
    }

    char abs_path[4096];
#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    if (_fullpath(abs_path, in_filename, 4096) == NULL) {
        strcpy(abs_path, in_filename);
    }
#else
    if (realpath(in_filename, abs_path) == NULL) {
        strcpy(abs_path, in_filename);
    }
#endif

    int len = (int)strlen(abs_path);
    if (len <= max_len) {
        strcpy(out_buf, abs_path);
        return;
    }

    /* Find base filename */
    const char *base = abs_path;
    for (int i = len - 1; i >= 0; i--) {
        if (abs_path[i] == '/' || abs_path[i] == '\\') {
            base = &abs_path[i + 1];
            break;
        }
    }

    int base_len = (int)strlen(base);
    if (base_len >= max_len) {
        strcpy(out_buf, base);
        return;
    }

#if defined(_WIN32) || defined(WIN32) || defined(__MSDOS__) || defined(__DOS__)
    {
        const char *prefix = "C:\\...\\";
        int prefix_len = (int)strlen(prefix);
        if (prefix_len + base_len < max_len) {
            strcpy(out_buf, prefix);
            strcat(out_buf, base);
        } else {
            int copy_len = base_len;
            if (copy_len > max_len - 1) copy_len = max_len - 1;
            memcpy(out_buf, base, (size_t)copy_len);
            out_buf[copy_len] = '\0';
        }
    }
#else
    {
        const char *prefix = "/.../";
        int prefix_len = (int)strlen(prefix);
        if (prefix_len + base_len < max_len) {
            strcpy(out_buf, prefix);
            strcat(out_buf, base);
        } else {
            int copy_len = base_len;
            if (copy_len > max_len - 1) copy_len = max_len - 1;
            memcpy(out_buf, base, (size_t)copy_len);
            out_buf[copy_len] = '\0';
        }
    }
#endif
}
