/* =====================================================================
 * What it does: Header file for FOR statement handler micro-library.
 * Why it exists: Fulfills 1-to-1 keyword-to-filename mapping and micro-library decoupling for FOR keyword.
 * Why it works this way: Declares FOR evaluation entry point and self-registration routine.
 * What can be changed: Pre-scan logic and default STEP behavior parameters.
 * What cannot be changed: BppError return type, C17 standard compliance.
 * What to expect: Pure execution pushing FOR stack frame or skipping body on initial failure.
 * What to do if something breaks: Check loop frame stack allocation or variable assignment.
 * Assumptions: VMContext and LexerContext initialized.
 * Portability concerns: Fully standard C17.
 * Future expansions: Support for array loop variables or complex iterator types.
 * ===================================================================== */

#ifndef STMT_FOR_H
#define STMT_FOR_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

BppError stmt_for_handler(VMContext *vm, LexerContext *lex);
void stmt_for_register(void);

#endif /* STMT_FOR_H */
