// FILENAME: ath.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (ath.h)
// Provides runtime implementation for the ATH$ function in BASIC++.

#include "eval/functions/string/conversion/ath.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "hal/hal.h"

static const LangDesc g_ath_desc = {
    .name = "ATH$",
    .category = "String Functions",
    .syntax = "ATH$(hex_str$)",
    .description = "Converts Hex string representation to raw ASCII byte string (Basic Four / BBx).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_ath_register(void) {
    lang_desc_register(&g_ath_desc);
}

static int hex_char_to_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return 0;
}

BValue func_ath_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
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
