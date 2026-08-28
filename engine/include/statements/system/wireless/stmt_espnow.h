// FILENAME: stmt_espnow.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_espnow.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Declares the ESPNOW statement handler for connectionless peer-to-peer 2.4 GHz communication.
//
// ---- Includes ----

#ifndef STMT_ESPNOW_H
#define STMT_ESPNOW_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses and executes the ESPNOW statement and sub-commands.
BppError stmt_espnow_handler(VMContext *vm, LexerContext *lex);

// @brief Registers ESPNOW micro-library metadata in the runtime registry.
void stmt_espnow_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_ESPNOW_H
