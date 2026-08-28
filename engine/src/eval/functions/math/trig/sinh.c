// FILENAME: sinh.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, sinh.h, string.c)
// Provides runtime implementation for the SINH built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/sinh.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_sinh_register(void) {
    static const MicroLibMetadata meta = {
        .name = "SINH",
        .category = "Math & Trigonometry",
        .syntax = "SINH(x) | HSN(x)",
        .help_text = "Returns the hyperbolic sine of x.",
        .error_codes = "Error 13: Type Mismatch (SINH expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_sinh_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "SINH") != 0 && runtime_strcmp(uname, "HSN") != 0 &&
        runtime_strcmp(uname, "_SINH") != 0 && runtime_strcmp(uname, "MATH.SINH") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SINH expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_sinh(args[0].as.number);
    return res;
}
