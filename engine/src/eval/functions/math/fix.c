/**
 * @file fix.c
 * @brief FIX integer truncation function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements FIX built-in math function, returning the truncated integer portion of x by stripping fractional digits toward zero.
 *
 * 2. WHY IT EXISTS:
 * Provides standard zero-truncation (unlike INT which floors toward negative infinity) matching GW-BASIC, QBASIC, and ANSI BASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument: for x >= 0 evaluates floor(x), for x < 0 evaluates ceil(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_fix'. Includes "eval/functions/math/fix.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support element-wise truncation for array/matrix operands.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Truncation toward zero rule (for negative values, FIX(-1.9) = -1.0, whereas INT(-1.9) = -2.0).
 *
 * 8. WHAT TO EXPECT:
 * Returns double float BValue representing truncated integer or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `floor()` and `ceil()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/fix.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/fix.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_fix_register(void) {
    MicroLibMetadata meta = {
        .name = "FIX",
        .category = "Math Functions",
        .syntax = "FIX(x)",
        .help_text = "Returns the truncated integer part of x (truncates towards zero).",
        .error_codes = "Error 13: Type Mismatch (FIX expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_fix_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "FIX") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "FIX expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = (args[0].as.number >= 0.0) ? floor(args[0].as.number) : ceil(args[0].as.number);
    return res;
}
