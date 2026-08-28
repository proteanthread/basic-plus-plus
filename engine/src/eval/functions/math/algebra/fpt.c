// FILENAME: fpt.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (fpt.h, math.c, string.c)
// Provides runtime implementation for the FPT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/fpt.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_fpt_register(void) {
    static const MicroLibMetadata meta = {
        .name = "FPT/IPT",
        .category = "Math Functions",
        .syntax = "FPT(num) / IPT(num)",
        .help_text = "Returns the fractional part (FPT) or integer part (IPT) of a number (Business BASIC / BBx).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_fpt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "FPT") != 0 && runtime_strcmp(uname, "_FPT") != 0 &&
        runtime_strcmp(uname, "MATH.FPT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "FPT expects 1 numeric argument";
        return res;
    }

    double intpart;
    res.as.number = runtime_modf(args[0].as.number, &intpart);
    return res;
}

BValue func_ipt_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "IPT") != 0 && runtime_strcmp(uname, "_IPT") != 0 &&
        runtime_strcmp(uname, "MATH.IPT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type != VAL_NUMBER) {
        err->code = 13;
        err->message = "IPT expects 1 numeric argument";
        return res;
    }

    double intpart;
    runtime_modf(args[0].as.number, &intpart);
    res.as.number = intpart;
    return res;
}

