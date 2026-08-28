// FILENAME: nil_compress.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_nil_compress.c)
// NEEDS: libcore (nil_compress.h, string.h)
// NEEDS: libengine (string.c)
// Implements RFC 51 COMSTRING and COMPVECTOR run-length/token stream compression.
//
// ---- Includes ----

#include "runtime/nil_compress.h"
#include <string.h>

// COMSTRING Token constants per RFC 51 Section 2.8.2
#define NIL_ESC_CHAR     0x1B
#define NIL_TAB_CHAR     0x09
#define NIL_RUN_FLAG     0x80

size_t nil_comstring_encode(const char *in, size_t in_len, char *out, size_t max_out) {
    if (!in || in_len == 0 || !out || max_out == 0) return 0;

    size_t in_idx = 0;
    size_t out_idx = 0;

    while (in_idx < in_len && out_idx + 3 < max_out) {
        char c = in[in_idx];

        // Check for space/whitespace repetition (RFC 51 TAB/Blank runs)
        if (c == ' ') {
            size_t run_len = 1;
            while (in_idx + run_len < in_len && in[in_idx + run_len] == ' ' && run_len < 127) {
                run_len++;
            }
            if (run_len >= 3) {
                out[out_idx++] = (char)NIL_TAB_CHAR;
                out[out_idx++] = (char)run_len;
                in_idx += run_len;
                continue;
            }
        }

        // Check for general character repetition
        size_t r = 1;
        while (in_idx + r < in_len && in[in_idx + r] == c && r < 127) {
            r++;
        }

        if (r >= 4) {
            out[out_idx++] = (char)NIL_ESC_CHAR;
            out[out_idx++] = (char)(NIL_RUN_FLAG | (uint8_t)r);
            out[out_idx++] = c;
            in_idx += r;
        } else {
            out[out_idx++] = c;
            in_idx++;
        }
    }

    if (out_idx < max_out) {
        out[out_idx] = '\0';
    }
    return out_idx;
}

size_t nil_comstring_decode(const char *in, size_t in_len, char *out, size_t max_out) {
    if (!in || in_len == 0 || !out || max_out == 0) return 0;

    size_t in_idx = 0;
    size_t out_idx = 0;

    while (in_idx < in_len && out_idx < max_out) {
        unsigned char c = (unsigned char)in[in_idx];

        if (c == (unsigned char)NIL_TAB_CHAR && in_idx + 1 < in_len) {
            in_idx++;
            uint8_t count = (uint8_t)in[in_idx++];
            for (uint8_t i = 0; i < count && out_idx < max_out - 1; i++) {
                out[out_idx++] = ' ';
            }
        } else if (c == (unsigned char)NIL_ESC_CHAR && in_idx + 2 < in_len) {
            in_idx++;
            uint8_t run_info = (uint8_t)in[in_idx++];
            uint8_t count = (run_info & 0x7F);
            char rep_c = in[in_idx++];
            for (uint8_t i = 0; i < count && out_idx < max_out - 1; i++) {
                out[out_idx++] = rep_c;
            }
        } else {
            out[out_idx++] = in[in_idx++];
        }
    }

    if (out_idx < max_out) {
        out[out_idx] = '\0';
    }
    return out_idx;
}

size_t nil_compvector_encode(const uint8_t *in, size_t in_len, uint8_t *out, size_t max_out) {
    if (!in || in_len == 0 || !out || max_out == 0) return 0;

    size_t in_idx = 0;
    size_t out_idx = 0;

    while (in_idx < in_len && out_idx + 2 < max_out) {
        uint8_t b = in[in_idx];
        size_t run = 1;
        while (in_idx + run < in_len && in[in_idx + run] == b && run < 255) {
            run++;
        }

        if (run >= 3) {
            out[out_idx++] = 0xFF; // RFC 51 Escape token
            out[out_idx++] = (uint8_t)run;
            out[out_idx++] = b;
            in_idx += run;
        } else {
            if (b == 0xFF) {
                out[out_idx++] = 0xFF;
                out[out_idx++] = 0x01;
                out[out_idx++] = 0xFF;
                in_idx++;
            } else {
                out[out_idx++] = b;
                in_idx++;
            }
        }
    }
    return out_idx;
}

size_t nil_compvector_decode(const uint8_t *in, size_t in_len, uint8_t *out, size_t max_out) {
    if (!in || in_len == 0 || !out || max_out == 0) return 0;

    size_t in_idx = 0;
    size_t out_idx = 0;

    while (in_idx < in_len && out_idx < max_out) {
        uint8_t b = in[in_idx++];
        if (b == 0xFF && in_idx + 1 < in_len) {
            uint8_t count = in[in_idx++];
            uint8_t val = in[in_idx++];
            for (uint8_t i = 0; i < count && out_idx < max_out; i++) {
                out[out_idx++] = val;
            }
        } else {
            out[out_idx++] = b;
        }
    }
    return out_idx;
}
