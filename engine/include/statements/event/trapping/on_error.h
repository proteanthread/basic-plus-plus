// FILENAME: on_error.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_error.c, on_timer.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ON_ERROR statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EVENT_ON_ERROR_H
#define STATEMENTS_EVENT_ON_ERROR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_on_error_handler(VMContext *vm, LexerContext *lex);

BppError stmt_onerr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_error_statement_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_EVENT_ON_ERROR_H
