/**
 * @file cos.c
 * @brief COS cosine function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements COS built-in math function, computing the trigonometric cosine of an angle in radians.
 *
 * 2. WHY IT EXISTS:
 * Provides standard trigonometric cosine calculation matching GW-BASIC, QBASIC, and ANSI BASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates standard ISO C cos(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_cos'. Includes "eval/functions/math/cos.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support degree mode angle inputs.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Angle input unit (radians) and single argument evaluation contract.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float value in [-1.0, 1.0] or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `cos()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/cos.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/cos.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_cos_register(void) {
    MicroLibMetadata meta = {
        .name = "COS",
        .category = "Math Functions",
        .syntax = "COS(radians)",
        .help_text = "Returns the trigonometric cosine of an angle given in radians.",
        .error_codes = "Error 13: Type Mismatch (COS expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_cos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "COS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "COS expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = cos(args[0].as.number);
    return res;
}
