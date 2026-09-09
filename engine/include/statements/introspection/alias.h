// FILENAME: alias.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (alias.c, exec_internal.h, scope.c)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libkernel (types.h)
// Declares the ALIAS statement handler interface in BASIC++.

#ifndef ALIAS_H
#define ALIAS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);
void stmt_alias_register(void);

bool vm_is_protected_keyword(const char *name);

#endif // ALIAS_H
