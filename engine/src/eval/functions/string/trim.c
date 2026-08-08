/**
 * @file trim.c
 * @brief TRIM$ whitespace trimming function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements TRIM$ built-in string function, returning a copy of input string `str$` with both leading and trailing whitespace characters removed.
 *
 * 2. WHY IT EXISTS:
 * Provides standard whitespace trimming matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single string argument, advances start pointer past leading `isspace()` characters, scans backwards past trailing `isspace()` characters, constructs a new `BppString`, and releases the input string operand `str_release()`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_trim'. Includes "eval/functions/string/trim.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <ctype.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support custom trim character set parameter (e.g. TRIM$(str$, " \t\r\n")).
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string`.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing trimmed string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify start pointer advancement and backward scan bounds logic and string refcount release `str_release(vm_get_str(vm), sr)`.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Input argument `args[0]` is a valid `VAL_STRING` BValue.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Standard ISO C `<ctype.h>` `isspace()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/trim.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/trim.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <ctype.h>
#include <string.h>

void func_trim_register(void) {
    MicroLibMetadata meta = {
        .name = "TRIM$",
        .category = "String Functions",
        .syntax = "TRIM$(str$)",
        .help_text = "Returns a copy of str$ with both leading and trailing whitespace removed.",
        .error_codes = "Error 13: Type Mismatch (TRIM$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_trim_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "TRIM$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    const char *src = str_data(sr);
    size_t len = str_len(sr);
    size_t start = 0;
    while (start < len && isspace((unsigned char)src[start])) {
        start++;
    }
    size_t end = len;
    while (end > start && isspace((unsigned char)src[end - 1])) {
        end--;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), src + start, end - start);
    str_release(vm_get_str(vm), sr);
    return res;
}
