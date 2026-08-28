// FILENAME: wend.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (wend.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the WEND statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_WEND_H
#define STMT_WEND_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_wend_handler(VMContext *vm, LexerContext *lex);
void stmt_wend_register(void);

#endif // STMT_WEND_H
