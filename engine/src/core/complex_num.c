// FILENAME: complex_num.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (complex_fn.c)
// NEEDS: libcore (complex_num.h, math.h)
// NEEDS: libengine (math.c)
// Provides core logic and interface definitions for complex_num within BASIC++.
//
// ---- Includes ----

#include "core/complex_num.h"
#include "runtime/math/math.h"


BppComplex complex_create(double real, double imag) {
    BppComplex z;
    z.real = real;
    z.imag = imag;
    return z;
}

BppComplex complex_add(BppComplex a, BppComplex b) {
    BppComplex z;
    z.real = a.real + b.real;
    z.imag = a.imag + b.imag;
    return z;
}

BppComplex complex_sub(BppComplex a, BppComplex b) {
    BppComplex z;
    z.real = a.real - b.real;
    z.imag = a.imag - b.imag;
    return z;
}

BppComplex complex_mul(BppComplex a, BppComplex b) {
    BppComplex z;
    z.real = (a.real * b.real) - (a.imag * b.imag);
    z.imag = (a.real * b.imag) + (a.imag * b.real);
    return z;
}

BppComplex complex_div(BppComplex a, BppComplex b, bool *div_by_zero) {
    BppComplex z = {0.0, 0.0};
    double denom = (b.real * b.real) + (b.imag * b.imag);
    if (denom == 0.0) {
        if (div_by_zero) *div_by_zero = true;
        return z;
    }
    if (div_by_zero) *div_by_zero = false;
    z.real = ((a.real * b.real) + (a.imag * b.imag)) / denom;
    z.imag = ((a.imag * b.real) - (a.real * b.imag)) / denom;
    return z;
}

double complex_abs(BppComplex z) {
    return runtime_sqrt((z.real * z.real) + (z.imag * z.imag));
}

double complex_arg(BppComplex z) {
    return runtime_atan2(z.imag, z.real);
}

BppComplex complex_conj(BppComplex z) {
    BppComplex res;
    res.real = z.real;
    res.imag = -z.imag;
    return res;
}

BppComplex complex_sin(BppComplex z) {
    BppComplex res;
    res.real = runtime_sin(z.real) * runtime_cosh(z.imag);
    res.imag = runtime_cos(z.real) * runtime_sinh(z.imag);
    return res;
}

BppComplex complex_cos(BppComplex z) {
    BppComplex res;
    res.real = runtime_cos(z.real) * runtime_cosh(z.imag);
    res.imag = -runtime_sin(z.real) * runtime_sinh(z.imag);
    return res;
}

BppComplex complex_exp(BppComplex z) {
    BppComplex res;
    double r = runtime_exp(z.real);
    res.real = r * runtime_cos(z.imag);
    res.imag = r * runtime_sin(z.imag);
    return res;
}

BppComplex complex_log(BppComplex z) {
    BppComplex res;
    res.real = runtime_log(complex_abs(z));
    res.imag = complex_arg(z);
    return res;
}

BppComplex complex_sqr(BppComplex z) {
    BppComplex res;
    double r = runtime_sqrt(complex_abs(z));
    double theta = complex_arg(z) / 2.0;
    res.real = r * runtime_cos(theta);
    res.imag = r * runtime_sin(theta);
    return res;
}

