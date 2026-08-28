// FILENAME: extend.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (extend.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h)
// Provides runtime implementation for the EXTEND statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_EXTEND_H
#define STATEMENTS_CONTROL_EXTEND_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_extend_handler(VMContext *vm, LexerContext *lex);
BppError stmt_noextend_handler(VMContext *vm, LexerContext *lex);
void stmt_extend_register(void);

#endif // STATEMENTS_CONTROL_EXTEND_H
