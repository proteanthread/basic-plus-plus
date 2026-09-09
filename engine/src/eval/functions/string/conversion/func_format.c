// FILENAME: func_format.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (func_format.h, vm.h)
// Provides runtime implementation for the FORMAT$ string formatting function in BASIC++.

#include "eval/functions/string/conversion/func_format.h"
#include "runtime/language_descriptor.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/using.h"
#include "runtime/format/snprintf.h"

static const LangDesc g_func_format_desc = {
    .name = "FORMAT$",
    .category = "String Functions",
    .syntax = "FORMAT$(val, mask$)",
    .description = "Formats a numeric or string value using a USING mask or format string into a new string.",
    .error_summary = "Error 13: Type Mismatch",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};

void func_format_register(void) {
    lang_desc_register(&g_func_format_desc);
}

BValue func_format_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (arg_count != 2 || args[1].type != VAL_STRING) {
        err->code = 13;
        err->message = "FORMAT$ expects (val, mask$)";
        for (int i = 0; i < arg_count; ++i) {
            if (args[i].type == VAL_STRING && args[i].as.string) {
                str_release(vm_get_str(vm), args[i].as.string);
            }
        }
        return res;
    }

    BppStringRef sr_mask = args[1].as.string;
    const char *mask_str = str_data(sr_mask);
    char out_buf[512];
    runtime_memset(out_buf, 0, sizeof(out_buf));

    if (mask_str[0] == '%') {
        // C-style format string support (e.g. %8.2f, %04X, %b)
        if (args[0].type == VAL_STRING && args[0].as.string) {
            runtime_snprintf(out_buf, sizeof(out_buf), mask_str, str_data(args[0].as.string));
        } else {
            double n = args[0].as.number;
            if (mask_str[runtime_strlen(mask_str) - 1] == 'X' || mask_str[runtime_strlen(mask_str) - 1] == 'x' ||
                mask_str[runtime_strlen(mask_str) - 1] == 'd' || mask_str[runtime_strlen(mask_str) - 1] == 'i' ||
                mask_str[runtime_strlen(mask_str) - 1] == 'u') {
                runtime_snprintf(out_buf, sizeof(out_buf), mask_str, (long long)n);
            } else {
                runtime_snprintf(out_buf, sizeof(out_buf), mask_str, n);
            }
        }
    } else {
        // Classic USING mask engine
        UsingMask umask;
        using_parse_mask(mask_str, &umask);
        int mask_idx = 0;
        using_format_output(vm, &umask, &mask_idx, args[0], out_buf, sizeof(out_buf));
    }

    if (args[0].type == VAL_STRING && args[0].as.string) {
        str_release(vm_get_str(vm), args[0].as.string);
    }
    str_release(vm_get_str(vm), sr_mask);

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out_buf, runtime_strlen(out_buf));
    return res;
}
