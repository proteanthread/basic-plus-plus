// FILENAME: session_stmts.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (session_stmts.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SESSION_STMTS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_SESSION_STMTS_H
#define STATEMENTS_SYSTEM_SESSION_STMTS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_session_register(void);

BppError stmt_login_handler(VMContext *vm, LexerContext *lex);
BppError stmt_who_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tty_handler(VMContext *vm, LexerContext *lex);
BppError stmt_priority_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_SYSTEM_SESSION_STMTS_H
