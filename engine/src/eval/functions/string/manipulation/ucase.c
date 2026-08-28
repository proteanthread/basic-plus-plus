// FILENAME: ucase.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ucase.h)
// Provides runtime implementation for the UCASE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/ucase.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_ucase_register(void) {
    MicroLibMetadata meta = {
        .name = "UCASE$",
        .category = "String Functions",
        .syntax = "UCASE$(str$)",
        .help_text = "Returns a copy of str$ with all lowercase letters converted to uppercase.",
        .error_codes = "Error 13: Type Mismatch (UCASE$ expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_ucase_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "UCASE$ expects one string argument";
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
        buf[i] = (char)runtime_toupper((unsigned char)src[i]);
    }
    buf[len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    str_release(vm_get_str(vm), sr);
    return res;
}
