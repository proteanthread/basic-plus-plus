// FILENAME: algebra.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (math.h)
// NEEDED BY: libengine (ast_internal.h, eval_internal.h)
// NEEDS: libcore (algebra.h, basic.h, basic.c, float_limits.h)
// Freestanding algebra, power, and square root implementations.
//
// ---- Includes ----

#include "runtime/math/algebra.h"
#include "runtime/math/basic.h"
#include "runtime/math/float_limits.h"
#include <stdint.h>

double runtime_sqrt(double x) {
    if (x < 0.0) {
        return RUNTIME_NAN;
    }
    if (x == 0.0 || runtime_isnan(x) || runtime_isinf(x)) {
        return x;
    }

    // Fast initial reciprocal square root estimate via IEEE 754 bit manipulation
    union {
        double d;
        uint64_t i;
    } conv;
    conv.d = x;
    conv.i = (0x5fe6eb50c7b537a9ULL - (conv.i >> 1));
    double y = conv.d;

    // Newton-Raphson iterations: y = y * (1.5 - 0.5 * x * y * y)
    double xhalf = 0.5 * x;
    y = y * (1.5 - xhalf * y * y);
    y = y * (1.5 - xhalf * y * y);
    y = y * (1.5 - xhalf * y * y);
    y = y * (1.5 - xhalf * y * y);

    // Multiply reciprocal root by x to get sqrt(x)
    double res = x * y;
    // One final standard Newton step for exact precision: res = 0.5 * (res + x / res)
    res = 0.5 * (res + x / res);
    return res;
}

double runtime_cbrt(double x) {
    if (x == 0.0 || runtime_isnan(x) || runtime_isinf(x)) {
        return x;
    }
    bool neg = (x < 0.0);
    double ax = runtime_fabs(x);

    // Initial estimate via bit manipulation
    union {
        double d;
        uint64_t i;
    } conv;
    conv.d = ax;
    conv.i = conv.i / 3 + 0x2a9f7893c554e1b3ULL;
    double y = conv.d;

    // Halley's method iterations
    for (int i = 0; i < 6; ++i) {
        double y3 = y * y * y;
        y = y * (y3 + 2.0 * ax) / (2.0 * y3 + ax);
    }

    return neg ? -y : y;
}

double runtime_exp(double x) {
    if (runtime_isnan(x)) return x;
    if (x > 709.78271289338399) return RUNTIME_INFINITY;
    if (x < -708.3964185322641) return 0.0;
    if (x == 0.0) return 1.0;

    // e^x = 2^k * e^r, where k = round(x / ln2), r = x - k * ln2
    double k = runtime_round(x * RUNTIME_LOG2E);
    int ik = (int)k;
    double r = (x - k * 0.69314718055994528623) - k * 2.3190468138462995584e-17;

    // Horner's evaluation of Taylor series for exp(r) on [-ln2/2, ln2/2]
    double sum = 1.0 / 87178291200.0; // 1/14!
    sum = sum * r + 1.0 / 6227020800.0; // 1/13!
    sum = sum * r + 1.0 / 479001600.0;  // 1/12!
    sum = sum * r + 1.0 / 39916800.0;   // 1/11!
    sum = sum * r + 1.0 / 3628800.0;    // 1/10!
    sum = sum * r + 1.0 / 362880.0;     // 1/9!
    sum = sum * r + 1.0 / 40320.0;      // 1/8!
    sum = sum * r + 1.0 / 5040.0;       // 1/7!
    sum = sum * r + 1.0 / 720.0;        // 1/6!
    sum = sum * r + 1.0 / 120.0;        // 1/5!
    sum = sum * r + 1.0 / 24.0;         // 1/4!
    sum = sum * r + 1.0 / 6.0;          // 1/3!
    sum = sum * r + 0.5;                // 1/2!
    sum = sum * r + 1.0;                // 1/1!
    double exp_r = sum * r + 1.0;       // 1/0!

    return runtime_ldexp(exp_r, ik);
}

double runtime_log(double x) {
    if (x < 0.0) return RUNTIME_NAN;
    if (x == 0.0) return -RUNTIME_INFINITY;
    if (runtime_isinf(x)) return x;
    if (runtime_isnan(x)) return x;
    if (x == 1.0) return 0.0;

    int exp;
    double m = runtime_frexp(x, &exp);
    // m is in [0.5, 1.0). Scale so m is in [1/sqrt(2), sqrt(2)) ~ [0.70710678, 1.41421356)
    if (m < RUNTIME_SQRT1_2) {
        m *= 2.0;
        exp -= 1;
    }

    // z = (m - 1) / (m + 1), |z| < 0.171572875
    double z = (m - 1.0) / (m + 1.0);
    double z2 = z * z;

    // Series: log(m) = 2z * (1 + z^2/3 + z^4/5 + z^6/7 + z^8/9 + z^10/11 + z^12/13 + z^14/15 + z^16/17 + z^18/19 + z^20/21)
    double poly = z2 * (1.0 / 21.0);
    poly = (poly + 1.0 / 19.0) * z2;
    poly = (poly + 1.0 / 17.0) * z2;
    poly = (poly + 1.0 / 15.0) * z2;
    poly = (poly + 1.0 / 13.0) * z2;
    poly = (poly + 1.0 / 11.0) * z2;
    poly = (poly + 1.0 / 9.0) * z2;
    poly = (poly + 1.0 / 7.0) * z2;
    poly = (poly + 1.0 / 5.0) * z2;
    poly = (poly + 1.0 / 3.0) * z2;
    poly = poly + 1.0;

    double log_m = 2.0 * z * poly;
    return (double)exp * RUNTIME_LN2 + log_m;
}


double runtime_log10(double x) {
    return runtime_log(x) * RUNTIME_LOG10E;
}

double runtime_log2(double x) {
    return runtime_log(x) * RUNTIME_LOG2E;
}

double runtime_pow(double base, double exp) {
    if (runtime_isnan(base) || runtime_isnan(exp)) return RUNTIME_NAN;
    if (exp == 0.0) return 1.0;
    if (base == 1.0) return 1.0;
    if (base == 0.0) {
        if (exp > 0.0) return 0.0;
        return RUNTIME_INFINITY;
    }

    // Integer exponent fast path
    if (exp == (double)(int64_t)exp) {
        int64_t iexp = (int64_t)exp;
        bool neg = (iexp < 0);
        uint64_t uexp = neg ? (uint64_t)(-iexp) : (uint64_t)iexp;

        double result = 1.0;
        double b = base;
        while (uexp > 0) {
            if (uexp & 1) {
                result *= b;
            }
            b *= b;
            uexp >>= 1;
        }
        return neg ? (1.0 / result) : result;
    }

    // Negative base with fractional exponent is complex/NaN
    if (base < 0.0) {
        return RUNTIME_NAN;
    }

    return runtime_exp(exp * runtime_log(base));
}
