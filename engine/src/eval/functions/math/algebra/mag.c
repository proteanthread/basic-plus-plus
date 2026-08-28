// FILENAME: mag.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (math.h, micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (mag.h, math.c, string.c)
// Provides runtime implementation for the MAG built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/algebra/mag.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_mag_register(void) {
    static const MicroLibMetadata meta = {
        .name = "MAG",
        .category = "Math Functions",
        .syntax = "MAG(x, y) | MAG(complex_z) | MAG(x)",
        .help_text = "Returns the vector magnitude (hypotenuse) or absolute magnitude/modulus.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_mag_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    BValue res;
    res.type = VAL_NONE;
    res.as.number = 0.0;
    (void)vm; (void)uname;

    if (arg_count == 1) {
        if (args[0].type == VAL_COMPLEX) {
            res.type = VAL_NUMBER;
            res.as.number = runtime_hypot(args[0].as.complex_val.real, args[0].as.complex_val.imag);
            return res;
        }
        if (args[0].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = runtime_fabs(args[0].as.number);
        return res;
    }

    if (arg_count == 2) {
        if (args[0].type == VAL_STRING || args[1].type == VAL_STRING) {
            err->code = ERR_TYPE_MISMATCH;
            return res;
        }
        res.type = VAL_NUMBER;
        res.as.number = runtime_hypot(args[0].as.number, args[1].as.number);
        return res;
    }

    err->code = ERR_ILLEGAL_FUNCTION_CALL;
    err->message = "MAG expects 1 or 2 arguments";
    return res;
}
