 // ---
 // BASIC++ Interpreter - builtins_math.c
 // ---
 //
 // Math function handlers for the built-in function registry.
 //
 // Contains all FCAT_MATH handlers: trigonometric, logarithmic,
 // rounding, comparison, and complex number functions.
 //
//
// HOW TO EXTEND:
//   To add a new built-in function:
//   1. Write a handler: BValue my_func(BValue *args, int argc, void *ctx)
//   2. Register it in the init function with funcreg_add().
//   3. Specify min/max argument counts and return type.
//
// TROUBLESHOOTING:
//   - Wrong arg count: check min_args/max_args in registration.
//   - Type mismatch: use bval_to_float/bval_to_int for conversion.
 // ---

#include <math.h>
#include "builtins.h"
#include "runtime.h"
#include "dialect.h"
#include "value.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)

 // ABS(x) - Absolute value.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
 // Works on integers and floats.
BValue builtin_abs(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_abs(&args[0], 0);
}

 // SGN(x) - Sign function.
 // Returns -1, 0, or 1.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_sgn(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_sgn(&args[0], 0);
}

 // INT(x) - Truncate to integer.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_int_func(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_int_func(&args[0], 0);
}

 // SQR(x) - Square root.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_sqr(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_sqr(&args[0], 0);
}

 // SIN(x) - Sine.
 // If OPTION ANGLE DEGREES is active, x is in degrees.
 // Otherwise x is in radians (default).
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_sin(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 if (state->angle_degrees) {
 double x = bval_to_float(&args[0]) * DEG_TO_RAD;
 return bval_float(sin(x));
 }
 return bval_sin(&args[0], 0);
}

 // COS(x) - Cosine.
 // Degree/radian mode controlled by OPTION ANGLE.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_cos(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 if (state->angle_degrees) {
 double x = bval_to_float(&args[0]) * DEG_TO_RAD;
 return bval_float(cos(x));
 }
 return bval_cos(&args[0], 0);
}

 // TAN(x) - Tangent.
 // Degree/radian mode controlled by OPTION ANGLE.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_tan(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 if (state->angle_degrees) {
 double x = bval_to_float(&args[0]) * DEG_TO_RAD;
 return bval_float(tan(x));
 }
 return bval_tan(&args[0], 0);
}

 // ATN(x) - Arctangent.
 // If OPTION ANGLE DEGREES, returns result in degrees.
 // Otherwise returns radians (default).
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_atn(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 (void)argc;
 if (state->angle_degrees) {
 double x = bval_to_float(&args[0]);
 return bval_float(atan(x) * RAD_TO_DEG);
 }
 return bval_atn(&args[0], 0);
}

 // LOG(x) - Natural logarithm.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_log(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_log(&args[0], 0);
}

 // EXP(x) - e raised to x.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_exp(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_exp(&args[0], 0);
}

 // FIX(x) - Truncate toward zero.
 // Unlike INT (floor), FIX truncates:
 // FIX(3.7) = 3, FIX(-3.7) = -3
 // INT(3.7) = 3, INT(-3.7) = -4
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_fix(BValue *args, int argc, void *rt)
{
 double x;
 (void)argc; (void)rt;
 x = bval_to_float(&args[0]);
 if (x >= 0.0)
 return bval_int((long)x);
 else
 return bval_int(-((long)(-x)));
}

 // COMPLEX(real, imag) - Create complex number.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_complex(BValue *args, int argc, void *rt)
{
 double re, im;
 (void)argc; (void)rt;
 re = bval_to_float(&args[0]);
 im = bval_to_float(&args[1]);
 return bval_complex(re, im);
}

 // REAL(z) - Extract real part of complex number.
 // For non-complex numerics, returns the value itself.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_real(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0]))
  return bval_float(args[0].v.cval.real);
 return bval_float(bval_to_float(&args[0]));
}

 // IMAG(z) - Extract imaginary part of complex number.
 // For non-complex numerics, returns 0.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_imag(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0]))
  return bval_float(args[0].v.cval.imag);
 return bval_float(0.0);
}
 // CONJ(z) - Complex conjugate.
 // Returns the conjugate of a complex number (negates imaginary part).
 // For non-complex numerics, returns the value itself.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_conj(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0]))
  return bval_complex(args[0].v.cval.real,
   -args[0].v.cval.imag);
 return bval_float(bval_to_float(&args[0]));
}

 // CABS(z) - Complex absolute value (magnitude).
 // Returns sqrt(real^2 + imag^2).
 // For non-complex numerics, returns ABS.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_cabs(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0])) {
  double re = args[0].v.cval.real;
  double im = args[0].v.cval.imag;
  return bval_float(sqrt(re * re + im * im));
 }
 return bval_float(fabs(bval_to_float(&args[0])));
}

 // CSQR(z) - Complex square root.
 // sqrt(a+bi) = sqrt(r)*(cos(t/2) + i*sin(t/2))
 // where r = |z|, t = arg(z).
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_csqr(BValue *args, int argc, void *rt)
{
 double a, b, r, sr, t;
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0])) {
  a = args[0].v.cval.real;
  b = args[0].v.cval.imag;
 } else {
  a = bval_to_float(&args[0]);
  b = 0.0;
 }
 r = sqrt(a * a + b * b);
 sr = sqrt(r);
 t = atan2(b, a);
 return bval_complex(sr * cos(t / 2.0), sr * sin(t / 2.0));
}

 // CEXP(z) - Complex exponential.
 // e^(a+bi) = e^a * (cos(b) + i*sin(b))
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_cexp(BValue *args, int argc, void *rt)
{
 double a, b, ea;
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0])) {
  a = args[0].v.cval.real;
  b = args[0].v.cval.imag;
 } else {
  a = bval_to_float(&args[0]);
  b = 0.0;
 }
 ea = exp(a);
 return bval_complex(ea * cos(b), ea * sin(b));
}

 // CLOG(z) - Complex natural logarithm.
 // ln(a+bi) = ln(|z|) + i*arg(z)
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_clog(BValue *args, int argc, void *rt)
{
 double a, b, r;
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0])) {
  a = args[0].v.cval.real;
  b = args[0].v.cval.imag;
 } else {
  a = bval_to_float(&args[0]);
  b = 0.0;
 }
 r = sqrt(a * a + b * b);
 if (r == 0.0) return bval_complex(0.0, 0.0);
 return bval_complex(log(r), atan2(b, a));
}

 // CARG(z) - Complex argument (phase angle in radians).
 // arg(a+bi) = atan2(b, a)
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_carg(BValue *args, int argc, void *rt)
{
 double a, b;
 (void)argc; (void)rt;
 if (bval_is_complex(&args[0])) {
  a = args[0].v.cval.real;
  b = args[0].v.cval.imag;
 } else {
  a = bval_to_float(&args[0]);
  b = 0.0;
 }
 return bval_float(atan2(b, a));
}

 // CPOW(z, w) - Complex power: z^w = e^(w * ln(z)).
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_cpow(BValue *args, int argc, void *rt)
{
 double za, zb, wa, wb;
 double lr, lt; // ln(z) components
 double ra, rb; // w * ln(z)
 double er; // e^ra
 (void)argc; (void)rt;

 // Extract z
 if (bval_is_complex(&args[0])) {
  za = args[0].v.cval.real;
  zb = args[0].v.cval.imag;
 } else {
  za = bval_to_float(&args[0]);
  zb = 0.0;
 }

 // Extract w
 if (bval_is_complex(&args[1])) {
  wa = args[1].v.cval.real;
  wb = args[1].v.cval.imag;
 } else {
  wa = bval_to_float(&args[1]);
  wb = 0.0;
 }

 // ln(z) = ln(|z|) + i*arg(z)
 {
 double r = sqrt(za * za + zb * zb);
 if (r == 0.0) return bval_complex(0.0, 0.0);
 lr = log(r);
 lt = atan2(zb, za);
 }

 // w * ln(z) = (wa+wbi)(lr+lti)
 ra = wa * lr - wb * lt;
 rb = wa * lt + wb * lr;

 // e^(ra+rbi)
 er = exp(ra);
 return bval_complex(er * cos(rb), er * sin(rb));
}

 // MIN(a, b, ...) - Minimum value.
 // Returns the smallest of two or more numeric arguments.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_min(BValue *args, int argc, void *rt)
{
 double result;
 int i;
 (void)rt;
 result = bval_to_float(&args[0]);
 for (i = 1; i < argc; i++) {
 double v = bval_to_float(&args[i]);
 if (v < result) result = v;
 }
 return bval_float(result);
}

 // MAX(a, b, ...) - Maximum value.
 // Returns the largest of two or more numeric arguments.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_max(BValue *args, int argc, void *rt)
{
 double result;
 int i;
 (void)rt;
 result = bval_to_float(&args[0]);
 for (i = 1; i < argc; i++) {
 double v = bval_to_float(&args[i]);
 if (v > result) result = v;
 }
 return bval_float(result);
}

 // AVG(a, b, ...) - Arithmetic average (mean).
 // Returns the sum of all arguments divided by the count.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_avg(BValue *args, int argc, void *rt)
{
 double sum = 0.0;
 int i;
 (void)rt;
 if (argc == 0) return bval_float(0.0);
 for (i = 0; i < argc; i++) {
 sum += bval_to_float(&args[i]);
 }
 return bval_float(sum / (double)argc);
}

 // MED(a, b, ...) - Median value.
 // Returns the middle value when arguments are sorted.
 // If even number of arguments, returns the average of the
 // two middle values.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
 //
 // Uses insertion sort (C89 compatible). Max 64 values.
