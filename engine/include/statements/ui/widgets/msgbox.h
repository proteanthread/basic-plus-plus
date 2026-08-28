// FILENAME: msgbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (msgbox.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the MSGBOX statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_UI_MSGBOX_H
#define STATEMENTS_UI_MSGBOX_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_msgbox_register(void);
BppError stmt_msgbox_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_UI_MSGBOX_H
