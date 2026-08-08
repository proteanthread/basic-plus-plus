/**
 * @file clamp.c
 * @brief CLAMP (_CLAMP / MATH.CLAMP) range bounding function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CLAMP (_CLAMP, MATH.CLAMP) function, clamping a numeric value `val` between specified `min_val` and `max_val` limits.
 *
 * 2. WHY IT EXISTS:
 * Provides essential range constraining logic for game physics, audio volume controls, and UI layout bounds.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates three numeric arguments, evaluating `val < min_val ? min_val : (val > max_val ? max_val : val)`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_clamp'. Includes "eval/functions/math/clamp.h",
 * "runtime/micro_lib_metadata.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support smooth step clamping or multi-dimensional vector clamping.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Three numeric argument signature `CLAMP(val, min_val, max_val)`.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float value within range [min_val, max_val] or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument count (3) and non-string type checking for all 3 arguments.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. IEEE 754 float comparison safety.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/clamp.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/clamp.h"
#include "runtime/micro_lib_metadata.h"
#include <string.h>

void func_clamp_register(void) {
    MicroLibMetadata meta = {
        .name = "CLAMP",
        .category = "Math Functions",
        .syntax = "CLAMP(val, min_val, max_val)",
        .help_text = "Clamps a value to be within the range [min_val, max_val].",
        .error_codes = "Error 13: Type Mismatch (CLAMP expects three numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_clamp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_CLAMP") != 0 && strcmp(uname, "CLAMP") != 0 && strcmp(uname, "MATH.CLAMP") != 0) {
        return res;
    }

    if (arg_count != 3 || args[0].type == VAL_STRING || args[1].type == VAL_STRING || args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "CLAMP expects three numeric arguments";
        return res;
    }

    double x = args[0].as.number;
    double min_val = args[1].as.number;
    double max_val = args[2].as.number;

    if (x < min_val) x = min_val;
    if (x > max_val) x = max_val;

    res.type = VAL_NUMBER;
    res.as.number = x;
    return res;
}
