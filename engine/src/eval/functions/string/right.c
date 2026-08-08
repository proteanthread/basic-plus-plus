/**
 * @file right.c
 * @brief RIGHT$ rightmost substring extraction function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements RIGHT$ built-in string function, extracting the rightmost `n` characters of a source string `str$`.
 *
 * 2. WHY IT EXISTS:
 * Provides standard rightmost substring extraction matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates string argument `args[0]` and numeric length argument `args[1]`. Computes starting index `len - n`, slices substring into a new `BppString`, releases the input string operand `str_release()`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_right'. Includes "eval/functions/string/right.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support multibyte character slicing vs byte counting.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string` on both success and error paths.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing rightmost n characters or ERR_ILLEGAL_FUNCTION_CALL (error 5) or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify rightmost offset calculation `offset = len - n` and input string refcount release `str_release()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Bounded substring memory copy using `memcpy`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/right.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/right.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_right_register(void) {
    MicroLibMetadata meta = {
        .name = "RIGHT$",
        .category = "String Functions",
        .syntax = "RIGHT$(str$, n)",
        .help_text = "Returns the rightmost n characters of str$.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (RIGHT$ expects string and numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_right_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "RIGHT$") != 0 && strcmp(uname, "RIGHT") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "RIGHT$ expects one string argument and one numeric argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int n = (int)args[1].as.number;

    if (n < 0) {
        err->code = 5;
        err->message = "Illegal function call in RIGHT$";
        str_release(vm_get_str(vm), sr);
        return res;
    }

    const char *data = str_data(sr);
    size_t len = str_len(sr);
    size_t out_len = (size_t)n < len ? (size_t)n : len;
    size_t start_offset = len - out_len;

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), data + start_offset, out_len);
    str_release(vm_get_str(vm), sr);
    return res;
}
