// FILENAME: alias.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (alias.c, exec_internal.h, scope.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ALIAS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_ALIAS_H
#define STATEMENTS_DIALECT_ALIAS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

// @brief Main statement handler for ALIAS.
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);
void stmt_alias_register(void);

// @brief Check if a name is a protected system keyword that cannot be aliased.
bool vm_is_protected_keyword(const char *name);

#endif // STATEMENTS_DIALECT_ALIAS_H
