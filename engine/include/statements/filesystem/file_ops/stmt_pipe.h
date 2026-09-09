// FILENAME: stmt_pipe.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, BASIC++ runtime
// Header for PIPE and STREAMPIPE statements in BASIC++.

#ifndef STMT_PIPE_H
#define STMT_PIPE_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

BppError stmt_pipe_handler(VMContext *vm, LexerContext *lex);
void     stmt_pipe_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_PIPE_H
