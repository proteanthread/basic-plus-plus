// FILENAME: ath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (hal.h, memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (ath.h, string.c)
// Provides runtime implementation for the ATH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/ath.h"
#include "runtime/strings.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "hal/hal.h"
void func_ath_register(void) {
    static const MicroLibMetadata meta = {
        .name = "HTA$/ATH$",
        .category = "String Functions",
        .syntax = "HTA$(str$) / ATH$(hex_str$)",
        .help_text = "Converts ASCII characters to Hex string (HTA$) or Hex string to ASCII bytes (ATH$) (Basic Four / BBx).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_hta_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (runtime_strcmp(uname, "HTA") != 0 && runtime_strcmp(uname, "HTA$") != 0 &&
        runtime_strcmp(uname, "_HTA") != 0 && runtime_strcmp(uname, "_HTA$") != 0 &&
        runtime_strcmp(uname, "STRING.HTA") != 0 && runtime_strcmp(uname, "STRING.HTA$") != 0) {
        return res;
    }

    if (arg_count != 1) {
        err->code = 13;
        err->message = "HTA$ expects 1 argument";
        return res;
    }

    if (args[0].type == VAL_STRING) {
        const char *src = str_data(args[0].as.string);
        size_t slen = str_len(args[0].as.string);
        if (slen == 0 || !src) {
            res.as.string = str_create(vm_get_str(vm), "", 0);
            return res;
        }

        size_t out_len = slen * 2;
        char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(out_len + 1) : NULL);
        if (!buf) {
            err->code = 7; err->message = "Out of memory in HTA$";
            return res;
        }

        for (size_t i = 0; i < slen; ++i) {
            runtime_snprintf(buf + (i * 2), 3, "%02X", (unsigned char)src[i]);
        }
        buf[out_len] = '\0';
        res.as.string = str_create(vm_get_str(vm), buf, out_len);
        if (buf && hal_get()) hal_get()->mem.free(buf);
        return res;
    } else if (args[0].type == VAL_NUMBER) {
        unsigned long val = (unsigned long)args[0].as.number;
        char buf[32];
        runtime_snprintf(buf, sizeof(buf), "%lX", val);
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    } else {
        err->code = 13;
        err->message = "HTA$ expects string or numeric argument";
        return res;
    }
}

static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

BValue func_ath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_STRING;
    res.as.string = NULL;

    if (runtime_strcmp(uname, "ATH") != 0 && runtime_strcmp(uname, "ATH$") != 0 &&
        runtime_strcmp(uname, "_ATH") != 0 && runtime_strcmp(uname, "_ATH$") != 0 &&
        runtime_strcmp(uname, "STRING.ATH") != 0 && runtime_strcmp(uname, "STRING.ATH$") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "ATH$ expects 1 string argument";
        return res;
    }

    const char *hex = str_data(args[0].as.string);
    size_t hlen = str_len(args[0].as.string);
    if (hlen == 0 || !hex) {
        res.as.string = str_create(vm_get_str(vm), "", 0);
        return res;
    }

    size_t out_len = hlen / 2;
    char *buf = (char *)(hal_get() ? hal_get()->mem.alloc(out_len + 1) : NULL);
    if (!buf) {
        err->code = 7; err->message = "Out of memory in ATH$";
        return res;
    }

    for (size_t i = 0; i < out_len; ++i) {
        int hi = hex_char_to_val(hex[i * 2]);
        int lo = hex_char_to_val(hex[i * 2 + 1]);
        buf[i] = (char)((hi << 4) | lo);
    }
    buf[out_len] = '\0';

    res.as.string = str_create(vm_get_str(vm), buf, out_len);
    if (buf && hal_get()) hal_get()->mem.free(buf);
    return res;
}
