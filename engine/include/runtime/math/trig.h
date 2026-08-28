// FILENAME: trig.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (math.h, trig.c)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: platform, memory
// Freestanding trigonometry and hyperbolic functions.
//
// ---- Includes ----

#ifndef RUNTIME_MATH_TRIG_H
#define RUNTIME_MATH_TRIG_H

#ifdef __cplusplus
extern "C" {
#endif

double runtime_sin(double x);
double runtime_cos(double x);
double runtime_tan(double x);

double runtime_asin(double x);
double runtime_acos(double x);
double runtime_atan(double x);
double runtime_atan2(double y, double x);

double runtime_sinh(double x);
double runtime_cosh(double x);
double runtime_tanh(double x);

double runtime_sec(double x);
double runtime_csc(double x);
double runtime_cot(double x);

double runtime_hypot(double x, double y);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_MATH_TRIG_H
