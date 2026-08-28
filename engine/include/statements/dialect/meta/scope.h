// FILENAME: scope.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c)
// NEEDED BY: libengine (scope.c, sub_internal.h)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SCOPE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_SCOPE_H
#define STATEMENTS_DIALECT_SCOPE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// @brief Main statement handler for SCOPE.
BppError stmt_scope_handler(VMContext *vm, LexerContext *lex);
void stmt_scope_register(void);

#endif // STATEMENTS_DIALECT_SCOPE_H
