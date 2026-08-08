/**
 * @file lerp.c
 * @brief LERP (_LERP / MATH.LERP) linear interpolation function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LERP (_LERP, MATH.LERP) function, performing linear interpolation between `a` and `b` using blend factor `t` (a + (b - a) * t).
 *
 * 2. WHY IT EXISTS:
 * Provides smooth numerical transitions for animation, game state interpolation, color blending, and signal processing.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates three numeric arguments (a, b, t) and evaluates exact floating-point blend formula `a + t * (b - a)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_lerp'. Includes "eval/functions/math/lerp.h",
 * "runtime/micro_lib_metadata.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support non-linear easing functions (smoothstep, cubic lerp).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Three numeric argument signature `LERP(a, b, t)`.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float interpolated value or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument count (3) and type validation for all 3 operands.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Standard double-precision IEEE 754 arithmetic.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/lerp.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/lerp.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void func_lerp_register(void) {
    MicroLibMetadata meta = {
        .name = "LERP",
        .category = "Math Functions",
        .syntax = "LERP(a, b, t)",
        .help_text = "Performs linear interpolation between a and b using weight t (a + (b - a) * t).",
        .error_codes = "Error 13: Type Mismatch (LERP expects three numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_lerp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_LERP") != 0 && strcmp(uname, "LERP") != 0 && strcmp(uname, "MATH.LERP") != 0) {
        return res;
    }

    if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "LERP expects three numeric arguments";
        return res;
    }

    double a = args[0].as.number;
    double b = args[1].as.number;
    double t = args[2].as.number;

    res.type = VAL_NUMBER;
    res.as.number = a + t * (b - a);
    return res;
}
