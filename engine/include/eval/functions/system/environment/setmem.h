// FILENAME: setmem.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (setmem.c)
// NEEDS: libcore (types.h)
// Declares the SETMEM built-in function interface in BASIC++.

#ifndef SETMEM_H
#define SETMEM_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_setmem_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_setmem_register(void);

#endif // SETMEM_H
