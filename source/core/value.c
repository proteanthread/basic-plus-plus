// ---
// BASIC++ Interpreter - value.c
// ---
//
// Implementation of the tagged BValue type and all value operations.
//
// PURPOSE:
//   BValue is the universal value type for the BASIC++ interpreter.
//   Every variable, expression result, function return, and constant
//   is a BValue. This file provides constructors, type queries,
//   coercion, arithmetic, comparison, and string operations.
//
// HOW IT WORKS:
//   BValue is a tagged union with four possible types:
//
//     VAL_INTEGER  -> long integer (32-bit on most platforms)
//     VAL_FLOAT    -> double-precision floating point (64-bit IEEE 754)
//     VAL_STRING   -> (char *data, int length) pointer into string pool
//     VAL_COMPLEX  -> (double real, double imag) complex number
//
//   Type Coercion Rules (classic BASIC):
//     Integer + Float   -> Float  (auto-promotion)
//     Integer + Integer -> Integer (stays integer)
//     Any + Complex     -> Complex (auto-promotion)
//     String + String   -> concatenation (via bval_concat only)
//     String + Numeric  -> ERR_WHAT (type mismatch)
//     VAL("123")        -> Integer 123
//     VAL("3.14")       -> Float 3.14
//
//   Division always produces:
//     Integer / Integer -> Integer (truncated, like C)
//     Any float operand -> Float result
//     Division by zero  -> ERR_HOW
//
// HOW TO EXTEND / CUSTOMIZE:
//   Adding a new value type (e.g., VAL_BYTE, VAL_INT64):
//   1. Add the type constant to the ValType enum in value.h.
//   2. Add a union member in the BValue struct (value.h).
//   3. Add a constructor function (bval_xxx) here.
//   4. Add a type query (bval_is_xxx) here.
//   5. Update bval_to_int(), bval_to_float(), bval_to_string_buf()
//      to handle the new type.
//   6. Update all arithmetic functions (bval_add, bval_sub, etc.)
//      to handle the new type in the promotion hierarchy.
//   7. Update bval_compare() for comparison support.
//
//   Changing arithmetic precision:
//   - Replace 'double' with 'long double' for extended precision.
//   - Change 'long' to 'int64_t' for 64-bit integers.
//   - Update all coercion functions and format strings accordingly.
//
//   Adding a new math function:
//   1. Add the function below (follow bval_sin as a template).
//   2. Register it in builtins_math.c for the function name.
//   3. Add the BASIC keyword in lexer.c with appropriate DFLAG.
//
// FINE-TUNING:
//   - Integer overflow: NOT checked. BASIC convention is to wrap.
//     To add overflow detection, check before each arithmetic op.
//   - Float formatting: "%G" format removes trailing zeros.
//     Change to "%.15G" for more precision, or "%.6f" for fixed.
//   - String-to-number conversion: VAL() uses strtol/strtod.
//     Unparseable strings return 0 (GW-BASIC convention).
//   - Array subscript rounding: Uses ECMA-55 s6.4 "round to nearest."
//     Change bval_to_subscript() for different rounding modes.
//
// TROUBLESHOOTING:
//   - "WHAT?" on arithmetic:
//     One operand is a string. Use VAL() to convert strings to
//     numbers before arithmetic, or use bval_concat for strings.
//
//   - "HOW?" on division/MOD:
//     Division by zero. Check the divisor before the operation.
//     LOG() of zero or negative also raises HOW.
//
//   - "HOW?" on CHR$():
//     Code is outside 0-255 range. Only ASCII/Latin-1 is supported.
//
//   - Wrong integer results from float operations:
//     Integer division truncates toward zero (C behavior).
//     Use INT() for floor division, or work in float mode.
//
// PERFORMANCE:
//   - All constructors are O(1) -- fill struct fields.
//   - Type queries are O(1) -- single comparison.
//   - Arithmetic is O(1) -- one or two math operations.
//   - String operations are O(n) -- memcpy of string data.
//   - Comparison is O(min(len_a, len_b)) for strings.
//   - Complex arithmetic adds ~2x cost vs real arithmetic.
//
// MINIMALIZATION:
//   For a minimal build (PATB-only, integer-only):
//   - Remove VAL_FLOAT support: delete all float branches and
//     the math.h dependency. Saves ~3 KB code.
//   - Remove VAL_COMPLEX support: delete complex branches.
//     Saves ~2 KB code.
//   - Remove VAL_STRING support: delete string operations.
//     Saves ~4 KB code. (Requires removing string variables.)
//   - Keep VAL_INTEGER only for Tiny BASIC compatibility.
//
// DEPENDENCIES:
//   - value.h       (BValue struct, ValType enum, MAX_LINE_LENGTH)
//   - errors.h      (error_raise, error_occurred)
//   - stringpool.h  (strpool_alloc for string operations)
//   - math.h        (floor, ceil, fabs, sqrt, sin, cos, tan, etc.)
//
// C17 COMPLIANCE:
//   - Uses only C17 standard library functions.
//   - No platform-specific code.
//   - Compiles cleanly on gcc -std=c17 and MSVC /std:c17.
//
// ---

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "value.h"
#include "errors.h"
#include "stringpool.h"

