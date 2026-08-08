/**
 * @file tan.c
 * @brief TAN tangent function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements TAN built-in math function, computing the trigonometric tangent of an angle in radians.
 *
 * 2. WHY IT EXISTS:
 * Provides standard trigonometric tangent calculation matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates standard ISO C tan(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_tan'. Includes "eval/functions/math/tan.h",
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
 * Returns double float tangent value or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `tan()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/tan.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/tan.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_tan_register(void) {
    MicroLibMetadata meta = {
        .name = "TAN",
        .category = "Math Functions",
        .syntax = "TAN(radians)",
        .help_text = "Returns the trigonometric tangent of an angle given in radians.",
        .error_codes = "Error 13: Type Mismatch (TAN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_tan_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "TAN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "TAN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = tan(args[0].as.number);
    return res;
}
