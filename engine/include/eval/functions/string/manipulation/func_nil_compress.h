// FILENAME: func_nil_compress.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_nil_compress.c, sys_fn.c)
// NEEDS: libengine (vm.h)
// NEEDS: libkernel (types.h)
// Provides prototypes for COMSTR$(), DECOMSTR$(), NIL.COMPRESS$(), NIL.DECOMPRESS$().
//
// ---- Includes ----

#ifndef EVAL_FUNCTIONS_STRING_MANIPULATION_FUNC_NIL_COMPRESS_H
#define EVAL_FUNCTIONS_STRING_MANIPULATION_FUNC_NIL_COMPRESS_H

#include "vm/vm.h"
#include "types/types.h"

BValue func_comstr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_decomstr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNCTIONS_STRING_MANIPULATION_FUNC_NIL_COMPRESS_H
