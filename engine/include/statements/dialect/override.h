/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file override.h
 * @brief Header for OVERRIDE statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares entry point for OVERRIDE statement handling.
 * - Why it exists: Provides language-level statement for replacing statement and function behaviors.
 * - Why it works this way: Standard handler signature returning BppError.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Header guards and additional helper prototypes.
 * - What cannot be changed: Declared handler function signature.
 * - What to expect: Pure interface header.
 * - What to do if something breaks: Check included types and lexer headers.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Strict C17 standard compliance across MSVC and GCC.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Add additional OVERRIDE syntax forms.
 */

#ifndef STATEMENTS_DIALECT_OVERRIDE_H
#define STATEMENTS_DIALECT_OVERRIDE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

/**
 * @brief Main statement handler for OVERRIDE.
 * Handles OVERRIDE <target> WITH GOSUB <line>, OVERRIDE <target> WITH <sub_name>,
 * OVERRIDE CLEAR.
 */
BppError stmt_override_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_DIALECT_OVERRIDE_H */
