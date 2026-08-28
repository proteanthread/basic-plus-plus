// FILENAME: mat_arith.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_ARITH statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/mat_internal.h"

//
// ---- Binary Matrix Arithmetic ----

BppError mat_op_add(VMContext *vm, const char *dest, const char *name_a, const char *name_b) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);

    int bounds_a[4] = {0}, bounds_b[4] = {0};
    int dims_a = arr_get_dimensions(arr, name_a, bounds_a, 4);
    int dims_b = arr_get_dimensions(arr, name_b, bounds_b, 4);
    if (dims_a != dims_b) {
        err.code = 9; err.message = "Array dimension mismatch in MAT +";
        return err;
    }
    for (int i = 0; i < dims_a; i++) {
        if (bounds_a[i] != bounds_b[i]) {
            err.code = 9; err.message = "Array shape mismatch in MAT +";
            return err;
        }
    }

    ensure_array_dims(arr, dest, dims_a, bounds_a, &err);
    int sz = 0;
    BValue *ea = arr_get_flat_elements(arr, name_a, &sz);
    BValue *eb = arr_get_flat_elements(arr, name_b, NULL);
    BValue *ec = arr_get_flat_elements(arr, dest, NULL);
    if (ea && eb && ec) {
        for (int i = 0; i < sz; i++) {
            double va = (ea[i].type == VAL_NUMBER) ? ea[i].as.number : 0.0;
            double vb = (eb[i].type == VAL_NUMBER) ? eb[i].as.number : 0.0;
            ec[i].type = VAL_NUMBER;
            ec[i].as.number = va + vb;
        }
    }
    err.code = 0;
    return err;
}

BppError mat_op_sub(VMContext *vm, const char *dest, const char *name_a, const char *name_b) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);

    int bounds_a[4] = {0}, bounds_b[4] = {0};
    int dims_a = arr_get_dimensions(arr, name_a, bounds_a, 4);
    int dims_b = arr_get_dimensions(arr, name_b, bounds_b, 4);
    if (dims_a != dims_b) {
        err.code = 9; err.message = "Array dimension mismatch in MAT -";
        return err;
    }
    for (int i = 0; i < dims_a; i++) {
        if (bounds_a[i] != bounds_b[i]) {
            err.code = 9; err.message = "Array shape mismatch in MAT -";
            return err;
        }
    }

    ensure_array_dims(arr, dest, dims_a, bounds_a, &err);
    int sz = 0;
    BValue *ea = arr_get_flat_elements(arr, name_a, &sz);
    BValue *eb = arr_get_flat_elements(arr, name_b, NULL);
    BValue *ec = arr_get_flat_elements(arr, dest, NULL);
    if (ea && eb && ec) {
        for (int i = 0; i < sz; i++) {
            double va = (ea[i].type == VAL_NUMBER) ? ea[i].as.number : 0.0;
            double vb = (eb[i].type == VAL_NUMBER) ? eb[i].as.number : 0.0;
            ec[i].type = VAL_NUMBER;
            ec[i].as.number = va - vb;
        }
    }
    err.code = 0;
    return err;
}

