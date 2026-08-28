// FILENAME: defseg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (defseg.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEFSEG statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_DEFSEG_H
#define STATEMENTS_SYSTEM_DEFSEG_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_SYSTEM_DEFSEG_H