// -----------------------------------------------------------------
// Constructors
// -----------------------------------------------------------------
// Each constructor creates a BValue with the given type and data.
// These are the ONLY way to create BValues -- never fill the struct
// fields directly (that bypasses the type tag).
// -----------------------------------------------------------------

// bval_int - Create an integer BValue.
//
// Parameters:
//   val - the integer value (long, typically 32-bit)
//
// Returns:
//   A BValue with type=VAL_INTEGER and v.ival=val.
//
BValue bval_int(long val)
{
    BValue r;
    r.type = VAL_INTEGER;
    r.v.ival = val;
    return r;
}

// bval_float - Create a floating-point BValue.
//
// Parameters:
//   val - the float value (double, 64-bit IEEE 754)
//
// Returns:
//   A BValue with type=VAL_FLOAT and v.fval=val.
//
BValue bval_float(double val)
{
    BValue r;
    r.type = VAL_FLOAT;
    r.v.fval = val;
    return r;
}

// bval_string - Create a string BValue.
//
// Parameters:
//   data   - pointer to string data in the string pool (NOT owned)
//   length - number of bytes (NOT null-terminated)
//
// Returns:
//   A BValue with type=VAL_STRING and v.sval={data, length}.
//
// Note: The data pointer must be stable for the lifetime of this
// BValue. It points into the string pool managed by stringpool.c.
// If the pool is compacted, the pointer is updated automatically
// by strpool_compact().
//
BValue bval_string(char *data, int length)
{
    BValue r;
    r.type = VAL_STRING;
    r.v.sval.data = data;
    r.v.sval.length = length;
    return r;
}

// bval_complex - Create a complex number BValue.
//
// Parameters:
//   real - real part (double)
//   imag - imaginary part (double)
//
// Returns:
//   A BValue with type=VAL_COMPLEX and v.cval={real, imag}.
//
// Note: Complex numbers are a SUPER BASIC (SDS 940) feature.
// They are available in all dialects in BASIC++ but were
// historically only available in SUPER BASIC and later dialects.
//
BValue bval_complex(double real, double imag)
{
    BValue r;
    r.type = VAL_COMPLEX;
    r.v.cval.real = real;
    r.v.cval.imag = imag;
    return r;
}

// -----------------------------------------------------------------
// Type Queries
// -----------------------------------------------------------------
// Each returns 1 (true) if the BValue is of the specified type,
// 0 (false) otherwise. These are simple equality checks on the
// type tag.
// -----------------------------------------------------------------

int bval_is_int(const BValue *v)     { return v->type == VAL_INTEGER; }
int bval_is_float(const BValue *v)   { return v->type == VAL_FLOAT; }
int bval_is_string(const BValue *v)  { return v->type == VAL_STRING; }
int bval_is_complex(const BValue *v) { return v->type == VAL_COMPLEX; }

