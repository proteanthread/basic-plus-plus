/**
 * @file space.c
 * @brief SPACE$ space character string generation function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements SPACE$ built-in string function, generating a string consisting of `n` space characters.
 *
 * 2. WHY IT EXISTS:
 * Provides standard space padding string generation matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single non-negative numeric count argument `n`, allocates heap memory, fills buffer with ASCII space characters (`0x20`), creates a `BppString`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_space'. Includes "eval/functions/string/space.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add fast zero-allocation static buffer paths for small space counts (e.g. n <= 32).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Negative count `n < 0` returning ERR_ILLEGAL_FUNCTION_CALL (error 5).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing n space characters or ERR_ILLEGAL_FUNCTION_CALL (error 5) or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify non-negative bounds check `n < 0` and string creation via `str_create()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses `memset` for space filling.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/space.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/space.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdlib.h>
#include <string.h>

void func_space_register(void) {
    MicroLibMetadata meta = {
        .name = "SPACE$",
        .category = "String Functions",
        .syntax = "SPACE$(n)",
        .help_text = "Returns a string consisting of n space characters.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (SPACE$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_space_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SPACE$ expects one numeric argument";
        return res;
    }

    int n = (int)args[0].as.number;
    if (n < 0) {
        err->code = 5;
        err->message = "Negative count in SPACE$";
        return res;
    }

    char *buf = (char *)calloc(1, n + 1);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    memset(buf, ' ', n);
    buf[n] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, n);
    free(buf);
    return res;
}
