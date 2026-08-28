// FILENAME: select.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libplatform (plat_clipboard.c, plat_console.c, plat_dl.c)
// NEEDED BY: libplatform (plat_fs.c, plat_net.c, plat_regex.c, plat_sys.c)
// NEEDED BY: libplatform (plat_thread.c, plat_time.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the SELECT statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_STMT_EXTENDED_STMT_SELECT_H
#define STATEMENTS_STMT_EXTENDED_STMT_SELECT_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_select_handler(VMContext *vm, LexerContext *lex);
void stmt_select_register(void);

#endif // STATEMENTS_STMT_EXTENDED_STMT_SELECT_H
