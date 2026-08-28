// FILENAME: if.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (if.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// Provides runtime implementation for the IF statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_IF_H
#define STMT_IF_H

#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Handles execution of the IF/THEN/ELSE statement.
// @param vm Active Virtual Machine context.
// @param lex Active Lexer context positioned at token after IF.
// @return BppError structure indicating success or error details.
BppError stmt_if_handler(VMContext *vm, LexerContext *lex);
void stmt_if_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_IF_H
