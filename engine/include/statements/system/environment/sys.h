// FILENAME: sys.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (sys.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the SYS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_SYSTEM_SYS_H
#define STATEMENTS_SYSTEM_SYS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex);
void stmt_sys_register(void);

#endif // STATEMENTS_SYSTEM_SYS_H
