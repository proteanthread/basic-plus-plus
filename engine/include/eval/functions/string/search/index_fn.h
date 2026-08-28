// FILENAME: index_fn.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (index_fn.c, string_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (errors.h, types.h)
// Provides runtime implementation for the INDEX_FN built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_STRING_INDEX_FN_H
#define EVAL_FUNCTIONS_STRING_INDEX_FN_H

#include "types/types.h"
#include "types/errors.h"
#include "vm/vm.h"

void func_index_fn_register(void);
BValue func_index_fn_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_STRING_INDEX_FN_H
