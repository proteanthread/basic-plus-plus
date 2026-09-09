// FILENAME: readu.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, readu.c)
// NEEDS: libengine (lexer.h, vm.h)
// Provides runtime implementation for the READU statement in BASIC++.
//
// ---- Includes ----

#ifndef READU_H
#define READU_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_readu_handler(VMContext *vm, LexerContext *lex);
void stmt_readu_register(void);

#ifdef __cplusplus
}
#endif

#endif // READU_H