BppError mat_op_mul(VMContext *vm, const char *dest, const char *name_a, const char *name_b) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    int base = arr_get_option_base(arr);

    int bounds_a[4] = {0}, bounds_b[4] = {0};
    int dims_a = arr_get_dimensions(arr, name_a, bounds_a, 4);
    int dims_b = arr_get_dimensions(arr, name_b, bounds_b, 4);

    if (dims_a == 2 && dims_b == 2) {
        int rows_a = bounds_a[0] - base + 1;
        int cols_a = bounds_a[1] - base + 1;
        int rows_b = bounds_b[0] - base + 1;
        int cols_b = bounds_b[1] - base + 1;

        if (cols_a != rows_b) {
            err.code = 9; err.message = "Matrix dimension mismatch in MAT *";
            return err;
        }

        int bounds_c[2] = {base + rows_a - 1, base + cols_b - 1};
        double *temp = (double *)calloc((size_t)(rows_a * cols_b), sizeof(double));
        if (!temp) { err.code = 14; err.message = "Out of memory in MAT *"; return err; }

        int sz_a = 0, sz_b = 0;
        BValue *ea = arr_get_flat_elements(arr, name_a, &sz_a);
        BValue *eb = arr_get_flat_elements(arr, name_b, &sz_b);
        if (ea && eb) {
            for (int r = 0; r < rows_a; r++) {
                for (int k = 0; k < cols_a; k++) {
                    int idx_a = r * cols_a + k;
                    double va = (idx_a < sz_a && ea[idx_a].type == VAL_NUMBER) ? ea[idx_a].as.number : 0.0;
                    for (int c = 0; c < cols_b; c++) {
                        int idx_b = k * cols_b + c;
                        double vb = (idx_b < sz_b && eb[idx_b].type == VAL_NUMBER) ? eb[idx_b].as.number : 0.0;
                        temp[r * cols_b + c] += va * vb;
                    }
                }
            }
        }

        ensure_array_dims(arr, dest, 2, bounds_c, &err);
        int sz_c = 0;
        BValue *ec = arr_get_flat_elements(arr, dest, &sz_c);
        if (ec) {
            int count = rows_a * cols_b;
            for (int i = 0; i < count && i < sz_c; i++) {
                ec[i].type = VAL_NUMBER;
                ec[i].as.number = temp[i];
            }
        }
        free(temp);
        err.code = 0;
        return err;
    } else if (dims_a == 1 && dims_b == 1) {
        int len_a = bounds_a[0] - base + 1;
        int len_b = bounds_b[0] - base + 1;
        if (len_a != len_b) {
            err.code = 9; err.message = "Vector dimension mismatch in MAT *";
            return err;
        }

        double dot = 0.0;
        for (int i = 0; i < len_a; i++) {
            int ia[1] = {base + i}, ib[1] = {base + i};
            BValue *pa = arr_get_element(arr, name_a, 1, ia, &err);
            BValue *pb = arr_get_element(arr, name_b, 1, ib, &err);
            double va = (pa && pa->type == VAL_NUMBER) ? pa->as.number : 0.0;
            double vb = (pb && pb->type == VAL_NUMBER) ? pb->as.number : 0.0;
            dot += va * vb;
        }

        int bounds_c[1] = {base};
        ensure_array_dims(arr, dest, 1, bounds_c, &err);
        int ic[1] = {base};
        BValue *pc = arr_get_element(arr, dest, 1, ic, &err);
        if (pc) {
            pc->type = VAL_NUMBER;
            pc->as.number = dot;
        }
        err.code = 0;
        return err;
    } else if (dims_a == 1 && dims_b == 2) {
        int len_a = bounds_a[0] - base + 1;
        int rows_b = bounds_b[0] - base + 1;
        int cols_b = bounds_b[1] - base + 1;
        if (len_a != rows_b) {
            err.code = 9; err.message = "Vector-Matrix dimension mismatch in MAT *";
            return err;
        }

        double *temp = (double *)calloc((size_t)cols_b, sizeof(double));
        if (!temp) { err.code = 14; err.message = "Out of memory in MAT *"; return err; }

        for (int c = 0; c < cols_b; c++) {
            double sum = 0.0;
            for (int k = 0; k < len_a; k++) {
                int ia[1] = {base + k};
                int ib[2] = {base + k, base + c};
                BValue *pa = arr_get_element(arr, name_a, 1, ia, &err);
                BValue *pb = arr_get_element(arr, name_b, 2, ib, &err);
                double va = (pa && pa->type == VAL_NUMBER) ? pa->as.number : 0.0;
                double vb = (pb && pb->type == VAL_NUMBER) ? pb->as.number : 0.0;
                sum += va * vb;
            }
            temp[c] = sum;
        }

        int bounds_c[1] = {base + cols_b - 1};
        ensure_array_dims(arr, dest, 1, bounds_c, &err);
        for (int c = 0; c < cols_b; c++) {
            int ic[1] = {base + c};
            BValue *pc = arr_get_element(arr, dest, 1, ic, &err);
            if (pc) {
                pc->type = VAL_NUMBER;
                pc->as.number = temp[c];
            }
        }
        free(temp);
        err.code = 0;
        return err;
    } else if (dims_a == 2 && dims_b == 1) {
        int rows_a = bounds_a[0] - base + 1;
        int cols_a = bounds_a[1] - base + 1;
        int len_b = bounds_b[0] - base + 1;
        if (cols_a != len_b) {
            err.code = 9; err.message = "Matrix-Vector dimension mismatch in MAT *";
            return err;
        }

        double *temp = (double *)calloc((size_t)rows_a, sizeof(double));
        if (!temp) { err.code = 14; err.message = "Out of memory in MAT *"; return err; }

        for (int r = 0; r < rows_a; r++) {
            double sum = 0.0;
            for (int k = 0; k < cols_a; k++) {
                int ia[2] = {base + r, base + k};
                int ib[1] = {base + k};
                BValue *pa = arr_get_element(arr, name_a, 2, ia, &err);
                BValue *pb = arr_get_element(arr, name_b, 1, ib, &err);
                double va = (pa && pa->type == VAL_NUMBER) ? pa->as.number : 0.0;
                double vb = (pb && pb->type == VAL_NUMBER) ? pb->as.number : 0.0;
                sum += va * vb;
            }
            temp[r] = sum;
        }

        int bounds_c[1] = {base + rows_a - 1};
        ensure_array_dims(arr, dest, 1, bounds_c, &err);
        for (int r = 0; r < rows_a; r++) {
            int ic[1] = {base + r};
            BValue *pc = arr_get_element(arr, dest, 1, ic, &err);
            if (pc) {
                pc->type = VAL_NUMBER;
                pc->as.number = temp[r];
            }
        }
        free(temp);
        err.code = 0;
        return err;
    }

    err.code = 9; err.message = "Unsupported array dimensions in MAT *";
    return err;
}

