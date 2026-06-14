#ifndef STDLIB_CORE_H
#define STDLIB_CORE_H

#include "value.h"

/*
 * BASIC++ Core Immutable API
 * These functions are guaranteed across all dialects and 
 * interact directly with the VM memory pool.
 */

void stdlib_core_register(void);

BValue stdlib_core_len(BValue *args, int argc, void *rt);
BValue stdlib_core_chr(BValue *args, int argc, void *rt);
BValue stdlib_core_str(BValue *args, int argc, void *rt);
BValue stdlib_core_rnd(BValue *args, int argc, void *rt);

/* MS-BASIC Common Math Functions */
BValue stdlib_core_abs(BValue *args, int argc, void *rt);
BValue stdlib_core_sgn(BValue *args, int argc, void *rt);
BValue stdlib_core_int(BValue *args, int argc, void *rt);
BValue stdlib_core_sqr(BValue *args, int argc, void *rt);
BValue stdlib_core_sin(BValue *args, int argc, void *rt);
BValue stdlib_core_cos(BValue *args, int argc, void *rt);
BValue stdlib_core_tan(BValue *args, int argc, void *rt);
BValue stdlib_core_atn(BValue *args, int argc, void *rt);
BValue stdlib_core_log(BValue *args, int argc, void *rt);
BValue stdlib_core_exp(BValue *args, int argc, void *rt);

/* MS-BASIC Common String Functions */
BValue stdlib_core_left(BValue *args, int argc, void *rt);
BValue stdlib_core_right(BValue *args, int argc, void *rt);
BValue stdlib_core_mid(BValue *args, int argc, void *rt);
BValue stdlib_core_instr(BValue *args, int argc, void *rt);
BValue stdlib_core_err_str(BValue *args, int argc, void *rt);

#endif
