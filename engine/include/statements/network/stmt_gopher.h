// FILENAME: stmt_gopher.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (stmt_gopher.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Declares the GOPHER statement handler for RFC 1436 Gopher protocol navigation and serving.
//
// ---- Includes ----

#ifndef STMT_GOPHER_H
#define STMT_GOPHER_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Parses and executes the GOPHER statement (e.g. GOPHER.SERVE port, root$).
BppError stmt_gopher_handler(VMContext *vm, LexerContext *lex);

// @brief Registers GOPHER statement metadata in runtime registry.
void stmt_gopher_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_GOPHER_H
