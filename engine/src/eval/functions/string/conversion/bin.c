// FILENAME: bin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bin.h, string.c)
// Provides runtime implementation for the BIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/bin.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"

static const LangDesc g_bin_desc = {
    .name = "BIN$",
    .category = "String Functions",
    .syntax = "BIN$(x)",
    .description = "Returns the binary string representation of integer x.",
    .error_summary = "Error 13: Type Mismatch (BIN$ expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_bin_register(void) {
    lang_desc_register(&g_bin_desc);
}

BValue func_bin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "BIN$ expects one numeric argument";
        return res;
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char raw[68];
    int raw_bits = 0;
    if (uv == 0) {
        raw[raw_bits++] = '0';
    } else {
        while (uv > 0 && raw_bits < 64) {
            raw[raw_bits++] = (char)('0' + (int)(uv & 1));
            uv >>= 1;
        }
    }
    int num_bytes = (raw_bits + 7) / 8;
    int total_bits = num_bytes * 8;
    while (raw_bits < total_bits) {
        raw[raw_bits++] = '0';
    }
    char out[80];
    int o = 0;
    for (int idx = total_bits - 1; idx >= 0; idx--) {
        out[o++] = raw[idx];
        if (idx > 0 && (idx % 8) == 0) {
            out[o++] = ' ';
        }
    }
    out[o] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out, o);
    return res;
}
