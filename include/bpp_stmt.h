/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */
/**
 * @file bpp_stmt.h
 * @brief Statement Registry and Command Dispatcher API.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares registry interfaces and handler signatures for BASIC statement commands
 *   (e.g. PRINT, LET, GOTO).
 * - Why it exists: Decouples the parser from command implementations. Adding a new command does not
 *   require editing the parser code, preventing regression failures.
 * - Why it works this way: It maintains a lookup map from KeywordId to BppStmtHandler function pointers.
 *   Built-in and external plugin commands register themselves at boot, and the parser dispatches keywords
 *   via this table.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Handler signatures, flag parameters, registry bounds.
 * - What cannot be changed: Obligation to register all statement commands.
 * - What to expect: Registering a command binds it dynamically to the keyword dispatch loop.
 * - What to do if something breaks: If a statement fails to execute or syntax errors occur,
 *   verify that the statement was registered during Boot Phase 4.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Handlers return a structured BppError on failure, and return an empty error on success.
 * - Portability concerns: None. C17 standard compliant.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add command scoping, alias mappings, or pre-parse validators.
 * - How to write external extensions: External plugins call stmt_register to add custom keywords
 *   to the registry at runtime.
 */

#ifndef BPP_STMT_H
#define BPP_STMT_H

#include <stdint.h>
#include <stdbool.h>

#include "bpp_types.h"
#include "bpp_lexer.h"
#include "bpp_vm.h"

/* Flags for statement behavior configuration */
#define STMT_FLAG_NONE       0
#define STMT_FLAG_IMMEDIATE  (1 << 0)  /* Can be run directly in REPL (no line number) */
#define STMT_FLAG_PROGRAM    (1 << 1)  /* Can be run inside program lines */
#define STMT_FLAG_BOTH       (STMT_FLAG_IMMEDIATE | STMT_FLAG_PROGRAM)

/* Statement handler function pointer signature */
typedef BppError (*BppStmtHandler)(VMContext *vm, LexerContext *lex);
BppError stmt_defseg_handler(VMContext *vm, LexerContext *lex);

/* Opaque Statement Registry Context */
typedef struct StmtRegistry StmtRegistry;

/**
 * @brief Initialize the statement registry context.
 */
StmtRegistry *stmt_registry_init(MemoryContext *mem);

/**
 * @brief Shutdown the statement registry.
 */
void stmt_registry_shutdown(StmtRegistry *reg);

/**
 * @brief Register a statement handler.
 * @param reg Statement registry pointer.
 * @param kw Keyword ID to map.
 * @param handler Statement execution callback.
 * @param name Diagnostic string name of the command.
 * @param flags Configuration behavior flags.
 */
void stmt_register(StmtRegistry *reg, BppKeywordId kw, BppStmtHandler handler, const char *name, uint32_t flags);

/**
 * @brief Look up a registered statement handler.
 * @return Function pointer if registered, NULL otherwise.
 */
BppStmtHandler stmt_lookup(StmtRegistry *reg, BppKeywordId kw);

BppError vm_call_sub_procedure(VMContext *vm, const char *sub_name, BValue *args, int arg_count, const char *ret_pos);
bool find_procedure(VMContext *vm, const char *name, BppKeywordId proc_kw, BppLineNumber *out_line, const char **out_text);

BppError stmt_assert_handler(VMContext *vm, LexerContext *lex);
BppError stmt_tron_handler(VMContext *vm, LexerContext *lex);
BppError stmt_troff_handler(VMContext *vm, LexerContext *lex);
BppError stmt_break_handler(VMContext *vm, LexerContext *lex);
BppError stmt_vars_handler(VMContext *vm, LexerContext *lex);
const char *error_get_message(int code);

#endif /* BPP_STMT_H */
