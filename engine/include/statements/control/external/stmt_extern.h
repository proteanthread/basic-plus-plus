// FILENAME: stmt_extern.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// Header for dynamic FFI EXTERN statement in BASIC++.

#ifndef STMT_EXTERN_H
#define STMT_EXTERN_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_extern_handler(VMContext *vm, LexerContext *lex);
void     stmt_extern_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_EXTERN_H
