// FILENAME: rinstr.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c, strings.h, strings.c)
// NEEDS: libengine (rinstr.h, string.c, vm.h)
// Provides runtime implementation for the RINSTR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/search/rinstr.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "platform/platform.h"

static const LangDesc g_rinstr_desc = {
    .name = "RINSTR",
    .category = "String Functions",
    .syntax = "RINSTR([start,] str$, match$ [, mode])",
    .description = "Returns 1-based position of the last occurrence of match$ in str$ searching backwards.",
    .error_summary = "Error 13: Type Mismatch (expects string arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_rinstr_register(void) {
    lang_desc_register(&g_rinstr_desc);
}

BValue func_rinstr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RINSTR") != 0 && runtime_strcmp(uname, "RINSTR$") != 0) {
        return res;
    }

    int start = 0; // 0 means start from end of string
    int mode = 0;
    BppStringRef s1 = NULL;
    BppStringRef s2 = NULL;

    if (arg_count == 2) {
        if (args[0].type != VAL_STRING || args[1].type != VAL_STRING) {
            err->code = 13;
            err->message = "RINSTR expects (str$, match$)";
            return res;
        }
        s1 = args[0].as.string;
        s2 = args[1].as.string;
    } else if (arg_count == 3) {
        if (args[0].type == VAL_STRING && args[1].type == VAL_STRING && args[2].type != VAL_STRING) {
            s1 = args[0].as.string;
            s2 = args[1].as.string;
            mode = (int)args[2].as.number;
        } else if (args[0].type != VAL_STRING && args[1].type == VAL_STRING && args[2].type == VAL_STRING) {
            start = (int)args[0].as.number;
            s1 = args[1].as.string;
            s2 = args[2].as.string;
        } else {
            err->code = 13;
            err->message = "RINSTR expects (start, str$, match$) or (str$, match$, mode)";
            return res;
        }
    } else if (arg_count == 4) {
        if (args[0].type == VAL_STRING || args[1].type != VAL_STRING || args[2].type != VAL_STRING || args[3].type == VAL_STRING) {
            err->code = 13;
            err->message = "RINSTR expects (start, str$, match$, mode)";
            return res;
        }
        start = (int)args[0].as.number;
        s1 = args[1].as.string;
        s2 = args[2].as.string;
        mode = (int)args[3].as.number;
    } else {
        err->code = 13;
        err->message = "Invalid argument count for RINSTR";
        return res;
    }

    int len1 = s1 ? (int)str_len(s1) : 0;
    int len2 = s2 ? (int)str_len(s2) : 0;

    if (len1 == 0 || len2 == 0 || len2 > len1) {
        res.as.number = 0.0;
        return res;
    }

    int max_start = len1 - len2 + 1;
    int search_start = max_start;

    if (start > 0) {
        search_start = (start < max_start) ? start : max_start;
    } else if (start < 0) {
        int calc = len1 + start + 1;
        if (calc < 1) {
            res.as.number = 0.0;
            return res;
        }
        search_start = (calc < max_start) ? calc : max_start;
    }

    const char *str1 = str_data(s1);
    const char *str2 = str_data(s2);

    for (int i = search_start; i >= 1; i--) {
        size_t offset = (size_t)(i - 1);
        bool match = false;
        if (mode == 1) {
            match = (runtime_strncasecmp(str1 + offset, str2, (size_t)len2) == 0);
        } else {
            match = (runtime_memcmp(str1 + offset, str2, (size_t)len2) == 0);
        }
        if (match) {
            res.as.number = (double)i;
            return res;
        }
    }

    res.as.number = 0.0;
    return res;
}
