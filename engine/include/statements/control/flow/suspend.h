// FILENAME: suspend.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (suspend.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SUSPEND statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_SUSPEND_H
#define STMT_SUSPEND_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_suspend_handler(VMContext *vm, LexerContext *lex);
void stmt_suspend_register(void);

#endif // STMT_SUSPEND_H
