// FILENAME: interrupt.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (call.c, exec_internal.h, interrupt.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the INTERRUPT statement in BASIC++.
//
// ---- Includes ----

#ifndef STMT_INTERRUPT_H
#define STMT_INTERRUPT_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// @brief Handles INTERRUPT int_num%, inregs, outregs
BppError stmt_interrupt_handler(VMContext *vm, LexerContext *lex);

// @brief Handles INTERRUPTX int_num%, inregsX, outregsX
BppError stmt_interruptx_handler(VMContext *vm, LexerContext *lex);

void stmt_interrupt_register(void);

#ifdef __cplusplus
}
#endif

#endif // STMT_INTERRUPT_H
