/**
 * @file environ.c
 * @brief ENVIRON$ environment variable function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements ENVIRON$ built-in system function, querying host environment variable `var_name$` and returning its string value (or empty string if not set).
 *
 * 2. WHY IT EXISTS:
 * Provides standard environment variable lookup matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single string argument `args[0]`, queries host environment via `platform_getenv()`, creates a `BppString`, releases the input string operand `str_release()`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_environ'. Includes "eval/functions/system/environ.h",
 * "platform/platform.h", "runtime/micro_lib_metadata.h", "runtime/strings.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support numeric index environment variable enumeration (e.g. ENVIRON$(1)).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string`.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing environment value string or empty string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify `platform_getenv()` behavior and input string refcount release `str_release()`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Input argument `args[0]` is a valid `VAL_STRING` BValue.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses platform abstraction `platform_getenv()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/platform/platform.c
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/system/environ.h
 * - engine/include/platform/platform.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/system/environ.h"
#include "platform/platform.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <string.h>

void func_environ_register(void) {
    MicroLibMetadata meta = {
        .name = "ENVIRON$",
        .category = "System Functions",
        .syntax = "ENVIRON$(var_name$)",
        .help_text = "Returns the value of host environment variable var_name$. Returns empty string if not set.",
        .error_codes = "Error 13: Type Mismatch (ENVIRON$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_environ_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "ENVIRON$ expects one string argument";
        return res;
    }

    char *val = platform_getenv(str_data(args[0].as.string));
    res.type = VAL_STRING;
    if (val) {
        res.as.string = str_create(vm_get_str(vm), val, strlen(val));
    } else {
        res.as.string = str_create(vm_get_str(vm), "", 0);
    }

    str_release(vm_get_str(vm), args[0].as.string);
    return res;
}
