/**
 * @file log.c
 * @brief LOG / LN natural logarithm function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LOG (LN) built-in math function, computing the natural logarithm (base e) of numeric expression x > 0.
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical natural logarithm calculations matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument x > 0 and evaluates standard ISO C log(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_log'. Includes "eval/functions/math/log.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support log base 10 or log base 2 functions.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Domain validation check x > 0 returning ERR_ILLEGAL_FUNCTION_CALL (error 5) for x <= 0.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float natural log value or ERR_ILLEGAL_FUNCTION_CALL (error 5) for domain violation.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument range check (`args[0].as.number <= 0.0`).
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `log()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/log.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/log.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_log_register(void) {
    MicroLibMetadata meta = {
        .name = "LOG",
        .category = "Math Functions",
        .syntax = "LOG(x)",
        .help_text = "Returns the natural logarithm (base e) of a numeric expression x > 0.",
        .error_codes = "Error 5: Illegal Function Call (LOG of zero or negative number), Error 13: Type Mismatch (LOG expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_log_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "LOG") != 0 && strcmp(uname, "LN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "LOG expects one numeric argument";
        return res;
    }

    if (args[0].as.number <= 0.0) {
        err->code = 5;
        err->message = "LOG of zero or negative number";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = log(args[0].as.number);
    return res;
}
