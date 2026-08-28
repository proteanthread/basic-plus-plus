// FILENAME: sscanf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libcore (ctype.h, ctype.c, float_parse.h, float_parse.c)
// NEEDS: libcore (memops.h, memops.c, num_parse.h, num_parse.c, sscanf.h)
// NEEDS: libcore (strops.h, strops.c)
// Freestanding sscanf scanning implementation.
//
// ---- Includes ----

#include "runtime/format/sscanf.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include <stdint.h>
#include <stdbool.h>

int runtime_vsscanf(const char *str, const char *format, va_list ap) {
    if (!str || !format) {
        return -1;
    }

    int assigned = 0;
    const char *s = str;
    const char *f = format;

    while (*f && *s) {
        if (runtime_isspace((unsigned char)*f)) {
            while (runtime_isspace((unsigned char)*f)) f++;
            while (runtime_isspace((unsigned char)*s)) s++;
            continue;
        }

        if (*f != '%') {
            if (*f != *s) {
                break;
            }
            f++;
            s++;
            continue;
        }

        f++; // Skip '%'

        bool suppress = false;
        if (*f == '*') {
            suppress = true;
            f++;
        }

        // Field width
        int width = 0;
        while (runtime_isdigit((unsigned char)*f)) {
            width = width * 10 + (*f++ - '0');
        }

        // Length modifiers
        enum { LEN_NONE, LEN_H, LEN_HH, LEN_L, LEN_LL, LEN_Z } length_mod = LEN_NONE;
        if (*f == 'h') {
            if (f[1] == 'h') { length_mod = LEN_HH; f += 2; }
            else { length_mod = LEN_H; f++; }
        } else if (*f == 'l') {
            if (f[1] == 'l') { length_mod = LEN_LL; f += 2; }
            else { length_mod = LEN_L; f++; }
        } else if (*f == 'z') {
            length_mod = LEN_Z; f++;
        }

        char spec = *f++;
        if (!spec) break;

        // Skip whitespace before numbers and strings
        if (spec != 'c' && spec != '[') {
            while (runtime_isspace((unsigned char)*s)) s++;
        }

        if (!*s && spec != 'n') {
            break;
        }

        switch (spec) {
            case 'd':
            case 'i': {
                char *endp = NULL;
                int base = (spec == 'i') ? 0 : 10;
                int64_t val = runtime_strtoll(s, &endp, base);
                if (endp == s) {
                    return assigned;
                }
                s = endp;
                if (!suppress) {
                    if (length_mod == LEN_LL) *va_arg(ap, long long *) = (long long)val;
                    else if (length_mod == LEN_L) *va_arg(ap, long *) = (long)val;
                    else if (length_mod == LEN_HH) *va_arg(ap, signed char *) = (signed char)val;
                    else if (length_mod == LEN_H) *va_arg(ap, short *) = (short)val;
                    else *va_arg(ap, int *) = (int)val;
                    assigned++;
                }
                break;
            }

            case 'u':
            case 'x':
            case 'X':
            case 'o': {
                char *endp = NULL;
                int base = 10;
                if (spec == 'x' || spec == 'X') base = 16;
                else if (spec == 'o') base = 8;

                uint64_t uval = runtime_strtoull(s, &endp, base);
                if (endp == s) {
                    return assigned;
                }
                s = endp;
                if (!suppress) {
                    if (length_mod == LEN_LL) *va_arg(ap, unsigned long long *) = (unsigned long long)uval;
                    else if (length_mod == LEN_L) *va_arg(ap, unsigned long *) = (unsigned long)uval;
                    else if (length_mod == LEN_HH) *va_arg(ap, unsigned char *) = (unsigned char)uval;
                    else if (length_mod == LEN_H) *va_arg(ap, unsigned short *) = (unsigned short)uval;
                    else *va_arg(ap, unsigned int *) = (unsigned int)uval;
                    assigned++;
                }
                break;
            }

            case 'f':
            case 'g':
            case 'e':
            case 'E': {
                char *endp = NULL;
                double dval = runtime_strtod(s, &endp);
                if (endp == s) {
                    return assigned;
                }
                s = endp;
                if (!suppress) {
                    if (length_mod == LEN_L) *va_arg(ap, double *) = dval;
                    else *va_arg(ap, float *) = (float)dval;
                    assigned++;
                }
                break;
            }

            case 's': {
                char *dest = suppress ? NULL : va_arg(ap, char *);
                int count = 0;
                while (*s && !runtime_isspace((unsigned char)*s) && (width == 0 || count < width)) {
                    if (dest) {
                        *dest++ = *s;
                    }
                    s++;
                    count++;
                }
                if (dest) {
                    *dest = '\0';
                }
                if (count > 0 && !suppress) {
                    assigned++;
                }
                break;
            }

            case 'c': {
                int read_count = (width > 0) ? width : 1;
                char *dest = suppress ? NULL : va_arg(ap, char *);
                int count = 0;
                while (*s && count < read_count) {
                    if (dest) {
                        *dest++ = *s;
                    }
                    s++;
                    count++;
                }
                if (count == read_count && !suppress) {
                    assigned++;
                }
                break;
            }

            case 'n': {
                if (!suppress) {
                    int *out_n = va_arg(ap, int *);
                    if (out_n) {
                        *out_n = (int)(s - str);
                    }
                }
                break;
            }

            case '%': {
                if (*s != '%') return assigned;
                s++;
                break;
            }

            default:
                return assigned;
        }
    }

    return assigned;
}

int runtime_sscanf(const char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = runtime_vsscanf(str, format, ap);
    va_end(ap);
    return res;
}
