// FILENAME: float_limits.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (algebra.c, basic.c, float_parse.c, math.h, trig.c)
// NEEDS: platform, memory
// Freestanding floating-point numerical limits and constants.
//
// ---- Includes ----

#ifndef RUNTIME_MATH_FLOAT_LIMITS_H
#define RUNTIME_MATH_FLOAT_LIMITS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RUNTIME_PI          3.141592653589793238462643383279502884
#define RUNTIME_PI_2        1.570796326794896619231321691639751442
#define RUNTIME_PI_4        0.785398163397448309615660845819875721
#define RUNTIME_2_PI        6.283185307179586476925286766559005768
#define RUNTIME_E           2.718281828459045235360287471352662497
#define RUNTIME_LN2         0.693147180559945309417232121458176568
#define RUNTIME_LN10        2.302585092994045684017991454684364208
#define RUNTIME_LOG2E       1.442695040888963407359924681001892137
#define RUNTIME_LOG10E      0.434294481903251827651128918916605082
#define RUNTIME_SQRT2       1.414213562373095048801688724209698079
#define RUNTIME_SQRT1_2     0.707106781186547524400844362104849039

#define RUNTIME_DBL_EPSILON 2.2204460492503131e-16
#define RUNTIME_DBL_MAX     1.7976931348623157e+308
#define RUNTIME_DBL_MIN     2.2250738585072014e-308

static inline double runtime_make_inf(void) {

    union { uint64_t u; double d; } val;
    val.u = 0x7FF0000000000000ULL;
    return val.d;
}

static inline double runtime_make_nan(void) {
    union { uint64_t u; double d; } val;
    val.u = 0x7FF8000000000000ULL;
    return val.d;
}

#define RUNTIME_INFINITY    (runtime_make_inf())
#define RUNTIME_NAN         (runtime_make_nan())
#define RUNTIME_HUGE_VAL    (RUNTIME_INFINITY)


#ifdef __cplusplus
}
#endif

#endif // RUNTIME_MATH_FLOAT_LIMITS_H
