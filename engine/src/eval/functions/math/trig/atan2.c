// FILENAME: atan2.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (atan2.h, math.c, string.c)
// Provides runtime implementation for the ATAN2 built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/atan2.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_atan2_register(void) {
    MicroLibMetadata meta = {
        .name = "ATAN2",
        .category = "Math Functions",
        .syntax = "ATAN2(y, x)",
        .help_text = "Returns the 2-argument arctangent of y and x in radians.",
        .error_codes = "Error 13: Type Mismatch (ATAN2 expects two numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_atan2_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_ATAN2") != 0 && runtime_strcmp(uname, "ATAN2") != 0 && runtime_strcmp(uname, "MATH.ATAN2") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "ATAN2 expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_atan2(args[0].as.number, args[1].as.number);
    return res;
}
