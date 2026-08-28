// FILENAME: hex.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (conversion_fn.c)
// NEEDS: libcore (funcreg.h, funcreg.c, hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (hex.h, string.c)
// Provides runtime implementation for the HEX built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/hex.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/funcreg.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static BValue func_hex_wang(VMContext *vm, BValue *args, int arg_count, BppError *err) {
    BValue res = { .type = VAL_STRING, .as.string = NULL };
    if (arg_count != 1) {
        err->code = 5; err->message = "HEX expects 1 string argument (hex digits)";
        return res;
    }

    if (args[0].type != VAL_STRING || !args[0].as.string) {
        err->code = 13; err->message = "Type mismatch: HEX expects string of hex digits";
        return res;
    }

    const char *hex_str = str_data(args[0].as.string);
    size_t hex_len = str_len(args[0].as.string);

    size_t out_len = hex_len / 2;
    char *out_buf = (char *)(hal_get() ? hal_get()->mem.alloc(out_len + 1) : NULL);
    if (!out_buf) {
        err->code = 7; err->message = "Out of memory in HEX decoding";
        return res;
    }

    size_t out_idx = 0;
    for (size_t i = 0; i + 1 < hex_len; i += 2) {
        int h1 = hex_char_to_val(hex_str[i]);
        int h2 = hex_char_to_val(hex_str[i + 1]);
        if (h1 < 0 || h2 < 0) {
            if (out_buf && hal_get()) hal_get()->mem.free(out_buf);
            err->code = 5; err->message = "Invalid hexadecimal digit in HEX";
            return res;
        }
        out_buf[out_idx++] = (char)((h1 << 4) | h2);
    }
    out_buf[out_idx] = '\0';

    res.as.string = str_create(vm_get_str(vm), out_buf, out_idx);
    if (out_buf && hal_get()) hal_get()->mem.free(out_buf);
    return res;
}

void func_hex_register(void) {
    static const MicroLibMetadata meta = {
        .name = "HEX$",
        .category = "String Functions",
        .syntax = "HEX$(x) | HEX(hex_str$)",
        .help_text = "HEX$(x) returns hexadecimal string of number x (GW-BASIC); HEX(s$) decodes hex string to raw binary byte string (Wang 3300).",
        .error_codes = "Error 13: Type Mismatch, Error 5: Illegal function call"
    };
    microlib_register(&meta);
}

BValue func_hex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 1) {
        err->code = 13;
        err->message = "HEX expects one argument";
        return res;
    }

    if (args[0].type == VAL_STRING) {
        return func_hex_wang(vm, args, arg_count, err);
    }

    unsigned long uv = (unsigned long)(long)args[0].as.number;
    char tmp[20];
    runtime_snprintf(tmp, sizeof(tmp), "%lX", uv);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), tmp, runtime_strlen(tmp));
    return res;
}
