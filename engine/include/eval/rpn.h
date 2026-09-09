// FILENAME: rpn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: types/types.h, lexer/lexer.h
// Provides core logic and interface definitions for HP-based Reverse Polish Notation (RPN / STACK.*) in BASIC++.

#ifndef EVAL_RPN_H
#define EVAL_RPN_H

#include "types/types.h"
#include "lexer/lexer.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VMContext;
typedef struct VMContext VMContext;

#define RPN_EXT_STACK_CAP 64

// HP-based operational stack and storage register context for RPN / STACK.*
typedef struct {
    BValue  x;                     // Level 1 / display register (STACK.X)
    BValue  y;                     // Level 2 (STACK.Y)
    BValue  z;                     // Level 3 (STACK.Z)
    BValue  t;                     // Level 4 (STACK.T)
    BValue  last_x;                // Retention register (STACK.LASTX)
    BValue  r[16];                 // 16 hexadecimal storage registers (STACK.R0 .. STACK.RF)
    int     wsize;                 // Word size (STACK.WSIZE)
    bool    carry;                 // Carry flag (STACK.CARRY)
    bool    overflow;              // Overflow flag (STACK.OVERFLOW)
    BValue  ext_stack[RPN_EXT_STACK_CAP];
    int     ext_depth;
} RpnContext;

// Reset RPN operational stack and storage registers
void rpn_reset(void);

// Retrieve active RPN context
RpnContext* rpn_get_context(void);

// Stack manipulation operations
bool rpn_stack_push(BValue val);
bool rpn_stack_pop(BValue *out_val);
void rpn_stack_swap(void);
void rpn_stack_rollup(void);
void rpn_stack_rolldn(void);
void rpn_stack_clear(void);

// Register getters and setters (e.g. "STACK.X", "X", "RPN.X", "STACK.R0", "R0")
bool rpn_reg_get(const char *name, BValue *out_val);
bool rpn_reg_set(const char *name, BValue val);

// Indexed storage register access for STACK[0..15]
bool rpn_reg_get_by_index(int index, BValue *out_val);
bool rpn_reg_set_by_index(int index, BValue val);

// Evaluates a Postfix Reverse Polish Notation (RPN) expression from the lexer stream
BValue eval_expression_rpn(VMContext *vm, LexerContext *lex, BppError *out_err);

// Evaluates an RPN expression string
BValue rpn_eval_expr_string(VMContext *vm, const char *expr_str, BppError *out_err);

#ifdef __cplusplus
}
#endif

#endif // EVAL_RPN_H
