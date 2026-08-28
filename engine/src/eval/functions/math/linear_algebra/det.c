// FILENAME: det.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c)
// NEEDS: libcore (arrays.h, arrays.c, math.h)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c, string.h)
// NEEDS: libengine (det.h, math.c, string.c, vm.h)
// Provides runtime implementation for the DET built-in function in BASIC++.
//
// ---- Includes ----

#include "eval/functions/math/linear_algebra/det.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/arrays.h"
#include "vm/vm.h"
#include "runtime/math.h"
#include "runtime/string.h"
void func_det_register(void) {
    static const MicroLibMetadata meta = {
        .name = "DET",
        .category = "Math Functions",
        .syntax = "DET(A)",
        .help_text = "ECMA-116 standard function returning the determinant of matrix A.",
        .error_codes = "Error 13: Type Mismatch"
    };
    microlib_register(&meta);
}

BValue func_det_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err) {
    (void)uname;
    BValue res;
    res.type = VAL_NUMBER;
    res.as.number = 0.0;

    if (!vm) return res;
    ArrayContext *arr = vm_get_arr(vm);

    if (arg_count == 0) {
        res.as.number = arr_get_last_det(arr);
        return res;
    }

    const char *arr_name = NULL;
    if (args[0].type == VAL_STRING && args[0].as.string) {
        arr_name = str_data(args[0].as.string);
    } else if (args[0].type == VAL_ARRAY_REF && args[0].as.string) {
        arr_name = str_data(args[0].as.string);
    } else {
        res.as.number = arr_get_last_det(arr);
        return res;
    }

    int bounds[4] = {0};
    int dims = arr_get_dimensions(arr, arr_name, bounds, 4);
    if (dims < 2 || bounds[0] != bounds[1] || bounds[0] < 1) {
        err->code = 9; err->message = "DET expects square matrix";
        return res;
    }

    int n = bounds[0];
    double det = 0.0;
    if (n == 2) {
        int i11[2] = {1, 1}, i12[2] = {1, 2}, i21[2] = {2, 1}, i22[2] = {2, 2};
        BValue *e11 = arr_get_element(arr, arr_name, 2, i11, err);
        BValue *e12 = arr_get_element(arr, arr_name, 2, i12, err);
        BValue *e21 = arr_get_element(arr, arr_name, 2, i21, err);
        BValue *e22 = arr_get_element(arr, arr_name, 2, i22, err);
        double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
        double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
        double c = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
        double d = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
        det = a * d - b * c;
    } else if (n == 3) {
        int i11[2]={1,1}, i12[2]={1,2}, i13[2]={1,3};
        int i21[2]={2,1}, i22[2]={2,2}, i23[2]={2,3};
        int i31[2]={3,1}, i32[2]={3,2}, i33[2]={3,3};
        BValue *e11 = arr_get_element(arr, arr_name, 2, i11, err);
        BValue *e12 = arr_get_element(arr, arr_name, 2, i12, err);
        BValue *e13 = arr_get_element(arr, arr_name, 2, i13, err);
        BValue *e21 = arr_get_element(arr, arr_name, 2, i21, err);
        BValue *e22 = arr_get_element(arr, arr_name, 2, i22, err);
        BValue *e23 = arr_get_element(arr, arr_name, 2, i23, err);
        BValue *e31 = arr_get_element(arr, arr_name, 2, i31, err);
        BValue *e32 = arr_get_element(arr, arr_name, 2, i32, err);
        BValue *e33 = arr_get_element(arr, arr_name, 2, i33, err);
        double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
        double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
        double c = (e13 && e13->type == VAL_NUMBER) ? e13->as.number : 0.0;
        double d = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
        double e = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
        double f = (e23 && e23->type == VAL_NUMBER) ? e23->as.number : 0.0;
        double g = (e31 && e31->type == VAL_NUMBER) ? e31->as.number : 0.0;
        double h = (e32 && e32->type == VAL_NUMBER) ? e32->as.number : 0.0;
        double i_val = (e33 && e33->type == VAL_NUMBER) ? e33->as.number : 0.0;
        det = a * (e * i_val - f * h) - b * (d * i_val - f * g) + c * (d * h - e * g);
    } else {
        err->code = 5; err->message = "DET supports up to 3x3 matrices";
        return res;
    }

    arr_set_last_det(arr, det);
    res.as.number = det;
    return res;
}
