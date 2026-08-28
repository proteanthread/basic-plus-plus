// FILENAME: name.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (name.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the NAME statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_NAME_H
#define STATEMENTS_FILESYSTEM_NAME_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_name_handler(VMContext *vm, LexerContext *lex);

BppError stmt_setattr_handler(VMContext *vm, LexerContext *lex);

BppError stmt_environ_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_FILESYSTEM_NAME_H
