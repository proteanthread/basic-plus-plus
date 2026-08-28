// FILENAME: atn.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (atn.h, math.c, string.c)
// Provides runtime implementation for the ATN built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/atn.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_atn_register(void) {
    MicroLibMetadata meta = {
        .name = "ATN",
        .category = "Math Functions",
        .syntax = "ATN(x)",
        .help_text = "Returns the arctangent of a numeric expression in radians.",
        .error_codes = "Error 13: Type Mismatch (ATN expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_atn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "ATN") != 0 && runtime_strcmp(uname, "ATAN") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "ATN expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_atan(args[0].as.number);
    return res;
}
