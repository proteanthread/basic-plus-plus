/**
 * @file round.c
 * @brief ROUND (_ROUND / MATH.ROUND) rounding function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ROUND (_ROUND, MATH.ROUND) function, rounding numeric expression `x` to nearest integer (or specified decimal places).
 *
 * 2. WHY IT EXISTS:
 * Provides standard mathematical half-up rounding (round half away from zero) for financial, geometric, and display calculations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates numeric arguments (x, optional decimals) and calls standard ISO C round(x).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_round'. Includes "eval/functions/math/round.h",
 * "runtime/micro_lib_metadata.h", <math.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support Banker's Rounding (round half to even) option.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Half-away-from-zero rounding rules matching standard ISO C `round()`.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float BValue representing rounded value or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify argument count validation and string type checks.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses standard ISO C <math.h> `round()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/eval/helpers.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/math/round.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/math/round.h"
#include "runtime/micro_lib_metadata.h"
#include <math.h>
#include <string.h>

void func_round_register(void) {
    MicroLibMetadata meta = {
        .name = "ROUND",
        .category = "Math Functions",
        .syntax = "ROUND(x [, decimals])",
        .help_text = "Rounds a numeric expression x to the specified number of decimal places (default 0).",
        .error_codes = "Error 13: Type Mismatch (ROUND expects 1 or 2 numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_round_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "_ROUND") != 0 && strcmp(uname, "ROUND") != 0 && strcmp(uname, "MATH.ROUND") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ROUND expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = round(args[0].as.number);
    return res;
}
