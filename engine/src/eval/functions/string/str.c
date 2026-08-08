/**
 * @file str.c
 * @brief STR$ number-to-string conversion function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements STR$ built-in string function, formatting a numeric expression value into a standard string representation.
 *
 * 2. WHY IT EXISTS:
 * Provides standard number-to-string conversion matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards (including leading space for positive numbers).
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single numeric argument `args[0]`, formats number using `num_format_display()`, creates a new `BppString`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_str'. Includes "eval/functions/string/str.h",
 * "runtime/micro_lib_metadata.h", "runtime/num_format.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support culture-aware decimal separator formatting (comma vs dot).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Standard GW-BASIC/QBASIC leading space formatting convention for non-negative numbers.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing formatted number string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `num_format_display()` buffer formatting in runtime/num_format.c.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Single numeric argument passed in `args[0]`.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. 15-digit precision display formatting.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/num_format.c
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/str.h
 * - engine/include/runtime/num_format.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/str.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_str_register(void) {
    MicroLibMetadata meta = {
        .name = "STR$",
        .category = "String Functions",
        .syntax = "STR$(x)",
        .help_text = "Returns the string representation of numeric expression x.",
        .error_codes = "Error 13: Type Mismatch (STR$ expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "STR$") != 0 && strcmp(uname, "STR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "STR$ expects one numeric argument";
        return res;
    }

    char buf[64];
    num_format_display(buf, sizeof(buf), args[0].as.number, true, false);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, strlen(buf));
    return res;
}
