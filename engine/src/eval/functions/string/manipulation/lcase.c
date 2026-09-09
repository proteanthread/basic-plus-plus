// FILENAME: lcase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (lcase.h, string.c)
// Provides runtime implementation for the LCASE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/lcase.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_lcase_desc = {
    .name = "LCASE$",
    .category = "String Functions",
    .syntax = "LCASE$(str$)",
    .description = "Returns a copy of str$ with all uppercase letters converted to lowercase.",
    .error_summary = "Error 13: Type Mismatch (LCASE$ expects one string argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_lcase_register(void) {
    lang_desc_register(&g_lcase_desc);
}

BValue func_lcase_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "LCASE$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    size_t len = str_len(sr);
    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (len + 1)) : NULL);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        str_release(vm_get_str(vm), sr);
        return res;
    }

    const char *src = str_data(sr);
    for (size_t i = 0; i < len; i++) {
        buf[i] = (char)runtime_tolower((unsigned char)src[i]);
    }
    buf[len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    str_release(vm_get_str(vm), sr);
    return res;
}
