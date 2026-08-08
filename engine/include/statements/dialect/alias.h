/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file alias.h
 * @brief Header for ALIAS statement handler and alias table operations.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the entry point for ALIAS statement execution and keyword aliasing API.
 * - Why it exists: Allows users to create statement aliases, operator aliases, and query alias tables.
 * - Why it works this way: Function signature conforms to standard BppError (*)(VMContext*, LexerContext*).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Helper functions for alias management.
 * - What cannot be changed: Declared handler function signature and error return semantics.
 * - What to expect: Pure interface declarations with no inline global variables.
 * - What to do if something breaks: Ensure prerequisite types.h, vm.h, and lexer.h headers are included.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Self-contained header with direct required includes.
 * - Portability concerns: C17 compliant, portable across MSVC and GCC targets.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional helper functions within the module namespace.
 */

#ifndef STATEMENTS_DIALECT_ALIAS_H
#define STATEMENTS_DIALECT_ALIAS_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

/**
 * @brief Main statement handler for ALIAS.
 * Handles ALIAS <name> = <expansion>, ALIAS LIST, ALIAS CLEAR, ALIAS REMOVE <name>,
 * ALIAS COUNT, ALIAS SAVE <file>, ALIAS LOAD <file>, ALIAS OPER ..., etc.
 */
BppError stmt_alias_handler(VMContext *vm, LexerContext *lex);

/**
 * @brief Check if a name is a protected system keyword that cannot be aliased.
 */
bool vm_is_protected_keyword(const char *name);

#endif /* STATEMENTS_DIALECT_ALIAS_H */
