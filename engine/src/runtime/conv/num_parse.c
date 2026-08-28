// FILENAME: num_parse.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libstandard
// NEEDS: libcore (ctype.h, ctype.c, num_parse.h)
// Freestanding integer and radix conversion implementation.
//
// ---- Includes ----

#include "runtime/conv/num_parse.h"
#include "runtime/ctype/ctype.h"
#include <limits.h>

int64_t runtime_strtoll(const char *nptr, char **endptr, int base) {
    if (!nptr) {
        if (endptr) *endptr = NULL;
        return 0;
    }

    const char *s = nptr;
    // Skip whitespace
    while (runtime_isspace((unsigned char)*s)) {
        s++;
    }

    bool negative = false;
    if (*s == '-') {
        negative = true;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Auto-detect base if base == 0
    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                base = 16;
                s += 2;
            } else if (s[1] == 'b' || s[1] == 'B') {
                base = 2;
                s += 2;
            } else if (s[1] == 'o' || s[1] == 'O') {
                base = 8;
                s += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            s += 2;
        }
    } else if (base == 2) {
        if (s[0] == '0' && (s[1] == 'b' || s[1] == 'B')) {
            s += 2;
        }
    }

    if (base < 2 || base > 36) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }

    const char *digits_start = s;
    uint64_t result = 0;
    uint64_t cutoff = negative ? (uint64_t)(-(INT64_MIN + 1)) + 1 : (uint64_t)INT64_MAX;
    int cutlim = (int)(cutoff % (uint64_t)base);
    cutoff /= (uint64_t)base;
    bool overflow = false;

    while (*s) {
        int val;
        if (runtime_isdigit((unsigned char)*s)) {
            val = *s - '0';
        } else if (runtime_isalpha((unsigned char)*s)) {
            val = runtime_toupper((unsigned char)*s) - 'A' + 10;
        } else {
            break;
        }

        if (val >= base) {
            break;
        }

        if (overflow || result > cutoff || (result == cutoff && val > cutlim)) {
            overflow = true;
        } else {
            result = result * (uint64_t)base + (uint64_t)val;
        }
        s++;
    }

    if (s == digits_start) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }

    if (endptr) {
        *endptr = (char *)s;
    }

    if (overflow) {
        return negative ? INT64_MIN : INT64_MAX;
    }

    return negative ? -(int64_t)result : (int64_t)result;
}

uint64_t runtime_strtoull(const char *nptr, char **endptr, int base) {
    if (!nptr) {
        if (endptr) *endptr = NULL;
        return 0;
    }

    const char *s = nptr;
    while (runtime_isspace((unsigned char)*s)) {
        s++;
    }

    bool negative = false;
    if (*s == '-') {
        negative = true;
        s++;
    } else if (*s == '+') {
        s++;
    }

    if (base == 0) {
        if (*s == '0') {
            if (s[1] == 'x' || s[1] == 'X') {
                base = 16;
                s += 2;
            } else if (s[1] == 'b' || s[1] == 'B') {
                base = 2;
                s += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            s += 2;
        }
    }

    if (base < 2 || base > 36) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }

    const char *digits_start = s;
    uint64_t result = 0;
    uint64_t cutoff = UINT64_MAX / (uint64_t)base;
    int cutlim = (int)(UINT64_MAX % (uint64_t)base);
    bool overflow = false;

    while (*s) {
        int val;
        if (runtime_isdigit((unsigned char)*s)) {
            val = *s - '0';
        } else if (runtime_isalpha((unsigned char)*s)) {
            val = runtime_toupper((unsigned char)*s) - 'A' + 10;
        } else {
            break;
        }

        if (val >= base) {
            break;
        }

        if (overflow || result > cutoff || (result == cutoff && val > cutlim)) {
            overflow = true;
        } else {
            result = result * (uint64_t)base + (uint64_t)val;
        }
        s++;
    }

    if (s == digits_start) {
        if (endptr) *endptr = (char *)nptr;
        return 0;
    }

    if (endptr) {
        *endptr = (char *)s;
    }

    if (overflow) {
        return UINT64_MAX;
    }

    return negative ? (uint64_t)(-(int64_t)result) : result;
}

long runtime_strtol(const char *nptr, char **endptr, int base) {
    int64_t val = runtime_strtoll(nptr, endptr, base);
    if (val > LONG_MAX) return LONG_MAX;
    if (val < LONG_MIN) return LONG_MIN;
    return (long)val;
}

unsigned long runtime_strtoul(const char *nptr, char **endptr, int base) {
    uint64_t val = runtime_strtoull(nptr, endptr, base);
    if (val > ULONG_MAX) return ULONG_MAX;
    return (unsigned long)val;
}

int runtime_atoi(const char *str) {
    return (int)runtime_strtol(str, NULL, 10);
}

long runtime_atol(const char *str) {
    return runtime_strtol(str, NULL, 10);
}

int64_t runtime_atoll(const char *str) {
    return runtime_strtoll(str, NULL, 10);
}

size_t runtime_uint_to_str(uint64_t value, char *buf, size_t buf_size, int base, bool uppercase) {
    if (!buf || buf_size == 0 || base < 2 || base > 36) {
        return 0;
    }

    char tmp[65];
    size_t i = 0;
    const char *digits = uppercase ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   : "0123456789abcdefghijklmnopqrstuvwxyz";

    if (value == 0) {
        tmp[i++] = '0';
    } else {
        while (value > 0) {
            tmp[i++] = digits[value % (uint64_t)base];
            value /= (uint64_t)base;
        }
    }

    if (i >= buf_size) {
        return 0; // Buffer too small
    }

    for (size_t j = 0; j < i; ++j) {
        buf[j] = tmp[i - 1 - j];
    }
    buf[i] = '\0';
    return i;
}

size_t runtime_int_to_str(int64_t value, char *buf, size_t buf_size, int base) {
    if (!buf || buf_size == 0) {
        return 0;
    }

    if (value < 0 && base == 10) {
        if (buf_size < 2) return 0;
        buf[0] = '-';
        size_t len = runtime_uint_to_str((uint64_t)(-(value + 1)) + 1, buf + 1, buf_size - 1, base, false);
        return len > 0 ? (len + 1) : 0;
    }

    return runtime_uint_to_str((uint64_t)value, buf, buf_size, base, false);
}
