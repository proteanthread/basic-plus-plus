/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: string_algo_core.c
 * Subsystem: String Utilities and Radix Converter
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Converts integers to binary/octal/hex and finds index.
 *
 * 2. WHAT TO EXPECT:
 *    Returns string representations of numeric values.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Radix converter boundaries.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Character mapping tables.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If conversion overflows, verify input scale.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE STRING ALGORITHMS CORE
 * File: string_algo_core.c
 * ===================================================================== */

#include <string.h>
#include "string_algo_core.h"

int string_algo_core_hex(unsigned long val, char *dest, int max_len)
{
    char tmp[32];
    int i = (int)(sizeof(tmp) - 1);
    tmp[i] = '\0';

    if (val == 0) {
        tmp[--i] = '0';
    } else {
        while (val > 0 && i > 0) {
            int d = (int)(val & 0xF);
            tmp[--i] = (char)(d < 10 ? '0' + d : 'A' + d - 10);
            val >>= 4;
        }
    }

    int len = (int)(sizeof(tmp) - 1 - (size_t)i);
    if (len >= max_len) {
        len = max_len - 1;
    }
    memcpy(dest, tmp + i, (size_t)len);
    dest[len] = '\0';
    return len;
}

int string_algo_core_oct(unsigned long val, char *dest, int max_len)
{
    char tmp[32];
    int i = (int)(sizeof(tmp) - 1);
    tmp[i] = '\0';

    if (val == 0) {
        tmp[--i] = '0';
    } else {
        while (val > 0 && i > 0) {
            tmp[--i] = (char)('0' + (int)(val & 7));
            val >>= 3;
        }
    }

    int len = (int)(sizeof(tmp) - 1 - (size_t)i);
    if (len >= max_len) {
        len = max_len - 1;
    }
    memcpy(dest, tmp + i, (size_t)len);
    dest[len] = '\0';
    return len;
}

int string_algo_core_bin(unsigned long val, char *dest, int max_len)
{
    char raw[68];
    char out[80];
    int raw_bits, num_bytes, total_bits;
    int i, o;

    if (val == 0) {
        raw_bits = 1;
        raw[0] = '0';
    } else {
        raw_bits = 0;
        while (val > 0 && raw_bits < 64) {
            raw[raw_bits++] = (char)('0' + (int)(val & 1));
            val >>= 1;
        }
    }

    num_bytes = (raw_bits + 7) / 8;
    total_bits = num_bytes * 8;
    while (raw_bits < total_bits) {
        raw[raw_bits++] = '0';
    }

    o = 0;
    for (i = total_bits - 1; i >= 0; i--) {
        out[o++] = raw[i];
        if (i > 0 && (i % 8) == 0) {
            out[o++] = ' ';
        }
    }
    out[o] = '\0';

    if (o >= max_len) {
        o = max_len - 1;
    }
    memcpy(dest, out, (size_t)o);
    dest[o] = '\0';
    return o;
}

int string_algo_core_instr(const char *haystack, int hl, const char *needle, int nl, int start_off)
{
    int i;

    if (haystack == NULL || needle == NULL || nl > hl || nl == 0) {
        return 0;
    }
    if (start_off < 0) start_off = 0;
    if (start_off > hl - nl) return 0;

    for (i = start_off; i <= hl - nl; i++) {
        if (memcmp(haystack + i, needle, (size_t)nl) == 0) {
            return i + 1; // 1-based
        }
    }
    return 0;
}
