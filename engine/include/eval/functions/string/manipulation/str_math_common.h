// FILENAME: str_math_common.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sum.c, dif.c, prod.c, quo.c, place.c)
// NEEDS: libcore (hal.h, math.h, memory.h, strings.h)
// Provides shared arbitrary-precision decimal string arithmetic structures and helpers.

#ifndef STR_MATH_COMMON_H
#define STR_MATH_COMMON_H

#include "runtime/strings.h"
#include "runtime/memory.h"
#include "runtime/math.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "types/errors.h"
#include "hal/hal.h"
#include <stdbool.h>
#include "runtime/ctype/ctype.h"

#define DEFAULT_PRECISION 31
#define MAX_PRECISION 1024

typedef struct {
    bool negative;
    char *digits;   // Integer and fractional digits without decimal point
    int len;
    int dot_pos;    // Number of integer digits before dot
} BigDec;

static inline void bigdec_free(BigDec *bd) {
    if (bd->digits) {
        if (bd->digits && hal_get()) hal_get()->mem.free(bd->digits);
        bd->digits = NULL;
    }
}

static inline bool bigdec_parse(const char *str, BigDec *bd) {
    bd->negative = false;
    bd->digits = NULL;
    bd->len = 0;
    bd->dot_pos = 0;

    if (!str) return false;
    while (*str && runtime_isspace((unsigned char)*str)) str++;
    if (*str == '-') {
        bd->negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }

    size_t slen = runtime_strlen(str);
    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(slen + 2) : NULL);
    if (!buf) return false;

    int d_idx = 0;
    int dot = -1;

    for (size_t i = 0; i < slen; i++) {
        if (str[i] == '.') {
            if (dot != -1) { if (buf && hal_get()) hal_get()->mem.free(buf); return false; }
            dot = d_idx;
        } else if (runtime_isdigit((unsigned char)str[i])) {
            buf[d_idx++] = str[i] - '0';
        } else if (runtime_isspace((unsigned char)str[i])) {
            break;
        } else {
            if (buf && hal_get()) hal_get()->mem.free(buf);
            return false;
        }
    }

    if (d_idx == 0) {
        buf[0] = 0;
        d_idx = 1;
        dot = 1;
    }

    if (dot == -1) dot = d_idx;

    bd->digits = buf;
    bd->len = d_idx;
    bd->dot_pos = dot;
    return true;
}

static inline int bigdec_cmp_abs(const BigDec *a, const BigDec *b) {
    int a_int = a->dot_pos;
    int b_int = b->dot_pos;
    int a_start = 0;
    while (a_start < a_int - 1 && a->digits[a_start] == 0) a_start++;
    int b_start = 0;
    while (b_start < b_int - 1 && b->digits[b_start] == 0) b_start++;

    int a_eff_int = a_int - a_start;
    int b_eff_int = b_int - b_start;
    if (a_eff_int != b_eff_int) return (a_eff_int > b_eff_int) ? 1 : -1;

    for (int i = 0; i < a_eff_int; i++) {
        if (a->digits[a_start + i] != b->digits[b_start + i])
            return (a->digits[a_start + i] > b->digits[b_start + i]) ? 1 : -1;
    }

    int a_frac = a->len - a->dot_pos;
    int b_frac = b->len - b->dot_pos;
    int max_frac = (a_frac > b_frac) ? a_frac : b_frac;
    for (int i = 0; i < max_frac; i++) {
        int da = (i < a_frac) ? a->digits[a->dot_pos + i] : 0;
        int db = (i < b_frac) ? b->digits[b->dot_pos + i] : 0;
        if (da != db) return (da > db) ? 1 : -1;
    }

    return 0;
}

static inline char *bigdec_format(const BigDec *bd, int max_decimals) {
    int alloc_sz = bd->len + 64 + (max_decimals > 0 ? max_decimals : 0);
    char *out = (char *)(hal_get() ? hal_get()->mem.alloc(alloc_sz) : NULL);
    if (!out) return NULL;

    int out_idx = 0;
    if (bd->negative) {
        bool all_zero = true;
        for (int i = 0; i < bd->len; i++) {
            if (bd->digits[i] != 0) { all_zero = false; break; }
        }
        if (!all_zero) out[out_idx++] = '-';
    }

    int int_start = 0;
    while (int_start < bd->dot_pos - 1 && bd->digits[int_start] == 0) int_start++;
    for (int i = int_start; i < bd->dot_pos; i++) {
        out[out_idx++] = '0' + bd->digits[i];
    }

    int frac_len = bd->len - bd->dot_pos;
    int emit_frac = frac_len;
    if (max_decimals >= 0 && emit_frac > max_decimals) emit_frac = max_decimals;

    if (emit_frac > 0) {
        int last_nonzero = emit_frac - 1;
        while (last_nonzero >= 0 && bd->digits[bd->dot_pos + last_nonzero] == 0) last_nonzero--;
        if (last_nonzero >= 0) {
            out[out_idx++] = '.';
            for (int i = 0; i <= last_nonzero; i++) {
                out[out_idx++] = '0' + bd->digits[bd->dot_pos + i];
            }
        }
    }

    out[out_idx] = '\0';
    return out;
}

#endif // STR_MATH_COMMON_H
