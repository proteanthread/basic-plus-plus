// FILENAME: dot.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (dot.h, math.c, string.c, vm.h)
// Provides runtime implementation for the DOT built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/dot.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/arrays.h"
#include "vm/vm.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_dot_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DOT",
        .category = "Math Functions",
        .syntax = "DOT(u, v)",
        .help_text = "ECMA-116 standard function returning the dot product of vectors u and v.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_dot_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (!vm || arg_count < 2) {
        if (err) { err->code = 13; err->message = "DOT expects two vector arguments"; }
        return res;
    }

    const char *arr1 = (args[0].type == VAL_STRING || args[0].type == VAL_ARRAY_REF) ? str_data(args[0].as.string) : NULL;
    const char *arr2 = (args[1].type == VAL_STRING || args[1].type == VAL_ARRAY_REF) ? str_data(args[1].as.string) : NULL;

    if (!arr1 || !arr2) {
        if (err) { err->code = 13; err->message = "Invalid array arguments in DOT"; }
        return res;
    }

    ArrayContext *arr = vm_get_arr(vm);
    int sz1 = 0, sz2 = 0;
    BValue *e1 = arr_get_flat_elements(arr, arr1, &sz1);
    BValue *e2 = arr_get_flat_elements(arr, arr2, &sz2);

    if (!e1 || !e2) {
        if (err) { err->code = 9; err->message = "Array not dimensioned in DOT"; }
        return res;
    }

    int base = arr_get_option_base(arr);
    int start = (sz1 > base && sz2 > base) ? base : 0;
    int limit = (sz1 < sz2) ? sz1 : sz2;

    double dot = 0.0;
    for (int i = start; i < limit; i++) {
        double v1 = (e1[i].type == VAL_NUMBER) ? e1[i].as.number : 0.0;
        double v2 = (e2[i].type == VAL_NUMBER) ? e2[i].as.number : 0.0;
        dot += v1 * v2;
    }

    res.as.number = dot;
    return res;
}
