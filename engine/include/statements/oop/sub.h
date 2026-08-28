// FILENAME: sub.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, eval_expr_internal.h, exec_control_internal.h)
// NEEDED BY: libengine (exec_dispatch.c, exec_internal.h, let.c, ops.c)
// NEEDED BY: libengine (sub_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SUB statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_OOP_SUB_H
#define STATEMENTS_OOP_SUB_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);

BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);
BppError stmt_subend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_subexit_handler(VMContext *vm, LexerContext *lex);

BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex);

BppError vm_call_sub_procedure(VMContext *vm, LexerContext *lex);
BValue invoke_user_function(VMContext *vm, const char *name, BValue *args, int argc, BppError *err);
bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text);
bool find_procedure_ex(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text, bool *out_is_lib);

#endif // STATEMENTS_OOP_SUB_H
