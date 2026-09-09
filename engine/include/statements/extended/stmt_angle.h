// FILENAME: stmt_angle.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, stmt_angle.c)
// NEEDS: libkernel (vm.h, lexer.h, errors.h)
// Declares statement handlers for DEGREE, RADIAN, and GRAD angle modes.
//
// ---- Includes ----

#ifndef BPP_STMT_ANGLE_H
#define BPP_STMT_ANGLE_H

#include "types/errors.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

void stmt_angle_register(void);
BppError stmt_degree_handler(VMContext *vm, LexerContext *lex);
BppError stmt_radian_handler(VMContext *vm, LexerContext *lex);
BppError stmt_grad_handler(VMContext *vm, LexerContext *lex);

#endif // BPP_STMT_ANGLE_H
