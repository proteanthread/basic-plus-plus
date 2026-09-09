// FILENAME: ltrim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ltrim.h, string.c)
// Provides runtime implementation for the LTRIM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/ltrim.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_ltrim_desc = {
    .name = "LTRIM$",
    .category = "String Functions",
    .syntax = "LTRIM$(str$)",
    .description = "Returns a copy of str$ with leading whitespace removed.",
    .error_summary = "Error 13: Type Mismatch (LTRIM$ expects one string argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_ltrim_register(void) {
    lang_desc_register(&g_ltrim_desc);
}

BValue func_ltrim_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "LTRIM$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    const char *src = str_data(sr);
    size_t len = str_len(sr);
    size_t start = 0;
    while (start < len && runtime_isspace((unsigned char)src[start])) {
        start++;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), src + start, len - start);
    str_release(vm_get_str(vm), sr);
    return res;
}
