// FILENAME: rmdir.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (rmdir.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the RMDIR statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_FILESYSTEM_RMDIR_H
#define STATEMENTS_FILESYSTEM_RMDIR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_rmdir_handler(VMContext *vm, LexerContext *lex);
void stmt_rmdir_register(void);

#endif // STATEMENTS_FILESYSTEM_RMDIR_H
