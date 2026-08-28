// FILENAME: def_usr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (def_usr.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DEF_USR statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_DEF_USR_H
#define STATEMENTS_SYSTEM_DEF_USR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_def_usr_handler(VMContext *vm, LexerContext *lex);
void stmt_def_usr_register(void);

#endif // STATEMENTS_SYSTEM_DEF_USR_H
