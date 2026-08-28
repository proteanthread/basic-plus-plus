// FILENAME: declare.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (declare.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DECLARE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_DECLARE_H
#define STATEMENTS_CONTROL_DECLARE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_declare_handler(VMContext *vm, LexerContext *lex);
void stmt_declare_register(void);

#endif // STATEMENTS_CONTROL_DECLARE_H
