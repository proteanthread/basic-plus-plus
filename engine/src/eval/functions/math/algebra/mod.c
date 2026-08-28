// FILENAME: mod.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, mod.h, string.c)
// Provides runtime implementation for the MOD built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/mod.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_mod_register(void) {
    MicroLibMetadata meta = {
        .name = "MOD",
        .category = "Math Functions",
        .syntax = "MOD(val1, val2) or val1 MOD val2",
        .help_text = "Returns the integer remainder of val1 divided by val2 (supports dual prefix & infix notation).",
        .error_codes = "Error 11: Division by zero, Error 13: Type Mismatch (MOD expects numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_mod_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_MOD") != 0 && runtime_strcmp(uname, "MOD") != 0 && runtime_strcmp(uname, "MATH.MOD") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "MOD expects two numeric arguments";
        return res;
    }

    int64_t b = (int64_t)args[1].as.number;
    if (b == 0) {
        err->code = 11;
        err->message = "Division by zero in MOD";
        return res;
    }

    int64_t a = (int64_t)args[0].as.number;
    res.type = VAL_NUMBER;
    res.as.number = (double)(a % b);
    return res;
}
