// FILENAME: bin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (bin.h, string.c)
// Provides runtime implementation for the BIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/bin.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_bin_register(void) {
    MicroLibMetadata meta = {
        .name = "BIN$",
        .category = "String Functions",
        .syntax = "BIN$(x)",
        .help_text = "Returns the binary string representation of integer x.",
        .error_codes = "Error 13: Type Mismatch (BIN$ expects one numeric argument)"
    };
    microlib_register(&meta);
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
