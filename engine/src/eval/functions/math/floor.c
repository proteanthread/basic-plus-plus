/**
 * @file floor.c
 * @brief FLOOR (_FLOOR / MATH.FLOOR) floor function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements FLOOR (_FLOOR, MATH.FLOOR) function, returning the largest integer value less than or equal to numeric expression x.
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical floor rounding required for grid math, alignment calculations, and array coordinate indexing.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates standard ISO C floor(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_floor'. Includes "eval/functions/math/floor.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support element-wise floor computation for array/matrix operands.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Double precision float floor calculation rules (floors toward negative infinity).
 *
 * 8. WHAT TO EXPECT:
 * Returns double float BValue representing floor integer or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `floor()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/floor.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/floor.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_floor_register(void) {
    MicroLibMetadata meta = {
        .name = "FLOOR",
        .category = "Math Functions",
        .syntax = "FLOOR(x)",
        .help_text = "Returns the largest integer less than or equal to x.",
        .error_codes = "Error 13: Type Mismatch (FLOOR expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_floor_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_FLOOR") != 0 && strcmp(uname, "FLOOR") != 0 && strcmp(uname, "MATH.FLOOR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "FLOOR expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = floor(args[0].as.number);
    return res;
}
