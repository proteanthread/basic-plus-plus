/**
 * @file string.c
 * @brief STRING$ repeating character string generation function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements STRING$ built-in string function, generating a string consisting of `n` repeating characters specified either by ASCII code or by the 1st character of a target string.
 *
 * 2. WHY IT EXISTS:
 * Provides standard repeated character string generation matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates count `args[0]` and char specifier `args[1]` (numeric code or `VAL_STRING`). Extracts 1st byte of string if `args[1]` is a string (releasing `args[1].as.string`), allocates memory, fills buffer via `memset()`, creates a `BppString`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_string'. Includes "eval/functions/string/string.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support multi-character pattern repetition string generation.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on `args[1].as.string` when character specifier is a string. Negative count `n < 0` returning ERR_ILLEGAL_FUNCTION_CALL (error 5).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing n repeating characters or ERR_ILLEGAL_FUNCTION_CALL (error 5) or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcount release `str_release()` when `args[1]` is a string.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated count and character specifier BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Standard ISO C `memset`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/string.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/string.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdlib.h>
#include <string.h>

void func_string_register(void) {
    MicroLibMetadata meta = {
        .name = "STRING$",
        .category = "String Functions",
        .syntax = "STRING$(n, char_spec)",
        .help_text = "Returns a string of n repeating characters specified by ASCII code or 1st char of string.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (STRING$ argument type error)"
    };
    microlib_register(&meta);
}

BValue func_string_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 2 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "STRING$ expects a number and a character/string";
        return res;
    }

    int n = (int)args[0].as.number;
    if (n < 0) {
        err->code = 5;
        err->message = "Negative count in STRING$";
        return res;
    }

    char c = ' ';
    if (args[1].type == VAL_NUMBER) {
        c = (char)args[1].as.number;
    } else {
        BppStringRef sr = args[1].as.string;
        const char *s = str_data(sr);
        if (str_len(sr) > 0) c = s[0];
        str_release(vm_get_str(vm), sr);
    }

    char *buf = (char *)calloc(1, n + 1);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        return res;
    }

    memset(buf, c, n);
    buf[n] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, n);
    free(buf);
    return res;
}
