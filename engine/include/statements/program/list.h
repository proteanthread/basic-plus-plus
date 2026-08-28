// FILENAME: list.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (list.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the LIST statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_PROGRAM_LIST_H
#define STATEMENTS_PROGRAM_LIST_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_list_handler(VMContext *vm, LexerContext *lex);

BppError stmt_auto_handler(VMContext *vm, LexerContext *lex);

BppError stmt_renum_handler(VMContext *vm, LexerContext *lex);

BppError stmt_delete_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_PROGRAM_LIST_H
