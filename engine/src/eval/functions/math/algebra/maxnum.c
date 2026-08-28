// FILENAME: maxnum.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (math.c, maxnum.h, string.c)
// Provides runtime implementation for the MAXNUM built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/maxnum.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_maxnum_register(void) {
    MicroLibMetadata meta = {
        .name = "MAXNUM",
        .category = "Math Functions",
        .syntax = "MAXNUM",
        .help_text = "Returns maximum positive representable finite number (ANSI Full BASIC 1987).",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_maxnum_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    (void)args;
    (void)err;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "MAXNUM") != 0 && runtime_strcmp(uname, "_MAXNUM") != 0 && runtime_strcmp(uname, "MATH.MAXNUM") != 0) {
        return res;
    }

    if (arg_count != 0) {
        err->code = 5;
        err->message = "MAXNUM expects zero arguments";
        return res;
    }

    res.as.number = 1.7976931348623157e+308;
    res.type = VAL_NUMBER;

    return res;
}
