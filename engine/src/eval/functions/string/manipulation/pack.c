// FILENAME: pack.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mux.h, mux.c, pack.h, string.c)
// Provides runtime implementation for the PACK built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/pack.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/mux.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_pack_register(void) {
    MicroLibMetadata meta = {
        .name = "PACK$",
        .category = "String Functions",
        .syntax = "PACK$(fmt$, val1 [, val2, ...])",
        .help_text = "Packs binary data values into a binary string according to format template fmt$.",
        .error_codes = "Error 5: Illegal Function Call (invalid format character), Error 13: Type Mismatch (expects string format)"
    };
    microlib_register(&meta);
}

BValue func_pack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count == 0) {
        err->code = 13;
        err->message = "PACK$ requires at least one argument";
        return res;
    }

    size_t out_len = 0;
    char *buf = pack_fields(args, (size_t)arg_count, &out_len);
    if (!buf) {
        err->code = 5;
        err->message = "PACK$ field serialization failed";
        return res;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, out_len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}