// bval_is_numeric - Check if a BValue is any numeric type.
//
// Returns 1 for VAL_INTEGER, VAL_FLOAT, or VAL_COMPLEX.
// Returns 0 for VAL_STRING.
//
// Used by arithmetic operations to validate operands before
// performing calculations.
//
int bval_is_numeric(const BValue *v)
{
    return v->type == VAL_INTEGER || v->type == VAL_FLOAT ||
        v->type == VAL_COMPLEX;
}

// -----------------------------------------------------------------
// Type Coercion
// -----------------------------------------------------------------
// These functions convert a BValue from one type to another.
// They follow classic BASIC rules:
//   - Integer passes through unchanged.
//   - Float is truncated (for to_int) or passes through.
//   - String is parsed as a number (VAL-like behavior).
//   - Complex uses the real part for scalar conversion.
// -----------------------------------------------------------------

// bval_to_int - Convert any BValue to a long integer.
//
// Conversion rules:
//   VAL_INTEGER -> direct value
//   VAL_FLOAT   -> truncated toward zero (C cast)
//   VAL_STRING  -> parsed via strtol(); unparseable -> 0
//   VAL_COMPLEX -> truncated real part
//
long bval_to_int(const BValue *v)
{
    switch (v->type) {
    case VAL_INTEGER: return v->v.ival;
    case VAL_FLOAT:   return (long)v->v.fval;
    case VAL_STRING: {
        // Parse string as integer (VAL function behavior).
        // Uses a temporary NUL-terminated buffer since string pool
        // strings are not null-terminated.
        char buf[MAX_LINE_LENGTH + 1];
        int len = v->v.sval.length;
        if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
        memcpy(buf, v->v.sval.data, (size_t)len);
        buf[len] = '\0';
        return strtol(buf, NULL, 10);
    }
    case VAL_COMPLEX:
        return (long)v->v.cval.real;
    }
    return 0;
}

