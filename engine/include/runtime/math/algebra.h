// FILENAME: algebra.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (algebra.c, math.h)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: platform, memory
// Freestanding algebraic, exponential, and floating-point math functions.
//
// ---- Includes ----

#ifndef RUNTIME_MATH_ALGEBRA_H
#define RUNTIME_MATH_ALGEBRA_H

#ifdef __cplusplus
extern "C" {
#endif

double runtime_sqrt(double x);
double runtime_cbrt(double x);
double runtime_pow(double base, double exp);
double runtime_exp(double x);
double runtime_log(double x);
double runtime_log10(double x);
double runtime_log2(double x);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_MATH_ALGEBRA_H
