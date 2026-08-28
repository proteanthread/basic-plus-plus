// FILENAME: val.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libcore (memory.h, memory.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libcore (strings.h, strings.c)
// NEEDS: libengine (string.c, val.h)
// Provides runtime implementation for the VAL built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/string/conversion/val.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/string.h"
#include "runtime/memory.h"
void func_val_register(void) {
    MicroLibMetadata meta = {
        .name = "VAL",
        .category = "String Functions",
        .syntax = "VAL(str$)",
        .help_text = "Returns the numeric value represented by string str$. Returns 0 if str$ is not a valid number.",
        .error_codes = "Error 13: Type Mismatch (VAL expects one string argument)"
    };
    microlib_register(&meta);
}

BValue func_val_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "VAL") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_STRING) {
        err->code = 13;
        err->message = "VAL expects one string argument";
        return res;
    }

    BppStringRef sr = args[0].as.string;
    res.type = VAL_NUMBER;
    res.as.number = runtime_strtod(str_data(sr), NULL);
    str_release(vm_get_str(vm), sr);
    return res;
}
