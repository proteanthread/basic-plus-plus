// FILENAME: on_timer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (on_timer.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the ON_TIMER statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_EVENT_ON_TIMER_H
#define STATEMENTS_EVENT_ON_TIMER_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_on_timer_handler(VMContext *vm, LexerContext *lex);

BppError stmt_on_handler(VMContext *vm, LexerContext *lex);

BppError stmt_timer_handler(VMContext *vm, LexerContext *lex);

BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex);

BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex);

BppError stmt_set_handler(VMContext *vm, LexerContext *lex);

#endif // STATEMENTS_EVENT_ON_TIMER_H
