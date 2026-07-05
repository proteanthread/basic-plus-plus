/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: matrix_core.c
 * Subsystem: Primitive Matrix Calculations Utility
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Performs matrix math (addition, transposition, inversion).
 *
 * 2. WHAT TO EXPECT:
 *    Calculates matrix operations directly on double arrays.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Dimension boundaries.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Gauss-Jordan inversion algorithm.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If inversion fails, verify determinant is non-zero.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE MATRIX MATHEMATICS ENGINE
 * File: matrix_core.c
 * ===================================================================== */

#include "matrix_core.h"

void matrix_core_transpose(const double *src, int rows, int cols, double *dest)
{
    int r, c;
    for (r = 0; r < rows; r++) {
        for (c = 0; c < cols; c++) {
            dest[c * rows + r] = src[r * cols + c];
        }
    }
}

int matrix_core_invert(const double *src, int n, double *dest)
{
    /* Supports up to 32x32 matrix for inverse */
    double work[32][64];
    double pivot, factor;
    int r, c, p;

    if (n > 32 || n < 1) {
        return -1;
    }

    /* Build augmented matrix [src | I] */
    for (r = 0; r < n; r++) {
        for (c = 0; c < n; c++) {
            work[r][c] = src[r * n + c];
            work[r][c + n] = (r == c) ? 1.0 : 0.0;
        }
    }

    /* Gauss-Jordan elimination */
    for (p = 0; p < n; p++) {
        /* Find pivot */
        int max_row = p;
        double max_val = work[p][p];
        if (max_val < 0) max_val = -max_val;

        for (r = p + 1; r < n; r++) {
            double v = work[r][p];
            if (v < 0) v = -v;
            if (v > max_val) {
                max_val = v;
                max_row = r;
            }
        }

        /* Swap rows if needed */
        if (max_row != p) {
            for (c = 0; c < 2 * n; c++) {
                double tmp = work[p][c];
                work[p][c] = work[max_row][c];
                work[max_row][c] = tmp;
            }
        }

        pivot = work[p][p];
        if (pivot > -1e-12 && pivot < 1e-12) {
            /* Singular matrix */
            return -1;
        }

        /* Scale pivot row */
        for (c = 0; c < 2 * n; c++) {
            work[p][c] /= pivot;
        }

        /* Eliminate column */
        for (r = 0; r < n; r++) {
            if (r == p) continue;
            factor = work[r][p];
            for (c = 0; c < 2 * n; c++) {
                work[r][c] -= factor * work[p][c];
            }
        }
    }

    /* Extract inverse from right half */
    for (r = 0; r < n; r++) {
        for (c = 0; c < n; c++) {
            dest[r * n + c] = work[r][c + n];
        }
    }

    return 0;
}

void matrix_core_multiply(const double *a, int a_rows, int a_cols,
                           const double *b, int b_rows, int b_cols, double *dest)
{
    (void)b_rows;
    int r, c, k;
    for (r = 0; r < a_rows; r++) {
        for (c = 0; c < b_cols; c++) {
            double sum = 0.0;
            for (k = 0; k < a_cols; k++) {
                sum += a[r * a_cols + k] * b[k * b_cols + c];
            }
            dest[r * b_cols + c] = sum;
        }
    }
}

void matrix_core_add(const double *a, const double *b, int rows, int cols, double *dest)
{
    int i, total = rows * cols;
    for (i = 0; i < total; i++) {
        dest[i] = a[i] + b[i];
    }
}

void matrix_core_subtract(const double *a, const double *b, int rows, int cols, double *dest)
{
    int i, total = rows * cols;
    for (i = 0; i < total; i++) {
        dest[i] = a[i] - b[i];
    }
}

void matrix_core_scalar_multiply(const double *src, int rows, int cols, double k, double *dest)
{
    int i, total = rows * cols;
    for (i = 0; i < total; i++) {
        dest[i] = src[i] * k;
    }
}
