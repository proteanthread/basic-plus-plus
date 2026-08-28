// FILENAME: complex.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (complex.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the COMPLEX statement in BASIC++.
//
// ---- Includes ----

#ifndef STATEMENTS_VARIABLES_COMPLEX_H
#define STATEMENTS_VARIABLES_COMPLEX_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_complex_register(void);
BppError stmt_complex_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_VARIABLES_COMPLEX_H
