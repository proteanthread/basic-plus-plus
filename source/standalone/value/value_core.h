/* =====================================================================
 * BASIC++ DEVELOPER & MAINTENANCE REFERENCE
 * File: value_core.h
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
 * File: value_core.h
 * ===================================================================== */

#ifndef BASICPP_STANDALONE_VALUE_CORE_H
#define BASICPP_STANDALONE_VALUE_CORE_H

#include "../../value.h"

BValue bval_core_add(const BValue *a, const BValue *b, int *err);
BValue bval_core_sub(const BValue *a, const BValue *b, int *err);
BValue bval_core_mul(const BValue *a, const BValue *b, int *err);
BValue bval_core_div(const BValue *a, const BValue *b, int *err);
BValue bval_core_neg(const BValue *v, int *err);
BValue bval_core_mod(const BValue *a, const BValue *b, int *err);
int bval_core_compare(const BValue *a, const BValue *b, int *err);

BValue bval_core_abs(const BValue *v, int *err);
BValue bval_core_sgn(const BValue *v, int *err);
BValue bval_core_int_func(const BValue *v, int *err);
BValue bval_core_sqr(const BValue *v, int *err);
BValue bval_core_sin(const BValue *v, int *err);
BValue bval_core_cos(const BValue *v, int *err);
BValue bval_core_tan(const BValue *v, int *err);
BValue bval_core_atn(const BValue *v, int *err);
BValue bval_core_log(const BValue *v, int *err);
BValue bval_core_exp(const BValue *v, int *err);

BValue bval_core_len(const BValue *v, int *err);
BValue bval_core_asc(const BValue *v, int *err);
BValue bval_core_val(const BValue *v, int *err);
BValue bval_core_chr(const BValue *v, int *err, void *pool);
BValue bval_core_str(const BValue *v, int *err, void *pool);
BValue bval_core_left(const BValue *s, const BValue *n, int *err, void *pool);
BValue bval_core_right(const BValue *s, const BValue *n, int *err, void *pool);
BValue bval_core_mid(const BValue *s, const BValue *start, const BValue *len, int *err, void *pool);
BValue bval_core_concat(const BValue *a, const BValue *b, int *err, void *pool);

#endif // BASICPP_STANDALONE_VALUE_CORE_H
