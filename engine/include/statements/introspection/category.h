// FILENAME: category.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (category.c)
// NEEDS: libengine (stmt.h)
// Declares the CATEGORY statement handler interface in BASIC++.

#ifndef CATEGORY_H
#define CATEGORY_H

#include "stmt/stmt.h"

BppError stmt_category_handler(VMContext *vm, LexerContext *lex);
void stmt_category_register(void);

#endif // CATEGORY_H
