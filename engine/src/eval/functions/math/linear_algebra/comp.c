// FILENAME: comp.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (comp.h, math.c, string.c)
// Provides runtime implementation for the COMP built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/comp.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_comp_register(void) {
    static const MicroLibMetadata meta = {
        .name = "COMP",
        .category = "Math Functions",
        .syntax = "COMP(a, b)",
        .help_text = "Compares two numeric expressions a and b; returns -1 if a < b, 0 if a == b, and 1 if a > b.",
        .error_codes = "Error 13: Type Mismatch (COMP expects two numeric arguments)"
    };
    microlib_register(&meta);
}

BValue func_comp_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)vm;
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;

    if (runtime_strcmp(uname, "COMP") != 0 && runtime_strcmp(uname, "_COMP") != 0 && runtime_strcmp(uname, "MATH.COMP") != 0) {
        return res;
    }

    if (arg_count != 2 || args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
        err->code = 13;
        err->message = "COMP expects two numeric arguments";
        return res;
    }

    res.type = VAL_NUMBER;
    double a = args[0].as.number;
    double b = args[1].as.number;
    if (a < b) {
        res.as.number = -1.0;
    } else if (a > b) {
        res.as.number = 1.0;
    } else {
        res.as.number = 0.0;
    }
    return res;
}
