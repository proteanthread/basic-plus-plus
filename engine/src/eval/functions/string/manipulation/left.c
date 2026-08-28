// FILENAME: left.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (left.h, string.c, vm.h)
// Provides runtime implementation for the LEFT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/manipulation/left.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "vm/vm.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_left_register(void) {
    MicroLibMetadata meta = {
        .name = "LEFT$",
        .category = "String Functions",
        .syntax = "LEFT$(str$, n)",
        .help_text = "Returns the leftmost n characters of str$.",
        .error_codes = "Error 5: Illegal Function Call (n < 0), Error 13: Type Mismatch (LEFT$ expects string and numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_left_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "LEFT$") != 0 && runtime_strcmp(uname, "LEFT") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type != VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "LEFT$ expects one string argument and one numeric argument";
        if (args[0].type == VAL_STRING && args[0].as.string) {
            str_release(vm_get_str(vm), args[0].as.string);
        }
        return res;
    }

    BppStringRef sr = args[0].as.string;
    int n = (int)args[1].as.number;

    if (n < 0) {
        err->code = 5;
        err->message = "Illegal function call in LEFT$";
        str_release(vm_get_str(vm), sr);
        return res;
    }

    const char *data = str_data(sr);
    size_t len = str_len(sr);
    size_t out_len = (size_t)n < len ? (size_t)n : len;

    res.type = VAL_STRING;
    res.as.string = str_create(vm_get_str(vm), data, out_len);
    str_release(vm_get_str(vm), sr);
    return res;
}
