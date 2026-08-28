// FILENAME: int.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (int.h, math.c, string.c)
// Provides runtime implementation for the INT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/int.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_int_register(void) {
    MicroLibMetadata meta = {
        .name = "INT",
        .category = "Math Functions",
        .syntax = "INT(x)",
        .help_text = "Returns the largest integer less than or equal to x (floor conversion).",
        .error_codes = "Error 13: Type Mismatch (INT expects one numeric argument)"
    };
    microlib_register(&meta);
}

BValue func_int_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "INT") != 0) {
        return res;
    }

    if (arg_count != 1 || args[0].type == VAL_STRING) {
        err->code = 13;
        err->message = "INT expects one numeric argument";
        return res;
    }

    res.type = VAL_NUMBER;
    res.as.number = runtime_floor(args[0].as.number);
    return res;
}
