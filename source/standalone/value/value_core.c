/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: value_core.c
 * Subsystem: Variant Type Value Coercion Engine
 * =====================================================================
 * 1. PURPOSE & OPERATION:
 *    Computes BValue variant constructors and arithmetic coercions.
 *
 * 2. WHAT TO EXPECT:
 *    Performs conversions conforming to dialect rules.
 *
 * 3. WHAT CAN BE CHANGED:
 *    Coercion priority levels.
 *
 * 4. WHAT CANNOT BE CHANGED:
 *    Type descriptors structures.
 *
 * 5. TROUBLESHOOTING & FAILURE MODES:
 *    If math returns bad types, check dialect definition.
 * ===================================================================== */

/* =====================================================================
 * PORTABLE STANDALONE VARIANT TYPE VALUE SYSTEM CORE
 * File: value_core.c
 * ===================================================================== */

#include "value_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "standalone/stringpool/stringpool_core.h"

#ifndef MAX_LINE_LENGTH
#define MAX_LINE_LENGTH 256
#endif

// --- Constructors ---
BValue bval_int(long val)
{
    BValue r;
    r.type = VAL_INTEGER;
    r.v.ival = val;
    return r;
}

int g_arithmetic_decimal = 0;

double bval_decimal_round(double val)
{
    if (val != val) return val; /* NaN check */
    if (isinf(val)) return val;
    if (val == 0.0) return 0.0;
    
    double abs_val = fabs(val);
    double log10_val = log10(abs_val);
    double exp_val = floor(log10_val);
    double scale = pow(10.0, 12.0 - 1.0 - exp_val);
    return round(val * scale) / scale;
}

BValue bval_float(double val)
{
    BValue r;
    r.type = VAL_FLOAT;
    if (g_arithmetic_decimal) {
        val = bval_decimal_round(val);
    }
    r.v.fval = val;
    return r;
}

BValue bval_string(char *data, int length)
{
    BValue r;
    r.type = VAL_STRING;
    r.v.sval.data = data;
    r.v.sval.length = length;
    return r;
}

BValue bval_complex(double real, double imag)
{
    BValue r;
    r.type = VAL_COMPLEX;
    r.v.cval.real = real;
    r.v.cval.imag = imag;
    return r;
}

BValue bval_object(long obj_id)
{
    BValue r;
    r.type = VAL_OBJECT;
    r.v.obj_id = obj_id;
    return r;
}

// --- Type Queries ---
int bval_is_int(const BValue *v)     { return v->type == VAL_INTEGER; }
int bval_is_float(const BValue *v)   { return v->type == VAL_FLOAT; }
int bval_is_string(const BValue *v)  { return v->type == VAL_STRING; }
int bval_is_complex(const BValue *v) { return v->type == VAL_COMPLEX; }
int bval_is_object(const BValue *v)  { return v->type == VAL_OBJECT; }

int bval_is_numeric(const BValue *v)
{
    return v->type == VAL_INTEGER || v->type == VAL_FLOAT || v->type == VAL_COMPLEX;
}

// --- Type Coercion ---
long bval_to_int(const BValue *v)
{
    switch (v->type) {
    case VAL_INTEGER: return v->v.ival;
    case VAL_FLOAT:   return (long)v->v.fval;
    case VAL_STRING: {
        char buf[MAX_LINE_LENGTH + 1];
        int len = v->v.sval.length;
        if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
        memcpy(buf, v->v.sval.data, (size_t)len);
        buf[len] = '\0';
        return strtol(buf, NULL, 10);
    }
    case VAL_COMPLEX:
        return (long)v->v.cval.real;
    case VAL_OBJECT:
        return 0;
    }
    return 0;
}

long bval_to_subscript(const BValue *v)
{
    if (v->type == VAL_FLOAT) {
        double d = v->v.fval;
        return (long)(d >= 0.0 ? floor(d + 0.5) : ceil(d - 0.5));
    }
    return bval_to_int(v);
}

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
    case VAL_OBJECT:
        return 0.0;
    }
    return 0.0;
}

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
            return sprintf(buf, "(%G+%Gi)", v->v.cval.real, v->v.cval.imag);
        else
            return sprintf(buf, "(%G%Gi)", v->v.cval.real, v->v.cval.imag);
    case VAL_OBJECT:
        buf[0] = '\0';
        return 0;
    }
    buf[0] = '\0';
    return 0;
}

// --- Helpers ---
static int check_numeric_core(const BValue *a, const BValue *b, int *err)
{
    if (!bval_is_numeric(a) || !bval_is_numeric(b)) {
        if (err) *err = 1; // ERR_WHAT
        return 0;
    }
    return 1;
}

static int should_use_complex(const BValue *a, const BValue *b)
{
    return (a->type == VAL_COMPLEX || b->type == VAL_COMPLEX);
}

