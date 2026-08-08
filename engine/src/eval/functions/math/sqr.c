/**
 * @file sqr.c
 * @brief SQR square root function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements SQR built-in math function, computing the non-negative square root of numeric expression x >= 0.
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical square root calculations matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument x >= 0 and evaluates standard ISO C sqrt(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_sqr'. Includes "eval/functions/math/sqr.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support complex square root evaluation (returning complex number for x < 0).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Domain check x >= 0 returning ERR_ILLEGAL_FUNCTION_CALL (error 5) for negative inputs.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float non-negative square root or ERR_ILLEGAL_FUNCTION_CALL (error 5) for domain violation.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify negative input domain check (`args[0].as.number < 0.0`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `sqrt()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/sqr.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/sqr.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_sqr_register(void) {
    MicroLibMetadata meta = {
        .name = "SQR",
        .category = "Math Functions",
        .syntax = "SQR(x)",
        .help_text = "Returns the non-negative square root of a numeric expression x >= 0.",
        .error_codes = "Error 5: Illegal Function Call (SQR of negative number), Error 13: Type Mismatch (SQR expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_sqr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "SQR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SQR expects one numeric argument";
        return res;
    }

    if (args[0].as.number < 0.0) {
        err->code = 5;
        err->message = "SQR of negative number";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = sqrt(args[0].as.number);
    return res;
}
