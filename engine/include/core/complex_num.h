// FILENAME: complex_num.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (complex_num.c)
// NEEDED BY: libengine (complex_fn.c)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for complex_num within BASIC++.
//
// ---- Includes ----

#ifndef COMPLEX_NUM_H
#define COMPLEX_NUM_H

#include "types/types.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double real;
    double imag;
} BppComplex;

BppComplex complex_create(double real, double imag);
BppComplex complex_add(BppComplex a, BppComplex b);
BppComplex complex_sub(BppComplex a, BppComplex b);
BppComplex complex_mul(BppComplex a, BppComplex b);
BppComplex complex_div(BppComplex a, BppComplex b, bool *div_by_zero);
double     complex_abs(BppComplex z);
double     complex_arg(BppComplex z);
BppComplex complex_conj(BppComplex z);
BppComplex complex_sin(BppComplex z);
BppComplex complex_cos(BppComplex z);
BppComplex complex_exp(BppComplex z);
BppComplex complex_log(BppComplex z);
BppComplex complex_sqr(BppComplex z);

#ifdef __cplusplus
}
#endif

#endif // COMPLEX_NUM_H