#define MED_MAX_VALS 64
BValue builtin_med(BValue *args, int argc, void *rt)
{
 double vals[MED_MAX_VALS];
 int n, i, j;
 double key;
 (void)rt;
 if (argc == 0) return bval_float(0.0);
 n = argc;
 if (n > MED_MAX_VALS) n = MED_MAX_VALS;
 for (i = 0; i < n; i++) {
 vals[i] = bval_to_float(&args[i]);
 }
 // Insertion sort
 for (i = 1; i < n; i++) {
 key = vals[i];
 j = i - 1;
 while (j >= 0 && vals[j] > key) {
 vals[j + 1] = vals[j];
 j--;
 }
 vals[j + 1] = key;
 }
 if (n % 2 == 1) {
 return bval_float(vals[n / 2]);
 }
 return bval_float((vals[n / 2 - 1] + vals[n / 2]) / 2.0);
}

 // ROUND(x, n) - Round to N decimal places.
 // ROUND(3.14159, 2) = 3.14
 // ROUND(2.5, 0) = 3
 // ROUND(1234, -2) = 1200  (negative n rounds to tens/hundreds)
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_round(BValue *args, int argc, void *rt)
{
 double x, factor;
 int n;
 (void)rt;
 x = bval_to_float(&args[0]);
 n = (argc >= 2) ? (int)bval_to_int(&args[1]) : 0;
 // Compute 10^n
 factor = 1.0;
 if (n > 0) {
 int k;
 for (k = 0; k < n && k < 15; k++) factor *= 10.0;
 } else if (n < 0) {
 int k;
 for (k = 0; k < -n && k < 15; k++) factor /= 10.0;
 }
 return bval_float(floor(x * factor + 0.5) / factor);
}

 // ASIN(x) - Arcsine.
 // SUPER BASIC (Tymshare) extended math.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_asin(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(asin(bval_to_float(&args[0])));
}

 // ACOS(x) - Arccosine.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_acos(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(acos(bval_to_float(&args[0])));
}

 // SINH(x) - Hyperbolic sine.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_sinh(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(sinh(bval_to_float(&args[0])));
}

 // COSH(x) - Hyperbolic cosine.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_cosh(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(cosh(bval_to_float(&args[0])));
}

 // TANH(x) - Hyperbolic tangent.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_tanh(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(tanh(bval_to_float(&args[0])));
}

 // LOG10(x) - Logarithm base 10.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_log10(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(log10(bval_to_float(&args[0])));
}

 // LOG2(x) - Logarithm base 2.
 // Computed as LOG(x) / LOG(2) for C89 compatibility.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
