/**
 * @file pi.c
 * @brief PI (_PI / MATH.PI / PI) mathematical constant function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements PI (_PI, MATH.PI, PI) function, returning the mathematical constant pi (~3.14159265358979323846).
 *
 * 2. WHY IT EXISTS:
 * Provides standard high-precision pi constant without requiring user manual floating point literals.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates zero arguments and returns double float M_PI.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_pi'. Includes "eval/functions/math/pi.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add arbitrary precision pi constant lookup for high precision modes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 0 argument count requirement for PI().
 *
 * 8. WHAT TO EXPECT:
 * Returns double float M_PI or ERR_ILLEGAL_FUNCTION_CALL (error 5) if arguments passed.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument count validation (`arg_count != 0`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * No arguments passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Defines `M_PI` if not defined by host `<math.h>`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/pi.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/pi.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void func_pi_register(void) {
    MicroLibMetadata meta = {
        .name = "PI",
        .category = "Math Functions",
        .syntax = "PI()",
        .help_text = "Returns the mathematical constant pi (~3.141592653589793).",
        .error_codes = "Error 5: Illegal Function Call (PI expects 0 arguments)"
    };
    microlib_register(&meta);
}

BValue func_pi_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_PI") != 0 && strcmp(uname, "PI") != 0 && strcmp(uname, "MATH.PI") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 13;
        err->message = "PI expects no arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 3.14159265358979323846;
    return res;
}
