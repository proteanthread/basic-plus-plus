/**
 * @file atan2.c
 * @brief ATAN2 (_ATAN2 / MATH.ATAN2) 2-argument arctangent function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ATAN2 (_ATAN2, MATH.ATAN2) function, computing the 4-quadrant inverse tangent (atan2) of y and x in radians.
 *
 * 2. WHY IT EXISTS:
 * Provides robust 4-quadrant arctangent calculation handling zero denominators and sign tracking.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates two numeric arguments and calls standard ISO C atan2(y, x), returning angle in range (-pi, pi].
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_atan2'. Includes "eval/functions/math/atan2.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support degree mode output option.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Calling order atan2(y, x) matching standard mathematical convention (y coordinate first, then x).
 *
 * 8. WHAT TO EXPECT:
 * Returns double float angle in radians (-pi, pi] or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check argument count validation and string type checks.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `atan2()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/atan2.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/atan2.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_atan2_register(void) {
    MicroLibMetadata meta = {
        .name = "ATAN2",
        .category = "Math Functions",
        .syntax = "ATAN2(y, x)",
        .help_text = "Returns the 2-argument arctangent of y and x in radians.",
        .error_codes = "Error 13: Type Mismatch (ATAN2 expects two numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_atan2_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_ATAN2") != 0 && strcmp(uname, "ATAN2") != 0 && strcmp(uname, "MATH.ATAN2") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "ATAN2 expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = atan2(args[0].as.number, args[1].as.number);
    return res;
}
