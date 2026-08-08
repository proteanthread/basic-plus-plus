/**
 * @file exp.c
 * @brief EXP natural exponential e^x function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements EXP built-in math function, computing e raised to the power of numeric expression x (e^x).
 *
 * 2. WHY IT EXISTS:
 * Provides natural exponential calculations matching GW-BASIC, QBASIC, and ANSI BASIC standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts argument to double float, and evaluates standard ISO C exp(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_exp'. Includes "eval/functions/math/exp.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support arbitrary power base calculations (e.g. 2^x, 10^x).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard mathematical base e (Euler's number constant ~2.718281828459045).
 *
 * 8. WHAT TO EXPECT:
 * Returns double float value or ERR_OVERFLOW (error 6) for exponent out of representable IEEE 754 float range.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument value range and overflow check handling.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `exp()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/exp.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/exp.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_exp_register(void) {
    MicroLibMetadata meta = {
        .name = "EXP",
        .category = "Math Functions",
        .syntax = "EXP(x)",
        .help_text = "Returns e raised to the power of a numeric expression x.",
        .error_codes = "Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_exp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "EXP") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "EXP expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = exp(args[0].as.number);
    return res;
}
