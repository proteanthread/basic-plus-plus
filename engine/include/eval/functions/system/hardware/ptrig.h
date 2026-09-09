// FILENAME: ptrig.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (ptrig.c)
// NEEDS: libcore (types.h)
// Declares the PTRIG built-in function interface in BASIC++.

#ifndef PTRIG_H
#define PTRIG_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_ptrig_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_ptrig_register(void);

#endif // PTRIG_H
