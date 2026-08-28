// FILENAME: min.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, min.h, string.c)
// Provides runtime implementation for the MIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/min.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_min_register(void) {
    MicroLibMetadata meta = {
        .name = "MIN",
        .category = "Math Functions",
        .syntax = "MIN(val1, val2 [, ...]) or val1 MIN val2",
        .help_text = "Returns the minimum of two or more numeric values (supports dual prefix & infix notation).",
        .error_codes = "Error 13: Type Mismatch (MIN expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_min_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_MIN") != 0 && runtime_strcmp(uname, "MIN") != 0 && runtime_strcmp(uname, "MATH.MIN") != 0) {
        return res;
    }

    if (arg_count < 2) {
        err->code = 13;
        err->message = "MIN expects at least two numeric arguments";
        return res;
    }

    for (int i = 0; i < arg_count; i++) {
        if (args[i].type == VAL_STRING) {
            err->code = 13;
            err->message = "MIN expects numeric arguments";
            return res;
        }
    }

    double min_val = args[0].as.number;
    for (int i = 1; i < arg_count; i++) {
        if (args[i].as.number < min_val) {
            min_val = args[i].as.number;
        }
    }

    res.type = VAL_NUMBER;
    res.as.number = min_val;
    return res;
}
