// FILENAME: instr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (instr.h, string.c)
// Provides runtime implementation for the INSTR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/instr.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_instr_desc = {
    .name = "INSTR",
    .category = "String Functions",
    .syntax = "INSTR([start,] str$, match$ [, mode])",
    .description = "Returns the 1-based position of match$ in str$ (supports negative start for reverse search).",
    .error_summary = "Error 13: Type Mismatch (expects string arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_instr_register(void) {
    lang_desc_register(&g_instr_desc);
}

BValue func_instr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "INSTR") != 0 && runtime_strcmp(uname, "INSTR$") != 0) {
        return res;
    }

    int start = 1;
    int mode = 0;
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
    } else if (arg_count == 4) {
        if (args[0].type == VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13;
            err->message = "INSTR expects start, s1$, s2$, mode";
            return res;
        }
        start = (int)args[0].as.number;
        s1 = args[1].as.string;
        s2 = args[2].as.string;
        mode = (int)args[3].as.number;
    } else {
        err->code = 13;
        err->message = "Invalid argument count for INSTR";
        return res;
    }

    int len1 = s1 ? (int)str_len(s1) : 0;
    int len2 = s2 ? (int)str_len(s2) : 0;
    double found_idx = 0.0;

    if (start < 0) {
        int calc = len1 + start + 1;
        if (calc >= 1 && calc <= len1 && len2 > 0 && len2 <= len1) {
            int max_start = len1 - len2 + 1;
            int s_pos = (calc < max_start) ? calc : max_start;
            const char *str1 = str_data(s1);
            const char *str2 = str_data(s2);
            for (int i = s_pos; i >= 1; i--) {
                size_t offset = (size_t)(i - 1);
                bool match = false;
                if (mode == 1) {
                    match = (runtime_strncasecmp(str1 + offset, str2, (size_t)len2) == 0);
                } else {
                    match = (runtime_memcmp(str1 + offset, str2, (size_t)len2) == 0);
                }
                if (match) {
                    found_idx = (double)i;
                    break;
                }
            }
        }
    } else if (start <= 0 || start > len1) {
        found_idx = 0.0;
    } else if (len2 == 0) {
        found_idx = (double)start;
    } else {
        const char *str1 = str_data(s1);
        const char *str2 = str_data(s2);
        const char *ptr = NULL;
        if (mode == 1) {
            ptr = runtime_strcasestr(str1 + (start - 1), str2);
        } else {
            ptr = runtime_strstr(str1 + (start - 1), str2);
        }
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
