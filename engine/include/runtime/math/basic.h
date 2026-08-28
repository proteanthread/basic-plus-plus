// FILENAME: basic.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (algebra.c, basic.c, math.h, trig.c)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: platform, memory
// Freestanding basic arithmetic and sign operations.
//
// ---- Includes ----

#ifndef RUNTIME_MATH_BASIC_H
#define RUNTIME_MATH_BASIC_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Integer absolute value.
int runtime_abs(int j);

// @brief Long integer absolute value.
long runtime_labs(long j);

// @brief 64-bit integer absolute value.
int64_t runtime_llabs(int64_t j);

// @brief Double-precision floating-point absolute value.
double runtime_fabs(double x);

// @brief Floor function (largest integer value not greater than x).
double runtime_floor(double x);

// @brief Ceiling function (smallest integer value not less than x).
double runtime_ceil(double x);

// @brief Round to nearest integer (halfway cases rounded away from zero).
double runtime_round(double x);

// @brief Truncate to integer value toward zero.
double runtime_trunc(double x);

// @brief Floating-point remainder of x / y.
double runtime_fmod(double x, double y);

// @brief Signum function (-1 if x < 0, 0 if x == 0, 1 if x > 0).
int runtime_sgn(double x);

// @brief Clamps x between min_val and max_val.
double runtime_clamp(double x, double min_val, double max_val);

// @brief Linear interpolation between a and b by factor t.
double runtime_lerp(double a, double b, double t);

// @brief Returns minimum of two doubles.
double runtime_fmin(double x, double y);

// @brief Returns maximum of two doubles.
double runtime_fmax(double x, double y);

// @brief Checks if x is NaN.
bool runtime_isnan(double x);

// @brief Checks if x is infinite.
bool runtime_isinf(double x);

// @brief Checks if x is finite (neither NaN nor infinite).
bool runtime_isfinite(double x);

// @brief Freestanding modf (decomposes x into integral and fractional parts).
double runtime_modf(double x, double *iptr);

// @brief Freestanding frexp (breaks x into normalized fraction and power of 2).
double runtime_frexp(double x, int *exp);

// @brief Freestanding ldexp (computes x * 2^exp).
double runtime_ldexp(double x, int exp);


#ifdef __cplusplus
}
#endif


#endif // RUNTIME_MATH_BASIC_H
