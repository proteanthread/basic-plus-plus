/**
 * @file asin.c
 * @brief ASIN (_ASIN / MATH.ASIN) arcsine function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ASIN (_ASIN, MATH.ASIN) function, computing the inverse sine (arcsine) in radians for a ratio in range [-1.0, 1.0].
 *
 * 2. WHY IT EXISTS:
 * Provides trigonometric inverse sine calculations required for geometric and scientific computing.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates domain [-1.0, 1.0] and calls standard ISO C asin(x), returning angle in radians [-pi/2, pi/2].
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_asin'. Includes "eval/functions/math/asin.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support degree mode conversion option.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Domain validation check x in [-1.0, 1.0] returning ERR_ILLEGAL_FUNCTION_CALL (error 5) on out of domain values.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float angle in radians or ERR_ILLEGAL_FUNCTION_CALL (error 5) for domain violation.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument range validation logic (`val < -1.0 || val > 1.0`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `asin()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/asin.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/asin.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_asin_register(void) {
    MicroLibMetadata meta = {
        .name = "ASIN",
        .category = "Math Functions",
        .syntax = "ASIN(x)",
        .help_text = "Returns the arcsine of x in radians for -1.0 <= x <= 1.0.",
        .error_codes = "Error 5: Illegal Function Call (ASIN argument out of range [-1, 1]), Error 13: Type Mismatch (ASIN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_asin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_ASIN") != 0 && strcmp(uname, "ASIN") != 0 && strcmp(uname, "MATH.ASIN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ASIN expects one numeric argument";
        return res;
    }

    if (args[0].as.number < -1.0 || args[0].as.number > 1.0) {
        err->code = 5;
        err->message = "ASIN domain error";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = asin(args[0].as.number);
    return res;
}
