// FILENAME: ups.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, ups.h)
// Provides runtime implementation for the UPS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/ups.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include "runtime/ctype/ctype.h"

static const LangDesc g_ups_desc = {
    .name = "UPS$",
    .category = "String Functions",
    .syntax = "UPS$(str_expr) | UPS(str_expr)",
    .description = "Converts all alphabetic characters in a string to uppercase (HP 3000 TSB).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_ups_register(void) {
    lang_desc_register(&g_ups_desc);
}

BValue func_ups_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 1) {
        err->code = ERR_ILLEGAL_FUNCTION_CALL;
        err->message = "Expected 1 string argument";
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "Type mismatch: expected string argument";
        return res;
    }

    size_t len = str_len(args[0].as.string);
    const char *data = str_data(args[0].as.string);

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(len + 1) : NULL);
    if (!buf) {
        err->code = ERR_OUT_OF_MEMORY;
        return res;
    }

    for (size_t i = 0; i < len; i++) {
        buf[i] = (char)runtime_toupper((unsigned char)data[i]);
    }
    buf[len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}
