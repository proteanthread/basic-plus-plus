// FILENAME: category.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (category.c, conversion_fn.c)
// NEEDS: libengine (stmt.h)
// Provides runtime implementation for the CATEGORY statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_DIALECT_CATEGORY_H
#define STATEMENTS_DIALECT_CATEGORY_H

#include "stmt/stmt.h"

BppError stmt_category_handler(VMContext *vm, LexerContext *lex);
void stmt_category_register(void);

#endif // STATEMENTS_DIALECT_CATEGORY_H
