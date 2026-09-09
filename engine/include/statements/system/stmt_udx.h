// FILENAME: stmt_udx.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c, common_reg_stmts.c
// NEEDS: types/types.h, vm/vm.h, lexer/lexer.h
// Implements runtime statements for Universal Data Exchange (UDX, XCHG, PUSH, POP).

#ifndef ENGINE_STATEMENTS_SYSTEM_STMT_UDX_H
#define ENGINE_STATEMENTS_SYSTEM_STMT_UDX_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Handler for UDX / XCHG statements
BppError stmt_udx_handler(VMContext *vm, LexerContext *lex);

// Handler for PUSH statement (PUSH [FIFO|LIFO|LILO|FILO] [, chan$] expr)
BppError stmt_push_handler(VMContext *vm, LexerContext *lex);

// Handler for POP statement (POP [FIFO|LIFO|LILO|FILO] [, chan$] var)
BppError stmt_pop_handler(VMContext *vm, LexerContext *lex);

// Registration for UDX, PUSH, POP statements
void stmt_udx_register(void);

#ifdef __cplusplus
}
#endif

#endif // ENGINE_STATEMENTS_SYSTEM_STMT_UDX_H
