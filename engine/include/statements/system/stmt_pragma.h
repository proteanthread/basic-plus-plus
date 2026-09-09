// FILENAME: stmt_pragma.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_stmt.c, exec_dispatch.c
// NEEDS: types/types.h, lexer/lexer.h
// Prototypes for dedicated beginning-of-line !! pragma execution and legacy :: directive validation.

#ifndef STATEMENTS_SYSTEM_STMT_PRAGMA_H
#define STATEMENTS_SYSTEM_STMT_PRAGMA_H

#include "types/types.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct VMContext;
typedef struct VMContext VMContext;

// Executes a dedicated beginning-of-line !! pragma
BppError execute_pragma(VMContext *vm, LexerContext *lex, BppToken pragma_tok);

// Executes or validates a legacy :: compiler directive with deprecation diagnostics
BppError execute_directive(VMContext *vm, LexerContext *lex, BppToken dir_tok);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_SYSTEM_STMT_PRAGMA_H
