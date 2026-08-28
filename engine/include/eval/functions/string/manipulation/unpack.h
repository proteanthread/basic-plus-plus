// FILENAME: unpack.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (unpack.c)
// NEEDS: libengine (lexer.h, lexer.c, vm.h)
// NEEDS: libkernel (types.h)
// Provides runtime implementation for the UNPACK built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCS_STRING_FUNC_UNPACK_H
#define EVAL_FUNCS_STRING_FUNC_UNPACK_H

#include "types/types.h"
#include "vm/vm.h"
#include "lexer/lexer.h"

BValue func_unpack_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_unpack_register(void);
void   func_unpack_register(void);

#endif // EVAL_FUNCS_STRING_FUNC_UNPACK_H
