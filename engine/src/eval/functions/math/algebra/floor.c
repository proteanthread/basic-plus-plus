// FILENAME: floor.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (floor.h, math.c, string.c)
// Provides runtime implementation for the FLOOR built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/floor.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_floor_register(void) {
    MicroLibMetadata meta = {
        .name = "FLOOR",
        .category = "Math Functions",
        .syntax = "FLOOR(x)",
        .help_text = "Returns the largest integer less than or equal to x.",
        .error_codes = "Error 13: Type Mismatch (FLOOR expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_floor_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "_FLOOR") != 0 && runtime_strcmp(uname, "FLOOR") != 0 && runtime_strcmp(uname, "MATH.FLOOR") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "FLOOR expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_floor(args[0].as.number);
    return res;
}
