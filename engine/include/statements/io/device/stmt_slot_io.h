// FILENAME: stmt_slot_io.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine, exec_dispatch.c
// NEEDS: libkernel (types.h, lexer.h, errors.h)
// Header for Apple II PR# and IN# statement micro-library.
//
// ---- Includes ----

#ifndef STATEMENTS_IO_DEVICE_STMT_SLOT_IO_H
#define STATEMENTS_IO_DEVICE_STMT_SLOT_IO_H

#include "types/types.h"
#include "types/errors.h"
#include "lexer/lexer.h"
#include "vm/vm.h"

#ifdef __cplusplus
extern "C" {
#endif

void     stmt_slot_io_register(void);
BppError stmt_pr_handler(VMContext *vm, LexerContext *lex);
BppError stmt_in_handler(VMContext *vm, LexerContext *lex);

#ifdef __cplusplus
}
#endif

#endif // STATEMENTS_IO_DEVICE_STMT_SLOT_IO_H