static void get_complex(const BValue *v, double *re, double *im)
{
    if (v->type == VAL_COMPLEX) {
        *re = v->v.cval.real;
        *im = v->v.cval.imag;
    } else {
        *re = bval_to_float(v);
        *im = 0.0;
    }
}

static int should_use_float(const BValue *a, const BValue *b)
{
    const DialectConfig *cfg = NULL;
    if (cfg && !cfg->has_float) {
        return 0;
    }
    return (a->type == VAL_FLOAT || b->type == VAL_FLOAT);
}

// --- Core Arithmetic Implementations ---
BValue bval_core_add(const BValue *a, const BValue *b, int *err)
{
    if (!check_numeric_core(a, b, err)) return bval_int(0);

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

BValue bval_core_sub(const BValue *a, const BValue *b, int *err)
{
    if (!check_numeric_core(a, b, err)) return bval_int(0);

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

BValue bval_core_mul(const BValue *a, const BValue *b, int *err)
{
    if (!check_numeric_core(a, b, err)) return bval_int(0);

    if (should_use_complex(a, b)) {
        double ar, ai, br, bi;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        return bval_complex(ar*br - ai*bi, ar*bi + ai*br);
    }
    if (should_use_float(a, b)) {
        return bval_float(bval_to_float(a) * bval_to_float(b));
    }
    return bval_int(a->v.ival * b->v.ival);
}

BValue bval_core_div(const BValue *a, const BValue *b, int *err)
{
    if (!check_numeric_core(a, b, err)) return bval_int(0);

    if (should_use_complex(a, b)) {
        double ar, ai, br, bi, denom;
        get_complex(a, &ar, &ai);
        get_complex(b, &br, &bi);
        denom = br*br + bi*bi;
        if (denom == 0.0) {
            if (err) *err = 2; // ERR_HOW
            return bval_complex(0.0, 0.0);
        }
        return bval_complex((ar*br + ai*bi) / denom, (ai*br - ar*bi) / denom);
    }
    if (should_use_float(a, b)) {
        double denom = bval_to_float(b);
        if (denom == 0.0) {
            if (err) *err = 2; // ERR_HOW
            return bval_float(0.0);
        }
        return bval_float(bval_to_float(a) / denom);
    }

    if (b->v.ival == 0) {
        if (err) *err = 2; // ERR_HOW
        return bval_int(0);
    }
    return bval_int(a->v.ival / b->v.ival);
}

BValue bval_core_neg(const BValue *v, int *err)
{
    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_int(0);
    }
    if (v->type == VAL_COMPLEX) {
        return bval_complex(-v->v.cval.real, -v->v.cval.imag);
    }
    if (v->type == VAL_FLOAT) {
        return bval_float(-v->v.fval);
    }
    return bval_int(-v->v.ival);
}

BValue bval_core_mod(const BValue *a, const BValue *b, int *err)
{
    long ia, ib;
    if (!check_numeric_core(a, b, err)) return bval_int(0);

    ia = bval_to_int(a);
    ib = bval_to_int(b);
    if (ib == 0) {
        if (err) *err = 2; // ERR_HOW
        return bval_int(0);
    }
    return bval_int(ia % ib);
}

int bval_core_compare(const BValue *a, const BValue *b, int *err)
{
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

    if (err) *err = 1; // ERR_WHAT
    return 0;
}

// --- Core Float Math functions ---
static double get_numeric_arg_core(const BValue *v, int *err)
{
    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return 0.0;
    }
    return bval_to_float(v);
}

BValue bval_core_abs(const BValue *v, int *err)
{
    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_int(0);
    }
    if (v->type == VAL_FLOAT) {
        return bval_float(fabs(v->v.fval));
    }
    return bval_int(v->v.ival < 0 ? -v->v.ival : v->v.ival);
}

BValue bval_core_sgn(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_int(0);
    if (d > 0.0) return bval_int(1);
    if (d < 0.0) return bval_int(-1);
    return bval_int(0);
}

BValue bval_core_int_func(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_int(0);
    return bval_int((long)floor(d));
}

BValue bval_core_sqr(const BValue *v, int *err)
{
    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_float(0.0);
    }
    if (v->type == VAL_COMPLEX) {
        double a = v->v.cval.real;
        double b = v->v.cval.imag;
        double r = sqrt(a * a + b * b);
        double sr = sqrt(r);
        double t = atan2(b, a);
        return bval_complex(sr * cos(t / 2.0), sr * sin(t / 2.0));
    }
    {
        double d = bval_to_float(v);
        if (d < 0.0) {
            return bval_complex(0.0, sqrt(-d));
        }
        return bval_float(sqrt(d));
    }
}

BValue bval_core_sin(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    return bval_float(sin(d));
}

BValue bval_core_cos(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    return bval_float(cos(d));
}

BValue bval_core_tan(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    return bval_float(tan(d));
}

