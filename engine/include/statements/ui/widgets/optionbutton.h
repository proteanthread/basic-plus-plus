// FILENAME: optionbutton.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (optionbutton.c)
// NEEDS: libcore (types.h)
// Declares the OPTIONBUTTON UI control statement handler interface in BASIC++.

#ifndef OPTIONBUTTON_H
#define OPTIONBUTTON_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_optionbutton_handler(VMContext *vm, LexerContext *lex);
void stmt_optionbutton_register(void);

#endif // OPTIONBUTTON_H
