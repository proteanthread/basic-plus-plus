/**
 * @file ucase.c
 * @brief UCASE$ uppercase conversion function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements UCASE$ built-in string function, returning a copy of input string `str$` with all lowercase ASCII letters converted to uppercase.
 *
 * 2. WHY IT EXISTS:
 * Provides standard uppercase conversion matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates single string argument, iterates through characters applying `toupper()`, allocates a new `BppString`, and releases the input string operand `str_release()`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_ucase'. Includes "eval/functions/string/ucase.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <ctype.h>, <stdlib.h>, <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support Unicode UTF-8 uppercase transformations.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string`.
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing uppercase string or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check string reference release and `calloc`/`malloc` buffer allocation.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Input argument `args[0]` is a valid `VAL_STRING` BValue.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Standard ISO C `<ctype.h>` `toupper()`.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/ucase.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/ucase.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

void func_ucase_register(void) {
    MicroLibMetadata meta = {
        .name = "UCASE$",
        .category = "String Functions",
        .syntax = "UCASE$(str$)",
        .help_text = "Returns a copy of str$ with all lowercase letters converted to uppercase.",
        .error_codes = "Error 13: Type Mismatch (UCASE$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_ucase_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "UCASE$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    size_t len = str_len(sr);
    char *buf = (char *)calloc(1, len + 1);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        str_release(vm_get_str(vm), sr);
        return res;
    }

    const char *src = str_data(sr);
    for (size_t i = 0; i < len; i++) {
        buf[i] = (char)toupper((unsigned char)src[i]);
    }
    buf[len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, len);
    free(buf);
    str_release(vm_get_str(vm), sr);
    return res;
}
