// FILENAME: microplex.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libcore (microplex.c)
// NEEDED BY: libengine (string_fn.c)
// NEEDS: libengine (eval.h, eval.c, vm.h)
// Provides core logic and interface definitions for microplex within BASIC++.
//
// ---- Includes ----

#ifndef EVAL_FUNC_MICROPLEX_H
#define EVAL_FUNC_MICROPLEX_H

#include "vm/vm.h"
#include "eval/eval.h"

BValue func_microplex_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);

#endif // EVAL_FUNC_MICROPLEX_H
