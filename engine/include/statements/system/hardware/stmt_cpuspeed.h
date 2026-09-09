// FILENAME: stmt_cpuspeed.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine
// Statement implementation for CPUSPEED statement (e.g. CPUSPEED EMULATE).

#ifndef STMT_CPUSPEED_H
#define STMT_CPUSPEED_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

void stmt_cpuspeed_register(void);
BppError stmt_cpuspeed_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STMT_CPUSPEED_H
