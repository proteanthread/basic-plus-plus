// FILENAME: float_parse.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (float_parse.c, snprintf.c, sscanf.c, string.h)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h, scan_number.c)
// NEEDED BY: libengine (vm_internal.h)
// NEEDED BY: libstandard (editor.c)
// NEEDS: platform, memory
// Freestanding high-precision floating point string parser.
//
// ---- Includes ----

#ifndef RUNTIME_CONV_FLOAT_PARSE_H
#define RUNTIME_CONV_FLOAT_PARSE_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Baseline precision specification
#define RUNTIME_FLOAT_PRECISION_DIGITS 16

// @brief Parses double-precision floating-point number from string.
double runtime_strtod(const char *nptr, char **endptr);

// @brief Converts string to double-precision float.
double runtime_atof(const char *str);

// @brief Format mode flags for float formatting
typedef enum {
    RUNTIME_FLOAT_FMT_AUTO = 0,    // Standard BASIC-style: fixed if within [1e-4, 1e16), else scientific
    RUNTIME_FLOAT_FMT_FIXED,       // Always %f
    RUNTIME_FLOAT_FMT_EXP,         // Always %e / %E
    RUNTIME_FLOAT_FMT_SHORTEST     // %g: shortest representation
} RuntimeFloatFmtMode;

// @brief High-precision double to ASCII string converter (15-16+ digits bit-exact).
// @param value The double value to format.
// @param mode Format mode (auto, fixed, exp, shortest).
// @param precision Decimal places (or significant digits for shortest/auto). 0 = default (16).
// @param uppercase Use 'E' instead of 'e' for exponent.
// @param buf Destination buffer.
// @param buf_size Capacity of destination buffer.
// @return Number of characters written (excluding terminating null byte), or 0 on failure.
size_t runtime_dtoa_format(double value, RuntimeFloatFmtMode mode, int precision, bool uppercase, char *buf, size_t buf_size);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_CONV_FLOAT_PARSE_H
