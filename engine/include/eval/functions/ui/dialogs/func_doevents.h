// FILENAME: func_doevents.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (func_doevents.c)
// NEEDS: libcore (types.h)
// Declares the DOEVENTS built-in function interface in BASIC++.

#ifndef FUNC_DOEVENTS_H
#define FUNC_DOEVENTS_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_doevents_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_doevents_register(void);

#endif // FUNC_DOEVENTS_H
