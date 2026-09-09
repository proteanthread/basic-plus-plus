// FILENAME: mat_transform.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_TRANSFORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/mat_internal.h"
#include "runtime/string/memops.h"
#include "runtime/memory/alloc.h"
#include "runtime/math/basic.h"

//
// ---- Matrix Transpose ----

BppError mat_op_trn(VMContext *vm, const char *dest, const char *src) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    int base = arr_get_option_base(arr);

    int sbounds[4] = {0};
    int sdims = arr_get_dimensions(arr, src, sbounds, 4);
    if (sdims != 2) {
        err.code = 9;
        err.message = "TRN requires 2D matrix";
        return err;
    }
    int rows = sbounds[0] - base + 1;
    int cols = sbounds[1] - base + 1;

    double *temp = (double *)runtime_calloc((size_t)(rows * cols), sizeof(double));
    if (!temp) {
        err.code = 14;
        err.message = "Out of memory in TRN";
        return err;
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int sidx[2] = {base + r, base + c};
            BValue *sp = arr_get_element(arr, src, 2, sidx, &err);
            if (sp && (sp->type == VAL_NUMBER || sp->type == VAL_INTEGER)) {
                temp[c * rows + r] = sp->as.number;
            }
        }
    }

    int dbounds[2] = {base + cols - 1, base + rows - 1};
    ensure_array_dims(arr, dest, 2, dbounds, &err);
    for (int c = 0; c < cols; c++) {
        for (int r = 0; r < rows; r++) {
            int didx[2] = {base + c, base + r};
            BValue *dp = arr_get_element(arr, dest, 2, didx, &err);
            if (dp) {
                dp->type = VAL_NUMBER;
                dp->as.number = temp[c * rows + r];
            }
        }
    }
    runtime_free(temp);
    err.code = 0;
    return err;
}

//
// ---- Matrix Inversion (General N x N Gaussian Elimination) ----

BppError mat_op_inv(VMContext *vm, const char *dest, const char *src) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    int base = arr_get_option_base(arr);

    int sbounds[4] = {0};
    int sdims = arr_get_dimensions(arr, src, sbounds, 4);
    if (sdims != 2 || sbounds[0] != sbounds[1]) {
        err.code = 9;
        err.message = "INV requires square 2D matrix";
        return err;
    }

    int n_size = sbounds[0] - base + 1;
    double *inv_buf = (double *)runtime_calloc((size_t)(n_size * n_size), sizeof(double));
    if (!inv_buf) {
        err.code = 14;
        err.message = "Out of memory in INV";
        return err;
    }

    size_t aug_cols = (size_t)(2 * n_size);
    double *aug = (double *)runtime_calloc((size_t)n_size * aug_cols, sizeof(double));
    if (!aug) {
        runtime_free(inv_buf);
        err.code = 14;
        err.message = "Out of memory in INV";
        return err;
    }

    // Initialize augmented matrix [A | I]
    for (int r = 0; r < n_size; r++) {
        for (int c = 0; c < n_size; c++) {
            int idx[2] = {base + r, base + c};
            BValue *sp = arr_get_element(arr, src, 2, idx, &err);
            double val = (sp && (sp->type == VAL_NUMBER || sp->type == VAL_INTEGER)) ? sp->as.number : 0.0;
            aug[r * aug_cols + c] = val;
        }
        aug[r * aug_cols + (n_size + r)] = 1.0;
    }

    double det = 1.0;
    int sign = 1;
    bool singular = false;

    for (int k = 0; k < n_size; k++) {
        // Find pivot in column k
        int pivot = k;
        double max_val = runtime_fabs(aug[k * aug_cols + k]);
        for (int r = k + 1; r < n_size; r++) {
            double v = runtime_fabs(aug[r * aug_cols + k]);
            if (v > max_val) {
                max_val = v;
                pivot = r;
            }
        }

        if (max_val < 1e-15) {
            singular = true;
            det = 0.0;
            break;
        }

        if (pivot != k) {
            // Swap row k and pivot
            for (size_t c = 0; c < aug_cols; c++) {
                double tmp = aug[k * aug_cols + c];
                aug[k * aug_cols + c] = aug[pivot * aug_cols + c];
                aug[pivot * aug_cols + c] = tmp;
            }
            sign = -sign;
        }

        double pv = aug[k * aug_cols + k];
        det *= pv;

        // Normalize pivot row
        for (size_t c = 0; c < aug_cols; c++) {
            aug[k * aug_cols + c] /= pv;
        }

        // Eliminate other rows
        for (int r = 0; r < n_size; r++) {
            if (r != k) {
                double factor = aug[r * aug_cols + k];
                if (factor != 0.0) {
                    for (size_t c = 0; c < aug_cols; c++) {
                        aug[r * aug_cols + c] -= factor * aug[k * aug_cols + c];
                    }
                }
            }
        }
    }

    det *= sign;

    if (singular || runtime_fabs(det) < 1e-15) {
        runtime_free(aug);
        runtime_free(inv_buf);
        err.code = 11;
        err.message = "Division by zero (Singular matrix)";
        return err;
    }

    // Extract inverse matrix
    for (int r = 0; r < n_size; r++) {
        for (int c = 0; c < n_size; c++) {
            inv_buf[r * n_size + c] = aug[r * aug_cols + (n_size + c)];
        }
    }
    runtime_free(aug);

    arr_set_last_det(arr, det);
    ensure_array_dims(arr, dest, 2, sbounds, &err);
    for (int r = 0; r < n_size; r++) {
        for (int c = 0; c < n_size; c++) {
            int didx[2] = {base + r, base + c};
            BValue *dp = arr_get_element(arr, dest, 2, didx, &err);
            if (dp) {
                dp->type = VAL_NUMBER;
                dp->as.number = inv_buf[r * n_size + c];
            }
        }
    }
    runtime_free(inv_buf);
    err.code = 0;
    return err;
}

