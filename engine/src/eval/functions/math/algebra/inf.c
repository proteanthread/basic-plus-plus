// FILENAME: inf.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (inf.h, math.c, string.c)
// Provides runtime implementation for the INF built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/inf.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_inf_register(void) {
    MicroLibMetadata meta = {
        .name = "INF",
        .category = "Math Functions",
        .syntax = "INF",
        .help_text = "Returns positive infinity value (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_inf_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "INF") != 0 && runtime_strcmp(uname, "_INF") != 0 && runtime_strcmp(uname, "MATH.INF") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 5;
        err->message = "INF expects zero arguments";
        return res;
    }

    res.as.number = RUNTIME_INFINITY;
    res.type = VAL_NUMBER;


    return res;
}