BValue bval_core_atn(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    return bval_float(atan(d));
}

BValue bval_core_log(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    if (d <= 0.0) {
        if (err) *err = 2; // ERR_HOW
        return bval_float(0.0);
    }
    return bval_float(log(d));
}

BValue bval_core_exp(const BValue *v, int *err)
{
    double d = get_numeric_arg_core(v, err);
    if (err && *err) return bval_float(0.0);
    return bval_float(exp(d));
}

// --- Core String Functions ---
BValue bval_core_len(const BValue *v, int *err)
{
    if (!bval_is_string(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_int(0);
    }
    return bval_int((long)v->v.sval.length);
}

BValue bval_core_asc(const BValue *v, int *err)
{
    if (!bval_is_string(v) || v->v.sval.length == 0) {
        if (err) *err = 1; // ERR_WHAT
        return bval_int(0);
    }
    return bval_int((long)(unsigned char)v->v.sval.data[0]);
}

BValue bval_core_val(const BValue *v, int *err)
{
    char buf[MAX_LINE_LENGTH + 1];
    char *endptr;
    long ival;
    int len;

    if (!bval_is_string(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_int(0);
    }

    len = v->v.sval.length;
    if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
    memcpy(buf, v->v.sval.data, (size_t)len);
    buf[len] = '\0';

    ival = strtol(buf, &endptr, 10);
    if (*endptr == '\0' || *endptr == ' ') {
        return bval_int(ival);
    }
    if (*endptr == '.') {
        double fval = strtod(buf, &endptr);
        return bval_float(fval);
    }
    return bval_int(0);
}

BValue bval_core_chr(const BValue *v, int *err, void *pool)
{
    long code;
    char *ptr;

    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    code = bval_to_int(v);
    if (code < 0 || code > 255) {
        if (err) *err = 2; // ERR_HOW
        return bval_string(NULL, 0);
    }

    ptr = strpool_core_alloc((StringPool *)pool, 1);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }
    ptr[0] = (char)code;
    return bval_string(ptr, 1);
}

BValue bval_core_str(const BValue *v, int *err, void *pool)
{
    char buf[64];
    int len;
    char *ptr;

    if (!bval_is_numeric(v)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    len = bval_to_string_buf(v, buf, sizeof(buf));
    ptr = strpool_core_alloc((StringPool *)pool, len);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }
    memcpy(ptr, buf, (size_t)len);
    return bval_string(ptr, len);
}

BValue bval_core_left(const BValue *s, const BValue *n, int *err, void *pool)
{
    int count;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(n)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    count = (int)bval_to_int(n);
    if (count < 0) count = 0;
    if (count > s->v.sval.length) count = s->v.sval.length;

    ptr = strpool_core_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data, (size_t)count);
    return bval_string(ptr, count);
}

BValue bval_core_right(const BValue *s, const BValue *n, int *err, void *pool)
{
    int count;
    int start;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(n)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    count = (int)bval_to_int(n);
    if (count < 0) count = 0;
    if (count > s->v.sval.length) count = s->v.sval.length;
    start = s->v.sval.length - count;

    ptr = strpool_core_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data + start, (size_t)count);
    return bval_string(ptr, count);
}

BValue bval_core_mid(const BValue *s, const BValue *start_val, const BValue *len_val, int *err, void *pool)
{
    int pos;
    int count;
    char *ptr;

    if (!bval_is_string(s) || !bval_is_numeric(start_val) || !bval_is_numeric(len_val)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    pos = (int)bval_to_int(start_val) - 1;
    count = (int)bval_to_int(len_val);

    if (pos < 0) pos = 0;
    if (pos > s->v.sval.length) pos = s->v.sval.length;
    if (count < 0) count = 0;
    if (pos + count > s->v.sval.length) {
        count = s->v.sval.length - pos;
    }

    ptr = strpool_core_alloc((StringPool *)pool, count);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }
    memcpy(ptr, s->v.sval.data + pos, (size_t)count);
    return bval_string(ptr, count);
}

BValue bval_core_concat(const BValue *a, const BValue *b, int *err, void *pool)
{
    int total;
    char *ptr;

    if (!bval_is_string(a) || !bval_is_string(b)) {
        if (err) *err = 1; // ERR_WHAT
        return bval_string(NULL, 0);
    }

    total = a->v.sval.length + b->v.sval.length;
    ptr = strpool_core_alloc((StringPool *)pool, total);
    if (ptr == NULL) {
        if (err) *err = 3; // ERR_SORRY
        return bval_string(NULL, 0);
    }

    if (a->v.sval.length > 0) {
        memcpy(ptr, a->v.sval.data, (size_t)a->v.sval.length);
    }
    if (b->v.sval.length > 0) {
        memcpy(ptr + a->v.sval.length, b->v.sval.data, (size_t)b->v.sval.length);
    }
    return bval_string(ptr, total);
}
