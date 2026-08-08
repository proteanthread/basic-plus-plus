/**
 * @file abs.c
 * @brief ABS(x) absolute value function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ABS(x) built-in math function, computing the non-negative magnitude (absolute value) of numeric expression x.
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical absolute value calculation per GW-BASIC, QBASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates fabs(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_abs'. Includes "eval/functions/math/abs.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support element-wise absolute value computation for matrix operands.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Double precision float absolute value calculation rules.
 *
 * 8. WHAT TO EXPECT:
 * Returns non-negative double float BValue or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify single numeric argument type check (`args[0].type != VAL_STRING`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `fabs()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/abs.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/abs.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_abs_register(void) {
    MicroLibMetadata meta = {
        .name = "ABS",
        .category = "Math Functions",
        .syntax = "ABS(x)",
        .help_text = "Returns the absolute value of a numeric expression.",
        .error_codes = "Error 13: Type Mismatch (ABS expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_abs_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "ABS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ABS expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = fabs(args[0].as.number);
    return res;
}
