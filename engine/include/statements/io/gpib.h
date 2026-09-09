// FILENAME: gpib.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (exec_dispatch.c, gpib.c, sys_fn.c, dispatch_call.c)
// NEEDS: libkernel (vm.h, eval.h, lexer.h, errors.h)
// Declares Tektronix 4050 GPIB instrument I/O statements and functions (WBYTE, RBYTE).
//
// ---- Includes ----

#ifndef BPP_GPIB_H
#define BPP_GPIB_H

#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"
#include "lexer/lexer.h"

void stmt_gpib_register(void);
BppError stmt_wbyte_handler(VMContext *vm, LexerContext *lex);
BppError stmt_rbyte_handler(VMContext *vm, LexerContext *lex);

BValue func_wbyte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_rbyte_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // BPP_GPIB_H
