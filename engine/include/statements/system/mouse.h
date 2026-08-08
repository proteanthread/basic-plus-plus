/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mouse.h
 * @brief Public interface header for MOUSE pointer tracking statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_mouse_handler(), stmt_mouseinput_handler(), stmt_mousehide_handler(), stmt_mouseshow_handler(), stmt_hmouse_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle mouse cursor visibility and event tracking statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_mouse'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Standard Edition ('baspp') strictly per Rule #1 (Excluded Subsystems from bpp/bs). Excluded from 'bpp' and 'bs'.
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add mouse event listener prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_SYSTEM_MOUSE_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/system/mouse.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_SYSTEM_MOUSE_H
#define STATEMENTS_SYSTEM_MOUSE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mouse_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mouseinput_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mousehide_handler(VMContext *vm, LexerContext *lex);

BppError stmt_mouseshow_handler(VMContext *vm, LexerContext *lex);

BppError stmt_hmouse_handler(VMContext *vm, LexerContext *lex);

BppError stmt_vmouse_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_SYSTEM_MOUSE_H */
