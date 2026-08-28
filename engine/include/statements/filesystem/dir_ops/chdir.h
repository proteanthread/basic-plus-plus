// FILENAME: chdir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (chdir.c, prefix.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the CHDIR statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_CHDIR_H
#define STATEMENTS_FILESYSTEM_CHDIR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_chdir_handler(VMContext *vm, LexerContext *lex);
void stmt_chdir_register(void);

#endif // STATEMENTS_FILESYSTEM_CHDIR_H
