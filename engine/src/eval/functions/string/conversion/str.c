// FILENAME: str.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (num_format.h, num_format.c, string.h, strings.h, strings.c)
// NEEDS: libengine (str.h, string.c, vm.h)
// Provides runtime implementation for the STR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/str.h"
#include "runtime/language_descriptor.h"
#include "runtime/num_format.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
#include "runtime/string/strops.h"

static const LangDesc g_str_desc = {
    .name = "STR$",
    .category = "String Functions",
    .syntax = "STR$(x)",
    .description = "Returns the string representation of numeric expression x.",
    .error_summary = "Error 13: Type Mismatch (STR$ expects one numeric argument)",
    .subsystem = SUBSYSTEM_ENGINE,
    .safety = SAFETY_IO,
    .type = FEATURE_FUNCTION
};
void func_str_register(void) {
    lang_desc_register(&g_str_desc);
}

BValue func_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "STR$") != 0 && runtime_strcmp(uname, "STR") != 0) {
        return res;
    }

    if ((arg_count != 1 && arg_count != 2) || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "STR$ expects numeric argument [, radix]";
        return res;
    }

    if (arg_count == 2 && args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "STR$ expects numeric radix";
        return res;
    }

    if (arg_count == 1) {
        char buf[64];
        num_format_display(buf, sizeof(buf), args[0].as.number, true, false);
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), buf, runtime_strlen(buf));
        return res;
    }

    // Custom radix conversion (base 2..36)
    int radix = (int)args[1].as.number;
    if (radix < 2 || radix > 36) {
        err->code = 5;
        err->message = "Illegal radix in STR$ (must be 2-36)";
        return res;
    }

    long long val = (long long)args[0].as.number;
    bool is_neg = false;
    if (val < 0) {
        is_neg = true;
        val = -val;
    }

    char tmp[66];
    int pos = 0;
    static const char digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (val == 0) {
        tmp[pos++] = '0';
    } else {
        while (val > 0 && pos < 64) {
            tmp[pos++] = digits[val % radix];
            val /= radix;
        }
    }

    if (is_neg && pos < 65) {
        tmp[pos++] = '-';
    }

    // Reverse into output buffer
    char out[66];
    for (int i = 0; i < pos; i++) {
        out[i] = tmp[pos - 1 - i];
    }
    out[pos] = '\0';

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), out, (size_t)pos);
    return res;
}
