// FILENAME: override.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, override.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the OVERRIDE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_OVERRIDE_H
#define STATEMENTS_DIALECT_OVERRIDE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// @brief Main statement handler for OVERRIDE.
BppError stmt_override_handler(VMContext *vm, LexerContext *lex);
void stmt_override_register(void);

#endif // STATEMENTS_DIALECT_OVERRIDE_H
