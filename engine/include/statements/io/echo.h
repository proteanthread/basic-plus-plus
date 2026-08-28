// FILENAME: echo.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (echo.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the ECHO statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_ECHO_H
#define STATEMENTS_IO_ECHO_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_echo_handler(VMContext *vm, LexerContext *lex);
BppError stmt_noecho_handler(VMContext *vm, LexerContext *lex);
void stmt_echo_register(void);

#endif // STATEMENTS_IO_ECHO_H
