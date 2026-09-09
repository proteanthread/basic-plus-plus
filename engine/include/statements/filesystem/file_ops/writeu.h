// FILENAME: writeu.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, writeu.c)
// NEEDS: libengine (lexer.h, vm.h)
// Provides runtime implementation for the WRITEU statement in BASIC++.
//
// ---- Includes ----

#ifndef WRITEU_H
#define WRITEU_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_writeu_handler(VMContext *vm, LexerContext *lex);
void stmt_writeu_register(void);

#ifdef __cplusplus
}
#endif

#endif // WRITEU_H