// bval_to_subscript - Round to nearest integer for array subscripts.
//
// ECMA-55 s6.4: "each subscript is rounded to the nearest integer."
// GW-BASIC/QBasic: uses CINT() for subscripts (round to nearest).
//
// This differs from bval_to_int() which truncates toward zero.
// For array indexing, rounding is more intuitive:
//   A(2.7) -> A(3), not A(2)
//   A(-1.3) -> A(-1), not A(-1)
//
// Rounding rule:
//   Positive: floor(x + 0.5)
//   Negative: ceil(x - 0.5)
//
long bval_to_subscript(const BValue *v)
{
    if (v->type == VAL_FLOAT) {
        double d = v->v.fval;
        return (long)(d >= 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
    }
    return bval_to_int(v);
}

// bval_to_float - Convert any BValue to a double.
//
// Conversion rules:
//   VAL_INTEGER -> promoted to double (no precision loss for <2^53)
//   VAL_FLOAT   -> direct value
//   VAL_STRING  -> parsed via strtod(); unparseable -> 0.0
//   VAL_COMPLEX -> real part only (imaginary discarded)
//
double bval_to_float(const BValue *v)
{
    switch (v->type) {
    case VAL_INTEGER: return (double)v->v.ival;
    case VAL_FLOAT:   return v->v.fval;
    case VAL_STRING: {
        char buf[MAX_LINE_LENGTH + 1];
        int len = v->v.sval.length;
        if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
        memcpy(buf, v->v.sval.data, (size_t)len);
        buf[len] = '\0';
        return strtod(buf, NULL);
    }
    case VAL_COMPLEX:
        return v->v.cval.real;
    }
    return 0.0;
}

// bval_to_string_buf - Format a BValue as a string into a buffer.
//
// Parameters:
//   v       - the value to format
//   buf     - output buffer (must be at least bufsize bytes)
//   bufsize - size of the output buffer
//
// Returns:
//   Number of characters written (excluding NUL terminator).
//
// Format rules:
//   VAL_INTEGER -> "%ld" (e.g., "42")
//   VAL_FLOAT   -> "%G"  (e.g., "3.14", removes trailing zeros)
//   VAL_STRING  -> copied directly (truncated to bufsize-1)
//   VAL_COMPLEX -> "(R+Ii)" or "(R-Ii)" (e.g., "(3+4i)")
//
// How to customize output format:
//   Change the printf format strings below. For fixed-point output,
//   use "%.6f" instead of "%G". For more precision, use "%.15G".
//
int bval_to_string_buf(const BValue *v, char *buf, int bufsize)
{
    switch (v->type) {
    case VAL_INTEGER:
        return sprintf(buf, "%ld", v->v.ival);
    case VAL_FLOAT:
        return sprintf(buf, "%G", v->v.fval);
    case VAL_STRING: {
        int len = v->v.sval.length;
        if (len >= bufsize) len = bufsize - 1;
        memcpy(buf, v->v.sval.data, (size_t)len);
        buf[len] = '\0';
        return len;
    }
    case VAL_COMPLEX:
        if (v->v.cval.imag >= 0.0)
            return sprintf(buf, "(%G+%Gi)",
                v->v.cval.real, v->v.cval.imag);
        else
            return sprintf(buf, "(%G%Gi)",
                v->v.cval.real, v->v.cval.imag);
    }
    buf[0] = '\0';
    return 0;
}

// -----------------------------------------------------------------
// Arithmetic Operations
// -----------------------------------------------------------------
// All arithmetic follows the type promotion hierarchy:
//   Integer < Float < Complex
//
// If either operand is Complex, the result is Complex.
// If either operand is Float, the result is Float.
// If both are Integer, the result is Integer.
//
// All operations check that both operands are numeric and raise
// ERR_WHAT if a string is used in arithmetic context.
// -----------------------------------------------------------------

// check_numeric - Validate that both operands are numeric.
//
// Returns 1 if both are numeric (Integer, Float, or Complex).
// Returns 0 and raises ERR_WHAT if either is a string.
//
static int check_numeric(const BValue *a, const BValue *b, int line_num)
{
    if (!bval_is_numeric(a) || !bval_is_numeric(b)) {
        error_raise(ERR_WHAT, line_num);
        return 0;
    }
    return 1;
}

// should_use_complex - Check if complex arithmetic is needed.
//
// Returns 1 if either operand is VAL_COMPLEX.
//
static int should_use_complex(const BValue *a, const BValue *b)
{
    return (a->type == VAL_COMPLEX || b->type == VAL_COMPLEX);
}

// get_complex - Extract real and imaginary parts from any numeric.
//
// For complex values, extracts both parts directly.
// For integer/float, sets real = numeric value, imag = 0.
//
static void get_complex(const BValue *v,
    double *re, double *im)
{
    if (v->type == VAL_COMPLEX) {
        *re = v->v.cval.real;
        *im = v->v.cval.imag;
    } else {
        *re = bval_to_float(v);
        *im = 0.0;
    }
}

// should_use_float - Check if float arithmetic is needed.
//
// Returns 1 if either operand is VAL_FLOAT.
// (Complex is checked first by should_use_complex.)
//
static int should_use_float(const BValue *a, const BValue *b)
{
    return (a->type == VAL_FLOAT || b->type == VAL_FLOAT);
}

// bval_add - Addition: a + b
//
// Type promotion: Integer -> Float -> Complex.
// String operands raise ERR_WHAT (use bval_concat for strings).
//
BValue bval_add(const BValue *a, const BValue *b, int line_num)
{
    // String concatenation is handled by bval_concat, not here.
    // If both are strings, the parser should call bval_concat.
    if (!check_numeric(a, b, line_num)) return bval_int(0);

    if (should_use_complex(a, b)) {
        double ar, ai, br, bi;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        return bval_complex(ar + br, ai + bi);
    }
    if (should_use_float(a, b)) {
        return bval_float(bval_to_float(a) + bval_to_float(b));
    }
    return bval_int(a->v.ival + b->v.ival);
}

// bval_sub - Subtraction: a - b
//
BValue bval_sub(const BValue *a, const BValue *b, int line_num)
{
    if (!check_numeric(a, b, line_num)) return bval_int(0);

    if (should_use_complex(a, b)) {
        double ar, ai, br, bi;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        return bval_complex(ar - br, ai - bi);
    }
    if (should_use_float(a, b)) {
        return bval_float(bval_to_float(a) - bval_to_float(b));
    }
    return bval_int(a->v.ival - b->v.ival);
}

// bval_mul - Multiplication: a * b
//
// Complex multiplication uses the identity:
//   (ar + ai*i) * (br + bi*i) = (ar*br - ai*bi) + (ar*bi + ai*br)*i
//
BValue bval_mul(const BValue *a, const BValue *b, int line_num)
{
    if (!check_numeric(a, b, line_num)) return bval_int(0);

    if (should_use_complex(a, b)) {
        // (ar+ai*i)(br+bi*i) = (ar*br-ai*bi)+(ar*bi+ai*br)i
        double ar, ai, br, bi;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        return bval_complex(ar*br - ai*bi,
            ar*bi + ai*br);
    }
    if (should_use_float(a, b)) {
        return bval_float(bval_to_float(a) * bval_to_float(b));
    }
    return bval_int(a->v.ival * b->v.ival);
}

// bval_div - Division: a / b
//
// Division by zero raises ERR_HOW for all types.
// Complex division uses the conjugate method:
//   (ar+ai*i)/(br+bi*i) = ((ar*br+ai*bi) + (ai*br-ar*bi)*i) / (br+bi)
//
BValue bval_div(const BValue *a, const BValue *b, int line_num)
{
    if (!check_numeric(a, b, line_num)) return bval_int(0);

    if (should_use_complex(a, b)) {
        // Complex division using conjugate multiplication
        double ar, ai, br, bi, denom;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        denom = br*br + bi*bi;
        if (denom == 0.0) {
            error_raise(ERR_HOW, line_num);
            return bval_complex(0.0, 0.0);
        }
        return bval_complex(
            (ar*br + ai*bi) / denom,
            (ai*br - ar*bi) / denom);
    }
    if (should_use_float(a, b)) {
        double denom = bval_to_float(b);
        if (denom == 0.0) {
            error_raise(ERR_HOW, line_num);
            return bval_float(0.0);
        }
        return bval_float(bval_to_float(a) / denom);
    }

    // Integer division -- truncates toward zero (C behavior)
    if (b->v.ival == 0) {
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }
    return bval_int(a->v.ival / b->v.ival);
}

// bval_neg - Unary negation: -v
//
BValue bval_neg(const BValue *v, int line_num)
{
    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }
    if (v->type == VAL_COMPLEX) {
        return bval_complex(-v->v.cval.real,
            -v->v.cval.imag);
    }
    if (v->type == VAL_FLOAT) {
        return bval_float(-v->v.fval);
    }
    return bval_int(-v->v.ival);
}

