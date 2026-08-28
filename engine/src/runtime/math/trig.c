// FILENAME: trig.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (math.h)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: libcore (basic.h, basic.c, float_limits.h, trig.h)
// Freestanding trigonometric series approximations implementation.
//
// ---- Includes ----

#include "runtime/math/trig.h"
#include "runtime/math/basic.h"
#include "runtime/math/float_limits.h"

// Taylor / Chebyshev polynomial for sin(r) on [-pi/4, pi/4]
static double sin_poly(double r) {
    double r2 = r * r;
    return r * (1.0 + r2 * (-0.1666666666666666664 +
           r2 * ( 0.0083333333333333100 +
           r2 * (-0.0001984126984120184 +
           r2 * ( 0.0000027557319214999 +
           r2 * (-0.0000000250521067982 +
           r2 *   0.0000000001605893649))))));
}

// Taylor / Chebyshev polynomial for cos(r) on [-pi/4, pi/4]
static double cos_poly(double r) {
    double r2 = r * r;
    return 1.0 + r2 * (-0.5 +
           r2 * ( 0.041666666666666664 +
           r2 * (-0.001388888888888730 +
           r2 * ( 0.000024801587301371 +
           r2 * (-0.000000275573187032 +
           r2 *   0.000000002087675429)))));
}

double runtime_sin(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) {
        return RUNTIME_NAN;
    }

    // Quadrant reduction: k = round(x / (pi/2)), r = x - k * (pi/2)
    double k_flt = runtime_round(x * (2.0 / RUNTIME_PI));
    double r = x - k_flt * RUNTIME_PI_2;
    int64_t k = (int64_t)k_flt;
    int q = (int)(k % 4);
    if (q < 0) q += 4;

    switch (q) {
        case 0: return sin_poly(r);
        case 1: return cos_poly(r);
        case 2: return -sin_poly(r);
        case 3: return -cos_poly(r);
    }
    return 0.0;
}

double runtime_cos(double x) {
    if (runtime_isnan(x) || runtime_isinf(x)) {
        return RUNTIME_NAN;
    }

    double k_flt = runtime_round(x * (2.0 / RUNTIME_PI));
    double r = x - k_flt * RUNTIME_PI_2;
    int64_t k = (int64_t)k_flt;
    int q = (int)(k % 4);
    if (q < 0) q += 4;

    switch (q) {
        case 0: return cos_poly(r);
        case 1: return -sin_poly(r);
        case 2: return -cos_poly(r);
        case 3: return sin_poly(r);
    }
    return 1.0;
}

double runtime_tan(double x) {
    double c = runtime_cos(x);
    if (c == 0.0) {
        return runtime_sin(x) >= 0 ? RUNTIME_INFINITY : -RUNTIME_INFINITY;
    }
    return runtime_sin(x) / c;
}

double runtime_atan(double x) {
    if (runtime_isnan(x)) return x;
    if (x > 1e16) return RUNTIME_PI_2;
    if (x < -1e16) return -RUNTIME_PI_2;

    bool neg = (x < 0.0);
    if (neg) x = -x;

    bool inv = (x > 1.0);
    if (inv) x = 1.0 / x;

    // Polynomial for atan on [0, 1]
    double x2 = x * x;
    double res = x * (1.0 + x2 * (-0.3333314528 +
                 x2 * ( 0.1999355085 +
                 x2 * (-0.1420889944 +
                 x2 * ( 0.1065626393 +
                 x2 * (-0.0752896400 +
                 x2 * ( 0.0429096138 +
                 x2 * (-0.0161657367 +
                 x2 *   0.0028662257))))))));

    if (inv) res = RUNTIME_PI_2 - res;
    return neg ? -res : res;
}

double runtime_atan2(double y, double x) {
    if (runtime_isnan(x) || runtime_isnan(y)) return RUNTIME_NAN;
    if (x == 0.0) {
        if (y > 0.0) return RUNTIME_PI_2;
        if (y < 0.0) return -RUNTIME_PI_2;
        return 0.0;
    }
    if (x > 0.0) {
        return runtime_atan(y / x);
    }
    if (y >= 0.0) {
        return runtime_atan(y / x) + RUNTIME_PI;
    }
    return runtime_atan(y / x) - RUNTIME_PI;
}

double runtime_asin(double x) {
    if (x < -1.0 || x > 1.0) {
        return RUNTIME_NAN;
    }
    if (x == 1.0) return RUNTIME_PI_2;
    if (x == -1.0) return -RUNTIME_PI_2;
    return runtime_atan2(x, runtime_fabs(x) == 1.0 ? 0.0 : runtime_hypot(1.0, 0.0) * (1.0 - x * x > 0 ? 1.0 : 0.0));
}

double runtime_acos(double x) {
    if (x < -1.0 || x > 1.0) {
        return RUNTIME_NAN;
    }
    return RUNTIME_PI_2 - runtime_asin(x);
}

double runtime_hypot(double x, double y) {
    x = runtime_fabs(x);
    y = runtime_fabs(y);
    if (x < y) {
        double tmp = x;
        x = y;
        y = tmp;
    }
    if (x == 0.0) return 0.0;
    double t = y / x;

    // sqrt(1 + t^2) * x using Newton iteration for sqrt
    double val = 1.0 + t * t;
    double root = (val > 1.0) ? val * 0.5 : 1.0;
    for (int i = 0; i < 8; ++i) {
        root = 0.5 * (root + val / root);
    }
    return x * root;
}

// Forward declare for exp in hyperbolic functions
double runtime_exp(double x);

double runtime_sinh(double x) {
    if (runtime_fabs(x) > 700.0) {
        return (x > 0) ? RUNTIME_INFINITY : -RUNTIME_INFINITY;
    }
    double ep = runtime_exp(x);
    double em = 1.0 / ep;
    return (ep - em) * 0.5;
}

double runtime_cosh(double x) {
    if (runtime_fabs(x) > 700.0) {
        return RUNTIME_INFINITY;
    }
    double ep = runtime_exp(x);
    double em = 1.0 / ep;
    return (ep + em) * 0.5;
}

double runtime_tanh(double x) {
    if (x > 20.0) return 1.0;
    if (x < -20.0) return -1.0;
    double ep = runtime_exp(x);
    double em = 1.0 / ep;
    return (ep - em) / (ep + em);
}

double runtime_sec(double x) {
    double c = runtime_cos(x);
    return (c == 0.0) ? RUNTIME_INFINITY : (1.0 / c);
}

double runtime_csc(double x) {
    double s = runtime_sin(x);
    return (s == 0.0) ? RUNTIME_INFINITY : (1.0 / s);
}

double runtime_cot(double x) {
    double s = runtime_sin(x);
    return (s == 0.0) ? RUNTIME_INFINITY : (runtime_cos(x) / s);
}
