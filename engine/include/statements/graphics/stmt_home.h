// FILENAME: stmt_home.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_home.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the HOME statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_GRAPHICS_STMT_HOME_H
#define STATEMENTS_GRAPHICS_STMT_HOME_H

#include "vm/vm.h"
#include "lexer/lexer.h"
#include "types/errors.h"

void     stmt_home_register(void);
BppError stmt_home_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_GRAPHICS_STMT_HOME_H
