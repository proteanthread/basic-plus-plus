// FILENAME: verify.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (verify.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the VERIFY statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DEBUG_VERIFY_H
#define STATEMENTS_DEBUG_VERIFY_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_verify_handler(VMContext *vm, LexerContext *lex);
void stmt_verify_register(void);

#endif // STATEMENTS_DEBUG_VERIFY_H
