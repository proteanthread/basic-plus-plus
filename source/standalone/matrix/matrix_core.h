/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: matrix_core.h
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
 * File: matrix_core.h
 * ===================================================================== */

#ifndef STANDALONE_MATRIX_CORE_H
#define STANDALONE_MATRIX_CORE_H

/* Matrix transposition: dest = transpose(src).
 * src has dimensions: rows x cols.
 * dest must have dimensions: cols x rows. */
void matrix_core_transpose(const double *src, int rows, int cols, double *dest);

/* Matrix inversion via Gauss-Jordan elimination.
 * n is the matrix dimension (n x n).
 * Returns 0 on success, -1 if matrix is singular (non-invertible). */
int matrix_core_invert(const double *src, int n, double *dest);

/* Matrix multiplication: dest = a * b.
 * a has dimensions: a_rows x a_cols.
 * b has dimensions: b_rows x b_cols.
 * dest must have dimensions: a_rows x b_cols. (Note: a_cols must equal b_rows). */
void matrix_core_multiply(const double *a, int a_rows, int a_cols,
                           const double *b, int b_rows, int b_cols, double *dest);

/* Matrix addition: dest = a + b.
 * Dimensions: rows x cols. */
void matrix_core_add(const double *a, const double *b, int rows, int cols, double *dest);

/* Matrix subtraction: dest = a - b.
 * Dimensions: rows x cols. */
void matrix_core_subtract(const double *a, const double *b, int rows, int cols, double *dest);

/* Scalar multiplication: dest = src * k.
 * Dimensions: rows x cols. */
void matrix_core_scalar_multiply(const double *src, int rows, int cols, double k, double *dest);

#endif /* STANDALONE_MATRIX_CORE_H */
