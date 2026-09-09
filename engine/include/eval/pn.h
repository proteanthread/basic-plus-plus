// FILENAME: pn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// NEEDS: libkernel (types.h, lexer.h)
// Provides core logic and interface definitions for HP-based Prefix Polish Notation (PN) in BASIC++.

#ifndef EVAL_PN_H
#define EVAL_PN_H

#include "types/types.h"
#include "lexer/lexer.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct VMContext;
typedef struct VMContext VMContext;

#define PN_EXT_STACK_CAP 64

// HP-based prefix calculator register and stack context
typedef struct {
    BValue  x;                     // Primary prefix result register / ACC (PN.X, PN.ACC)
    BValue  y;                     // Second operand (PN.Y)
    BValue  z;                     // Third operand (PN.Z)
    BValue  t;                     // Fourth operand (PN.T)
    BValue  last_x;                // Prior result (PN.LASTX)
    BValue  r[16];                 // 16 hexadecimal storage registers (PN.R0 .. PN.RF)
    int     wsize;                 // Word size (PN.WSIZE)
    BValue  ext_stack[PN_EXT_STACK_CAP];
    int     ext_depth;
} PnContext;

// Reset PN registers and stack
void pn_reset(void);

// Retrieve active PN context
PnContext* pn_get_context(void);

// Register accessors for PN (e.g. "PN.X", "PN.ACC", "PN.R0", "R0")
bool pn_reg_get(const char *name, BValue *out_val);
bool pn_reg_set(const char *name, BValue val);

// Indexed register access for PN[0..15]
bool pn_reg_get_by_index(int index, BValue *out_val);
bool pn_reg_set_by_index(int index, BValue val);

// Evaluates a Prefix Polish Notation (PN) expression from the lexer stream
BValue eval_expression_pn(VMContext *vm, LexerContext *lex, BppError *out_err);

// Evaluates a PN expression string
BValue pn_eval_expr_string(VMContext *vm, const char *expr_str, BppError *out_err);

// Checks if a token is a prefix operator or prefix function keyword
bool eval_is_pn_operator(BppToken tok);

#ifdef __cplusplus
}
#endif

#endif // EVAL_PN_H
