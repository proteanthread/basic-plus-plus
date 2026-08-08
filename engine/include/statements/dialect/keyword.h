/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file keyword.h
 * @brief Header for KEYWORD property management statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares the entry point function for KEYWORD statement execution.
 * - Why it exists: Provides statement-level controls for inspecting and modifying keyword attributes.
 * - Why it works this way: Function signature conforms to standard BppError (*)(VMContext*, LexerContext*).
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Header guards and additional helper prototypes.
 * - What cannot be changed: Declared handler function signature.
 * - What to expect: Pure interface declarations with self-contained direct header includes.
 * - What to do if something breaks: Ensure prerequisite types.h, vm.h, and lexer.h headers are included.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: C17 compliant across MSVC and GCC targets.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional KEYWORD sub-command handlers.
 */

#ifndef STATEMENTS_DIALECT_KEYWORD_H
#define STATEMENTS_DIALECT_KEYWORD_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

/**
 * @brief Main statement handler for KEYWORD.
 * Handles KEYWORD <kw> SET <prop> = <val>, KEYWORD <kw> GET <prop>,
 * KEYWORD <kw> LIST, KEYWORD CLEAR, etc.
 */
BppError stmt_keyword_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_DIALECT_KEYWORD_H */
