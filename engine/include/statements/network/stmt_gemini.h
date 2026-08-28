// FILENAME: stmt_gemini.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_gemini.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Declares the GEMINI statement handler for Gemini protocol transactions and serving.
//
// ---- Includes ----

#ifndef STMT_GEMINI_H
#define STMT_GEMINI_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses and executes the GEMINI statement (e.g. GEMINI.SERVE port, root$).
BppError stmt_gemini_handler(VMContext *vm, LexerContext *lex);

// @brief Registers GEMINI statement metadata in runtime registry.
void stmt_gemini_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_GEMINI_H
