/* Copyleft (c) 2026, BASIC++ Community. All wrongs reserved.
 *
 * This file is part of BASIC++ - a modular, portable BASIC language framework.
 * See LICENSE for terms. See docs/ for programmer guides.
 */

/**
 * @file mux.h
 * @brief Public interface header for MUX, DEMUX, UNPACK, and BITMUX multiplexing statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_mux_handler(), stmt_demux_handler(), stmt_unpack_handler(), stmt_bitmux_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle multiplexer operations.
 *
 * 3. WHY IT WORKS THIS WAY:
 * Conforms to standard statement handler signature BppError (*)(VMContext*, LexerContext*).
 *
 * 4. DEPENDENCIES & COMPILATION:
 * Micro-library target 'stmt_mux'. Includes "types/types.h", "vm/vm.h", "lexer/lexer.h".
 *
 * 5. EDITION INCLUSION & EXCLUSION:
 * Included in all editions ('baspp', 'bpp', 'bs').
 *
 * 6. HOW TO MODIFY OR EXTEND IT:
 * Add hardware pin demuxing prototypes.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_SYSTEM_MUX_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/system/mux.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_STMT_EXTENDED_STMT_MUX_H
#define STATEMENTS_STMT_EXTENDED_STMT_MUX_H

#include "lexer/lexer.h"
#include "vm/vm.h"

BppError stmt_mux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_demux_handler(VMContext *vm, LexerContext *lex);
BppError stmt_unpack_handler(VMContext *vm, LexerContext *lex);
BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);
void stmt_mux_register(void);

#endif /* STATEMENTS_STMT_EXTENDED_STMT_MUX_H */
