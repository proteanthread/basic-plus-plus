// FILENAME: mat_transform.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libengine (mat_internal.h)
// Provides runtime implementation for the MAT_TRANSFORM statement in BASIC++.
//
// ---- Includes ----

#include "statements/matrices/mat_internal.h"

//
// ---- Matrix Transpose ----

BppError mat_op_trn(VMContext *vm, const char *dest, const char *src) {
    BppError err;
    memset(&err, 0, sizeof(err));
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
    int dbounds[2] = {base + cols - 1, base + rows - 1};

    double *temp = (double *)calloc((size_t)(rows * cols), sizeof(double));
    if (!temp) {
        err.code = 14;
        err.message = "Out of memory in TRN";
        return err;
    }

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int sidx[2] = {base + r, base + c};
            BValue *sp = arr_get_element(arr, src, 2, sidx, &err);
            temp[c * rows + r] = (sp && sp->type == VAL_NUMBER) ? sp->as.number : 0.0;
        }
    }

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
    free(temp);
    err.code = 0;
    return err;
}

//
// ---- Matrix Inversion ----

BppError mat_op_inv(VMContext *vm, const char *dest, const char *src) {
    BppError err;
    memset(&err, 0, sizeof(err));
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
    double det = 0.0;
    double *inv_buf = (double *)calloc((size_t)(n_size * n_size), sizeof(double));
    if (!inv_buf) {
        err.code = 14;
        err.message = "Out of memory in INV";
        return err;
    }

    if (n_size == 1) {
        int idx[2] = {base, base};
        BValue *sp = arr_get_element(arr, src, 2, idx, &err);
        det = (sp && sp->type == VAL_NUMBER) ? sp->as.number : 0.0;
        if (det == 0.0) {
            free(inv_buf);
            err.code = 11;
            err.message = "Division by zero (Singular matrix)";
            return err;
        }
        inv_buf[0] = 1.0 / det;
    } else if (n_size == 2) {
        int i11[2] = {base, base}, i12[2] = {base, base + 1};
        int i21[2] = {base + 1, base}, i22[2] = {base + 1, base + 1};
        BValue *e11 = arr_get_element(arr, src, 2, i11, &err);
        BValue *e12 = arr_get_element(arr, src, 2, i12, &err);
        BValue *e21 = arr_get_element(arr, src, 2, i21, &err);
        BValue *e22 = arr_get_element(arr, src, 2, i22, &err);
        double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
        double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
        double c = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
        double d = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
        det = a * d - b * c;
        if (det == 0.0) {
            free(inv_buf);
            err.code = 11;
            err.message = "Division by zero (Singular matrix)";
            return err;
        }
        inv_buf[0] = d / det;    inv_buf[1] = -b / det;
        inv_buf[2] = -c / det;   inv_buf[3] = a / det;
    } else if (n_size == 3) {
        int r1 = base, r2 = base + 1, r3 = base + 2;
        int c1 = base, c2 = base + 1, c3 = base + 2;
        int i11[2]={r1,c1}, i12[2]={r1,c2}, i13[2]={r1,c3};
        int i21[2]={r2,c1}, i22[2]={r2,c2}, i23[2]={r2,c3};
        int i31[2]={r3,c1}, i32[2]={r3,c2}, i33[2]={r3,c3};
        BValue *e11 = arr_get_element(arr, src, 2, i11, &err);
        BValue *e12 = arr_get_element(arr, src, 2, i12, &err);
        BValue *e13 = arr_get_element(arr, src, 2, i13, &err);
        BValue *e21 = arr_get_element(arr, src, 2, i21, &err);
        BValue *e22 = arr_get_element(arr, src, 2, i22, &err);
        BValue *e23 = arr_get_element(arr, src, 2, i23, &err);
        BValue *e31 = arr_get_element(arr, src, 2, i31, &err);
        BValue *e32 = arr_get_element(arr, src, 2, i32, &err);
        BValue *e33 = arr_get_element(arr, src, 2, i33, &err);
        double a = (e11 && e11->type == VAL_NUMBER) ? e11->as.number : 0.0;
        double b = (e12 && e12->type == VAL_NUMBER) ? e12->as.number : 0.0;
        double c = (e13 && e13->type == VAL_NUMBER) ? e13->as.number : 0.0;
        double d = (e21 && e21->type == VAL_NUMBER) ? e21->as.number : 0.0;
        double e = (e22 && e22->type == VAL_NUMBER) ? e22->as.number : 0.0;
        double f = (e23 && e23->type == VAL_NUMBER) ? e23->as.number : 0.0;
        double g = (e31 && e31->type == VAL_NUMBER) ? e31->as.number : 0.0;
        double h = (e32 && e32->type == VAL_NUMBER) ? e32->as.number : 0.0;
        double i_v = (e33 && e33->type == VAL_NUMBER) ? e33->as.number : 0.0;

        det = a * (e * i_v - f * h) - b * (d * i_v - f * g) + c * (d * h - e * g);
        if (det == 0.0) {
            free(inv_buf);
            err.code = 11;
            err.message = "Division by zero (Singular matrix)";
            return err;
        }

        inv_buf[0] = (e * i_v - f * h) / det;
        inv_buf[1] = (c * h - b * i_v) / det;
        inv_buf[2] = (b * f - c * e) / det;
        inv_buf[3] = (f * g - d * i_v) / det;
        inv_buf[4] = (a * i_v - c * g) / det;
        inv_buf[5] = (c * d - a * f) / det;
        inv_buf[6] = (d * h - e * g) / det;
        inv_buf[7] = (b * g - a * h) / det;
        inv_buf[8] = (a * e - b * d) / det;
    } else {
        free(inv_buf);
        err.code = 5;
        err.message = "INV supports up to 3x3 matrices";
        return err;
    }

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
    free(inv_buf);
    err.code = 0;
    return err;
}

//
// ---- Vector Cross Product ----

BppError mat_op_cross(VMContext *vm, const char *dest, const char *v1_name, const char *v2_name) {
    BppError err;
    memset(&err, 0, sizeof(err));
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
