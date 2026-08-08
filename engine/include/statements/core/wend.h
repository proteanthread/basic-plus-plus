/* =====================================================================
 * What it does: Header file for WEND statement handler micro-library.
 * Why it exists: Fulfills 1-to-1 keyword-to-filename mapping for WEND keyword.
 * Why it works this way: Declares WEND evaluation entry point and self-registration routine.
 * What can be changed: Error message strings.
 * What cannot be changed: BppError return type, C17 standard compliance.
 * What to expect: Pure execution jumping back to matching WHILE statement.
 * What to do if something breaks: Check active WhileStack frames in VM.
 * Assumptions: VMContext initialized.
 * Portability concerns: Fully standard C17.
 * Future expansions: Optimizations for backward jump target resolution.
 * ===================================================================== */

#ifndef STMT_WEND_H
#define STMT_WEND_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_wend_handler(VMContext *vm, LexerContext *lex);
void stmt_wend_register(void);

#endif /* STMT_WEND_H */
