/**
 * @file system.h
 * @brief Public interface header for SYSTEM, BYE, SHELL, PAUSE, and state management statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_system_handler(), stmt_bye_handler(), stmt_shell_handler(), stmt_pause_handler(), stmt_sys_handler(), stmt_devices_handler(), stmt_nwrite_handler(), stmt_statesave_handler(), stmt_stateload_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle OS shell, process control, and VM state persistence statements.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_system'. Includes "lexer/lexer.h", "vm/vm.h", "types/errors.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add system snapshot prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_SYSTEM_SYSTEM_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/system/system.c
 * Prerequisite Header Files:
 * - engine/include/lexer/lexer.h
 * - engine/include/vm/vm.h
 * - engine/include/types/errors.h
 */

#ifndef STATEMENTS_SYSTEM_SYSTEM_H
#define STATEMENTS_SYSTEM_SYSTEM_H

#include "lexer/lexer.h"
#include "vm/vm.h"
#include "types/errors.h"

BppError stmt_system_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bye_handler(VMContext *vm, LexerContext *lex);
BppError stmt_shell_handler(VMContext *vm, LexerContext *lex);

BppError stmt_pause_handler(VMContext *vm, LexerContext *lex);

BppError stmt_sys_handler(VMContext *vm, LexerContext *lex);

BppError stmt_devices_handler(VMContext *vm, LexerContext *lex);

BppError stmt_nwrite_handler(VMContext *vm, LexerContext *lex);

BppError stmt_statesave_handler(VMContext *vm, LexerContext *lex);

BppError stmt_stateload_handler(VMContext *vm, LexerContext *lex);

#endif /* STATEMENTS_SYSTEM_SYSTEM_H */
