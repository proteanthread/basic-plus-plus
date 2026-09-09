// FILENAME: frame.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (frame.c)
// NEEDS: libcore (types.h)
// Declares the FRAME UI control statement handler interface in BASIC++.

#ifndef FRAME_H
#define FRAME_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_frame_handler(VMContext *vm, LexerContext *lex);
void stmt_frame_register(void);

#endif // FRAME_H
