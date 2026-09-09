// FILENAME: attr.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (attr.c)
// NEEDS: libcore (types.h)
// Declares the ATTR built-in function interface in BASIC++.

#ifndef ATTR_H
#define ATTR_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_attr_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_attr_register(void);

#endif // ATTR_H
