/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file mod_arrayext.h
 * @brief Advanced array operations (MAP, FILTER, REDUCE, JOIN, Aggregates)
 */

#ifndef MODULE_ARRAYEXT_H
#define MODULE_ARRAYEXT_H

#include "vm/vm.h"
#include "types/types.h"
#include "lexer/lexer.h"

/* Statement-based execution functions (ARRAY MAP, ARRAY FILTER) */
BppError arrayext_execute_map(VMContext *vm, const char *src_arr, const char *dst_arr, const char *fn_name, const char *label_name, const char *expr_str);

/* Expression-based execution functions */
BValue arrayext_func_map(VMContext *vm, LexerContext *lex, BppError *err);
BValue arrayext_func_filter(VMContext *vm, LexerContext *lex, BppError *err);
BValue arrayext_func_reduce(VMContext *vm, LexerContext *lex, BppError *err);
BValue arrayext_func_aggregate(VMContext *vm, LexerContext *lex, int agg_type, BppError *err);

#define AGG_MIN 1
#define AGG_MAX 2
#define AGG_SUM 3

#endif /* MODULE_ARRAYEXT_H */