// bval_mod - Modulo: a MOD b
//
// Both operands are converted to integer first (truncated).
// Division by zero raises ERR_HOW.
//
BValue bval_mod(const BValue *a, const BValue *b, int line_num)
{
    long ia, ib;
    if (!check_numeric(a, b, line_num)) return bval_int(0);

    ia = bval_to_int(a);
    ib = bval_to_int(b);
    if (ib == 0) {
        error_raise(ERR_HOW, line_num);
        return bval_int(0);
    }
    return bval_int(ia % ib);
}

// -----------------------------------------------------------------
// Comparison
// -----------------------------------------------------------------

// bval_compare - Three-way comparison of two BValues.
//
// Returns:
//   -1 if a < b
//    0 if a == b
//    1 if a > b
//
// String comparison is lexicographic (memcmp-based).
// Numeric comparison follows float promotion rules.
// Mixed string/numeric raises ERR_WHAT.
//
int bval_compare(const BValue *a, const BValue *b, int line_num)
{
    // String vs String: lexicographic comparison
    if (bval_is_string(a) && bval_is_string(b)) {
        int la = a->v.sval.length;
        int lb = b->v.sval.length;
        int minlen = (la < lb) ? la : lb;
        int cmp = memcmp(a->v.sval.data, b->v.sval.data, (size_t)minlen);
        if (cmp != 0) return (cmp < 0) ? -1 : 1;
        if (la < lb) return -1;
        if (la > lb) return 1;
        return 0;
    }

    // Numeric vs Numeric: promote to float if needed
    if (bval_is_numeric(a) && bval_is_numeric(b)) {
        if (should_use_float(a, b)) {
            double fa = bval_to_float(a);
            double fb = bval_to_float(b);
            if (fa < fb) return -1;
            if (fa > fb) return 1;
            return 0;
        }
        if (a->v.ival < b->v.ival) return -1;
        if (a->v.ival > b->v.ival) return 1;
        return 0;
    }

    // Mixed string/numeric -- type mismatch
    error_raise(ERR_WHAT, line_num);
    return 0;
}

