// FILENAME: trim.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, trim.h)
// Provides runtime implementation for the TRIM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/trim.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_trim_register(void) {
    MicroLibMetadata meta = {
        .name = "TRIM$",
        .category = "String Functions",
        .syntax = "TRIM$(str$)",
        .help_text = "Returns a copy of str$ with both leading and trailing whitespace removed.",
        .error_codes = "Error 13: Type Mismatch (TRIM$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_trim_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "TRIM$ expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    const char *src = str_data(sr);
    size_t len = str_len(sr);
    size_t start = 0;
    while (start < len && runtime_isspace((unsigned char)src[start])) {
        start++;
    }
    size_t end = len;
    while (end > start && runtime_isspace((unsigned char)src[end - 1])) {
        end--;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), src + start, end - start);
    str_release(vm_get_str(vm), sr);
    return res;
}
