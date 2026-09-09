// FILENAME: pol_rec.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (math_fn.c, pol_rec.c, dispatch_call.c)
// NEEDS: libkernel (vm.h, eval.h)
// Declares POL and REC polar/rectangular coordinate conversion functions.
//
// ---- Includes ----

#ifndef BPP_POL_REC_H
#define BPP_POL_REC_H

#include "types/errors.h"
#include "vm/vm.h"
#include "eval/eval.h"

void func_pol_rec_register(void);
BValue func_pol_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
BValue func_rec_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // BPP_POL_REC_H
