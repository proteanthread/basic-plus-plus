/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file stmt_randomize.h
 * @brief RANDOMIZE statement micro-library header.
 *
 * WHAT IT DOES:
 * Declares statement handler and self-registration metadata interface for RANDOMIZE statement.
 *
 * WHY IT EXISTS:
 * Modularizes random seed generator initialization into an independent micro-library.
 *
 * WHY IT WORKS THIS WAY:
 * Provides clean header surface with zero cross-module leaks.
 *
 * WHAT CAN BE CHANGED:
 * Add PRNG algorithm configurations.
 *
 * WHAT CANNOT BE CHANGED:
 * C17 standards compliance and signature invariants.
 *
 * WHAT TO EXPECT:
 * Callable self-registration routine for boot initialization.
 *
 * WHAT TO DO IF SOMETHING BREAKS:
 * Verify system entropy / time seed.
 *
 * ASSUMPTIONS:
 * VMContext and LexerContext initialized.
 *
 * PORTABILITY CONCERNS:
 * Strict C17 compliance.
 *
 * FUTURE EXPANSIONS:
 * Hardware RNG seed generation.
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H
#define STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_randomize_handler(VMContext *vm, LexerContext *lex);
void stmt_randomize_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_RANDOMIZE_H */
