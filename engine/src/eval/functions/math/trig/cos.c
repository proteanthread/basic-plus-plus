// FILENAME: cos.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (cos.h, math.c, string.c)
// Provides runtime implementation for the COS built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/trig/cos.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_cos_register(void) {
    MicroLibMetadata meta = {
        .name = "COS",
        .category = "Math Functions",
        .syntax = "COS(radians)",
        .help_text = "Returns the trigonometric cosine of an angle given in radians.",
        .error_codes = "Error 13: Type Mismatch (COS expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_cos_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "COS") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "COS expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_cos(args[0].as.number);
    return res;
}