#define BPP_LN2 0.69314718055994530942
BValue builtin_log2(BValue *args, int argc, void *rt)
{
 (void)argc; (void)rt;
 return bval_float(log(bval_to_float(&args[0])) / BPP_LN2);
}

 // COMP(a, b) - Compare two values.
 // Returns -1 if a < b, 0 if a = b, +1 if a > b.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_comp(BValue *args, int argc, void *rt)
{
 double a, b;
 (void)argc; (void)rt;
 a = bval_to_float(&args[0]);
 b = bval_to_float(&args[1]);
 if (a < b) return bval_float(-1.0);
 if (a > b) return bval_float(1.0);
 return bval_float(0.0);
}

 // PDIF(a, b) - Positive difference.
 // Returns MAX(a - b, 0).
 // Category: FCAT_MATH | Safety: FSAFE_PURE
BValue builtin_pdif(BValue *args, int argc, void *rt)
{
 double diff;
 (void)argc; (void)rt;
 diff = bval_to_float(&args[0]) - bval_to_float(&args[1]);
 return bval_float(diff > 0.0 ? diff : 0.0);
}

 // PI - Constant PI (3.14159265358979...).
 // Takes 0 arguments.
 // Category: FCAT_MATH | Safety: FSAFE_PURE
#define BPP_PI 3.14159265358979323846
BValue builtin_pi(BValue *args, int argc, void *rt)
{
 (void)args; (void)argc; (void)rt;
 return bval_float(BPP_PI);
}

 // RND(n) - Random number generation.
 //
 // Dialect-aware behavior:
 //
 // PATB (Palo Alto Tiny BASIC):
 // RND(n) returns a random integer in [1, n].
 //
 // GWBS (GW-BASIC / Dartmouth):
 // RND(n) where n>0: return float in [0, 1)
 // RND(0): repeat last random value
 // RND(n) where n<0: seed with |n|, return float
 //
 // Category: FCAT_MATH | Safety: FSAFE_STATE (modifies RNG seed)
BValue builtin_rnd(BValue *args, int argc, void *rt)
{
 RuntimeState *state = (RuntimeState *)rt;
 long n;
 // GW-BASIC: bare RND is equivalent to RND(1)
 n = (argc > 0) ? bval_to_int(&args[0]) : 1;

 if (dialect_get_config()->has_float) {
 // GW-BASIC mode: return float 0..1
 //
 // RND(n>0) = advance seed, return 0..1
 // RND(0) = return last value (don't advance)
 // RND(n<0) = seed with |n|, advance, return 0..1
 if (n < 0) {
 state->rnd_seed = (unsigned long)(-n);
 }
 if (n != 0) {
 state->rnd_seed =
 (state->rnd_seed * 1103515245UL +
 12345UL) & 0x7FFFFFFFUL;
 // Avoid seed=0 which produces RND=0.
 // ECMA-55: 0 < RND < 1 (exclusive).
 if (state->rnd_seed == 0)
 state->rnd_seed = 1;
 }
 return bval_float(
 (double)state->rnd_seed / 2147483648.0);
 }

 // PATB mode: return integer 1..n
 return bval_int(runtime_rnd(state, n));
}
