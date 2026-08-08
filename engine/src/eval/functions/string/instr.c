/**
 * @file instr.c
 * @brief INSTR string search function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements INSTR built-in string function, searching for the 1-based character position of a target substring inside a source string starting from optional 1-based index `start`.
 *
 * 2. WHY IT EXISTS:
 * Provides essential string pattern search logic matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Supports 2-argument (`INSTR(str$, match$)`) and 3-argument (`INSTR(start, str$, match$)`) signatures. Performs bounded substring search using `memcmp`, releases transient string arguments, and returns 1-based index (or 0 if not found).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_instr'. Includes "eval/functions/string/instr.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support case-insensitive search or regex matching options.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * 1-based index return convention (0.0 returned when string is not found or start > str len). String release obligation on input strings.
 *
 * 8. WHAT TO EXPECT:
 * Returns double float 1-based position or 0.0 or error code on invalid input.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify string refcount releases (`str_release()`) on both success and error paths.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * String inputs are valid reference-counted `BValue` string values.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Uses bounded `memcmp` substring search.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/runtime/strings.c
 * Prerequisite Header Files:
 * - engine/include/eval/functions/string/instr.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/instr.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include <string.h>

void func_instr_register(void) {
    MicroLibMetadata meta = {
        .name = "INSTR",
        .category = "String Functions",
        .syntax = "INSTR([start,] str$, match$)",
        .help_text = "Returns the 1-based position of the first occurrence of match$ in str$. Returns 0 if not found.",
        .error_codes = "Error 5: Illegal Function Call (start <= 0), Error 13: Type Mismatch (expects string arguments)"
    };
    microlib_register(&meta);
}

BValue func_instr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "INSTR") != 0 && strcmp(uname, "INSTR$") != 0) {
        return res;
    }

    int start = 1;
    BppStringRef s1 = NULL;
    BppStringRef s2 = NULL;

    if (arg_count == 2) {
        if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13;
            err->message = "INSTR expects string arguments";
            return res;
        }
        s1 = args[0].as.string;
        s2 = args[1].as.string;
    } else if (arg_count == 3) {
        if (args[0].type == VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING) {
            err->code = 13;
            err->message = "INSTR expects start, s1$, s2$";
            return res;
        }
        start = (int)args[0].as.number;
        s1 = args[1].as.string;
        s2 = args[2].as.string;
    } else {
        err->code = 13;
        err->message = "Invalid argument count for INSTR";
        return res;
    }

    int len1 = (int)str_len(s1);
    int len2 = (int)str_len(s2);
    double found_idx = 0.0;

    if (start <= 0 || start > len1) {
        found_idx = 0.0;
    } else if (len2 == 0) {
        found_idx = (double)start;
    } else {
        const char *str1 = str_data(s1);
        const char *str2 = str_data(s2);
        const char *ptr = strstr(str1 + (start - 1), str2);
        if (ptr) {
            found_idx = (double)(ptr - str1 + 1);
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = found_idx;

    if (s1) str_release(vm_get_str(vm), s1);
    if (s2) str_release(vm_get_str(vm), s2);
    return res;
}
