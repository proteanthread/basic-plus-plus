// FILENAME: float_parse.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (snprintf.c, sscanf.c, string.h)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h, scan_number.c)
// NEEDED BY: libengine (vm_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: libcore (ctype.h, ctype.c, float_limits.h, float_parse.h)
// NEEDS: libcore (memops.h, memops.c, num_parse.h, num_parse.c)
// NEEDS: libcore (strops.h, strops.c)
// Freestanding high-precision floating point string parsing implementation.
//
// ---- Includes ----

#include "runtime/conv/float_parse.h"
#include "runtime/conv/num_parse.h"
#include "runtime/math/float_limits.h"
#include "runtime/ctype/ctype.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include <stdint.h>


// Exact power of 10 lookup tables for double precision
static const double pow10_pos[] = {
    1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10,
    1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20,
    1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30,
    1e31, 1e32, 1e64, 1e128, 1e256
};

static const double pow10_neg[] = {
    1e-0, 1e-1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7, 1e-8, 1e-9, 1e-10,
    1e-11, 1e-12, 1e-13, 1e-14, 1e-15, 1e-16, 1e-17, 1e-18, 1e-19, 1e-20,
    1e-21, 1e-22, 1e-23, 1e-24, 1e-25, 1e-26, 1e-27, 1e-28, 1e-29, 1e-30,
    1e-31, 1e-32, 1e-64, 1e-128, 1e-256
};

static double calc_pow10(int exp) {
    if (exp == 0) return 1.0;
    if (exp > 308) return RUNTIME_INFINITY;
    if (exp < -324) return 0.0;

    double result = 1.0;
    int abs_exp = (exp < 0) ? -exp : exp;
    bool is_neg = (exp < 0);

    // Fast table multiply
    if (abs_exp <= 32) {
        return is_neg ? pow10_neg[abs_exp] : pow10_pos[abs_exp];
    }

    double base = 10.0;
    int e = abs_exp;
    while (e > 0) {
        if (e & 1) {
            result *= base;
        }
        base *= base;
        e >>= 1;
    }

    return is_neg ? (1.0 / result) : result;
}

