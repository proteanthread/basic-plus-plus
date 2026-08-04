/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file eval_internal.h
 * @brief Internal shared declarations for the split eval subsystem.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Provides shared type definitions, forward declarations, and internal
 *   function prototypes used across the eval subsystem's split source files.
 * - Why it exists: When eval.c was refactored from a monolithic 5900+ line file into
 *   modular components (eval_helpers.c, eval_ops.c, eval_dispatch.c, eval_builtins.c,
 *   eval_rpn.c), these internal APIs needed a common declaration point.
 * - Why it works this way: Previously-static functions are promoted to non-static
 *   internal linkage with the eval_ prefix for cross-TU access.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Add new internal function declarations as new eval modules
 *   are created. Add new shared constants or macros.
 * - What cannot be changed: The public API signatures (eval_expression,
 *   eval_expression_rpn, invoke_user_function) defined in eval/eval.h.
 * - What to expect: Including this header provides access to all internal eval functions.
 * - What to do if something breaks: Check for missing includes in individual .c files.
 *   Verify that function signatures here match the actual implementations.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: All callers include this header. eval/eval.h is included transitively.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: New builtin categories can add their own
 *   eval_bi_*.c files and register implementations through eval_builtin_function_impl.
 */

#ifndef EVAL_INTERNAL_H
#define EVAL_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "eval/eval.h"
#include "platform/platform.h"
#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"
#include "runtime/strings.h"

/* ===================================================================
 * Constants
 * =================================================================== */

#ifndef MAX_EVAL_DEPTH
#define MAX_EVAL_DEPTH 128
#endif

/* ===================================================================
 * Shared Helper Functions (eval_helpers.c)
 * =================================================================== */

/**
 * @brief Parse a Sinclair/Atari string slice construct from the token stream.
 */
BValue eval_parse_string_slice(VMContext *vm, LexerContext *lex,
                               const char *var_name, BppTokenType open_tok,
                               BppError *out_err);

/**
 * @brief Split a dotted identifier into base variable and member chain.
 */
void eval_split_member_chain(const char *start, size_t len,
                             char *var_name, size_t var_name_max,
                             char member_chain[8][64], int *member_count);

/**
 * @brief Get the precedence of an operator token type.
 */
int eval_get_precedence(BppTokenType type);

/**
 * @brief Check if top-of-stack operator has precedence over new operator.
 */
bool eval_has_precedence(VMContext *vm, BppTokenType top, BppTokenType op);

/**
 * @brief Check if a token type is an operator.
 */
bool eval_is_operator(BppTokenType type);

/**
 * @brief Round a double to a specified number of significant digits.
 */
double eval_round_to_decimal(double val, int precision);

/**
 * @brief Format a double to clean string representation.
 */
void eval_format_double_clean(char *buf, size_t buf_size, double val,
                              bool leading_space, bool trailing_space);

/* ===================================================================
 * Operator Execution (eval_ops.c)
 * =================================================================== */

/**
 * @brief Execute a binary or unary operator on the value stack.
 * @return true on success, false on error (err populated).
 */
bool eval_execute_op(VMContext *vm, BppTokenType op,
                     BValue *val_stack, size_t *val_ptr, BppError *err);

/**
 * @brief Resolve member access chain on a value (dot notation).
 */
BValue eval_resolve_member_access(VMContext *vm, LexerContext *lex,
                                  BValue val, BppError *out_err);

/* ===================================================================
 * Builtin Dispatch (eval_dispatch.c)
 * =================================================================== */

/**
 * @brief Check if a name is a known builtin function.
 */
bool eval_is_builtin_function(const char *name);

/**
 * @brief Read a file's entire contents into a malloc'd string.
 * @return Heap-allocated NUL-terminated string, or NULL on failure. Caller frees.
 */
char *eval_read_file_to_string(const char *path);

/**
 * @brief Parse arguments from token stream and dispatch to builtin implementation.
 */
BValue eval_builtin_function(VMContext *vm, const char *name,
                             LexerContext *lex, bool has_parens,
                             BppError *err);

/* ===================================================================
 * Builtin Implementation (eval_builtins.c)
 * =================================================================== */

/**
 * @brief Master dispatch for builtin function evaluation by uppercase name.
 */
BValue eval_builtin_function_impl(VMContext *vm, const char *uname,
                                  int arg_count, BValue *args, BppError *err);

/**
 * @brief CRC-16 computation helper for BIOCHECKSUM.
 */
uint16_t eval_compute_crc16(const unsigned char *data, size_t len);

/**
 * @brief RPN expression evaluator (reverse Polish notation).
 */
BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err);

/* ===================================================================
 * Shared Global State
 * =================================================================== */

extern BppDirSearch *g_eval_dir_search;

/* ===================================================================
 * External Platform/VM Functions used across eval modules
 * =================================================================== */

extern bool find_procedure(struct VMContext *vm, const char *name,
                           BppKeywordId proc_kw, double *out_line,
                           const char **out_text);
extern int platform_inkey_char(void);
extern int platform_mouse_x(void);
extern int platform_mouse_y(void);
extern int platform_mouse_btn(void);
extern struct tm *platform_localtime(const time_t *timep, struct tm *result);
extern double vm_get_last_rnd(VMContext *vm);
extern void vm_set_last_rnd(VMContext *vm, double val);
extern double platform_get_timer(void);
extern double platform_get_uptime(void);
extern double vm_get_ti_offset(VMContext *vm);
extern void vm_set_ti_offset(VMContext *vm, double val);
extern void hash_string(const char *algo, const char *data,
                            char *out_buf, size_t out_size);

#endif /* EVAL_INTERNAL_H */
