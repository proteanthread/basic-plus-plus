// FILENAME: filesavebox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (filesavebox.c)
// NEEDS: libcore (types.h)
// Declares the FILESAVEBOX$ built-in function interface in BASIC++.

#ifndef FILESAVEBOX_H
#define FILESAVEBOX_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_filesavebox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_filesavebox_register(void);

#endif // FILESAVEBOX_H
