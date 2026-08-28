// FILENAME: editor_render.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (editor_core.h, memops.h, memops.c, strops.h, strops.c)
// Implements visual text editor subsystem components for editor_render.
//
// ---- Includes ----

#include "editor/editor_core.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"

// ------------------------------------------------------------------
// Tab stop coordinate translation
// ------------------------------------------------------------------
int editor_core_get_render_x(const TextLine *line, int cx, int tab_stop) {
    if (!line || !line->text) return 0;
    if (tab_stop <= 0) tab_stop = 4;
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
    if (!line || !line->text) return 0;
    if (tab_stop <= 0) tab_stop = 4;
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
    if (!line || !line->text || !out_buf || out_buf_size <= 0) return 0;
    if (tab_stop <= 0) tab_stop = 4;
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

// ------------------------------------------------------------------
// Filename formatting for status bars
// ------------------------------------------------------------------
void editor_core_format_filename(char *out_buf, const char *in_filename,
                                 int max_len) {
    if (!out_buf || max_len <= 0) return;
    if (!in_filename || !in_filename[0]) {
        runtime_strncpy(out_buf, "NEW FILE", (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }

    size_t len = runtime_strlen(in_filename);
    if ((int)len <= max_len) {
        runtime_strncpy(out_buf, in_filename, (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }

    // Find base filename
    const char *base = in_filename;
    for (int i = (int)len - 1; i >= 0; i--) {
        if (in_filename[i] == '/' || in_filename[i] == '\\') {
            base = &in_filename[i + 1];
            break;
        }
    }

    int base_len = (int)runtime_strlen(base);
    if (base_len >= max_len) {
        runtime_strncpy(out_buf, base, (size_t)max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }

    const char *prefix = ".../";
    int prefix_len = (int)runtime_strlen(prefix);
    if (prefix_len + base_len < max_len) {
        runtime_strcpy(out_buf, prefix);
        runtime_strcat(out_buf, base);
    } else {
        int copy_len = base_len;
        if (copy_len > max_len - 1) copy_len = max_len - 1;
        runtime_memcpy(out_buf, base, (size_t)copy_len);
        out_buf[copy_len] = '\0';
    }
}
