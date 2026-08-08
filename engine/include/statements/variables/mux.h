/**
 * @file mux.h
 * @brief Public interface header for MUX, DEMUX, UNPACK, and BITMUX statement handlers.
 *
 * 1. WHAT IT DOES:
 * Declares public handler functions stmt_mux_handler(), stmt_demux_handler(), stmt_unpack_handler(), and stmt_bitmux_handler().
 *
 * 2. WHY IT EXISTS:
 * Exposes public API interface allowing VM statement dispatcher to handle variable and bit multiplexing statements.
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
 * Add helper function declarations for bitfield packing.
 *
 * 7. WHAT CANNOT BE CHANGED:
 * Public function signatures stmt_*_handler(VMContext*, LexerContext*).
 *
 * 8. WHAT TO EXPECT:
 * Declares BppError return types.
 *
 * 9. WHAT TO DO IF SOMETHING BREAKS:
 * Verify header guard STATEMENTS_VARIABLES_MUX_H and include surfaces.
 *
 * 10. ASSUMPTIONS & PRECONDITIONS:
 * Valid initialized VMContext and LexerContext pointers.
 *
 * 11. PORTABILITY & C17 CONCERNS:
 * Strict C17 compliance. Self-contained include guard.
 *
 * 12. COMPONENT DEPENDENCIES & PREREQUISITE SOURCE FILES:
 * Prerequisite Source Files:
 * - engine/src/statements/variables/mux.c
 * Prerequisite Header Files:
 * - engine/include/types/types.h
 * - engine/include/vm/vm.h
 * - engine/include/lexer/lexer.h
 */

#ifndef STATEMENTS_VARIABLES_MUX_H
#define STATEMENTS_VARIABLES_MUX_H
#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_mux_handler(VMContext *vm, LexerContext *lex);

BppError stmt_demux_handler(VMContext *vm, LexerContext *lex);

BppError stmt_unpack_handler(VMContext *vm, LexerContext *lex);

BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);

#endif
