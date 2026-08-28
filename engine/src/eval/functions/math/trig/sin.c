// FILENAME: sin.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, sin.h, string.c)
// Provides runtime implementation for the SIN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/sin.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_sin_register(void) {
    MicroLibMetadata meta = {
        .name = "SIN",
        .category = "Math Functions",
        .syntax = "SIN(radians)",
        .help_text = "Returns the trigonometric sine of an angle given in radians.",
        .error_codes = "Error 13: Type Mismatch (SIN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_sin_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "SIN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "SIN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_sin(args[0].as.number);
    return res;
}
