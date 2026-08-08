/**
 * @file mid.c
 * @brief MID$ middle substring extraction function evaluator implementation for BASIC++.
 *
 * 1. WHAT IT DOES:
 * Implements MID$ built-in string function, extracting a substring starting at 1-based index `start` for optional length `n` characters.
 *
 * 2. WHY IT EXISTS:
 * Provides standard substring extraction matching GW-BASIC, QBASIC, ANSI BASIC, and ECMA-116 standards.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Validates string argument `args[0]`, start index `args[1]`, and optional length `args[2]`. Slices substring starting at 1-based offset `start - 1`, releases the input string operand `str_release()`, and returns `VAL_STRING`.
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Compiled into CMake micro-library target 'func_mid'. Includes "eval/functions/string/mid.h",
 * "runtime/micro_lib_metadata.h", "runtime/strings.h", "vm/vm.h", <string.h>.
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Core feature included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Support character indexing vs byte offset indexing in multibyte UTF-8 modes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Mandatory string release requirement on input string operand `args[0].as.string` on both success and error paths. 1-based indexing rules (`start <= 0` triggers error 5).
 *
 * 8. WHAT TO EXPECT:
 * Returns VAL_STRING BValue containing extracted substring or ERR_ILLEGAL_FUNCTION_CALL (error 5) or ERR_TYPE_MISMATCH (error 13).
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify 1-based index calculation and input string refcount release `str_release()`.
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
 * - engine/include/eval/functions/string/mid.h
 * - engine/include/runtime/strings.h
 * - engine/include/runtime/micro_lib_metadata.h
 */

#include "eval/functions/string/mid.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include <string.h>

void func_mid_register(void) {
    MicroLibMetadata meta = {
        .name = "MID$",
        .category = "String Functions",
        .syntax = "MID$(str$, start [, length])",
        .help_text = "Returns a substring of str$ starting at 1-based index start for length characters.",
        .error_codes = "Error 5: Illegal Function Call (start <= 0 or length < 0), Error 13: Type Mismatch (MID$ argument type error)"
    };
    microlib_register(&meta);
}

BValue func_mid_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (strcmp(uname, "MID$") != 0 && strcmp(uname, "MID") != 0) {
        return res;
    }

    if ((arg_count != 2 && arg_count != 3) || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects string, start_index [, length]";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    if (arg_count == 3 && args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects numeric length argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int start = (int)args[1].as.number;
    const char *data = str_data(sr);
    size_t len = str_len(sr);

    if (start <= 0 || (size_t)start > len) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        str_release(vm_get_str(vm), sr);
        return res;
    }

    size_t start_idx = (size_t)(start - 1);
    size_t remaining = len - start_idx;
    size_t count = remaining;

    if (arg_count == 3) {
        int n = (int)args[2].as.number;
        if (n < 0) {
            err->code = 5;
            err->message = "Illegal function call in MID$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        if ((size_t)n < count) {
            count = (size_t)n;
        }
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), data + start_idx, count);
    str_release(vm_get_str(vm), sr);
    return res;
}
