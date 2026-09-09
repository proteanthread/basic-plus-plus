// FILENAME: scope.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, module.c, scope.c, sub_internal.h)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libkernel (types.h)
// Declares the SCOPE statement handler interface in BASIC++.

#ifndef STATEMENTS_INTROSPECTION_SCOPE_H
#define STATEMENTS_INTROSPECTION_SCOPE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_scope_handler(VMContext *vm, LexerContext *lex);
void stmt_scope_register(void);

#endif // STATEMENTS_INTROSPECTION_SCOPE_H
