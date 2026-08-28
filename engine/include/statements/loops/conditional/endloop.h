// FILENAME: endloop.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (endloop.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the ENDLOOP statement in BASIC++.
//
// ---- Includes ----

#ifndef ENDLOOP_H
#define ENDLOOP_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_endloop_register(void);
BppError stmt_endloop_handler(VMContext *vm, LexerContext *lex);

void stmt_exitif_register(void);
BppError stmt_exitif_handler(VMContext *vm, LexerContext *lex);

#endif // ENDLOOP_H
