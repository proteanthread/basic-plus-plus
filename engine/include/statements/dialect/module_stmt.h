/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file module_stmt.h
 * @brief Header for MODULE statement handler.
 *
 * SECTION 1: WHAT IT DOES, WHY IT EXISTS, AND WHY IT WORKS THIS WAY
 * - What it does: Declares entry point for MODULE statement handling.
 * - Why it exists: Provides language-level namespace boundaries and module loading constructs.
 * - Why it works this way: Standard handler signature returning BppError.
 *
 * SECTION 2: DEVELOPER MAINTENANCE & MODIFICATION GUIDE
 * - What can be changed: Header guards and additional helper prototypes.
 * - What cannot be changed: Declared handler function signature.
 * - What to expect: Interface header for MODULE statement execution.
 * - What to do if something breaks: Check included types, vm, and lexer headers.
 *
 * SECTION 3: ASSUMPTIONS & PORTABILITY CONCERNS
 * - Assumptions: Strict C17 standard compliance across MSVC and GCC targets.
 * - Portability concerns: None.
 *
 * SECTION 4: FUTURE EXPANSIONS & EXTENSION HOOKS
 * - How future expansion can occur safely: Support MODULE EXPORT syntax.
 */

#ifndef STATEMENTS_DIALECT_MODULE_STMT_H
#define STATEMENTS_DIALECT_MODULE_STMT_H

#include "stmt/stmt.h"

#endif /* STATEMENTS_DIALECT_MODULE_STMT_H */
