// FILENAME: cause.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (cause.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CAUSE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_CAUSE_H
#define STATEMENTS_CONTROL_CAUSE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_cause_handler(VMContext *vm, LexerContext *lex);
void stmt_cause_register(void);

#endif // STATEMENTS_CONTROL_CAUSE_H
