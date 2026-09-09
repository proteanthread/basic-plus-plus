// FILENAME: hta.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (common_reg_funcs.c, string_fn.c)
// NEEDS: libcore (hal.h, language_descriptor.h, strings.h)
// NEEDS: libengine (hta.h)
// Provides runtime implementation for the HTA$ function in BASIC++.

#include "eval/functions/string/conversion/hta.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

static const LangDesc g_hta_desc = {
    .name = "HTA$",
    .category = "String Functions",
    .syntax = "HTA$(str$ | num)",
    .description = "Converts ASCII characters or numeric values to Hex string representation (Basic Four / BBx).",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_PURE,
    .type = FEATURE_FUNCTION
};

void func_hta_register(void) {
    lang_desc_register(&g_hta_desc);
}

BValue func_hta_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)err;
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
