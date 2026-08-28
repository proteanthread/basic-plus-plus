// FILENAME: stop.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stop.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the STOP statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_STOP_H
#define STMT_STOP_H

#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_stop_handler(VMContext *vm, LexerContext *lex);
void stmt_stop_register(void);

#endif // STMT_STOP_H
