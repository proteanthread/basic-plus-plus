/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file sub.h
 * @brief Public interface header for SUB procedure statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_sub_handler(), stmt_end_sub_handler(), stmt_procedure_handler(), and vm_call_sub_procedure().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle SUB procedures and procedure invocations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_sub'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Fully included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs) per Rule #1 (SUPPORT_OOP).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add procedure helper function prototypes for VM execution loops.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures for SUB procedure handlers.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_OOP_SUB_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/oop/sub.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_OOP_SUB_H
#define STATEMENTS_OOP_SUB_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_sub_handler(VMContext *vm, LexerContext *lex);

BppError stmt_end_sub_handler(VMContext *vm, LexerContext *lex);

BppError stmt_procedure_handler(VMContext *vm, LexerContext *lex);

BppError vm_call_sub_procedure(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_OOP_SUB_H */
