/*
 * ---
 * BASIC++ Interpreter - value.c
 * ---
 *
 * Implementation of the tagged BValue type and all operations.
 *
 * DESIGN RATIONALE:
 * All type coercion follows classic BASIC rules:
 * - Integer + Float -> Float (auto-promotion)
 * - String in arithmetic -> ERR_WHAT (type mismatch)
 * - String + String -> concatenation (in bval_concat only)
 * - VAL("123") -> integer 123; VAL("3.14") -> float 3.14
 *
 * ---
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "value.h"
#include "errors.h"
#include "stringpool.h"

/* --- Constructors ---
 */

BValue bval_int(long val)
{
 BValue r;
 r.type = VAL_INTEGER;
 r.v.ival = val;
 return r;
}

BValue bval_float(double val)
{
 BValue r;
 r.type = VAL_FLOAT;
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

/* --- Type Queries ---
 */

int bval_is_int(const BValue *v) { return v->type == VAL_INTEGER; }
int bval_is_float(const BValue *v) { return v->type == VAL_FLOAT; }
int bval_is_string(const BValue *v) { return v->type == VAL_STRING; }

int bval_is_numeric(const BValue *v)
{
 return v->type == VAL_INTEGER || v->type == VAL_FLOAT;
}

/* --- Type Coercion ---
 */

long bval_to_int(const BValue *v)
{
 switch (v->type) {
 case VAL_INTEGER: return v->v.ival;
 case VAL_FLOAT: return (long)v->v.fval;
 case VAL_STRING: {
 /* Parse string as integer */
 char buf[MAX_LINE_LENGTH + 1];
 int len = v->v.sval.length;
 if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
 memcpy(buf, v->v.sval.data, (size_t)len);
 buf[len] = '\0';
 return strtol(buf, NULL, 10);
 }
 }
 return 0;
}

/*
 * bval_to_subscript - Round to nearest integer for array subscripts.
 *
 * ECMA-55 s6.4: "each subscript is rounded to the nearest integer."
 * GW-BASIC/QBasic: uses CINT() for subscripts (round to nearest).
 * This function rounds floats; integers pass through unchanged.
 * Rounding: floor(x + 0.5) for positive, ceil(x - 0.5) for negative.
 */
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
 case VAL_FLOAT: return v->v.fval;
 case VAL_STRING: {
 char buf[MAX_LINE_LENGTH + 1];
 int len = v->v.sval.length;
 if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
 memcpy(buf, v->v.sval.data, (size_t)len);
 buf[len] = '\0';
 return strtod(buf, NULL);
 }
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
 }
 buf[0] = '\0';
 return 0;
}

/* --- Arithmetic - Helper: check both operands are numeric ---
 */
static int check_numeric(const BValue *a, const BValue *b, int line_num)
{
 if (!bval_is_numeric(a) || !bval_is_numeric(b)) {
 error_raise(ERR_WHAT, line_num);
 return 0;
 }
 return 1;
}

/*
 * should_use_float - Returns 1 if either operand is float.
 */
static int should_use_float(const BValue *a, const BValue *b)
{
 return (a->type == VAL_FLOAT || b->type == VAL_FLOAT);
}

BValue bval_add(const BValue *a, const BValue *b, int line_num)
{
 /* String concatenation is handled by bval_concat, not here.
 * If both are strings in bval_add, that's a type error unless
 * the parser calls bval_concat instead. */
 if (!check_numeric(a, b, line_num)) return bval_int(0);

 if (should_use_float(a, b)) {
 return bval_float(bval_to_float(a) + bval_to_float(b));
 }
 return bval_int(a->v.ival + b->v.ival);
}

BValue bval_sub(const BValue *a, const BValue *b, int line_num)
{
 if (!check_numeric(a, b, line_num)) return bval_int(0);

 if (should_use_float(a, b)) {
 return bval_float(bval_to_float(a) - bval_to_float(b));
 }
 return bval_int(a->v.ival - b->v.ival);
}

