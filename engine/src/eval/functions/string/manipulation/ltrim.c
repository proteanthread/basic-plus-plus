// FILENAME: ltrim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ltrim.h, string.c)
// Provides runtime implementation for the LTRIM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/ltrim.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_ltrim_register(void) {
    MicroLibMetadata meta = {
        .name = "LTRIM$",
        .category = "String Functions",
        .syntax = "LTRIM$(str$)",
        .help_text = "Returns a copy of str$ with leading whitespace removed.",
        .error_codes = "Error 13: Type Mismatch (LTRIM$ expects one string argument)"
    };
    microlib_register(&meta);
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
