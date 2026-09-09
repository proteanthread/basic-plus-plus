// FILENAME: right.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (right.h, string.c, vm.h)
// Provides runtime implementation for the RIGHT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/right.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_right_desc = {
    .name = "RIGHT$",
    .category = "String Functions",
    .syntax = "RIGHT$(str$, n)",
    .description = "Returns the rightmost n characters of str$.",
    .error_summary = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (RIGHT$ expects string and numeric arguments)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_right_register(void) {
    lang_desc_register(&g_right_desc);
}

BValue func_right_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "RIGHT$") != 0 && runtime_strcmp(uname, "RIGHT") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "RIGHT$ expects one string argument and one numeric argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int n = (int)args[1].as.number;

    const char *data = str_data(sr);
    size_t len = str_len(sr);
    size_t out_len = 0;
    size_t start_offset = 0;

    if (n < 0) {
        size_t skip = (size_t)(-n);
        start_offset = (skip >= len) ? len : skip;
        out_len = len - start_offset;
    } else {
        out_len = (size_t)n < len ? (size_t)n : len;
        start_offset = len - out_len;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), data + start_offset, out_len);
    str_release(vm_get_str(vm), sr);
    return res;
}
