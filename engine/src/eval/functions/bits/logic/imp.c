// FILENAME: imp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys_fn.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libengine (imp.h)
// Provides runtime implementation for the IMP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/bits/logic/imp.h"
#include "runtime/micro_lib_metadata.h"

void func_imp_register(void) {
    MicroLibMetadata meta = {
        .name = "IMP",
        .category = "Bitwise & Logical Functions",
        .syntax = "IMP(val1, val2) or val1 IMP val2",
        .help_text = "Performs bitwise and logical implication (NOT val1 OR val2) on integers or boolean values.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_imp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_IMP") != 0 && runtime_strcmp(uname, "IMP") != 0 && runtime_strcmp(uname, "BITS.IMP") != 0) {
        return res;
    }


    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "IMP expects two numeric arguments";
        return res;
    }

    int64_t a = (int64_t)args[0].as.number;
    int64_t b = (int64_t)args[1].as.number;

    res.type = VAL_NUMBER;
    res.as.number = (double)(~a | b);
    return res;
}
