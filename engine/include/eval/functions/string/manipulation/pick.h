// FILENAME: pick.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (pick.c, string_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides runtime implementation for the PICK built-in function in BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_STRING_PICK_H
#define EVAL_FUNCTIONS_STRING_PICK_H

#include "vm/vm.h"
#include "eval/eval.h"

void func_pick_register(void);
BValue func_field_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_extract_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_count_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_count_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_dynarray_str_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_parse_dynarray_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);


#endif // EVAL_FUNCTIONS_STRING_PICK_H
