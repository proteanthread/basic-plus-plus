/* =====================================================================
 * What it does: Declarations for the BITMUX statement handler.
 * Why it exists: Provides public function signature for stmt_bitmux.c micro-library target.
 * Why it works this way: Defines stmt_bitmux_handler function prototype.
 * What can be changed: Additional helper function prototypes if expanded.
 * What cannot be changed: C17 header guards, VMContext / LexerContext types.
 * What to expect: Clean compilation when included independently.
 * What to do if something breaks: Check included header paths relative to engine/include/.
 * Assumptions: VMContext and LexerContext types are defined.
 * Portability concerns: Strict C17 compliant.
 * Future expansions: Multi-dimensional BITMUX statement declarations.
 * ===================================================================== */

#ifndef STATEMENTS_STMT_BITMUX_H
#define STATEMENTS_STMT_BITMUX_H

#include "vm/vm.h"
#include "lexer/lexer.h"

BppError stmt_bitmux_handler(VMContext *vm, LexerContext *lex);
void stmt_bitmux_register(void);

#endif /* STATEMENTS_STMT_BITMUX_H */
