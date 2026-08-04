/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file num_format.h
 * @brief Unified number-to-string formatting for the BASIC++ interpreter.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Provides two formatting functions for converting IEEE 754
 *   double values to human-readable strings. One variant targets user-facing
 *   display output (PRINT, STR$, NUM$, MAT PRINT, LPRINT) at 15 significant
 *   digits with trailing-zero stripping, and the other targets machine-readable
 *   serialization (PRINT #, map/state serialization, DISPLAY) at full 16-digit
 *   precision for round-trip data fidelity.
 * - Why it exists: Prior to this module, number formatting was duplicated in
 *   stmt_print.c and eval.c as identical static `format_double_clean` functions,
 *   and many other call sites (STR$, MAT PRINT, PRINT #, map_serialize) used
 *   raw `%g` with only 6 significant digits and a 9-digit integer threshold.
 *   Centralizing the logic eliminates duplication and ensures consistent
 *   formatting across the entire interpreter.
 * - Why it works this way: Uses a three-tier approach inherited from GW-BASIC
 *   conventions: (1) exact integers use %lld for clean integer display up to
 *   2^53, (2) values with fractional parts in a displayable range use %f with
 *   trailing-zero stripping, (3) very large or very small values fall back to
 *   scientific notation. The 2^53 threshold (9,007,199,254,740,992) is the
 *   largest integer exactly representable in IEEE 754 double precision.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: The significant digit counts (NUM_FORMAT_DISPLAY_DIGITS
 *   and NUM_FORMAT_SERIALIZE_DIGITS) can be tuned. The integer threshold
 *   (NUM_FORMAT_MAX_EXACT_INT) should not be changed unless the underlying
 *   float type changes.
 * - What cannot be changed: The three-tier formatting order (integer check first,
 *   then fixed-point, then scientific) must be preserved. The integer tier MUST
 *   use the exact-integer check (`val == (double)(long long)val`) to avoid
 *   displaying non-integer doubles as integers.
 * - What to expect: All numeric output from the interpreter should flow through
 *   one of these two functions. Changing precision here affects the entire system.
 * - What to do if something breaks: If numbers display incorrectly, check which
 *   tier the value falls into. Add temporary debug output showing `fabs(val)` and
 *   the tier selection to diagnose formatting.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: IEEE 754 double precision (64-bit) is used for all numeric values.
 *   `long long` is at least 64 bits (guaranteed by C17).
 * - Portability concerns: Fully portable across Windows (MSVC) and Linux (GCC).
 *   No platform-specific code. Uses only C17 standard library functions.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - Future dialects may define custom display precision via dialect configuration
 *   layers. The precision constants are defined as macros for easy override.
 * - If BASIC++ ever supports arbitrary precision or decimal types, the formatting
 *   functions should be extended with new type-specific variants.
 */

#ifndef NUM_FORMAT_H
#define NUM_FORMAT_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Display precision: 15 significant digits.
 * Hides IEEE 754 double rounding artifacts while preserving
 * all meaningful information. Safe for user-facing output.
 */
#define NUM_FORMAT_DISPLAY_DIGITS  15

/**
 * Serialization precision: 16 significant digits.
 * Preserves full IEEE 754 double round-trip fidelity for
 * file I/O and data serialization.
 */
#define NUM_FORMAT_SERIALIZE_DIGITS 16

/**
 * Maximum exact integer representable in IEEE 754 double.
 * 2^53 = 9,007,199,254,740,992.
 */
#define NUM_FORMAT_MAX_EXACT_INT 9007199254740992.0

/**
 * @brief Format a double for user-facing display output.
 *
 * Uses 15 significant digits with trailing-zero stripping.
 * Intended for: PRINT, LPRINT, STR$, NUM$, MAT PRINT.
 *
 * Three-tier formatting:
 * 1. Exact integers up to 2^53 -> plain integer (e.g., "99999999999999")
 * 2. Fractional values in [0.000001, 2^53] -> fixed-point with trailing zeros stripped
 * 3. Everything else -> scientific notation with 15 significant digits
 *
 * @param buf            Output buffer.
 * @param buf_size       Size of the output buffer in bytes.
 * @param val            The double value to format.
 * @param leading_space  If true, prepend " " for non-negative values (GW-BASIC convention).
 * @param trailing_space If true, append " " after the number.
 */
void num_format_display(char *buf, size_t buf_size, double val,
                        bool leading_space, bool trailing_space);

/**
 * @brief Format a double for serialization / data output.
 *
 * Uses 16 significant digits for full IEEE 754 round-trip fidelity.
 * Intended for: PRINT #, DISPLAY, map_serialize, state files.
 *
 * Same three-tier approach as num_format_display but with 16-digit precision
 * and NO trailing-zero stripping (preserves data fidelity).
 *
 * @param buf            Output buffer.
 * @param buf_size       Size of the output buffer in bytes.
 * @param val            The double value to format.
 */
void num_format_serialize(char *buf, size_t buf_size, double val);

#endif /* NUM_FORMAT_H */
