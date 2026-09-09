// FILENAME: menu.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (menu.c)
// NEEDS: libcore (types.h)
// Declares the MENU UI widget statement handler interface in BASIC++.

#ifndef MENU_H
#define MENU_H

#include "types/types.h"
#include "eval/eval.h"

BppError stmt_menu_handler(VMContext *vm, LexerContext *lex);
void stmt_menu_register(void);

#endif // MENU_H
