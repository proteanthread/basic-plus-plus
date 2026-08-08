/**
 * @file len.c
 * @brief LEN string length function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements LEN built-in function, returning the number of characters in string expression `str$`.
 *
 * 2. WHY IT EXISTS:
 * Provides standard string length query matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single string argument `args[0]`, measures length via `str_len(sr)`, releases input string `str_release()`, and returns `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_len'. Includes "eval/functions/string/len.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support multi-byte character length measuring vs raw byte count length.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string` before returning length value.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing non-negative double float length or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcount release `str_release(vm_get_str(vm), sr)`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Safe `size_t` to double float casting.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/len.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/len.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_len_register(void) {
    MicroLibMetadata meta = {
        .name = "LEN",
        .category = "String Functions",
        .syntax = "LEN(str$)",
        .help_text = "Returns the number of characters in str$.",
        .error_codes = "Error 13: Type Mismatch (LEN expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_len_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "LEN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "LEN expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    res.type = VAL_NUMBER;
    res.as.number = (double)str_len(sr);
    str_release(vm_get_str(vm), sr);
    return res;
}
