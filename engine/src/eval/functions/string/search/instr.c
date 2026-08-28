// FILENAME: instr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (instr.h, string.c)
// Provides runtime implementation for the INSTR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/instr.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
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

    if (runtime_strcmp(uname, "INSTR") != 0 && runtime_strcmp(uname, "INSTR$") != 0) {
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
        const char *ptr = runtime_strstr(str1 + (start - 1), str2);
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
