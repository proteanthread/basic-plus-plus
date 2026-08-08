/**
 * @file oct.c
 * @brief OCT$ octal string conversion function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements OCT$ built-in string function, converting an integer numeric argument to an octal representation string.
 *
 * 2. WHY IT EXISTS:
 * Provides standard octal string formatting matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts to unsigned integer, and formats using `snprintf(tmp, sizeof(tmp), "%lo", uv)` to construct a heap-allocated `BppString`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_oct'. Includes "eval/functions/string/oct.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support optional fixed octal digit padding (e.g. OCT$(x, 6)).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Octal output digit character set ('0'-'7').
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing octal string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `snprintf` buffer sizing and string creation via `str_create()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. ISO C `snprintf` with `%lo`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/oct.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/oct.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>

void func_oct_register(void) {
    MicroLibMetadata meta = {
        .name = "OCT$",
        .category = "String Functions",
        .syntax = "OCT$(x)",
        .help_text = "Returns the octal string representation of integer x.",
        .error_codes = "Error 13: Type Mismatch (OCT$ expects one numeric argument)"
    };
    microlib_register(&meta);
}
#include <string.h>

BValue func_oct_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "OCT$ expects one numeric argument";
        return res;
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char tmp[24];
    snprintf(tmp, sizeof(tmp), "%lo", uv);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    return res;
}
