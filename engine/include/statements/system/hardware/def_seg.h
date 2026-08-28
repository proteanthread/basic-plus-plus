// FILENAME: def_seg.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (def_seg.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEF_SEG statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_DEF_SEG_H
#define STATEMENTS_SYSTEM_DEF_SEG_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_def_seg_handler(VMContext *vm, LexerContext *lex);
void stmt_def_seg_register(void);

#endif // STATEMENTS_SYSTEM_DEF_SEG_H
