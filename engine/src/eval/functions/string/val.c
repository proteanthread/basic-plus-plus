/**
 * @file val.c
 * @brief VAL string-to-number parsing function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements VAL built-in function, parsing a numeric double float value from string expression `str$`.
 *
 * 2. WHY IT EXISTS:
 * Provides standard string-to-number parsing matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single string argument `args[0]`, parses string value using `strtod()`, releases the input string operand `str_release()`, and returns `VAL_NUMBER`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_val'. Includes "eval/functions/string/val.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support RADIX prefix parsing (`&H`, `&O`, `&B`).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string` before returning numeric value. Returns 0.0 for non-numeric strings without throwing runtime errors.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_NUMBER BValue containing parsed double float value or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcount release `str_release(vm_get_str(vm), sr)` and `strtod` end pointer handling.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Arguments array contains evaluated BValue elements.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. ISO C `strtod()` numeric conversion.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/val.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/val.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <stdlib.h>
#include <string.h>

void func_val_register(void) {
    MicroLibMetadata meta = {
        .name = "VAL",
        .category = "String Functions",
        .syntax = "VAL(str$)",
        .help_text = "Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number.",
        .error_codes = "Error 13: Type Mismatch (VAL expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_val_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "VAL") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "VAL expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    res.type = VAL_NUMBER;
    res.as.number = strtod(str_data(sr), NULL);
    str_release(vm_get_str(vm), sr);
    return res;
}
