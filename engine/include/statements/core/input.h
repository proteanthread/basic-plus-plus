/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file input.h
 * @brief Public interface header for INPUT statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares the public function signature stmt_input_handler() for interactive INPUT execution.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM dispatcher to trigger INPUT statement execution.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_input'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add line-reading option configuration signatures if expanding console input features.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signature stmt_input_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Returns BppError enum value.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Check include paths for types.h, vm.h, and lexer.h.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Included self-sufficiently by statement dispatch table.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * C17 compliant header guard STATEMENTS_CORE_INPUT_H.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/core/input.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_CORE_INPUT_H
#define STATEMENTS_CORE_INPUT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_input_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_CORE_INPUT_H */
