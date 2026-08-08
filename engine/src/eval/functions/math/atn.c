/**
 * @file atn.c
 * @brief ATN / ATAN arctangent function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ATN (ATAN) built-in math function, computing the inverse tangent (arctangent) in radians of a ratio.
 *
 * 2. WHY IT EXISTS:
 * Provides standard trigonometric arctangent calculations matching GW-BASIC, QBASIC, and ANSI BASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates atan(x), returning angle in range [-pi/2, pi/2].
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_atn'. Includes "eval/functions/math/atn.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support degree mode conversion option.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Angle return unit (radians) and single argument evaluation contract.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float angle in radians [-pi/2, pi/2] or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `atan()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/atn.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/atn.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_atn_register(void) {
    MicroLibMetadata meta = {
        .name = "ATN",
        .category = "Math Functions",
        .syntax = "ATN(x)",
        .help_text = "Returns the arctangent of a numeric expression in radians.",
        .error_codes = "Error 13: Type Mismatch (ATN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_atn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "ATN") != 0 && strcmp(uname, "ATAN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ATN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = atan(args[0].as.number);
    return res;
}