//
// ---- Vector Cross Product ----

BppError mat_op_cross(VMContext *vm, const char *dest, const char *v1_name, const char *v2_name) {
    BppError err;
    runtime_memset(&err, 0, sizeof(err));
    ArrayContext *arr = vm_get_arr(vm);
    int base = arr_get_option_base(arr);

    int sbounds1[4] = {0}, sbounds2[4] = {0};
    int sdims1 = arr_get_dimensions(arr, v1_name, sbounds1, 4);
    int sdims2 = arr_get_dimensions(arr, v2_name, sbounds2, 4);

    int start_idx1 = (sdims1 == 1 && base == 0 && sbounds1[0] == 2) ? 0 : 1;
    int start_idx2 = (sdims2 == 1 && base == 0 && sbounds2[0] == 2) ? 0 : 1;

    int idx1_1[1] = {start_idx1},     idx1_2[1] = {start_idx1 + 1}, idx1_3[1] = {start_idx1 + 2};
    int idx2_1[1] = {start_idx2},     idx2_2[1] = {start_idx2 + 1}, idx2_3[1] = {start_idx2 + 2};

    BValue *ev1 = arr_get_element(arr, v1_name, 1, idx1_1, &err);
    BValue *ev2 = arr_get_element(arr, v1_name, 1, idx1_2, &err);
    BValue *ev3 = arr_get_element(arr, v1_name, 1, idx1_3, &err);
    err.code = 0;

    BValue *ew1 = arr_get_element(arr, v2_name, 1, idx2_1, &err);
    BValue *ew2 = arr_get_element(arr, v2_name, 1, idx2_2, &err);
    BValue *ew3 = arr_get_element(arr, v2_name, 1, idx2_3, &err);
    err.code = 0;

    double u1 = (ev1 && ev1->type == VAL_NUMBER) ? ev1->as.number : 0.0;
    double u2 = (ev2 && ev2->type == VAL_NUMBER) ? ev2->as.number : 0.0;
    double u3 = (ev3 && ev3->type == VAL_NUMBER) ? ev3->as.number : 0.0;

    double w1 = (ew1 && ew1->type == VAL_NUMBER) ? ew1->as.number : 0.0;
    double w2 = (ew2 && ew2->type == VAL_NUMBER) ? ew2->as.number : 0.0;
    double w3 = (ew3 && ew3->type == VAL_NUMBER) ? ew3->as.number : 0.0;

    double cp1 = u2 * w3 - u3 * w2;
    double cp2 = u3 * w1 - u1 * w3;
    double cp3 = u1 * w2 - u2 * w1;

    int dest_start = 1;
    int dbounds[4] = {0};
    int ddims = arr_get_dimensions(arr, dest, dbounds, 4);
    if (ddims == 1 && base == 0 && dbounds[0] == 2) {
        dest_start = 0;
    }

    int bounds[1] = {dest_start == 0 ? 2 : 3};
    ensure_array_dims(arr, dest, 1, bounds, &err);

    int didx1[1] = {dest_start}, didx2[1] = {dest_start + 1}, didx3[1] = {dest_start + 2};
    BValue *p1 = arr_get_element(arr, dest, 1, didx1, &err);
    if (p1) { p1->type = VAL_NUMBER; p1->as.number = cp1; }
    BValue *p2 = arr_get_element(arr, dest, 1, didx2, &err);
    if (p2) { p2->type = VAL_NUMBER; p2->as.number = cp2; }
    BValue *p3 = arr_get_element(arr, dest, 1, didx3, &err);
    if (p3) { p3->type = VAL_NUMBER; p3->as.number = cp3; }
    err.code = 0;
    return err;
}
