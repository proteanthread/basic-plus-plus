// FILENAME: snprintf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot, libcore, libengine, libkernel, libserver, libstandard
// NEEDS: libcore (ctype.h, ctype.c, float_parse.h, float_parse.c)
// NEEDS: libcore (memops.h, memops.c, num_parse.h, num_parse.c, snprintf.h)
// NEEDS: libcore (strops.h, strops.c)
// Freestanding snprintf formatting implementation with 15-16 digit precision.
//
// ---- Includes ----

#include "runtime/format/snprintf.h"
#include "runtime/conv/num_parse.h"
#include "runtime/conv/float_parse.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include <stdint.h>

typedef struct {
    char *buf;
    size_t size;
    size_t written;
} FormatContext;

static inline void put_char(FormatContext *ctx, char c) {
    if (ctx->buf && ctx->written + 1 < ctx->size) {
        ctx->buf[ctx->written] = c;
    }
    ctx->written++;
}

static void put_string(FormatContext *ctx, const char *s, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        put_char(ctx, s[i]);
    }
}

int runtime_vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    if (!format) {
        return -1;
    }

    FormatContext ctx;
    ctx.buf = str;
    ctx.size = size;
    ctx.written = 0;

    const char *p = format;
    while (*p) {
        if (*p != '%') {
            put_char(&ctx, *p++);
            continue;
        }

        p++; // Skip '%'

        // Flags
        bool left_justify = false;
        bool plus_sign = false;
        bool space_sign = false;
        bool zero_pad = false;
        bool alt_form = false;

        while (*p) {
            if (*p == '-') left_justify = true;
            else if (*p == '+') plus_sign = true;
            else if (*p == ' ') space_sign = true;
            else if (*p == '0') zero_pad = true;
            else if (*p == '#') alt_form = true;
            else break;
            p++;
        }

        // Width
        int width = 0;
        if (*p == '*') {
            width = va_arg(ap, int);
            if (width < 0) {
                left_justify = true;
                width = -width;
            }
            p++;
        } else {
            while (runtime_isdigit((unsigned char)*p)) {
                width = width * 10 + (*p++ - '0');
            }
        }

        // Precision
        int precision = -1;
        if (*p == '.') {
            p++;
            if (*p == '*') {
                precision = va_arg(ap, int);
                p++;
            } else {
                precision = 0;
                while (runtime_isdigit((unsigned char)*p)) {
                    precision = precision * 10 + (*p++ - '0');
                }
            }
        }

        // Length modifiers
        enum { LEN_NONE, LEN_HH, LEN_H, LEN_L, LEN_LL, LEN_Z, LEN_J, LEN_T } length_mod = LEN_NONE;
        if (*p == 'h') {
            if (p[1] == 'h') { length_mod = LEN_HH; p += 2; }
            else { length_mod = LEN_H; p++; }
        } else if (*p == 'l') {
            if (p[1] == 'l') { length_mod = LEN_LL; p += 2; }
            else { length_mod = LEN_L; p++; }
        } else if (*p == 'z') {
            length_mod = LEN_Z; p++;
        } else if (*p == 'j') {
            length_mod = LEN_J; p++;
        } else if (*p == 't') {
            length_mod = LEN_T; p++;
        }

        char spec = *p++;
        if (!spec) break;

        char num_buf[128];
        char prefix[4];
        size_t prefix_len = 0;
        const char *out_str = NULL;
        size_t out_len = 0;

        switch (spec) {
            case 'd':
            case 'i': {
                int64_t val;
                if (length_mod == LEN_LL) val = va_arg(ap, long long);
                else if (length_mod == LEN_L) val = va_arg(ap, long);
                else if (length_mod == LEN_Z) val = (int64_t)va_arg(ap, size_t);
                else if (length_mod == LEN_HH) val = (signed char)va_arg(ap, int);
                else if (length_mod == LEN_H) val = (short)va_arg(ap, int);
                else val = va_arg(ap, int);

                if (val < 0) {
                    prefix[prefix_len++] = '-';
                    val = -val;
                } else if (plus_sign) {
                    prefix[prefix_len++] = '+';
                } else if (space_sign) {
                    prefix[prefix_len++] = ' ';
                }

                out_len = runtime_uint_to_str((uint64_t)val, num_buf, sizeof(num_buf), 10, false);
                out_str = num_buf;
                break;
            }

            case 'u':
            case 'x':
            case 'X':
            case 'o':
            case 'b': {
                uint64_t uval;
                if (length_mod == LEN_LL) uval = va_arg(ap, unsigned long long);
                else if (length_mod == LEN_L) uval = va_arg(ap, unsigned long);
                else if (length_mod == LEN_Z) uval = va_arg(ap, size_t);
                else if (length_mod == LEN_HH) uval = (unsigned char)va_arg(ap, unsigned int);
                else if (length_mod == LEN_H) uval = (unsigned short)va_arg(ap, unsigned int);
                else uval = va_arg(ap, unsigned int);

                int base = 10;
                bool uc = false;
                if (spec == 'x') {
                    base = 16;
                    if (alt_form && uval != 0) { prefix[prefix_len++] = '0'; prefix[prefix_len++] = 'x'; }
                } else if (spec == 'X') {
                    base = 16;
                    uc = true;
                    if (alt_form && uval != 0) { prefix[prefix_len++] = '0'; prefix[prefix_len++] = 'X'; }
                } else if (spec == 'o') {
                    base = 8;
                    if (alt_form && uval != 0) { prefix[prefix_len++] = '0'; }
                } else if (spec == 'b') {
                    base = 2;
                    if (alt_form && uval != 0) { prefix[prefix_len++] = '0'; prefix[prefix_len++] = 'b'; }
                }

                out_len = runtime_uint_to_str(uval, num_buf, sizeof(num_buf), base, uc);
                out_str = num_buf;
                break;
            }

            case 'p': {
                void *ptr = va_arg(ap, void *);
                if (!ptr) {
                    out_str = "(nil)";
                    out_len = 5;
                } else {
                    prefix[prefix_len++] = '0';
                    prefix[prefix_len++] = 'x';
                    out_len = runtime_uint_to_str((uintptr_t)ptr, num_buf, sizeof(num_buf), 16, false);
                    out_str = num_buf;
                }
                break;
            }

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G': {
                double fval = va_arg(ap, double);
                RuntimeFloatFmtMode fmode = RUNTIME_FLOAT_FMT_FIXED;
                bool uc = (spec == 'E' || spec == 'G' || spec == 'F');
                if (spec == 'e' || spec == 'E') {
                    fmode = RUNTIME_FLOAT_FMT_EXP;
                } else if (spec == 'g' || spec == 'G') {
                    fmode = RUNTIME_FLOAT_FMT_SHORTEST;
                }

                out_len = runtime_dtoa_format(fval, fmode, precision >= 0 ? precision : 6, uc, num_buf, sizeof(num_buf));
                out_str = num_buf;
                break;
            }

            case 's': {
                const char *s = va_arg(ap, const char *);
                if (!s) s = "(null)";
                size_t len = runtime_strlen(s);
                if (precision >= 0 && (size_t)precision < len) {
                    len = (size_t)precision;
                }
                out_str = s;
                out_len = len;
                break;
            }

            case 'c': {
                char c = (char)va_arg(ap, int);
                num_buf[0] = c;
                num_buf[1] = '\0';
                out_str = num_buf;
                out_len = 1;
                break;
            }

            case '%': {
                num_buf[0] = '%';
                num_buf[1] = '\0';
                out_str = num_buf;
                out_len = 1;
                break;
            }

            default:
                put_char(&ctx, '%');
                put_char(&ctx, spec);
                continue;
        }

        // Apply padding
        int total_len = (int)(prefix_len + out_len);
        int pad = (width > total_len) ? (width - total_len) : 0;

        if (!left_justify && !zero_pad) {
            while (pad-- > 0) put_char(&ctx, ' ');
        }

        // Output prefix
        for (size_t i = 0; i < prefix_len; ++i) {
            put_char(&ctx, prefix[i]);
        }

        if (!left_justify && zero_pad) {
            while (pad-- > 0) put_char(&ctx, '0');
        }

        // Output body
        put_string(&ctx, out_str, out_len);

        if (left_justify) {
            while (pad-- > 0) put_char(&ctx, ' ');
        }
    }

    // Null-terminate buffer
    if (ctx.buf && ctx.size > 0) {
        if (ctx.written < ctx.size) {
            ctx.buf[ctx.written] = '\0';
        } else {
            ctx.buf[ctx.size - 1] = '\0';
        }
    }

    return (int)ctx.written;
}

int runtime_snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = runtime_vsnprintf(str, size, format, ap);
    va_end(ap);
    return res;
}

int runtime_vsprintf(char *str, const char *format, va_list ap) {
    return runtime_vsnprintf(str, (size_t)-1, format, ap);
}

int runtime_sprintf(char *str, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int res = runtime_vsnprintf(str, (size_t)-1, format, ap);
    va_end(ap);
    return res;
}