BValue bval_mul(const BValue *a, const BValue *b, int line_num)
{
 if (!check_numeric(a, b, line_num)) return bval_int(0);

 if (should_use_float(a, b)) {
 return bval_float(bval_to_float(a) * bval_to_float(b));
 }
 return bval_int(a->v.ival * b->v.ival);
}

BValue bval_div(const BValue *a, const BValue *b, int line_num)
{
 if (!check_numeric(a, b, line_num)) return bval_int(0);

 if (should_use_float(a, b)) {
 double denom = bval_to_float(b);
 if (denom == 0.0) {
 error_raise(ERR_HOW, line_num);
 return bval_float(0.0);
 }
 return bval_float(bval_to_float(a) / denom);
 }

 if (b->v.ival == 0) {
 error_raise(ERR_HOW, line_num);
 return bval_int(0);
 }
 return bval_int(a->v.ival / b->v.ival);
}

BValue bval_neg(const BValue *v, int line_num)
{
 if (!bval_is_numeric(v)) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 if (v->type == VAL_FLOAT) {
 return bval_float(-v->v.fval);
 }
 return bval_int(-v->v.ival);
}

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

/* --- Comparison ---
 */

int bval_compare(const BValue *a, const BValue *b, int line_num)
{
 /* String vs String: lexicographic */
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

 /* Numeric vs Numeric */
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

 /* Mixed string/numeric - type error */
 error_raise(ERR_WHAT, line_num);
 return 0;
}

/* --- Float Math Functions ---
 */

static double get_numeric_arg(const BValue *v, int line_num)
{
 if (!bval_is_numeric(v)) {
 error_raise(ERR_WHAT, line_num);
 return 0.0;
 }
 return bval_to_float(v);
}

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

BValue bval_sgn(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_int(0);
 if (d > 0.0) return bval_int(1);
 if (d < 0.0) return bval_int(-1);
 return bval_int(0);
}

BValue bval_int_func(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_int(0);
 return bval_int((long)floor(d));
}

BValue bval_sqr(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 if (d < 0.0) {
 error_raise(ERR_HOW, line_num);
 return bval_float(0.0);
 }
 return bval_float(sqrt(d));
}

BValue bval_sin(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 return bval_float(sin(d));
}

BValue bval_cos(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 return bval_float(cos(d));
}

BValue bval_tan(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 return bval_float(tan(d));
}

BValue bval_atn(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 return bval_float(atan(d));
}

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

BValue bval_exp(const BValue *v, int line_num)
{
 double d = get_numeric_arg(v, line_num);
 if (error_occurred()) return bval_float(0.0);
 return bval_float(exp(d));
}

/* --- String Functions ---
 */

BValue bval_len(const BValue *v, int line_num)
{
 if (!bval_is_string(v)) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 return bval_int((long)v->v.sval.length);
}

BValue bval_asc(const BValue *v, int line_num)
{
 if (!bval_is_string(v) || v->v.sval.length == 0) {
 error_raise(ERR_WHAT, line_num);
 return bval_int(0);
 }
 return bval_int((long)(unsigned char)v->v.sval.data[0]);
}

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

 len = v->v.sval.length;
 if (len > MAX_LINE_LENGTH) len = MAX_LINE_LENGTH;
 memcpy(buf, v->v.sval.data, (size_t)len);
 buf[len] = '\0';

 /* Try integer first */
 ival = strtol(buf, &endptr, 10);
 if (*endptr == '\0' || *endptr == ' ') {
 return bval_int(ival);
 }

 /* Try float */
 if (*endptr == '.') {
 double fval = strtod(buf, &endptr);
 return bval_float(fval);
 }

 return bval_int(0); /* unparseable -> 0 */
}

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

 pos = (int)bval_to_int(start_val) - 1; /* BASIC is 1-based */
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