// -----------------------------------------------------------------
// Math Functions (Float)
// -----------------------------------------------------------------
// Each function validates its argument is numeric, extracts a
// double via get_numeric_arg(), performs the math operation, and
// returns a VAL_FLOAT result.
//
// Error behavior:
//   - Non-numeric argument -> ERR_WHAT
//   - Domain error (e.g., LOG of negative) -> ERR_HOW
//
// How to add a new math function:
//   1. Add the function here following the pattern below.
//   2. Register it in builtins_math.c (name -> function mapping).
//   3. Add the BASIC keyword in lexer.c keyword table.
// -----------------------------------------------------------------

// get_numeric_arg - Extract double from a numeric BValue.
// Raises ERR_WHAT if the value is a string.
//
static double get_numeric_arg(const BValue *v, int line_num)
{
    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return 0.0;
    }
    return bval_to_float(v);
}

// bval_abs - ABS(x) -- absolute value.
//
// Integer input -> integer result (avoids float promotion).
// Float input -> float result.
//
BValue bval_abs(const BValue *v, int line_num)
{
    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }
    if (v->type == VAL_FLOAT) {
        return bval_float(fabs(v->v.fval));
    }
    return bval_int(v->v.ival < 0 ? -v->v.ival : v->v.ival);
}

// bval_sgn - SGN(x) -- sign function.
//
// Returns:  1 if x > 0
//           0 if x == 0
//          -1 if x < 0
//
BValue bval_sgn(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_int(0);
    if (d > 0.0) return bval_int(1);
    if (d < 0.0) return bval_int(-1);
    return bval_int(0);
}

// bval_int_func - INT(x) -- floor function.
//
// Returns the greatest integer <= x (not truncation toward zero).
// INT(-2.3) = -3, not -2.
//
BValue bval_int_func(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_int(0);
    return bval_int((long)floor(d));
}

// bval_sqr - SQR(x) -- square root.
//
// Supports complex results:
//   - Complex input: uses polar form sqrt(r) * (cos(/2) + i*sin(/2))
//   - Negative real input: returns pure imaginary (0 + sqrt(|x|)*i)
//   - Positive real input: returns real sqrt
//
BValue bval_sqr(const BValue *v, int line_num)
{
    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_float(0.0);
    }
    // Complex input: sqrt(a+bi) = sqrt(r) * (cos(/2) + i*sin(/2))
    if (v->type == VAL_COMPLEX) {
        double a = v->v.cval.real;
        double b = v->v.cval.imag;
        double r = sqrt(a * a + b * b);
        double sr = sqrt(r);
        double t = atan2(b, a);
        return bval_complex(sr * cos(t / 2.0),
            sr * sin(t / 2.0));
    }
    {
        double d = bval_to_float(v);
        if (d < 0.0) {
            // Negative real -> pure imaginary result
            return bval_complex(0.0, sqrt(-d));
        }
        return bval_float(sqrt(d));
    }
}

