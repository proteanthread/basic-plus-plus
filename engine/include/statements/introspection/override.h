// FILENAME: override.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_control_internal.h, exec_internal.h, override.c)
// NEEDS: libengine (lexer.h, vm.h)
// NEEDS: libkernel (types.h)
// Declares the OVERRIDE statement handler interface in BASIC++.

#ifndef STATEMENTS_INTROSPECTION_OVERRIDE_H
#define STATEMENTS_INTROSPECTION_OVERRIDE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_override_handler(VMContext *vm, LexerContext *lex);
void stmt_override_register(void);

#endif // STATEMENTS_INTROSPECTION_OVERRIDE_H
