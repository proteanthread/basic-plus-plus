// FILENAME: xlate.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, xlate.h)
// Provides runtime implementation for the XLATE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/xlate.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_xlate_register(void) {
    static const MicroLibMetadata meta = {
        .name = "XLATE$",
        .category = "String Functions",
        .syntax = "XLATE$(src_str, table_str)",
        .help_text = "Translates characters in src_str using character mapping table_str.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_xlate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)uname;

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type != VAL_STRING ||
        !args[0].as.string || !args[1].as.string) {
        err->code = ERR_TYPE_MISMATCH;
        err->message = "XLATE$ expects two string arguments";
        return res;
    }

    const char *src = str_data(args[0].as.string);
    size_t src_len = str_len(args[0].as.string);
    const char *tbl = str_data(args[1].as.string);
    size_t tbl_len = str_len(args[1].as.string);

    char *out_buf = (char *)(hal_get() ? hal_get()->mem.alloc(src_len + 1) : NULL);
    if (!out_buf) {
        err->code = ERR_OUT_OF_STRING_SPACE;
        return res;
    }

    for (size_t i = 0; i < src_len; ++i) {
        unsigned char ch = (unsigned char)src[i];
        if (ch < tbl_len && tbl[ch] != '\0') {
            out_buf[i] = tbl[ch];
        } else {
            out_buf[i] = src[i];
        }
    }
    out_buf[src_len] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, src_len);
    if (out_buf && hal_get()) hal_get()->mem.free(out_buf);
    return res;
}
