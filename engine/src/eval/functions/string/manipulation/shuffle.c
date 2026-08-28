// FILENAME: shuffle.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (shuffle.h, string.c)
// Provides runtime implementation for the SHUFFLE built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/shuffle.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
#include <stdint.h>

static uint64_t g_shuffle_seed = 123456789ULL;

void func_shuffle_register(void) {
    MicroLibMetadata meta = {
        .name = "SHUFFLE$",
        .category = "String Functions",
        .syntax = "SHUFFLE$(str$ [, seed])",
        .help_text = "Returns a copy of str$ with all characters randomly scrambled using Fisher-Yates shuffle.",
        .error_codes = "Error 13: Type Mismatch (SHUFFLE$ expects string first argument, optional numeric seed)"
    };
    microlib_register(&meta);
}

BValue func_shuffle_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count < 1 || arg_count > 2 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "SHUFFLE$ expects string first argument, optional numeric seed";
        return res;
    }

    if (arg_count == 2 && args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "SHUFFLE$ seed argument must be numeric";
        return res;
    }

    // If explicit seed provided, reseed PRNG
    if (arg_count == 2) {
        g_shuffle_seed = (uint64_t)(uint32_t)args[1].as.number;
    }


    BppStringRef sr = args[0].as.string;
    size_t len = str_len(sr);
    if (len == 0) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        str_release(vm_get_str(vm), sr);
        return res;
    }

    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc((1) * (len + 1)) : NULL);
    if (!buf) {
        err->code = 14;
        err->message = "Out of memory";
        str_release(vm_get_str(vm), sr);
        return res;
    }

    const char *src = str_data(sr);
    runtime_memcpy(buf, src, len);
    buf[len] = '\0';

    // Fisher-Yates shuffle
    for (size_t i = len - 1; i > 0; i--) {
        size_t j = (size_t)((g_shuffle_seed = g_shuffle_seed * 6364136223846793005ULL + 1442695040888963407ULL), (int)((g_shuffle_seed >> 32) & 0x7FFFFFFF)) % (i + 1);
        char tmp = buf[i];
        buf[i] = buf[j];
        buf[j] = tmp;
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), buf, len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    str_release(vm_get_str(vm), sr);
    return res;
}