double runtime_strtod(const char *nptr, char **endptr) {
    if (!nptr) {
        if (endptr) *endptr = NULL;
        return 0.0;
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

    // Check special tokens (NAN, INF, INFINITY)
    if (runtime_strncasecmp(s, "infinity", 8) == 0) {
        if (endptr) *endptr = (char *)(s + 8);
        return negative ? -RUNTIME_INFINITY : RUNTIME_INFINITY;
    }
    if (runtime_strncasecmp(s, "inf", 3) == 0) {
        if (endptr) *endptr = (char *)(s + 3);
        return negative ? -RUNTIME_INFINITY : RUNTIME_INFINITY;
    }
    if (runtime_strncasecmp(s, "nan", 3) == 0) {
        if (endptr) *endptr = (char *)(s + 3);
        return RUNTIME_NAN;
    }

    // Parse significand up to 17-19 digits
    uint64_t mantissa = 0;
    int decimal_digits = 0;
    bool has_decimal = false;
    bool has_digits = false;
    int excess_exponent = 0;

    while (*s) {
        if (runtime_isdigit((unsigned char)*s)) {
            has_digits = true;
            if (mantissa < 100000000000000000ULL) { // 17 digits
                mantissa = mantissa * 10 + (*s - '0');
                if (has_decimal) {
                    decimal_digits++;
                }
            } else {
                // Excess digits beyond precision threshold: adjust scale
                if (!has_decimal) {
                    excess_exponent++;
                }
            }
            s++;
        } else if (*s == '.' && !has_decimal) {
            has_decimal = true;
            s++;
        } else {
            break;
        }
    }

    if (!has_digits) {
        if (endptr) *endptr = (char *)nptr;
        return 0.0;
    }

    // Parse exponent ('e', 'E', or BASIC 'd', 'D')
    int exp_val = 0;
    if (*s == 'e' || *s == 'E' || *s == 'd' || *s == 'D') {
        const char *exp_start = s;
        s++;
        bool exp_neg = false;
        if (*s == '-') {
            exp_neg = true;
            s++;
        } else if (*s == '+') {
            s++;
        }

        if (runtime_isdigit((unsigned char)*s)) {
            while (runtime_isdigit((unsigned char)*s)) {
                if (exp_val < 2000) {
                    exp_val = exp_val * 10 + (*s - '0');
                }
                s++;
            }
            if (exp_neg) {
                exp_val = -exp_val;
            }
        } else {
            // No valid exponent digits; rewind to exponent start
            s = exp_start;
        }
    }

    if (endptr) {
        *endptr = (char *)s;
    }

    int total_exponent = exp_val + excess_exponent - decimal_digits;
    double result = (double)mantissa;

    if (total_exponent > 0) {
        if (total_exponent <= 32) {
            result *= pow10_pos[total_exponent];
        } else {
            result *= calc_pow10(total_exponent);
        }
    } else if (total_exponent < 0) {
        int neg_e = -total_exponent;
        if (neg_e <= 32) {
            result /= pow10_pos[neg_e];
        } else {
            result /= calc_pow10(neg_e);
        }
    }

    return negative ? -result : result;
}

double runtime_atof(const char *str) {
    return runtime_strtod(str, NULL);
}

// 15-16 Significant digit double to ASCII formatting
size_t runtime_dtoa_format(double value, RuntimeFloatFmtMode mode, int precision, bool uppercase, char *buf, size_t buf_size) {
    if (!buf || buf_size < 32) {
        return 0;
    }

    char *out = buf;
    char *end = buf + buf_size;

    // Handle NaN & Inf
    if (value != value) { // NaN
        return runtime_strlcpy(buf, uppercase ? "NAN" : "nan", buf_size);
    }
    if (value > RUNTIME_DBL_MAX) { // +Infinity
        return runtime_strlcpy(buf, uppercase ? "INF" : "inf", buf_size);
    }
    if (value < -RUNTIME_DBL_MAX) { // -Infinity
        return runtime_strlcpy(buf, uppercase ? "-INF" : "-inf", buf_size);
    }


    // Sign handling
    if (value < 0.0) {
        *out++ = '-';
        value = -value;
    } else if (1.0 / value < 0.0) {
        // Handle -0.0
        *out++ = '-';
        value = 0.0;
    }

    if (precision <= 0) {
        precision = RUNTIME_FLOAT_PRECISION_DIGITS;
    }

    // Determine formatting mode
    bool use_exp = false;
    if (mode == RUNTIME_FLOAT_FMT_EXP) {
        use_exp = true;
    } else if (mode == RUNTIME_FLOAT_FMT_AUTO) {
        if (value > 0.0 && (value >= 1e16 || value < 1e-4)) {
            use_exp = true;
        }
    }

    if (use_exp) {
        // Scientific exponent notation (e.g. 1.234567890123456E+02)
        int exponent = 0;
        if (value > 0.0) {
            while (value >= 10.0) {
                value /= 10.0;
                exponent++;
            }
            while (value < 1.0) {
                value *= 10.0;
                exponent--;
            }
        }

        // Integer part
        int digit = (int)value;
        *out++ = (char)('0' + digit);
        value -= digit;

        // Fractional part up to precision digits
        if (precision > 1) {
            *out++ = '.';
            int frac_digits = precision - 1;
            for (int i = 0; i < frac_digits && out < (end - 8); ++i) {
                value *= 10.0;
                digit = (int)value;
                *out++ = (char)('0' + digit);
                value -= digit;
            }
            // Strip trailing zeros in auto mode
            if (mode == RUNTIME_FLOAT_FMT_AUTO || mode == RUNTIME_FLOAT_FMT_SHORTEST) {
                while (out[-1] == '0') out--;
                if (out[-1] == '.') out--;
            }
        }

        // Exponent suffix
        *out++ = uppercase ? 'E' : 'e';
        if (exponent >= 0) {
            *out++ = '+';
        } else {
            *out++ = '-';
            exponent = -exponent;
        }
        if (exponent < 10) {
            *out++ = '0';
        }
        out += runtime_uint_to_str((uint64_t)exponent, out, (size_t)(end - out), 10, false);
        *out = '\0';
        return (size_t)(out - buf);
    } else {
        // Fixed-point notation
        uint64_t int_part = (uint64_t)value;
        double frac_part = value - (double)int_part;

        // Write integer digits
        out += runtime_uint_to_str(int_part, out, (size_t)(end - out), 10, false);

        if (precision > 0 && out < (end - 2)) {
            *out++ = '.';
            char *frac_start = out;
            for (int i = 0; i < precision && out < (end - 1); ++i) {
                frac_part *= 10.0;
                int digit = (int)frac_part;
                *out++ = (char)('0' + digit);
                frac_part -= digit;
            }

            // Rounding adjustment on last digit
            if (frac_part >= 0.5 && out > frac_start) {
                char *p = out - 1;
                while (p >= frac_start && *p == '9') {
                    *p-- = '0';
                }
                if (p >= frac_start) {
                    (*p)++;
                }
            }

            if (mode == RUNTIME_FLOAT_FMT_AUTO || mode == RUNTIME_FLOAT_FMT_SHORTEST) {
                while (out > frac_start && out[-1] == '0') out--;
                if (out > buf && out[-1] == '.') out--;
            }
        }
        *out = '\0';
        return (size_t)(out - buf);
    }
}
