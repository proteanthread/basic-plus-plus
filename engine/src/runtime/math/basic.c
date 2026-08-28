// FILENAME: basic.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (algebra.c, math.h, trig.c)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: libcore (basic.h, float_limits.h)
// Freestanding basic arithmetic and sign implementations.
//
// ---- Includes ----

#include "runtime/math/basic.h"
#include "runtime/math/float_limits.h"


int runtime_abs(int j) {
    return (j < 0) ? -j : j;
}

long runtime_labs(long j) {
    return (j < 0) ? -j : j;
}

int64_t runtime_llabs(int64_t j) {
    return (j < 0) ? -j : j;
}

double runtime_fabs(double x) {
    return (x < 0.0) ? -x : x;
}

double runtime_floor(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) return x;
    int64_t i = (int64_t)x;
    double d = (double)i;
    if (x < 0.0 && x != d) {
        return d - 1.0;
    }
    return d;
}

double runtime_ceil(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) return x;
    int64_t i = (int64_t)x;
    double d = (double)i;
    if (x > 0.0 && x != d) {
        return d + 1.0;
    }
    return d;
}

double runtime_round(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) return x;
    return (x >= 0.0) ? runtime_floor(x + 0.5) : runtime_ceil(x - 0.5);
}

double runtime_trunc(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) return x;
    return (x >= 0.0) ? runtime_floor(x) : runtime_ceil(x);
}

double runtime_fmod(double x, double y) {
    if (runtime_isnan(x) || runtime_isnan(y) || y == 0.0 || runtime_isinf(x)) {
        return RUNTIME_NAN;
    }
    if (runtime_isinf(y)) {
        return x;
    }

    double quotient = runtime_trunc(x / y);
    return x - (quotient * y);
}

int runtime_sgn(double x) {
    if (runtime_isnan(x) || x == 0.0) return 0;
    return (x > 0.0) ? 1 : -1;
}

double runtime_clamp(double x, double min_val, double max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

double runtime_lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double runtime_fmin(double x, double y) {
    if (runtime_isnan(x)) return y;
    if (runtime_isnan(y)) return x;
    return (x < y) ? x : y;
}

double runtime_fmax(double x, double y) {
    if (runtime_isnan(x)) return y;
    if (runtime_isnan(y)) return x;
    return (x > y) ? x : y;
}

bool runtime_isnan(double x) {
    return (x != x);
}

bool runtime_isinf(double x) {
    return (x > RUNTIME_DBL_MAX || x < -RUNTIME_DBL_MAX);
}


bool runtime_isfinite(double x) {
    return (!runtime_isnan(x) && !runtime_isinf(x));
}

double runtime_modf(double x, double *iptr) {
    if (runtime_isnan(x)) {
        if (iptr) *iptr = RUNTIME_NAN;
        return RUNTIME_NAN;
    }
    if (runtime_isinf(x)) {
        if (iptr) *iptr = x;
        return (x < 0.0) ? -0.0 : 0.0;
    }
    double intpart = runtime_trunc(x);
    if (iptr) *iptr = intpart;
    return x - intpart;
}

double runtime_frexp(double x, int *exp) {

    if (x == 0.0) {
        if (exp) *exp = 0;
        return 0.0;
    }
    if (runtime_isnan(x) || runtime_isinf(x)) {
        if (exp) *exp = 0;
        return x;
    }
    union {
        double d;
        uint64_t u;
    } conv;
    conv.d = x;
    int e = (int)((conv.u >> 52) & 0x7FF) - 1022;
    conv.u &= 0x800FFFFFFFFFFFFFULL; // Clear exponent
    conv.u |= 0x3FE0000000000000ULL; // Set exponent to -1 (0.5 <= |mant| < 1.0)
    if (exp) *exp = e;
    return conv.d;
}

double runtime_ldexp(double x, int exp) {
    if (x == 0.0 || runtime_isnan(x) || runtime_isinf(x)) return x;
    union {
        double d;
        uint64_t u;
    } conv;
    conv.d = x;
    int cur_exp = (int)((conv.u >> 52) & 0x7FF);
    int new_exp = cur_exp + exp;
    if (new_exp >= 2047) {
        return (x < 0.0) ? -RUNTIME_INFINITY : RUNTIME_INFINITY;
    }
    if (new_exp <= 0) {
        return 0.0;
    }
    conv.u &= 0x800FFFFFFFFFFFFFULL;
    conv.u |= ((uint64_t)new_exp << 52);
    return conv.d;
}

