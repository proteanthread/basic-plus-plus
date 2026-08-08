/* =====================================================================
 * What it does: Header file for NEXT statement handler micro-library.
 * Why it exists: Fulfills 1-to-1 keyword-to-filename mapping for NEXT keyword.
 * Why it works this way: Declares NEXT evaluation entry point and self-registration routine.
 * What can be changed: Pre-scan logic and loop variable matching rules.
 * What cannot be changed: BppError return type, C17 standard compliance.
 * What to expect: Pure execution incrementing loop variable and jumping or popping frame.
 * What to do if something breaks: Check active ForStack frames in VM.
 * Assumptions: VMContext initialized.
 * Portability concerns: Fully standard C17.
 * Future expansions: Support for comma-separated NEXT variable lists.
 * ===================================================================== */

#ifndef STMT_NEXT_H
#define STMT_NEXT_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_next_handler(VMContext *vm, LexerContext *lex);
void stmt_next_register(void);

#endif /* STMT_NEXT_H */
