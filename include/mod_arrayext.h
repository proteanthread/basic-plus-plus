/**
 * @file mod_arrayext.h
 * @brief Advanced array operations (MAP, FILTER, REDUCE, JOIN, Aggregates)
 */

#ifndef BPP_MOD_ARRAYEXT_H
#define BPP_MOD_ARRAYEXT_H

#include "bpp_vm.h"
#include "bpp_types.h"
#include "bpp_lexer.h"

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

#endif /* BPP_MOD_ARRAYEXT_H */
