// FILENAME: mid.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (mid.h, string.c, vm.h)
// Provides runtime implementation for the MID built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/mid.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_mid_register(void) {
    MicroLibMetadata meta = {
        .name = "MID$",
        .category = "String Functions",
        .syntax = "MID$(str$, start [, length])",
        .help_text = "Returns a substring of str$ starting at 1-based index start for length characters.",
        .error_codes = "Error 5: Illegal Function Call (start <= 0 or length < 0), Error 13: Type Mismatch (MID$ argument type error)"
    };
    microlib_register(&meta);
}

BValue func_mid_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MID$") != 0 && runtime_strcmp(uname, "MID") != 0) {
        return res;
    }

    if ((arg_count != 2 && arg_count != 3) || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects string, start_index [, length]";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    if (arg_count == 3 && args[2].type == VAL_STRING) {
        err->code = 13;
        err->message = "MID$ expects numeric length argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int start = (int)args[1].as.number;
    const char *data = str_data(sr);
    size_t len = str_len(sr);

    if (start <= 0 || (size_t)start > len) {
        res.type = VAL_STRING;
        res.as.string = str_create(vm_get_str(vm), "", 0);
        str_release(vm_get_str(vm), sr);
        return res;
    }

    size_t start_idx = (size_t)(start - 1);
    size_t remaining = len - start_idx;
    size_t count = remaining;

    if (arg_count == 3) {
        int n = (int)args[2].as.number;
        if (n < 0) {
            err->code = 5;
            err->message = "Illegal function call in MID$";
            str_release(vm_get_str(vm), sr);
            return res;
        }
        if ((size_t)n < count) {
            count = (size_t)n;
        }
    }

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), data + start_idx, count);
    str_release(vm_get_str(vm), sr);
    return res;
}
