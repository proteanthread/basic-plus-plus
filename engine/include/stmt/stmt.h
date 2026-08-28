// FILENAME: stmt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore, libengine, libkernel, libstandard
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides core logic and interface definitions for stmt within BASIC++.
//
// ---- Includes ----

#ifndef STMT_H
#define STMT_H

#include <stdint.h>
#include <stdbool.h>

#include "types/types.h"
#include "lexer/lexer.h"
#include "vm/vm.h"



// Flags for statement behavior configuration
#define STMT_FLAG_NONE       0
#define STMT_FLAG_IMMEDIATE  (1 << 0)  // Can be run directly in REPL (no line number)
#define STMT_FLAG_PROGRAM    (1 << 1)  // Can be run inside program lines
#define STMT_FLAG_BOTH       (STMT_FLAG_IMMEDIATE | STMT_FLAG_PROGRAM)

// Statement handler function pointer signature
typedef BppError (*BppStmtHandler)(VMContext *vm, LexerContext *lex);
BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);

// Opaque Statement Registry Context
typedef struct StmtRegistry StmtRegistry;

// @brief Initialize the statement registry context.
StmtRegistry *stmt_registry_init(MemoryContext *mem);

// @brief Shutdown the statement registry.
void stmt_registry_shutdown(StmtRegistry *reg);

// @brief Register a statement handler.
// @param reg Statement registry pointer.
// @param kw Keyword ID to map.
// @param handler Statement execution callback.
// @param name Diagnostic string name of the command.
// @param flags Configuration behavior flags.
void stmt_register(StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags);

// @brief Look up a registered statement handler.
// @return Function pointer if registered, NULL otherwise.
BppStmtHandler stmt_lookup(StmtRegistry *reg, BppKeywordId kw);

BppError vm_call_sub_procedure(VMContext *vm, LexerContext *lex);
bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text);

BppError stmt_assert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex);
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_break_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex);
const char *error_get_message(int code);

#endif // STMT_H
