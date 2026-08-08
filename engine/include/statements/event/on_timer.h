/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file on_timer.h
 * @brief Public interface header for ON TIMER event trap statement handler.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions for ON TIMER, ON, TIMER, and ALARM statements.
 *
 * 2. WHY IT EXISTS:
 * Exposes the public API interface allowing the VM statement dispatcher to handle ON TIMER statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_on_timer'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in libbasicpp (baspp) and libbasicpp_lite (bpp, bs).
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add timer event status query function signatures for interactive debugging.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_on_timer_handler, stmt_on_handler, stmt_timer_handler, stmt_alarm_handler.
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return type.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_EVENT_ON_TIMER_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/event/on_timer.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_EVENT_ON_TIMER_H
#define STATEMENTS_EVENT_ON_TIMER_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_on_timer_handler(VMContext *vm, LexerContext *lex);

BppError stmt_on_handler(VMContext *vm, LexerContext *lex);

BppError stmt_timer_handler(VMContext *vm, LexerContext *lex);

BppError stmt_alarm_handler(VMContext *vm, LexerContext *lex);

BppError stmt_alarm_str_handler(VMContext *vm, LexerContext *lex);

BppError stmt_set_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_EVENT_ON_TIMER_H */
