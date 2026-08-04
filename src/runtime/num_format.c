/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file num_format.c
 * @brief Canonical implementation of unified number-to-string formatting.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Implements two public formatting functions:
 *   num_format_display() for user-facing output (15 significant digits, clean)
 *   and num_format_serialize() for data/file output (16 significant digits, raw).
 * - Why it exists: Replaces duplicated format_double_clean() functions in
 *   stmt_print.c and eval.c, and upgrades all raw %g call sites across the
 *   interpreter to use consistent, high-precision formatting.
 * - Why it works this way: Three-tier approach (integer -> fixed-point -> scientific)
 *   matches GW-BASIC/QBASIC display conventions. The integer tier uses an exact
 *   integer check (val == (double)(long long)val) to safely cast to long long
 *   without truncation errors. The 2^53 threshold is the hardware limit for
 *   exact integer representation in IEEE 754 double.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Precision digit counts via NUM_FORMAT_DISPLAY_DIGITS
 *   and NUM_FORMAT_SERIALIZE_DIGITS macros in num_format.h. Buffer sizes for
 *   intermediate formatting can be adjusted if needed.
 * - What cannot be changed: The three-tier order (integer first, then fixed,
 *   then scientific). The exact-integer check MUST precede the fixed-point
 *   path to avoid formatting 1000000.0 as "1000000.000000000000000".
 * - What to expect: Both functions write a null-terminated string to buf.
 *   Output length is bounded by buf_size. On buffer overflow, output is
 *   truncated by snprintf.
 * - What to do if something breaks: Test with edge cases: 0.0, -0.0, NaN,
 *   Inf, 9007199254740992.0 (2^53), 9007199254740993.0 (2^53+1, NOT exact),
 *   0.000001, 0.0000009, 1e20, -1e20.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: IEEE 754 double precision. long long is >= 64 bits.
 *   snprintf is available and C17-conformant.
 * - Portability concerns: Fully portable. Uses only <stdio.h>, <math.h>,
 *   <string.h>, <stdbool.h>, <stddef.h>. Compiles cleanly on MSVC and GCC.
 *   The strlen() calls in trailing-zero stripping operate on null-terminated
 *   buffers produced by snprintf, so bounded string rules are satisfied.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - A third variant (num_format_custom) could accept a precision parameter
 *   for dialect-configurable display widths.
 * - If BASIC++ adds integer types (INT32, INT64), dedicated integer formatters
 *   should be added rather than routing through these double formatters.
 */

#include "num_format.h"

#include <stdio.h>
#include <math.h>
#include <string.h>

/**
 * @brief Internal helper: strip trailing zeros and trailing decimal point
 *        from a fixed-point formatted string.
 *
 * Operates on a null-terminated string produced by snprintf with %f or %.Nf.
 * Only strips if a decimal point is present (avoids mangling integer strings).
 *
 * @param s  Null-terminated string buffer to modify in-place.
 */
static void strip_trailing_zeros(char *s) {
    /* Find the decimal point; if none, nothing to strip */
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

    /* Strip trailing '0' characters after the decimal point */
    char *end = p - 1; /* points to last non-null char */
    while (end > dot && *end == '0') {
        *end = '\0';
        end--;
    }
    /* If only the dot remains, strip it too */
    if (end == dot) {
        *end = '\0';
    }
}

void num_format_display(char *buf, size_t buf_size, double val,
                        bool leading_space, bool trailing_space) {
    const char *prefix = "";
    const char *suffix = "";

    if (leading_space) {
        prefix = (val >= 0.0) ? " " : "";
    }
    if (trailing_space) {
        suffix = " ";
    }

    /*
     * Tier 1: Exact integer display.
     * If the value is an exact integer representable in long long and within
     * the IEEE 754 exact range (2^53), display as a plain integer.
     */
    if (val == (double)(long long)val &&
        fabs(val) <= NUM_FORMAT_MAX_EXACT_INT) {
        snprintf(buf, buf_size, "%s%lld%s", prefix, (long long)val, suffix);
        return;
    }

    /*
     * Tier 2: Fixed-point decimal display.
     * For values within [0.000001, 2^53] that have fractional parts,
     * format with %.15f and strip trailing zeros for clean output.
     */
    if (fabs(val) <= NUM_FORMAT_MAX_EXACT_INT && fabs(val) >= 0.000001) {
        char temp[64];
        snprintf(temp, sizeof(temp), "%.15f", val);
        strip_trailing_zeros(temp);
        snprintf(buf, buf_size, "%s%s%s", prefix, temp, suffix);
        return;
    }

    /*
     * Tier 3: Scientific notation fallback.
     * For very large (> 2^53) or very small (< 0.000001) values,
     * use %.15g for 15 significant digits.
     */
    snprintf(buf, buf_size, "%s%.15g%s", prefix, val, suffix);
}

void num_format_serialize(char *buf, size_t buf_size, double val) {
    /*
     * Tier 1: Exact integer serialization.
     * Same integer check as display, but no leading/trailing spaces.
     */
    if (val == (double)(long long)val &&
        fabs(val) <= NUM_FORMAT_MAX_EXACT_INT) {
        snprintf(buf, buf_size, "%lld", (long long)val);
        return;
    }

    /*
     * Tier 2+3: Full precision serialization.
     * Use %.16g for maximum IEEE 754 double round-trip fidelity.
     * No trailing-zero stripping — data integrity is paramount.
     */
    snprintf(buf, buf_size, "%.16g", val);
}
