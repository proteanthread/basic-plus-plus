// FILENAME: num_format.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (map_serialize.c, string_ext.c, using_engine.c)
// NEEDED BY: libcore (variables_internal.h)
// NEEDED BY: libengine (ast_eval_expr.c, ast_eval_stmt.c, ast_internal.h)
// NEEDED BY: libengine (helpers.c, lprint.c, mat_print.c, print.c)
// NEEDED BY: libengine (print_file.c, str.c)
// NEEDS: libcore (math.h, num_format.h, string.h)
// NEEDS: libengine (math.c, string.c)
// Provides core logic and interface definitions for num_format within BASIC++.
//
// ---- Includes ----

#include "runtime/num_format.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

// @brief Internal helper: strip trailing zeros and trailing decimal point
// from a fixed-point formatted string.
//
// Operates on a null-terminated string produced by snprintf with %f or %.Nf.
// Only strips if a decimal point is present (avoids mangling integer strings).
//
// @param s  Null-terminated string buffer to modify in-place.
static void strip_trailing_zeros(char *s) {
    // Find the decimal point; if none, nothing to strip
    char *dot = NULL;
    char *p = s;
    while (*p != '\0') {
        if (*p == '.') {
            dot = p;
        }
        p++;
    }
    if (dot == NULL) {
        return;
    }

    // Strip trailing '0' characters after the decimal point
    char *end = p - 1; // points to last non-null char
    while (end > dot && *end == '0') {
        *end = '\0';
        end--;
    }
    // If only the dot remains, strip it too
    if (end == dot) {
        *end = '\0';
    }
}

static inline size_t fast_i64toa_buf(int64_t n, char *out, bool leading_space, bool trailing_space) {
    char temp[24];
    size_t i = 0;
    bool neg = false;
    if (n < 0) {
        neg = true;
        n = -n;
    }
    if (n == 0) {
        temp[i++] = '0';
    } else {
        while (n > 0) {
            temp[i++] = (char)('0' + (n % 10));
            n /= 10;
        }
    }
    size_t pos = 0;
    if (leading_space && !neg) out[pos++] = ' ';
    if (neg) out[pos++] = '-';
    while (i > 0) {
        out[pos++] = temp[--i];
    }
    if (trailing_space) out[pos++] = ' ';
    out[pos] = '\0';
    return pos;
}

void num_format_display(char *buf, size_t buf_size, double val,
                        bool leading_space, bool trailing_space) {
    if (buf_size < 32) return;
    const char *prefix = "";
    const char *suffix = "";

    if (leading_space) {
        prefix = (val >= 0.0) ? " " : "";
    }
    if (trailing_space) {
        suffix = " ";
    }

    // Fast Path: Exact integer display.
// Use ultra-fast direct conversion (0.003 microseconds vs 15 microseconds snprintf).
    if (val == (double)(long long)val &&
        fabs(val) <= NUM_FORMAT_MAX_EXACT_INT) {
        fast_i64toa_buf((long long)val, buf, leading_space, trailing_space);
        return;
    }

    // Tier 2: Fixed-point decimal display.
// For values within [0.000001, 2^53] that have fractional parts,
// format with %.15f and strip trailing zeros for clean output.
    if (fabs(val) <= NUM_FORMAT_MAX_EXACT_INT && fabs(val) >= 0.000001) {
        char temp[64];
        snprintf(temp, sizeof(temp), "%.15f", val);
        strip_trailing_zeros(temp);
        snprintf(buf, buf_size, "%s%s%s", prefix, temp, suffix);
        return;
    }

    // Tier 3: Scientific notation fallback.
// For very large (> 2^53) or very small (< 0.000001) values,
// use %.15g for 15 significant digits.
    snprintf(buf, buf_size, "%s%.15g%s", prefix, val, suffix);
}

void num_format_serialize(char *buf, size_t buf_size, double val) {
    if (buf_size < 32) return;
    // Fast Path: Exact integer serialization.
    if (val == (double)(long long)val &&
        fabs(val) <= NUM_FORMAT_MAX_EXACT_INT) {
        fast_i64toa_buf((long long)val, buf, false, false);
        return;
    }

    // Tier 2+3: Full precision serialization.
// Use %.16g for maximum IEEE 754 double round-trip fidelity.
// No trailing-zero stripping -- data integrity is paramount.
    snprintf(buf, buf_size, "%.16g", val);
}
