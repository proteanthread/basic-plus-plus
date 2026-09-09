// FILENAME: timercontrol.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (timercontrol.c)
// NEEDS: libcore (types.h)
// Declares the TIMERCONTROL UI control statement handler interface in BASIC++.

#ifndef TIMERCONTROL_H
#define TIMERCONTROL_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_timercontrol_handler(VMContext *vm, LexerContext *lex);
void stmt_timercontrol_register(void);

#endif // TIMERCONTROL_H
