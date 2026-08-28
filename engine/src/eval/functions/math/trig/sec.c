// FILENAME: sec.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, sec.h, string.c)
// Provides runtime implementation for the SEC built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/sec.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_sec_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SEC",
        .category = "Math & Trigonometry",
        .syntax = "SEC(x)",
        .help_text = "Returns the secant of angle x in radians (1 / runtime_cos(x)).",
        .error_codes = "Error 11: Division by Zero (runtime_cos(x) == 0), Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_sec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "SEC") != 0 && runtime_strcmp(uname, "_SEC") != 0 && runtime_strcmp(uname, "MATH.SEC") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SEC expects one numeric argument";
        return res;
    }

    double c = runtime_cos(args[0].as.number);
    if (c == 0.0) {
        err->code = 11;
        err->message = "Division by zero in SEC";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = 1.0 / c;
    return res;
}
