// FILENAME: doevents.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (doevents.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the DOEVENTS statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_CONTROL_DOEVENTS_H
#define STATEMENTS_CONTROL_DOEVENTS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_doevents_register(void);
BppError stmt_doevents_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_CONTROL_DOEVENTS_H
