/**
 * @file hex.c
 * @brief HEX$ hexadecimal string conversion function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements HEX$ built-in string function, converting an integer numeric argument to an uppercase hexadecimal string.
 *
 * 2. WHY IT EXISTS:
 * Provides standard hexadecimal string formatting matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument, converts to unsigned integer, and formats using `snprintf(tmp, sizeof(tmp), "%lX", uv)` to construct a heap-allocated `BppString`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_hex'. Includes "eval/functions/string/hex.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdio.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support optional fixed hex digit padding (e.g. HEX$(x, 4)).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Uppercase hexadecimal output character set ('0'-'9', 'A'-'F').
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing uppercase hex string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `snprintf` buffer sizing and string creation via `str_create()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. ISO C `snprintf` with `%lX`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/hex.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/hex.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdio.h>

void func_hex_register(void) {
    MicroLibMetadata meta = {
        .name = "HEX$",
        .category = "String Functions",
        .syntax = "HEX$(x)",
        .help_text = "Returns the hexadecimal string representation of integer x.",
        .error_codes = "Error 13: Type Mismatch (HEX$ expects one numeric argument)"
    };
    microlib_register(&meta);
}
#include <string.h>

BValue func_hex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "HEX$ expects one numeric argument";
        return res;
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char tmp[20];
    snprintf(tmp, sizeof(tmp), "%lX", uv);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), tmp, strlen(tmp));
    return res;
}
