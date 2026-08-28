// FILENAME: num_format.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (map_serialize.c, num_format.c, string_ext.c)
// NEEDED BY: libcore (using_engine.c, variables_internal.h)
// NEEDED BY: libengine (ast_eval_expr.c, ast_eval_stmt.c, ast_internal.h)
// NEEDED BY: libengine (helpers.c, lprint.c, mat_print.c, print.c)
// NEEDED BY: libengine (print_file.c, str.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for num_format within BASIC++.
//
// ---- Includes ----

#ifndef NUM_FORMAT_H
#define NUM_FORMAT_H

#include <stddef.h>
#include <stdbool.h>

// Display precision: 15 significant digits.
// Hides IEEE 754 double rounding artifacts while preserving
// all meaningful information. Safe for user-facing output.
#define NUM_FORMAT_DISPLAY_DIGITS  15

// Serialization precision: 16 significant digits.
// Preserves full IEEE 754 double round-trip fidelity for
// file I/O and data serialization.
#define NUM_FORMAT_SERIALIZE_DIGITS 16

// Maximum exact integer representable in IEEE 754 double.
// 2^53 = 9,007,199,254,740,992.
#define NUM_FORMAT_MAX_EXACT_INT 9007199254740992.0

// @brief Format a double for user-facing display output.
//
// Uses 15 significant digits with trailing-zero stripping.
// Intended for: PRINT, LPRINT, STR$, NUM$, MAT PRINT.
//
// Three-tier formatting:
// 1. Exact integers up to 2^53 -> plain integer (e.g., "99999999999999")
// 2. Fractional values in [0.000001, 2^53] -> fixed-point with trailing zeros stripped
// 3. Everything else -> scientific notation with 15 significant digits
//
// @param buf            Output buffer.
// @param buf_size       Size of the output buffer in bytes.
// @param val            The double value to format.
// @param leading_space  If true, prepend " " for non-negative values (GW-BASIC convention).
// @param trailing_space If true, append " " after the number.
void num_format_display(char *buf, size_t buf_size, double val,
                        bool leading_space, bool trailing_space);

// @brief Format a double for serialization / data output.
//
// Uses 16 significant digits for full IEEE 754 round-trip fidelity.
// Intended for: PRINT #, DISPLAY, map_serialize, state files.
//
// Same three-tier approach as num_format_display but with 16-digit precision
// and NO trailing-zero stripping (preserves data fidelity).
//
// @param buf            Output buffer.
// @param buf_size       Size of the output buffer in bytes.
// @param val            The double value to format.
void num_format_serialize(char *buf, size_t buf_size, double val);

#endif // NUM_FORMAT_H
