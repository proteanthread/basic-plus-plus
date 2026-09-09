// FILENAME: dateserial.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (dateserial.c)
// NEEDS: libcore (types.h)
// Declares the DATESERIAL built-in function interface in BASIC++.

#ifndef DATESERIAL_H
#define DATESERIAL_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_dateserial_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_dateserial_register(void);

#endif // DATESERIAL_H
