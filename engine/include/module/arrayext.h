/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file arrayext.h
 * @brief Module component implementation and public API surface for arrayext.h.
 *
 * WHAT IT DOES:
 * Implements the core responsibilities, data structures, and function evaluation logic for arrayext.h within the module subsystem.
 *
 * WHY IT EXISTS:
 * Ensures decoupled modularity, strict C17 portability, and clear micro-library architectural boundary enforcement.
 *
 * WHY IT WORKS THIS WAY:
 * Designed with zero-initialization defaults, bounded memory operations, and explicit error code propagation to the VM state.
 *
 * WHAT CAN BE CHANGED:
 * Subsystem configuration defaults, local execution helper routines, and documentation annotations.
 *
 * WHAT CANNOT BE CHANGED:
 * Public API symbol declarations, micro-library metadata structures, and thread-safe error reporting contracts.
 *
 * WHAT TO EXPECT:
 * High-performance deterministic execution with zero side-effects outside designated state structures.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify context initialization, trace BppError return codes, and inspect log outputs for bounds assertions.
 *
 * ASSUMPTIONS:
 * Valid subsystem contexts and required memory pools are allocated prior to executing API handlers.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance, 64-bit pointer safety, and pure ASCII string operations across desktop, IoT, and embedded targets.
 *
 * FUTURE EXPANSIONS:
 * Additional dialect compatibility mappings, telemetry instrumentation, and microcontroller payload stubs.
 */

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
