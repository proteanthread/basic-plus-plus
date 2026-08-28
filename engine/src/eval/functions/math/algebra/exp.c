// FILENAME: exp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (exp.h, math.c, string.c)
// Provides runtime implementation for the EXP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/exp.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_exp_register(void) {
    MicroLibMetadata meta = {
        .name = "EXP",
        .category = "Math Functions",
        .syntax = "EXP(x)",
        .help_text = "Returns e raised to the power of a numeric expression x.",
        .error_codes = "Error 6: Overflow (EXP exponent too large), Error 13: Type Mismatch (EXP expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_exp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "EXP") != 0 && runtime_strcmp(uname, "EXN") != 0 &&
        runtime_strcmp(uname, "_EXP") != 0 && runtime_strcmp(uname, "MATH.EXP") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "EXP expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_exp(args[0].as.number);
    return res;
}
