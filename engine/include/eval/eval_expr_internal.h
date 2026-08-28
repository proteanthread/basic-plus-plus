// FILENAME: eval_expr_internal.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (eval.c, eval_array.c, eval_ident.c, eval_new.c)
// NEEDED BY: libengine (eval_op.c)
// NEEDS: libcore, libengine, libkernel, libplatform
// Provides core logic and interface definitions for eval_expr_internal within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_EXPR_INTERNAL_H
#define EVAL_EXPR_INTERNAL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "runtime/ctype/ctype.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/memory/alloc.h"
#include "hal/hal.h"


#include "core/struct.h"
#include "eval/eval_internal.h"
#include "eval/functions/system/environment/command_fn.h"
#include "lexer/lexer.h"
#include "platform/platform.h"
#include "runtime/file.h"
#include "runtime/funcreg.h"
#include "runtime/map.h"
#include "runtime/variables.h"
#include "statements/oop/sub.h"
#include "statements/variables/declaration/def.h"
#include "types/types.h"
#include "vm/vm.h"

//
// ---- Shunting Yard Constants ----

#ifndef MAX_EVAL_DEPTH
#define MAX_EVAL_DEPTH 256
#endif

//
// ---- Internal Function Prototypes ----

bool eval_is_clause_delimiter(BppToken tok);

bool eval_parse_new_expression(VMContext *vm, LexerContext *lex, BValue *out_val, BppError *out_err);

bool eval_parse_identifier_expression(VMContext *vm, LexerContext *lex, BppToken tok, BValue *out_val, bool *out_is_func, BppError *out_err);

bool eval_parse_array_access(VMContext *vm, LexerContext *lex, const char *name_buf, BValue *out_val, BppError *out_err);

bool eval_process_operator(VMContext *vm, LexerContext *lex, BppToken tok, bool expect_operand, int open_parens, BValue *val_stack, size_t *val_ptr, BppTokenType *op_stack, size_t *op_ptr, bool *out_expect_operand, bool *out_break, BppError *out_err);

#endif // EVAL_EXPR_INTERNAL_H