// bval_sin - SIN(x) -- sine (radians).
BValue bval_sin(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    return bval_float(sin(d));
}

// bval_cos - COS(x) -- cosine (radians).
BValue bval_cos(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    return bval_float(cos(d));
}

// bval_tan - TAN(x) -- tangent (radians).
BValue bval_tan(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    return bval_float(tan(d));
}

// bval_atn - ATN(x) -- arctangent (returns radians).
BValue bval_atn(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    return bval_float(atan(d));
}

// bval_log - LOG(x) -- natural logarithm (base e).
//
// Domain error: x <= 0 raises ERR_HOW.
//
BValue bval_log(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    if (d <= 0.0) {
        error_raise(ERR_HOW, line_num);
        return bval_float(0.0);
    }
    return bval_float(log(d));
}

// bval_exp - EXP(x) -- e^x (natural exponential).
BValue bval_exp(const BValue *v, int line_num)
{
    double d = get_numeric_arg(v, line_num);
    if (error_occurred()) return bval_float(0.0);
    return bval_float(exp(d));
}

// -----------------------------------------------------------------
// String Functions
// -----------------------------------------------------------------
// These functions implement BASIC string operations. Each validates
// its argument types and raises ERR_WHAT on type mismatch.
//
// All string results are allocated from the string pool (passed
// as void *pool to avoid circular dependency with stringpool.h
// in value.h).
//
// How to add a new string function:
//   1. Add the function here following the pattern below.
//   2. Register it in builtins_string.c.
//   3. Add the BASIC keyword in lexer.c keyword table.
// -----------------------------------------------------------------

// bval_len - LEN(s$) -- return the length of a string.
//
// Returns an integer BValue with the string's byte length.
// Raises ERR_WHAT if the argument is not a string.
//
BValue bval_len(const BValue *v, int line_num)
{
    if (!bval_is_string(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }
    return bval_int((long)v->v.sval.length);
}

// bval_asc - ASC(s$) -- return the ASCII code of the first character.
//
// Returns an integer 0-255. Raises ERR_WHAT if the string is
// empty or the argument is not a string.
//
BValue bval_asc(const BValue *v, int line_num)
{
    if (!bval_is_string(v) || v->v.sval.length == 0) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }
    return bval_int((long)(unsigned char)v->v.sval.data[0]);
}

// bval_val - VAL(s$) -- convert string to number.
//
// Tries integer first (strtol), then float (strtod).
// Returns 0 if the string cannot be parsed as a number
// (GW-BASIC convention: VAL("abc") = 0).
//
BValue bval_val(const BValue *v, int line_num)
{
    char buf[MAX_LINE_LENGTH + 1];
    char *endptr;
    long ival;
    int len;

    if (!bval_is_string(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_int(0);
    }

    // Copy string to NUL-terminated buffer for strtol/strtod
    len = v->v.sval.length;
    if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
    memcpy(buf, v->v.sval.data, (size_t)len);
    buf[len] = '\0';

    // Try integer first
    ival = strtol(buf, &endptr, 10);
    if (*endptr == '\0' || *endptr == ' ') {
        return bval_int(ival);
    }

    // Try float (decimal point found)
    if (*endptr == '.') {
        double fval = strtod(buf, &endptr);
        return bval_float(fval);
    }

    return bval_int(0);  // unparseable -> 0
}

// bval_chr - CHR$(n) -- convert ASCII code to single-character string.
//
// Parameters:
//   v    - numeric BValue with the character code (0-255)
//   pool - string pool to allocate the result in
//
// Error behavior:
//   ERR_WHAT if v is not numeric.
//   ERR_HOW if code is outside 0-255.
//   ERR_SORRY if string pool is exhausted.
//
BValue bval_chr(const BValue *v, int line_num, void *pool)
{
    long code;
    char *ptr;

    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    code = bval_to_int(v);
    if (code < 0 || code > 255) {
        error_raise(ERR_HOW, line_num);
        return bval_string(NULL, 0);
    }

    ptr = strpool_alloc((StringPool *)pool, 1);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }
    ptr[0] = (char)code;
    return bval_string(ptr, 1);
}

