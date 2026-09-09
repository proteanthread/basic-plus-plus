// FILENAME: inputbox.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libengine (inputbox.c)
// NEEDS: libcore (types.h)
// Declares the INPUTBOX$ built-in function interface in BASIC++.

#ifndef INPUTBOX_H
#define INPUTBOX_H

#include "types/types.h"
#include "eval/eval.h"

BValue func_inputbox_eval(VMContext *vm, const char *uname, int arg_count, BValue *args, BppError *err);
void func_inputbox_register(void);

#endif // INPUTBOX_H
