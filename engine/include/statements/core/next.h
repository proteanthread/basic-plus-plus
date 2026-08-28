// FILENAME: next.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (next.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the NEXT statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_NEXT_H
#define STMT_NEXT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_next_handler(VMContext *vm, LexerContext *lex);
void stmt_next_register(void);

#endif // STMT_NEXT_H
