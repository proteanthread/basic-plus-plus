// FILENAME: epochdate.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (epochdate.c)
// NEEDS: libcore (types.h)
// Declares the EPOCHDATE built-in function interface in BASIC++.

#ifndef EPOCHDATE_H
#define EPOCHDATE_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_epochdate_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_epochdate_register(void);

#endif // EPOCHDATE_H
