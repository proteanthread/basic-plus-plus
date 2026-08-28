// FILENAME: mat_special.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_SPECIAL statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/mat_internal.h"

//
// ---- Special Matrix Initializers ----

BppError mat_op_zer(VMContext *vm, const char *dest, int dims, const int *bounds) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    (void)dims;
    (void)bounds;

    int sz = 0;
    BValue *elems = arr_get_flat_elements(arr, dest, &sz);
    if (elems) {
        for (int i = 0; i < sz; i++) {
            elems[i].type = VAL_NUMBER;
            elems[i].as.number = 0.0;
        }
    }
    return err;
}

BppError mat_op_con(VMContext *vm, const char *dest, int dims, const int *bounds) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    (void)dims;
    (void)bounds;

    int sz = 0;
    BValue *elems = arr_get_flat_elements(arr, dest, &sz);
    if (elems) {
        for (int i = 0; i < sz; i++) {
            elems[i].type = VAL_NUMBER;
            elems[i].as.number = 1.0;
        }
    }
    return err;
}

BppError mat_op_idn(VMContext *vm, const char *dest, int dims, const int *bounds) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    int base = arr_get_option_base(arr);

    if (dims != 2 || bounds[0] != bounds[1]) {
        err.code = 9;
        err.message = "IDN requires square 2D matrix";
        return err;
    }

    int max_r = bounds[0];
    int max_c = bounds[1];
    for (int r = base; r <= max_r; r++) {
        for (int c = base; c <= max_c; c++) {
            int idx[2] = {r, c};
            BValue *p = arr_get_element(arr, dest, 2, idx, &err);
            if (p) {
                p->type = VAL_NUMBER;
                p->as.number = (r == c) ? 1.0 : 0.0;
            }
        }
    }
    err.code = 0;
    return err;
}

BppError mat_op_nul(VMContext *vm, const char *dest, int dims, const int *bounds) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    (void)dims;
    (void)bounds;

    int sz = 0;
    BValue *elems = arr_get_flat_elements(arr, dest, &sz);
    if (elems) {
        for (int i = 0; i < sz; i++) {
            if (elems[i].type == VAL_STRING) {
                str_release(vm_get_str(vm), elems[i].as.string);
            }
            elems[i].type = VAL_STRING;
            elems[i].as.string = str_create(vm_get_str(vm), "", 0);
        }
    }
    err.code = 0;
    return err;
}