// bval_str - STR$(n) -- convert number to string.
//
// Parameters:
//   v    - numeric BValue to convert
//   pool - string pool to allocate the result in
//
BValue bval_str(const BValue *v, int line_num, void *pool)
{
    char buf[64];
    int len;
    char *ptr;

    if (!bval_is_numeric(v)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    len = bval_to_string_buf(v, buf, sizeof(buf));
    ptr = strpool_alloc((StringPool *)pool, len);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }
    memcpy(ptr, buf, (size_t)len);
    return bval_string(ptr, len);
}

// bval_left - LEFT$(s$, n) -- extract leftmost n characters.
//
// If n > LEN(s$), returns the entire string.
// If n < 0, treated as 0 (returns empty string).
//
BValue bval_left(const BValue *s, const BValue *n, int line_num,
    void *pool)
{
    int count;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(n)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    count = (int)bval_to_int(n);
    if (count < 0) count = 0;
    if (count > s->v.sval.length) count = s->v.sval.length;

    ptr = strpool_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data, (size_t)count);
    return bval_string(ptr, count);
}

// bval_right - RIGHT$(s$, n) -- extract rightmost n characters.
//
// If n > LEN(s$), returns the entire string.
// If n < 0, treated as 0 (returns empty string).
//
BValue bval_right(const BValue *s, const BValue *n, int line_num,
    void *pool)
{
    int count;
    int start;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(n)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    count = (int)bval_to_int(n);
    if (count < 0) count = 0;
    if (count > s->v.sval.length) count = s->v.sval.length;
    start = s->v.sval.length - count;

    ptr = strpool_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data + start, (size_t)count);
    return bval_string(ptr, count);
}

// bval_mid - MID$(s$, start, length) -- extract substring.
//
// BASIC uses 1-based indexing: MID$("HELLO", 2, 3) = "ELL".
// The start parameter is converted to 0-based internally.
//
// Boundary handling:
//   - start < 1 -> treated as 1
//   - start > LEN(s$) -> returns empty string
//   - length extends past end -> truncated to remaining
//
BValue bval_mid(const BValue *s, const BValue *start_val,
    const BValue *len_val, int line_num, void *pool)
{
    int pos;
    int count;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(start_val) ||
        !bval_is_numeric(len_val)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    pos = (int)bval_to_int(start_val) - 1;  // BASIC 1-based -> C 0-based
    count = (int)bval_to_int(len_val);

    if (pos < 0) pos = 0;
    if (pos > s->v.sval.length) pos = s->v.sval.length;
    if (count < 0) count = 0;
    if (pos + count > s->v.sval.length) {
        count = s->v.sval.length - pos;
    }

    ptr = strpool_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data + pos, (size_t)count);
    return bval_string(ptr, count);
}

// bval_concat - String concatenation: a$ + b$
//
// Both operands must be strings (ERR_WHAT otherwise).
// The result is a new string in the pool containing a$ followed by b$.
//
// This is the ONLY place where "+" on strings is valid.
// The arithmetic bval_add() does NOT handle strings.
//
BValue bval_concat(const BValue *a, const BValue *b, int line_num,
    void *pool)
{
    int total;
    char *ptr;

    if (!bval_is_string(a) || !bval_is_string(b)) {
        error_raise(ERR_WHAT, line_num);
        return bval_string(NULL, 0);
    }

    total = a->v.sval.length + b->v.sval.length;
    ptr = strpool_alloc((StringPool *)pool, total);
    if (ptr == NULL) {
        error_raise(ERR_SORRY, line_num);
        return bval_string(NULL, 0);
    }

    if (a->v.sval.length > 0) {
        memcpy(ptr, a->v.sval.data, (size_t)a->v.sval.length);
    }
    if (b->v.sval.length > 0) {
        memcpy(ptr + a->v.sval.length, b->v.sval.data,
            (size_t)b->v.sval.length);
    }
    return bval_string(ptr, total);
}
