/**
 * @file ceil.c
 * @brief CEIL (_CEIL / MATH.CEIL) ceiling function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements CEIL (_CEIL, MATH.CEIL) function, returning the smallest integer value greater than or equal to numeric expression x.
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical ceiling rounding required for discrete math, graphics, and layout calculations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates standard ISO C ceil(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_ceil'. Includes "eval/functions/math/ceil.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support element-wise ceiling computation for array/matrix operands.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Double precision float ceiling calculation rules.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float BValue representing ceiling integer or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `ceil()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/ceil.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/ceil.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_ceil_register(void) {
    MicroLibMetadata meta = {
        .name = "CEIL",
        .category = "Math Functions",
        .syntax = "CEIL(x)",
        .help_text = "Returns the smallest integer greater than or equal to x.",
        .error_codes = "Error 13: Type Mismatch (CEIL expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_ceil_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_CEIL") != 0 && strcmp(uname, "CEIL") != 0 && strcmp(uname, "MATH.CEIL") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "CEIL expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = ceil(args[0].as.number);
    return res;
}
