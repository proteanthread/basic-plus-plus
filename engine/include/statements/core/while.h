/* =====================================================================
 * What it does: Header file for WHILE statement handler micro-library.
 * Why it exists: Fulfills 1-to-1 keyword-to-filename mapping for WHILE keyword.
 * Why it works this way: Declares WHILE evaluation entry point and self-registration routine.
 * What can be changed: Pre-scan and skip-to-WEND logic.
 * What cannot be changed: BppError return type, C17 standard compliance.
 * What to expect: Pure execution pushing WhileStack frame or skipping to matching WEND.
 * What to do if something breaks: Check active WhileStack frames in VM.
 * Assumptions: VMContext initialized.
 * Portability concerns: Fully standard C17.
 * Future expansions: Support for short-circuit boolean condition evaluation.
 * ===================================================================== */

#ifndef STMT_WHILE_H
#define STMT_WHILE_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_while_handler(VMContext *vm, LexerContext *lex);
void stmt_while_register(void);

#endif /* STMT_WHILE_H */
