// FILENAME: def.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (def.c, eval_expr_internal.h, exec_internal.h)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEF statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_DEF_H
#define STATEMENTS_VARIABLES_DEF_H
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_def_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defint_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defsng_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defdbl_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defstr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defcpx_handler(VMContext *vm, LexerContext *lex);

BppError stmt_defusr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_common_handler(VMContext *vm, LexerContext *lex);

BppError stmt_fnend_handler(VMContext *vm, LexerContext *lex);

void def_fn_register(const char *name, int param_count, char param_names[][64], const char *expr_body);
void def_fn_register_multiline(const char *name, int param_count, char param_names[][64], BppLineNumber start_line, BppLineNumber end_line);
BValue def_fn_eval(VMContext *vm, const char *name, BValue *args, int argc, BppError *err, bool *found);
bool def_fn_exists(const char *name);

void stmt_def_register(void);

#endif
