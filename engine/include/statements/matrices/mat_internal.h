// FILENAME: mat_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (mat_arith.c, mat_ops.c, mat_special.c, mat_transform.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides runtime implementation for the MAT_INTERNAL statement in BASIC++.
//
// ---- Includes ----

#ifndef MAT_INTERNAL_H
#define MAT_INTERNAL_H

#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device/vdev.h"
#include "eval/eval.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/arrays.h"
#include "runtime/micro_lib_metadata.h"
#include "runtime/strings.h"
#include "runtime/variables.h"
#include "security/security.h"
#include "statements/matrices/io/mat_input.h"
#include "statements/matrices/ops/mat_ops.h"
#include "statements/matrices/io/mat_print.h"
#include "statements/matrices/io/mat_read.h"
#include "statements/matrices/io/mat_write.h"
#include "types/errors.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Array Helper Prototypes ----

bool parse_array_name(LexerContext *lex, char *out_name, size_t max_len);
bool ensure_array_dims(ArrayContext *arr, const char *name, int num_dims, const int *bounds, BppError *err);

//
// ---- Operation Handlers ----

BppError mat_op_add(VMContext *vm, const char *dest, const char *name_a, const char *name_b);
BppError mat_op_sub(VMContext *vm, const char *dest, const char *name_a, const char *name_b);
BppError mat_op_mul(VMContext *vm, const char *dest, const char *name_a, const char *name_b);
BppError mat_op_div_scalar(VMContext *vm, const char *dest, const char *name_a, double scalar);
BppError mat_op_scalar(VMContext *vm, const char *dest, double scalar, int op_type, const char *src);
BppError mat_op_copy(VMContext *vm, const char *dest, const char *src);

BppError mat_op_zer(VMContext *vm, const char *dest, int dims, const int *bounds);
BppError mat_op_con(VMContext *vm, const char *dest, int dims, const int *bounds);
BppError mat_op_idn(VMContext *vm, const char *dest, int dims, const int *bounds);
BppError mat_op_nul(VMContext *vm, const char *dest, int dims, const int *bounds);

BppError mat_op_trn(VMContext *vm, const char *dest, const char *src);
BppError mat_op_inv(VMContext *vm, const char *dest, const char *src);
BppError mat_op_cross(VMContext *vm, const char *dest, const char *v1_name, const char *v2_name);

#endif // MAT_INTERNAL_H
