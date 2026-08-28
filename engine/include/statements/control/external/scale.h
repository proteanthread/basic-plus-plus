// FILENAME: scale.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (scale.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the SCALE statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_SCALE_H
#define STATEMENTS_CONTROL_SCALE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_scale_handler(VMContext *vm, LexerContext *lex);
void stmt_scale_register(void);

#endif // STATEMENTS_CONTROL_SCALE_H