//
// ---- Scalar Matrix Operations ----

BppError mat_op_scalar(VMContext *vm, const char *dest, double scalar, int op_type, const char *src) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);

    int sbounds[4] = {0};
    int sdims = arr_get_dimensions(arr, src, sbounds, 4);
    if (sdims < 1) {
        err.code = 9; err.message = "Source array not dimensioned";
        return err;
    }

    ensure_array_dims(arr, dest, sdims, sbounds, &err);
    int sz = 0;
    BValue *src_elems = arr_get_flat_elements(arr, src, &sz);
    BValue *dst_elems = arr_get_flat_elements(arr, dest, NULL);
    if (src_elems && dst_elems) {
        for (int i = 0; i < sz; i++) {
            double val_src = (src_elems[i].type == VAL_NUMBER) ? src_elems[i].as.number : 0.0;
            dst_elems[i].type = VAL_NUMBER;
            if (op_type == TOK_MUL) {
                dst_elems[i].as.number = scalar * val_src;
            } else if (op_type == TOK_PLUS) {
                dst_elems[i].as.number = scalar + val_src;
            } else if (op_type == TOK_MINUS) {
                dst_elems[i].as.number = scalar - val_src;
            }
        }
    }
    err.code = 0;
    return err;
}

BppError mat_op_div_scalar(VMContext *vm, const char *dest, const char *name_a, double scalar) {
    BppError err;
    memset(&err, 0, sizeof(err));
    if (scalar == 0.0) {
        err.code = 11; err.message = "Division by zero in MAT /";
        return err;
    }
    ArrayContext *arr = vm_get_arr(vm);
    int sbounds[4] = {0};
    int sdims = arr_get_dimensions(arr, name_a, sbounds, 4);
    if (sdims < 1) {
        err.code = 9; err.message = "Source array not dimensioned";
        return err;
    }

    ensure_array_dims(arr, dest, sdims, sbounds, &err);
    int sz = 0;
    BValue *src_elems = arr_get_flat_elements(arr, name_a, &sz);
    BValue *dst_elems = arr_get_flat_elements(arr, dest, NULL);
    if (src_elems && dst_elems) {
        for (int i = 0; i < sz; i++) {
            dst_elems[i].type = VAL_NUMBER;
            dst_elems[i].as.number = ((src_elems[i].type == VAL_NUMBER) ? src_elems[i].as.number : 0.0) / scalar;
        }
    }
    err.code = 0;
    return err;
}

BppError mat_op_copy(VMContext *vm, const char *dest, const char *src) {
    BppError err;
    memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);

    int bounds_a[4] = {0};
    int dims_a = arr_get_dimensions(arr, src, bounds_a, 4);
    ensure_array_dims(arr, dest, dims_a, bounds_a, &err);
    int sz = 0;
    BValue *ea = arr_get_flat_elements(arr, src, &sz);
    BValue *ec = arr_get_flat_elements(arr, dest, NULL);
    if (ea && ec) {
        for (int i = 0; i < sz; i++) {
            if (ec[i].type == VAL_STRING) {
                str_release(vm_get_str(vm), ec[i].as.string);
            }
            ec[i].type = ea[i].type;
            if (ea[i].type == VAL_STRING) {
                if (ea[i].as.string) {
                    str_add_ref(ea[i].as.string);
                    ec[i].as.string = ea[i].as.string;
                } else {
                    ec[i].as.string = NULL;
                }
            } else {
                ec[i].as.number = ea[i].as.number;
            }
        }
    }
    err.code = 0;
    return err;
}
