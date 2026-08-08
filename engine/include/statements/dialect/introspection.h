/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file introspection.h
 * @brief Header for INTROSPECTION and CATALOG keyword lookup statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the entry point function for INTROSPECTION statement execution.
 * - Why it exists: Provides modular API definitions for inclusion across VM execution dispatchers.
 * - Why it works this way: Function signature conforms to standard BppError (*)(VMContext*, LexerContext*).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Header guards and additional inline state query helpers.
 * - What cannot be changed: Declared handler function signature and error return semantics.
 * - What to expect: Pure interface declarations with no inline global variables.
 * - What to do if something breaks: Ensure prerequisite types.h and vm.h headers are included.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Self-contained header with direct required includes.
 * - Portability concerns: C17 compliant, portable across MSVC and GCC targets.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional helper functions within the module namespace.
 */

#ifndef STATEMENTS_DIALECT_INTROSPECTION_H
#define STATEMENTS_DIALECT_INTROSPECTION_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_introspection_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_DIALECT_INTROSPECTION_H */
