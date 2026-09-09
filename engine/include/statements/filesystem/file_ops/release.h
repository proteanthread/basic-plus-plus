// FILENAME: release.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, release.c)
// NEEDS: libengine (lexer.h, vm.h)
// Provides runtime implementation for the RELEASE statement in BASIC++.
//
// ---- Includes ----

#ifndef RELEASE_H
#define RELEASE_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_release_handler(VMContext *vm, LexerContext *lex);
void stmt_release_register(void);

#ifdef __cplusplus
}
#endif

#endif // RELEASE_H
