/**
 * @file sgn.c
 * @brief SGN sign function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements SGN built-in math function, returning the mathematical sign indicator of numeric expression x (-1.0 if x < 0, 0.0 if x == 0, 1.0 if x > 0).
 *
 * 2. WHY IT EXISTS:
 * Provides standard sign testing matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, testing value with standard double-precision comparisons.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_sgn'. Includes "eval/functions/math/sgn.h",
 * "runtime/micro_lib_metadata.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support signum evaluation over complex numbers or matrix elements.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Three-state return values {-1.0, 0.0, 1.0}.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float -1.0, 0.0, or 1.0, or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. IEEE 754 float sign and zero comparisons.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/sgn.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/sgn.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void func_sgn_register(void) {
    MicroLibMetadata meta = {
        .name = "SGN",
        .category = "Math Functions",
        .syntax = "SGN(x)",
        .help_text = "Returns the sign of x: 1 if x > 0, 0 if x = 0, -1 if x < 0.",
        .error_codes = "Error 13: Type Mismatch (SGN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_sgn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "SGN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SGN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    double val = args[0].as.number;
    if (val > 0.0) res.as.number = 1.0;
    else if (val < 0.0) res.as.number = -1.0;
    else res.as.number = 0.0;
    return res;
}
